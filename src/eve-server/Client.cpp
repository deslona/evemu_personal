/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "LiveUpdateDB.h"
#include "PyBoundObject.h"
#include "chat/LSCService.h"
#include "character/CharUnboundMgrService.h"
#include "imageserver/ImageServer.h"
#include "npc/NPC.h"
#include "ship/DestinyManager.h"
#include "ship/ShipOperatorInterface.h"
#include "system/SystemGPoint.h"
#include "system/SystemManager.h"

static const uint32 PING_INTERVAL_US = 60000;

Client::Client(PyServiceMgr &services, EVETCPConnection** con)
: DynamicSystemEntity(NULL),
  EVEClientSession( con ),
  m_services(services),
  m_pingTimer(PING_INTERVAL_US),
  m_system(NULL),
//  m_destinyTimer(1000, true), //accurate timing is essential
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
  m_moveState(msIdle),
  m_moveTimer(500),
  m_movePoint(0, 0, 0),
  m_timeEndTrain(0),
  m_destinyEventQueue( new PyList ),
  m_destinyUpdateQueue( new PyList ),
  m_nextNotifySequence(1)
//  m_nextDestinyUpdate(46751)
{
    m_moveTimer.Disable();
    m_pingTimer.Start();

    m_dockStationID = 0;
    m_needToDock = false;
	m_justUndocked = false;
	m_justUndockedCount = 0;

	m_systemName = "";

    bKennyfied = false;     // by default, we do NOT want chat messages kennyfied, LOL

    // Start handshake
    Reset();
}

Client::~Client() {
    if( GetChar() ) {   // we have valid character
        // LSC logout
        m_services.lsc_service->CharacterLogout(GetCharacterID(), LSCChannel::_MakeSenderInfo(this));

        SaveAllToDatabase();

	//TODO  - for warping to random point when client logs out in space...
	//		1)  check client IsInSpace(?)
	//		2)  set timer to delay removing bubble/sysmgr/destiny...or check based on destiny->isstopped() or timer on destiny->ismoving()
	//		3)  set current position (DB::character_.logoutPosition?)  initial code in place for warp-in on login
	//		4)  generate random point to warp to ** use m_SGP.GetRandPointInSystem(systemID, distance)
	//		5)  _warp to random point, but DONT make/update new bubble with entering ship
	//		6)  remove client from sysmgr/destiny/server

        // remove ourselves from system
        if(m_system != NULL)
            m_system->RemoveClient(this);   //handles removing us from bubbles and sending RemoveBall events.

        //johnsus - characterOnline mod
        // switch character online flag to 0
        m_services.serviceDB().SetCharacterOnlineStatus(GetCharacterID(), false);
        m_services.serviceDB().SetAccountOnlineStatus(GetAccountID(), false);

        char ci[1];
        snprintf(ci, sizeof(ci), "");
        GetShip()->SetCustomInfo(ci);

        sLog.Error( "Client Logout", "%s(%u) Save Complete.", GetCharacterName().c_str(), GetCharacterID());
    }

    m_services.ClearBoundObjects(this);

    targets.DoDestruction();

    PyDecRef( m_destinyEventQueue );
    PyDecRef( m_destinyUpdateQueue );
}

bool Client::ProcessNet()
{
    if( GetState() != TCPConnection::STATE_CONNECTED )
        return false;

    if(m_pingTimer.Check()) {
        //_log(CLIENT__TRACE, "%s: Sending ping request.", GetName());
        _SendPingRequest();
    }

    PyPacket *p;
    while((p = PopPacket())) {
        {
            _log(CLIENT__IN_ALL, "Received packet:");
            PyLogDumpVisitor dumper(CLIENT__IN_ALL, CLIENT__IN_ALL);
            p->Dump(CLIENT__IN_ALL, dumper);
        }

        try
        {
            if( !DispatchPacket( p ) )
                sLog.Error( "Client", "%s: Failed to dispatch packet of type %s (%d).", GetName(), MACHONETMSG_TYPE_NAMES[ p->type ], (int)p->type );
        }
        catch( PyException& e )
        {
            _SendException( p->dest, p->source.callID, p->type, WRAPPEDEXCEPTION, &e.ssException );
        }

        SafeDelete( p );
    }

    // send queued updates
    _SendQueuedUpdates();

    return true;
}

void Client::Process() {
    if(m_moveTimer.Check(false)) {
        m_moveTimer.Disable();
        _MoveState s = m_moveState;
        m_moveState = msIdle;
        switch(s) {
        case msIdle:
            sLog.Error("Client","%s: Move timer expired when no move is pending.", GetName());
            break;
        //used to delay stargate animation
        case msJump:
  sLog.Warning( "Client::Process()", "case: msJump" );
            _ExecuteJump();
            break;
        }
    }

    // Check Character Save Timer Expiry:
    if( GetChar()->CheckSaveTimer() ) {
     // GetChar()->SaveFullCharacter();
        GetChar()->SaveCharacter();
		GetShip()->SaveShip();
    }

    // Check Module Manager Save Timer Expiry:
    //if( mModulesMgr.CheckSaveTimer() )
    //    mModulesMgr.SaveModules();

    if( m_timeEndTrain > 0 ) {
        if( m_timeEndTrain < EvilTimeNow() )    //Win32TimeNow()
            GetChar()->UpdateSkillQueue();
    }

    GetShip()->Process();

    SystemEntity::Process();
}

//this displays a modal error dialog on the client side.
void Client::SendErrorMsg( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    char* str = NULL;
    vasprintf( &str, fmt, args );
    assert( str );

    sLog.Error("Client","Sending Error Message to %s:", GetName() );
    log_messageVA( CLIENT__ERROR, fmt, args );
    va_end( args );

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomError";
    n.args[ "error" ] = new PyString( str );

    PyTuple* tmp = n.Encode();
    SendNotification( "OnRemoteMessage", "charid", &tmp );

    SafeFree( str );
}

void Client::SendErrorMsg( const char* fmt, va_list args )
{
    char* str = NULL;
    vasprintf( &str, fmt, args );
    assert( str );

    sLog.Error("Client","Sending Error Message to %s:", GetName() );
    log_messageVA( CLIENT__ERROR, fmt, args );

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomError";
    n.args[ "error" ] = new PyString( str );

    PyTuple* tmp = n.Encode();
    SendNotification( "OnRemoteMessage", "charid", &tmp );

    SafeFree( str );

}

//this displays a modal info dialog on the client side.
void Client::SendInfoModalMsg( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    char* str = NULL;
    vasprintf( &str, fmt, args );
    assert( str );

    sLog.Log("Client","Info Modal to %s:", GetName() );
    log_messageVA( CLIENT__MESSAGE, fmt, args );
    va_end( args );

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "ServerMessage";
    n.args[ "msg" ] = new PyString( str );

    PyTuple* tmp = n.Encode();
    SendNotification( "OnRemoteMessage", "charid", &tmp );

    SafeFree( str );
}

//this displays a little notice (like combat messages)
void Client::SendNotifyMsg( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    char* str = NULL;
    vasprintf( &str, fmt, args );
    assert( str );

    sLog.Log("Client","Notify to %s:", GetName() );
    log_messageVA( CLIENT__MESSAGE, fmt, args );
    va_end( args );

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomNotify";
    n.args[ "notify" ] = new PyString( str );

    PyTuple* tmp = n.Encode();
    SendNotification( "OnRemoteMessage", "charid", &tmp );

    SafeFree( str );
}

void Client::SendNotifyMsg( const char* fmt, va_list args )
{
    char* str = NULL;
    vasprintf( &str, fmt, args );
    assert( str );

    sLog.Log("Client","Notify to %s:", GetName() );
    log_messageVA( CLIENT__MESSAGE, fmt, args );

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomNotify";
    n.args[ "notify" ] = new PyString( str );

    PyTuple* tmp = n.Encode();
    SendNotification( "OnRemoteMessage", "charid", &tmp );

    SafeFree( str );
}

//there may be a less hackish way to do this.
void Client::SelfChatMessage( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    char* str = NULL;
    vasprintf( &str, fmt, args );
    assert( str );

    va_end( args );

    if( m_channels.empty() )
    {
        sLog.Error("Client", "%s: Tried to send self chat, but we are not joined to any channels: %s", GetName(), str );
        free( str );
        return;
    }

    sLog.Log("Client","%s: Self message on all channels: %s", GetName(), str );

    //this is such a pile of crap, but im not sure whats better.
    //maybe a private message...
    std::set<LSCChannel*>::iterator cur, end;
    cur = m_channels.begin();
    end = m_channels.end();
    for(; cur != end; ++cur)
        (*cur)->SendMessage( this, str, true );

    //m_channels[

    //just send it to the first channel we are in..
    /*LSCChannel *chan = *(m_channels.begin());
    char self_id[24];   //such crap..
    snprintf(self_id, sizeof(self_id), "%u", GetCharacterID());
    if(chan->GetName() == self_id) {
        if(m_channels.size() > 1) {
            chan = *(++m_channels.begin());
        }
    }*/

    SafeFree( str );
}

void Client::ChannelJoined(LSCChannel *chan) {
    m_channels.insert(chan);
}

void Client::ChannelLeft(LSCChannel *chan) {
    m_channels.erase(chan);
}

bool Client::EnterSystem(bool login) {
    uint32 systemID;
    if(m_autoPilot) {
        sLog.Warning( "Client::EnterSystem", "m_autoPilot = 1" );
	    if( IsStation( GetLocationID() ) )
            m_services.serviceDB().GetStationInfo( GetLocationID(), &systemID, NULL, NULL, NULL, NULL, NULL );
		else
		    systemID = GetLocationID();
	} else
	      systemID = GetSystemID();

    if(m_system != NULL && m_system->GetID() != systemID) {
        sLog.Warning( "Client::EnterSystem()", "m_system = %u, m_system->GetID(%u) != GetSystemID(%u)", m_system, m_system->GetID(), systemID );
        //we have different m_system
        m_system->RemoveClient(this);
        m_system = NULL;

        delete m_destiny;
        m_destiny = NULL;
    }
    if(m_system == NULL) {
        sLog.Warning( "Client::EnterSystem()", "m_system == NULL, GetSystemID() = %u, mLocation = %u", systemID, GetLocationID() );
        //m_system is NULL, so find our new system's manager and register ourself with it.
        m_system = m_services.entity_list.FindOrBootSystem(systemID);
        if(m_system == NULL) {
            sLog.Error("Client", "Failed to boot system %u for char %s (%u)", systemID, GetName(), GetCharacterID());
            SendErrorMsg("Unable to boot system %u", systemID);
            return false;
        }

        m_system->AddClient(this);
        m_systemName = m_system->GetName();
	}

    //  add char to Dynamic Data    updated for docked/inspace   -allan 28April14
	//			updated for bullshit client math	5Aug14
    GetChar()->chkDynamicSystemID(systemID);
    if(login) {
        if( IsInSpace() )
            GetChar()->AddPilotToDynamicData(systemID, false, true);
        else
            GetChar()->AddPilotToDynamicData(systemID, true, true);
    } else{
        if( IsInSpace() )
            GetChar()->AddPilotToDynamicData(systemID, false, false);
        else
            GetChar()->AddPilotToDynamicData(systemID, true, false);
    }
    return true;
}

bool Client::UpdateLocation(bool login) {
    if(IsStation(GetLocationID())) {
        sLog.Success( "Client::UpdateLocation()", "Character %s (%u) Docked.", GetName(), GetCharacterID() );
        //we entered station, delete m_destiny
        delete m_destiny;
        m_destiny = NULL;

        m_system->RemoveClient(this);
        m_system = NULL;

        OnCharNowInStation();
    } else if(IsSolarSystem(GetLocationID())) {
        sLog.Success( "Client::UpdateLocation()", "Character %s (%u) InSpace.", GetName(), GetCharacterID() );
        //we are in a system, so we need a destiny manager
        m_destiny = new DestinyManager(this, m_system);
		m_destiny->SetShipCapabilities(GetShip(), login);
		m_destiny->SetPosition(GetShip()->position());

        /*if( login )
        {
            // We are just logging in, so we need to warp to our last position from a
            // random vector 1000000000m away:
            GPoint warpToPoint( GetShip()->position() );
            GPoint warpFromPoint( GetShip()->position() );
            warpFromPoint.MakeRandomPointOnSphere( 15.0*ONE_AU_IN_METERS );
            m_destiny->SetPosition( warpFromPoint, true );
            WarpTo( warpToPoint, 0.0 );        // Warp ship from the random login point to the position saved on last disconnect
        }
        else {*/

            // This is NOT a login, so we always enter a system stopped.
			//  Halt() calls m_system->bubbles.UpdateBubble(m_self);  which sets our posistion to 0,0,0
            //m_destiny->Halt(true);
            //m_destiny->Stop(true);

			/**
            //set position.
			if( GetPosition().iszero() ) {
			  //TODO  select random point in system and set client position to it
			  //  will need to pick 2 random system celestials, make random point between them, and then set client to that point.
			  //    made class for random system points....SystemGPoint::SystemGPoint  used as m_SGP.xxx
                GPoint randPoint( GetShip()->position() );
                randPoint.MakeRandomPointOnSphere( 5.0 * ONE_AU_IN_METERS );  //<- this works!!
			    m_destiny->SetPosition(randPoint, false);
				sLog.Warning( "Client::UpdateLocation()", "client %s GetPosition = 0,0,0.  Setting to %.2f, %.2f, %.2f", GetName(), randPoint.x, randPoint.y, randPoint.z );
			} else {
			    m_destiny->SetPosition(GetShip()->position(), false);
				sLog.Warning( "Client::UpdateLocation()", "client %s GetPosition = %.2f, %.2f, %.2f", GetName(), x(), y(), z()  );
			}
			*/
	}

    return true;
}

void Client::UndockFromStation( uint32 stationID, uint32 systemID, uint32 constellationID, uint32 regionID, GPoint dockPosition, GPoint direction )
{   // made specific for undocking instead of 'generic' move function below
    sLog.Debug("Client::UndockFromStation()", "Character %s (%u) stationID() = %u  Position = %.2f, %.2f, %.2f  DockPosition = %.2f, %.2f, %.2f.",
               GetName(), GetCharacterID(), GetChar()->stationID(), x(), y(), z(), dockPosition.x, dockPosition.y, dockPosition.z );

    //  OnCharNoLongerInStation -> GetCriminalTimeStamps -> Undock -> OnItemsChanged (Undocking:xxxxxxxx) ->
	// tell ship its' undocking, which only sets shields and cap to full for now  (which was moved from aknor's original commit)
	GetShip()->Undock();
	//remove ship from station inv, add to system inv, and inform client.
    GetShip()->Move(systemID, flagAutoFit);
	//set ship.m_position and save
    GetShip()->Relocate(dockPosition);
    //set character location to current system, and save.
    GetChar()->SetLocation( 0, systemID, constellationID, regionID );
	//need mlocation set to current system before adding destiny manager and updating client with new position.
    _UpdateSession(GetChar());
	//register with system manager...this also updates bubble manager to add client to station's bubble.
    EnterSystem();
	//update client and set session change
	OnCharNoLongerInStation();
    //we are in a system, so we need a destiny manager
    m_destiny = new DestinyManager(this, m_system);
	//set undock information in protected variables in destiny
	m_destiny->Undock(dockPosition, direction);
    //update char session with the new values
    _SendSessionChange();

	//TODO: implement and set session change timer .....client knows already.  session change timer = 10s
	//TODO: implement and set undock invul until movement(but not on stop)

}

void Client::MoveToLocation( uint32 location, const GPoint& pt )
{
    if( GetLocationID() == location )
    {
        sLog.Warning( "Client::MoveToLocation()", "GetLocationID()" );
        // This is a warp or simple movement
        MoveToPosition( pt );
        return;
    }

    uint32 stationID, solarSystemID, constellationID, regionID;
    if( IsStation( location ) )
    {
        sLog.Warning( "Client::MoveToLocation()", "IsStation()" );
        // Entering station
        stationID = location;

        m_services.serviceDB().GetStationInfo(
            stationID,
            &solarSystemID, &constellationID, &regionID,
            NULL, NULL, NULL
        );

        GetShip()->Move( stationID, flagHangar );
    }
    else if( IsSolarSystem( location ) )
    {
        sLog.Warning( "Client::MoveToLocation()", "IsSolarSystem()" );
        // Entering a solarsystem   origin is GetLocation()    destination is location
        stationID = 0;
        solarSystemID = location;

        m_services.serviceDB().GetSystemInfo(
            solarSystemID,
            &constellationID, &regionID,
            NULL, NULL
        );

        GetShip()->Move( solarSystemID, flagAutoFit );
        GetShip()->Relocate( pt );
    }
    else
    {
        SendErrorMsg( "Move requested to unsupported location %u", location );
        return;
    }

    //move the character_ record... we really should derive the char's location from the entity table...
    GetChar()->SetLocation( stationID, solarSystemID, constellationID, regionID );
    //update session with new values
    _UpdateSession( GetChar() );

    EnterSystem();
    UpdateLocation();

    _SendSessionChange();
}


void Client::MoveToPosition(const GPoint &pt) {
    if(m_destiny == NULL)
        return;
    m_destiny->Halt();
    m_destiny->SetPosition(pt, true);
    GetShip()->Relocate(pt);
}

void Client::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag)
{
    m_services.item_factory.SetUsingClient( this );
    InventoryItemRef item = m_services.item_factory.GetItem( itemID );
    if( !item ) {
        sLog.Error("Client","%s: Unable to load item %u", GetName(), itemID);
        return;
    }

    bool was_module = ( item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast);

    //do the move. This will update the DB and send the notification.
    item->Move(location, flag);

    if(was_module || (item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast)) {
        //it was equipped, or is now. so mModulesMgr need to know.
        GetShip()->UpdateModules();
    }

    // Release the item factory now that the ItemFactory is finished being used:
    m_services.item_factory.UnsetUsingClient();
}

void Client::BoardShip(ShipRef new_ship) {

    if(!new_ship->singleton()) {
        sLog.Error("Client","%s: tried to board ship %u, which is not assembled.", GetName(), new_ship->itemID());
        SendErrorMsg("You cannot board a ship which is not assembled!");
        return;
    }

    if((m_system != NULL) && (IsInSpace()))
        m_system->RemoveClient(this);

    _SetSelf( new_ship );
  //  m_char->MoveInto( *new_ship, flagPilot, true );

    new_ship->GetOperator()->SetOperatorObject(this);

    m_shipId = new_ship->itemID();
    m_char->SetActiveShip(m_shipId);
    if (IsInSpace())
        mSession.SetInt( "shipid", new_ship->itemID() );

    GetShip()->UpdateModules();

    if((m_system != NULL) && (IsInSpace()))
        m_system->AddClient(this);

	if(m_destiny != NULL) {
		//  this needs to be updated to NOT use destiny
		//  players that board ships in stations WILL NEED updated ship stats for fitting and cargo.
	    m_destiny->SetShipCapabilities( GetShip() );
	}
	GetShip()->SaveShip();
}

void Client::UpdateCorpSession( const CharacterConstRef& character )
{
    if( !character ) return;

    mSession.SetInt( "corpid", character->corporationID() );
    mSession.SetInt( "hqID", character->corporationHQ() );
    mSession.SetLong( "corpRole", character->corpRole() );
    mSession.SetInt( "corpAccountKey", character->corpAccountKey() );
    mSession.SetLong( "rolesAtAll", character->rolesAtAll() );
    mSession.SetLong( "rolesAtBase", character->rolesAtBase() );
    mSession.SetLong( "rolesAtHQ", character->rolesAtHQ() );
    mSession.SetLong( "rolesAtOther", character->rolesAtOther() );

    _SendSessionChange();
}

void Client::UpdateFleetSession( const CharacterConstRef& character )
{
    if( !character ) return;

    mSession.SetInt( "fleetid", character->fleetID() );
    mSession.SetInt( "fleetrole", character->fleetRole() );
    mSession.SetInt( "fleetbooster", character->fleetBooster() );
    mSession.SetInt( "wingid", character->wingID() );
    mSession.SetInt( "squadid", character->squadID() );

    _SendSessionChange();
}

void Client::_UpdateSession( const CharacterConstRef& character )
{
    if( !character ) return;

    if( character->stationID() == 0 )
    {
        sLog.Warning( "Client::_UpdateSession()", "character->stationID() == 0" );
        mSession.Clear( "stationid" );
        mSession.Clear( "stationid2" );
        mSession.Clear( "worldspaceid" );

		if(!m_autoPilot)
		    mSession.SetInt( "solarsystemid", character->solarSystemID() );	//  used to tell client they are in space
        mSession.SetInt( "locationid", character->solarSystemID() );
    }
    else
    {
        sLog.Warning( "Client::_UpdateSession()", "character->stationID() == %u", character->stationID() );
        mSession.Clear( "solarsystemid" );

        mSession.SetInt( "stationid", character->stationID() );
        mSession.SetInt( "stationid2", character->stationID() );
        mSession.SetInt( "worldspaceid", character->stationID() );
        mSession.SetInt( "locationid", character->stationID() );
    }
    // solarsystemid2 is used by client to determine current system.  NOTE:  *MUST* be set to current system.
	mSession.SetInt( "solarsystemid2", character->solarSystemID() );
    mSession.SetInt( "constellationid", character->constellationID() );
    mSession.SetInt( "regionid", character->regionID() );
    mSession.SetInt( "shipid", GetShipID() );
}

void Client::_UpdateSession2( uint32 characterID )
{
    if( characterID == 0 )
    {
        sLog.Error( "Client::_UpdateSession2()", "characterID == 0" );
        return;
    }

    std::map<std::string, uint64> characterDataMap;
    ((CharUnboundMgrService *)(m_services.LookupService("charUnboundMgr")))->GetCharacterData( characterID, characterDataMap );

    if( characterDataMap.size() == 0 )
    {
        sLog.Error( "Client::_UpdateSession2()", "characterDataMap.size() returned zero." );
        return;
    }

    uint32 gender = static_cast<uint32>(characterDataMap["gender"]);
    uint32 corporationID = static_cast<uint32>(characterDataMap["corporationID"]);
    uint32 stationID = static_cast<uint32>(characterDataMap["stationID"]);
    uint32 solarSystemID = static_cast<uint32>(characterDataMap["solarSystemID"]);
    uint32 constellationID = static_cast<uint32>(characterDataMap["constellationID"]);
    uint32 regionID = static_cast<uint32>(characterDataMap["regionID"]);
    uint32 corporationHQ = static_cast<uint32>(characterDataMap["corporationHQ"]);
    int32 corpAccountKey = static_cast<int32>(characterDataMap["corpAccountKey"]);
    int64 corpRole = static_cast<int64>(characterDataMap["corpRole"]);
    int64 rolesAtAll = static_cast<int64>(characterDataMap["rolesAtAll"]);
    int64 rolesAtBase = static_cast<int64>(characterDataMap["rolesAtBase"]);
    int64 rolesAtHQ = static_cast<int64>(characterDataMap["rolesAtHQ"]);
    int64 rolesAtOther = static_cast<int64>(characterDataMap["rolesAtOther"]);
    uint32 locationID = static_cast<uint32>(characterDataMap["locationID"]);
    uint32 shipID = static_cast<uint32>(characterDataMap["shipID"]);

    mSession.SetInt( "genderID", gender );
    mSession.SetInt( "clientID", GetAccountID() );
    mSession.SetInt( "charid", characterID );
    mSession.SetInt( "corpid", corporationID );
    if( stationID == 0 )
    {
        mSession.Clear( "stationid" );
        mSession.Clear( "stationid2" );
        mSession.Clear( "worldspaceid" );

	//  used to tell client they are in space.
	//also used as current system in following menus:  JumpPortalBridgeMenu, GetHybridBeaconJumpMenu, GetHybridBridgeMenu,
        mSession.SetInt( "solarsystemid", solarSystemID );
        mSession.SetInt( "locationid", solarSystemID );
    }
    else
    {
        mSession.Clear( "solarsystemid" );

        mSession.SetInt( "stationid", stationID );
        mSession.SetInt( "stationid2", stationID );
        mSession.SetInt( "locationid", stationID );        // used to be locationID, I don't know if this change will screw up using medical clones and such -- Aknor Jaden
    }
    mSession.SetInt( "cloneLocationID", locationID );   // This is a CUSTOM key-value-pair that is NOT defined by CCP, so the question is, will this mess up the client?    - no.  it is not defined in the client, therefore is ignored.

    // solarsystemid2 is used by client to determine current system.
	//NOTE:  *MUST* be set to current system.
    mSession.SetInt( "solarsystemid2", solarSystemID );
    mSession.SetInt( "constellationid", constellationID );
    mSession.SetInt( "regionid", regionID );

    mSession.SetInt( "hqID", corporationHQ );
    mSession.SetInt( "corpAccountKey", corpAccountKey );
    mSession.SetLong( "corpRole", corpRole );
    mSession.SetLong( "rolesAtAll", rolesAtAll );
    mSession.SetLong( "rolesAtBase", rolesAtBase );
    mSession.SetLong( "rolesAtHQ", rolesAtHQ );
    mSession.SetLong( "rolesAtOther", rolesAtOther );

    m_shipId = shipID;

    mSession.SetInt( "shipid", shipID );

    mSession.SetInt( "inDetention", 0 );

/*  TODO:  NOT set on initial session unless previously in a fleet.
    mSession.SetInt( "fleetid", 0 );
    mSession.SetInt( "fleetrole", 0 );
    mSession.SetInt( "fleetbooster", 0 );
    mSession.SetInt( "wingid", 0 );
    mSession.SetInt( "squadid", 0 );
    */
}

void Client::_SendCallReturn( const PyAddress& source, uint64 callID, PyRep** return_value, const char* channel )
{
    //build the packet:
    PyPacket* p = new PyPacket;
    p->type_string = "macho.CallRsp";
    p->type = CALL_RSP;

    p->source = source;

    p->dest.type = PyAddress::Client;
    p->dest.typeID = GetAccountID();
    p->dest.callID = callID;

    p->userid = GetAccountID();

    p->payload = new PyTuple(1);
    p->payload->SetItem( 0, new PySubStream( *return_value ) );
    *return_value = NULL;   //consumed

    if(channel != NULL)
    {
        p->named_payload = new PyDict();
        p->named_payload->SetItemString( "channel", new PyString( channel ) );
    }

    FastQueuePacket( &p );
}

void Client::_SendException( const PyAddress& source, uint64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload )
{
    //build the packet:
    PyPacket* p = new PyPacket;
    p->type_string = "macho.ErrorResponse";
    p->type = ERRORRESPONSE;

    p->source = source;

    p->dest.type = PyAddress::Client;
    p->dest.typeID = GetAccountID();
    p->dest.callID = callID;

    p->userid = GetAccountID();

    macho_MachoException e;
    e.in_response_to = in_response_to;
    e.exception_type = exception_type;
    e.payload = *payload;
    *payload = NULL;    //consumed

    p->payload = e.Encode();
    FastQueuePacket(&p);
}

void Client::_SendSessionChange()
{
    if( !mSession.isDirty() )
        return;

    SessionChangeNotification scn;
    scn.changes = new PyDict;

    mSession.EncodeChanges( scn.changes );
    if( scn.changes->empty() )
        return;

    sLog.Debug("Client::_SendSessionChange()"," Session updated, sending session change");
    scn.changes->Dump(CLIENT__SESSION, "  Changes: ");

    //this is probably not necessary...
    scn.nodesOfInterest.push_back( services().GetNodeID() );

    //build the packet:
    PyPacket* p = new PyPacket;
    p->type_string = "macho.SessionChangeNotification";
    p->type = SESSIONCHANGENOTIFICATION;

    p->source.type = PyAddress::Node;
    p->source.typeID = services().GetNodeID();
    p->source.callID = 0;

    p->dest.type = PyAddress::Client;
    p->dest.typeID = GetAccountID();
    p->dest.callID = 0;

    p->userid = GetAccountID();

    p->payload = scn.Encode();

    p->named_payload = NULL;
    //p->named_payload = new PyDict();
    //p->named_payload->SetItemString( "channel", new PyString( "sessionchange" ) );

    //_log(CLIENT__IN_ALL, "Sending Session packet:");
    //PyLogDumpVisitor dumper(CLIENT__OUT_ALL, CLIENT__OUT_ALL);
    //p->Dump(CLIENT__OUT_ALL, dumper);

    FastQueuePacket( &p );
}

void Client::_SendPingRequest()
{
    PyPacket *ping_req = new PyPacket();

    ping_req->type = PING_REQ;
    ping_req->type_string = "macho.PingReq";

    ping_req->source.type = PyAddress::Node;
    ping_req->source.typeID = services().GetNodeID();
    ping_req->source.service = "ping";
    ping_req->source.callID = 0;

    ping_req->dest.type = PyAddress::Client;
    ping_req->dest.typeID = GetAccountID();
    ping_req->dest.callID = 0;

    ping_req->userid = GetAccountID();

    ping_req->payload = new_tuple( new PyList() ); //times
    ping_req->named_payload = new PyDict();

    FastQueuePacket(&ping_req);
}

void Client::_SendPingResponse( const PyAddress& source, uint64 callID )
{
    PyPacket* ret = new PyPacket;
    ret->type = PING_RSP;
    ret->type_string = "macho.PingRsp";

    ret->source = source;

    ret->dest.type = PyAddress::Client;
    ret->dest.typeID = GetAccountID();
    ret->dest.callID = callID;

    ret->userid = GetAccountID();

    /*  Here the hacking begins, the ping packet handles the timestamps of various packet handling steps.
        To really simulate/emulate that we need the various packet handlers which in fact we don't have ( :P ).
        So the next piece of code "fake's" it, with a slight delay on the received packet time.
    */
    PyList* pingList = new PyList;
    PyTuple* pingTuple;

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));        // this should be the time the packet was received (we cheat here a bit)
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));             // this is the time the packet is (handled/writen) by the (proxy/server) so we're cheating a bit again.
    pingTuple->SetItem(2, new PyString("proxy::handle_message"));
    pingList->AddItem( pingTuple );

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::writing"));
    pingList->AddItem( pingTuple );

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("server::handle_message"));
    pingList->AddItem( pingTuple );

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("server::turnaround"));
    pingList->AddItem( pingTuple );

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::handle_message"));
    pingList->AddItem( pingTuple );

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::writing"));
    pingList->AddItem( pingTuple );

    // Set payload
    ret->payload = new PyTuple( 1 );
    ret->payload->SetItem( 0, pingList );

    // Don't clone so it eats the ret object upon sending.
    FastQueuePacket( &ret );
}

//these are specialized Queue functions when our caller can
//easily provide us with our own copy of the data.
void Client::QueueDestinyUpdate(PyTuple **du)
{
    DoDestinyAction act;
    act.update_id = DestinyManager::GetStamp();
    act.update = *du;
    *du = NULL;

    m_destinyUpdateQueue->AddItem( act.Encode() );
}

void Client::QueueDestinyEvent(PyTuple** multiEvent)
{
    m_destinyEventQueue->AddItem( *multiEvent );
    *multiEvent = NULL;
}

void Client::_SendQueuedUpdates() {
    if( !m_destinyUpdateQueue->empty() )
    {
        DoDestinyUpdateMain dum;

        //first insert the destiny updates.
        dum.updates = m_destinyUpdateQueue;
        PyIncRef( m_destinyUpdateQueue );

        //encode any multi-events which go along with it.
        dum.events = m_destinyEventQueue;
        PyIncRef( m_destinyEventQueue );

        //right now, we never wait. I am sure they do this for a reason, but
        //I haven't found it yet
        dum.waitForBubble = false;		// set to 'true' causes blackscreen on login/undock.

        //now send it
        PyTuple* t = dum.Encode();
        t->Dump(DESTINY__UPDATES, "");
        SendNotification( "DoDestinyUpdate", "clientID", &t );
    }
    else if( !m_destinyEventQueue->empty() )
    {
        Notify_OnMultiEvent nom;

        //insert updates, clear our queue
        nom.events = m_destinyEventQueue;
        PyIncRef( m_destinyEventQueue );

        //send it
        PyTuple* t = nom.Encode();   //this is consumed below
        t->Dump(DESTINY__UPDATES, "");
        SendNotification( "OnMultiEvent", "charid", &t );
    } //else nothing to be sent ...

    // clear the queues now, after the packets have been sent
    m_destinyEventQueue->clear();
    m_destinyUpdateQueue->clear();
}

void Client::SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq) {

    //build a little notification out of it.
    EVENotificationStream notify;
    notify.remoteObject = 1;
    notify.args = *payload;
    *payload = NULL;    //consumed

    PyAddress dest;
    dest.type = PyAddress::Broadcast;
    dest.service = notifyType;
    dest.bcast_idtype = idType;

    //now send it to the client
    SendNotification(dest, notify, seq);
}


void Client::SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq) {

    //build the packet:
    PyPacket *p = new PyPacket();
    p->type_string = "macho.Notification";
    p->type = NOTIFICATION;

    p->source.type = PyAddress::Node;
    p->source.typeID = m_services.GetNodeID();

    p->dest = dest;

    p->userid = GetAccountID();

    p->payload = noti.Encode();

    if(seq) {
        p->named_payload = new PyDict();
        p->named_payload->SetItemString("sn", new PyInt(m_nextNotifySequence++));
    }
        //  changed log back to debug   -allan 01/11/14
    sLog.Debug("Client","Sending notify of type %s with ID type %s", dest.service.c_str(), dest.bcast_idtype.c_str());
    if(is_log_enabled(CLIENT__NOTIFY_REP))
    {
        PyLogDumpVisitor dumper(CLIENT__NOTIFY_REP, CLIENT__NOTIFY_REP, "", true, true);
        p->Dump(CLIENT__NOTIFY_REP, dumper);
    }

    FastQueuePacket(&p);
}

PyDict *Client::MakeSlimItem() const {
    PyDict *slim = DynamicSystemEntity::MakeSlimItem();

    slim->SetItemString("charID", new PyInt(GetCharacterID()));
    slim->SetItemString("corpID", new PyInt(GetCorporationID()));
    slim->SetItemString("allianceID", new PyInt(GetAllianceID()));
    slim->SetItemString("warFactionID", new PyInt(GetWarFactionID()));

    //encode the modules list, if we have any visible
    std::vector<InventoryItemRef> items;
    GetShip()->FindByFlagRange( flagLowSlot0, flagHiSlot7, items );
    if( !items.empty() )
    {
        PyList *l = new PyList();

        std::vector<InventoryItemRef>::iterator cur, end;
        cur = items.begin();
        end = items.end();
        for(; cur != end; cur++) {

            PyTuple* t = new_tuple( (*cur)->itemID(), (*cur)->typeID());
            l->AddItem(t);
        }

        slim->SetItemString("modules", l);
    }

    slim->SetItemString("bounty", new PyFloat(GetBounty()));
    slim->SetItemString("securityStatus", new PyFloat(GetSecurityRating()));

    return(slim);
}

PyRep *Client::GetAggressors() const {
    PyDict *dict = NULL;
    /*  items are set here as
     *               [PyInt 90971469]     <- entity
     *               [PyDict 1 kvp]       <- dictionary
     *                  [PyInt 1000127]   <- agression TO corpID
     *                  [PyIntegerVar 129550906897224125] <- timestamp (dont know if this is begin time or end time)
     */
    /*
     *    while (entity has agressions)
     *        dict->SetItem( PyInt(agressionToEntityID, Win32TimeNow()));
     */
    return dict;
}

void Client::SetAutoPilot(bool autoPilot) {
    if((autoPilot) && (!m_autoPilot)) {
	    mSession.SetInt( "solarsystemid", GetSystemID() );   //this is currrent system.
		m_autoPilot = true;
	}
}

void Client::StargateJump(uint32 fromGate, uint32 toGate) {
    if(m_moveState != msIdle || m_moveTimer.Enabled()) {
        sLog.Error("Client","%s: StargateJump called when a jump is already pending. Ignoring.", GetName());
        return;
    }

    uint32 solarSystemID;
    GPoint position;
    if(!m_services.serviceDB().GetStaticItemInfo(
        toGate, &solarSystemID, NULL, NULL, &position
    )) {
        sLog.Error("Client","%s: Failed to query information for stargate %u", GetName(), toGate);
        return;
    }

    GetShip()->DeactivateAllModules();

    m_moveSystemID = solarSystemID;
    m_movePoint = position;
    m_movePoint.MakeRandomPointOnSphere( 10000 );   // Make Jump-In point a random spot on a 10km radius sphere about the stargate

    char ci[35];
    snprintf(ci, sizeof(ci), "Jumping:%u", toGate);
    GetShip()->SetCustomInfo(ci);

    // add jump to mapDynamicData for showing in StarMap (F10)    -allan 06Mar14
    // this is the code for removing pilot from previous system
    uint32 fromSystem;
    if(!m_services.serviceDB().GetStaticItemInfo(
        fromGate,
        &fromSystem, NULL, NULL, NULL  )) {
        sLog.Error("Client","%s: Failed to query information for stargate %u", GetName(), fromGate);
        return;
    }
    //add jump in previous system
    GetChar()->chkDynamicSystemID(fromSystem);
    GetChar()->AddJumpToDynamicData(fromSystem);
    //add jump in this system
    GetChar()->chkDynamicSystemID(solarSystemID);
    GetChar()->AddJumpToDynamicData(solarSystemID);

    // used for showing Visited Systems in StarMap(F10)  -allan 30Jan14
    GetChar()->VisitSystem(solarSystemID);

    // call Stop() per packet sniff
    m_destiny->Stop();
    //  show gate animation in both to and from gate.
    m_destiny->SendJumpOut(fromGate);
    m_destiny->SendJumpOut(toGate);

    //delay the move so they can see the JumpOut animation
    _postMove(msJump, 5000);
}

void Client::_postMove(_MoveState type, uint32 wait_ms) {
    m_moveState = type;
    m_moveTimer.Start(wait_ms);
}

void Client::_ExecuteJump() {
    if(m_destiny == NULL)
        return;

	//   this is probably the best place to put it, as MoveToLocation() is generic function.
    MoveToLocation(m_moveSystemID, m_movePoint);
    //m_destiny->Cloak();
    //TODO check cloak timer to remove cloak
    //TODO enable uncloak on movement check/function
}

bool Client::AddBalance(double amount) {
    if(!GetChar()->AlterBalance(amount))
        return false;

    //send notification of change
    OnAccountChange ac;
    ac.accountKey = "cash";
    ac.ownerid = GetCharacterID();
    ac.balance = GetBalance();
    PyTuple *answer = ac.Encode();
    SendNotification("OnAccountChange", "cash", &answer, false);

    return true;
}

bool Client::SelectCharacter( uint32 char_id )
{
    m_services.item_factory.SetUsingClient( this );

    _UpdateSession2( char_id );

    m_char = m_services.item_factory.GetCharacter( char_id );
    if( !GetChar() )
    {
          sLog.Error("Client::SelectCharacter()", "GetChar for %u = NULL", char_id);
        // Release the item factory now that the ItemFactory is finished being used:
        m_services.item_factory.UnsetUsingClient();
        return false;
    }

    ShipRef ship = m_services.item_factory.GetShip( GetShipID() );
    if( !ship )
    {
          sLog.Error("Client::SelectCharacter()", "ship for %u = NULL", char_id);
        // Release the item factory now that the ItemFactory is finished being used:
        m_services.item_factory.UnsetUsingClient();
        return false;
    }

    ship->Load( m_services.item_factory, GetShipID() );
    BoardShip( ship );

    if( !EnterSystem( true ) )
    {
          sLog.Error("Client::SelectCharacter()", "EnterSystem for %u returned false", char_id);
        // Release the item factory now that the ItemFactory is finished being used:
        m_services.item_factory.UnsetUsingClient();
        return false;
    }

    UpdateLocation( true );

    //  this will set training end time, then we wait for the process tick which will check this then update skillqueue
    UpdateSkillTraining();

    //johnsus - characterOnline mod
    m_services.serviceDB().SetCharacterOnlineStatus( GetCharacterID(), true );

    _SendSessionChange();

    // Release the item factory now that the ItemFactory is finished being used:
    m_services.item_factory.UnsetUsingClient();
            sLog.Success("Client::SelectCharacter()", "SelectCharacter for %u completed", char_id);
    return true;
}

void Client::UpdateSkillTraining() {
    if( GetChar() )
        m_timeEndTrain = GetChar()->GetEndOfTraining();
    else
        m_timeEndTrain = 0;
}

void Client::TargetAdded( SystemEntity* who )
{
    PyTuple* up = NULL;

    DoDestiny_OnDamageStateChange odsc;
    odsc.entityID = who->GetID();
    odsc.state = who->MakeDamageState();

    up = odsc.Encode();
    QueueDestinyUpdate( &up );

    Notify_OnTarget te;
    te.mode = "add";
    te.targetID = who->GetID();

    up = te.Encode();
    QueueDestinyEvent( &up );
    PySafeDecRef( up );
}

void Client::TargetLost(SystemEntity *who)
{
    //OnMultiEvent: OnTarget lost
    Notify_OnTarget te;
    te.mode = "lost";
    te.targetID = who->GetID();

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem( te.Encode() );

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedAdd(SystemEntity *who) {
    //OnMultiEvent: OnTarget otheradd
    Notify_OnTarget te;
    te.mode = "otheradd";
    te.targetID = who->GetID();

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem( te.Encode() );

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedLost(SystemEntity *who)
{
    //OnMultiEvent: OnTarget otherlost
    Notify_OnTarget te;
    te.mode = "otherlost";
    te.targetID = who->GetID();

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem( te.Encode() );

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetsCleared()
{
    //OnMultiEvent: OnTarget clear
    Notify_OnTarget te;
    te.mode = "clear";
    te.targetID = 0;

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem( te.Encode() );

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::SavePosition() {
    if( !GetShip().get() || m_destiny == NULL ) {
        sLog.Debug("Client","%s: Unable to save position. We are probably not in space.", GetName());
        return;
    }
    GetShip()->Relocate( m_destiny->GetPosition() );
}

void Client::SaveAllToDatabase() {
    SavePosition();
    if( GetShip() ) GetShip()->SaveShip();  // Save Ship and Modules' attributes and info to DB

    GetChar()->SaveFullCharacter();         // Save Character info to DB
}

void Client::SpawnNewRookieShip(){
        EVEItemFlags flag = (EVEItemFlags)flagHangar;

        //create rookie ship of appropriate type
        uint32 typeID;
        if(GetChar()->race() == raceAmarr )  typeID = amarrRookie;
        else if(GetChar()->race() == raceCaldari )  typeID = caldariRookie;
        else if(GetChar()->race() == raceGallente )  typeID = gallenteRookie;
        else if(GetChar()->race() == raceMinmatar )  typeID = minmatarRookie;

        //create data for new rookie ship
        ItemData idata(
            typeID,
            GetCharacterID(),
            0, //temp location
            flag,
            1
        );
        //spawn rookie ship
        InventoryItemRef i = services().item_factory.SpawnItem( idata );

        if(!i)
            throw PyException( MakeCustomError( "Unable to generate rookie ship" ) );

        //move the new rookie ship into the players hanger in station
        i->Move( GetStationID(), flag, true );
}

bool Client::LaunchDrone(InventoryItemRef drone) {
#if 0
drop 166328265

OnItemChange ,*args= (Row(itemID: 166328265,typeID: 15508,
    ownerID: 105651216,locationID: 166363674,flag: 87,
    contraband: 0,singleton: 1,quantity: 1,groupID: 100,
    categoryID: 18,customInfo: (166363674, None)),
    {10: None}) ,**kw= {}

OnItemChange ,*args= (Row(itemID: 166328265,typeID: 15508,
    ownerID: 105651216,locationID: 30001369,flag: 0,
    contraband: 0,singleton: 1,quantity: 1,groupID: 100,
    categoryID: 18,customInfo: (166363674, None)),
    {3: 166363674, 4: 87}) ,**kw= {}

returns list of deployed drones.

internaly scatters:
    OnItemLaunch ,*args= ([166328265], [166328265])

DoDestinyUpdate ,*args= ([(31759,
    ('OnDroneStateChange',
        [166328265, 105651216, 166363674, 0, 15508, 105651216])
    ), (31759,
    ('OnDamageStateChange',
        (2100212375, [(1.0, 400000.0, 127997215757036940L), 1.0, 1.0]))
    ), (31759, ('AddBalls',
        ...
    True

DoDestinyUpdate ,*args= ([(31759,
    ('Orbit', (166328265, 166363674, 750.0))
    ), (31759,
    ('SetSpeedFraction', (166328265, 0.265625)))],
    False) ,**kw= {} )
#endif

    if(!IsSolarSystem(GetLocationID())) {
        sLog.Log("Client","%s: Trying to launch drone when not in space!", GetName());
        return false;
    }

    sLog.Log("Client","%s: Launching drone %u", GetName(), drone->itemID());

    //first, the item gets moved into space
    //TODO: set customInfo to a tuple: (shipID, None)
    drone->Move(GetSystemID(), flagAutoFit);
    //temp for testing:
    drone->Move(GetShipID(), flagDroneBay, false);

    //now we create an NPC to represent it.
    GPoint position(GetPosition());
    position.x += 750.0f;   //total crap.

    //this adds itself into the system.
    NPC *drone_npc = new NPC(
        m_system,
        m_services,
        drone,
        GetCorporationID(),
        GetAllianceID(),
        position);
    m_system->AddNPC(drone_npc);

    //now we tell the client that "its ALIIIIIVE"
    //DoDestinyUpdate:

    //drone_npc->Destiny()->SendAddBall();

    /*PyList *actions = new PyList();
    DoDestinyAction act;
    act.update_id = NextDestinyUpdateID();  //update ID?
    */
    //  OnDroneStateChange
/*  {
        DoDestiny_OnDroneStateChange du;
        du.droneID = drone->itemID();
        du.ownerID = GetCharacterID();
        du.controllerID = GetShipID();
        du.activityState = 0;
        du.droneTypeID = drone->typeID();
        du.controllerOwnerID = GetShip()->ownerID();
        act.update = du.Encode();
        actions->add( act.Encode() );
    }*/

    //  AddBall
    /*{
        DoDestiny_AddBall addball;
        drone_npc->MakeAddBall(addball, act.update_id);
        act.update = addball.Encode();
        actions->add( act.Encode() );
    }*/

    //  Orbit
/*  {
        DoDestiny_Orbit du;
        du.entityID = drone->itemID();
        du.orbitEntityID = GetCharacterID();
        du.distance = 750;
        act.update = du.Encode();
        actions->add( act.Encode() );
    }

    //  SetSpeedFraction
    {
        DoDestiny_SetSpeedFraction du;
        du.entityID = drone->itemID();
        du.fraction = 0.265625f;
        act.update = du.Encode();
        actions->add( act.Encode() );
    }*/

//testing:
/*  {
        DoDestiny_SetBallInteractive du;
        du.entityID = drone->itemID();
        du.interactive = 1;
        act.update = du.Encode();
        actions->add( act.Encode() );
    }
    {
        DoDestiny_SetBallInteractive du;
        du.entityID = GetCharacterID();
        du.interactive = 1;
        act.update = du.Encode();
        actions->add( act.Encode() );
    }*/

    //NOTE: we really want to broadcast this...
    //TODO: delay this until after the return.
    //_SendDestinyUpdate(&actions, false);

    return false;
}

//assumes that the backend DB stuff was already done.
void Client::JoinCorporationUpdate(uint32 corp_id) {
    //GetChar()->JoinCorporation(corp_id);

    _UpdateSession( GetChar() );

    //logs indicate that we need to push this update out asap.
    _SendSessionChange();
}

/************************************************************************/
/* character notification messages wrapper                              */
/************************************************************************/
void Client::OnCharNoLongerInStation()
{
    NotifyOnCharNoLongerInStation n;
    n.charID = GetCharacterID();
    n.corpID = GetCorporationID();
    n.allianceID = GetAllianceID();
	n.factionID = GetWarFactionID();

    PyTuple* tmp = n.Encode();
    // this entire line should be something like this Broadcast("OnCharNoLongerInStation", "stationid", &tmp);
    services().entity_list.Broadcast( "OnCharNoLongerInStation", "stationid", &tmp );
}

/* besides broadcasting the message this function should handle everything for this event */
void Client::OnCharNowInStation()
{
    NotifyOnCharNowInStation n;
    n.charID = GetCharacterID();
    n.corpID = GetCorporationID();
    n.allianceID = GetAllianceID();
	n.warFactionID = GetWarFactionID();

    PyTuple* tmp = n.Encode();
    services().entity_list.Broadcast( "OnCharNowInStation", "stationid", &tmp );
}

/************************************************************************/
/* EVEAdministration Interface                                          */
/************************************************************************/
void Client::DisconnectClient()
{
    //initiate closing the client TCP Connection
    CloseClientConnection();
}
void Client::BanClient()
{
    //send message to client
    SendNotifyMsg("You have been banned from this server and will be disconnected shortly.  You will no longer be able to log in");

    //ban the client
    services().serviceDB().SetAccountBanStatus( GetAccountID(), true );
}

/************************************************************************/
/* EVEClientSession interface                                           */
/************************************************************************/
void Client::_GetVersion( VersionExchangeServer& version )
{
    version.birthday = EVEBirthday;
    version.macho_version = MachoNetVersion;
    version.user_count = _GetUserCount();
    version.version_number = EVEVersionNumber;
    version.build_version = EVEBuildVersion;
    version.project_version = EVEProjectVersion;
}

uint32 Client::_GetUserCount()
{
    return services().entity_list.GetClientCount();
}

bool Client::_VerifyVersion( VersionExchangeClient& version )
{
    sLog.Log("Client","%s: Received Low Level Version Exchange:", GetAddress().c_str());
    version.Dump(NET__PRES_REP, "    ");

    if( version.birthday != EVEBirthday )
        sLog.Error("Client","%s: Client's birthday does not match ours!", GetAddress().c_str());

    if( version.macho_version != MachoNetVersion )
        sLog.Error("Client","%s: Client's macho_version not match ours!", GetAddress().c_str());

    if( version.version_number != EVEVersionNumber )
        sLog.Error("Client","%s: Client's version_number not match ours!", GetAddress().c_str());

    if( version.build_version != EVEBuildVersion )
        sLog.Error("Client","%s: Client's build_version not match ours!", GetAddress().c_str());

    if( version.project_version != EVEProjectVersion )
        sLog.Error("Client","%s: Client's project_version not match ours!", GetAddress().c_str());


    return true;
}

bool Client::_VerifyCrypto( CryptoRequestPacket& cr )
{
    if( cr.keyVersion != "placebo" )
    {
        //I'm sure cr.keyVersion can specify either CryptoAPI or PyCrypto, but its all binary so im not sure how.
        CryptoAPIRequestParams car;
        if( !car.Decode( cr.keyParams ) )
        {
            sLog.Error("Client","%s: Received invalid CryptoAPI request!", GetAddress().c_str());
        }
        else
        {
            sLog.Error("Client","%s: Unhandled CryptoAPI request: hashmethod=%s sessionkeylength=%d provider=%s sessionkeymethod=%s", GetAddress().c_str(), car.hashmethod.c_str(), car.sessionkeylength, car.provider.c_str(), car.sessionkeymethod.c_str());
            sLog.Error("Client","%s: You must change your client to use Placebo crypto in common.ini to talk to this server!\n", GetAddress().c_str());
        }

        return false;
    }
    else
    {
        sLog.Debug("Client","%s: Received Placebo crypto request, accepting.", GetAddress().c_str());

        //send out accept response
        PyRep* rsp = new PyString( "OK CC" );
        mNet->QueueRep( rsp );
        PyDecRef( rsp );

        return true;
    }
}

bool Client::_VerifyLogin( CryptoChallengePacket& ccp )
{
    std::string account_hash;
    std::string transport_closed_msg = "LoginAuthFailed";

    AccountInfo account_info;
    CryptoServerHandshake server_shake;

    /* send passwordVersion required: 1=plain, 2=hashed */
    PyRep* rsp = new PyInt( 2 );

    //sLog.Debug("Client","%s: Received Client Challenge.", GetAddress().c_str());
    //sLog.Debug("Client","Login with %s:", ccp.user_name.c_str());

    if (!services().serviceDB().GetAccountInformation(
				ccp.user_name.c_str(),
				ccp.user_password_hash.c_str(),
				account_info))
	{
        goto error_login_auth_failed;
    }

    /* check wether the account has been banned and if so send the semi correct message */
    if (account_info.banned) {
        transport_closed_msg = "ACCOUNTBANNED";
        goto error_login_auth_failed;
    }

    /* if we have stored a password we need to create a hash from the username and pass and remove the pass */
    if( account_info.password.empty() )
        account_hash = account_info.hash;
    else
    {
        /* here we generate the password hash ourselves */
        std::string password_hash;
        if( !PasswordModule::GeneratePassHash(
                ccp.user_name,
                account_info.password,
                password_hash ) )
        {
            sLog.Error("Client", "unable to generate password hash, sending LoginAuthFailed");
            goto error_login_auth_failed;
        }

        if( !services().serviceDB().UpdateAccountHash(
                ccp.user_name.c_str(),
                password_hash ) )
        {
            sLog.Error("Client", "unable to update account hash, sending LoginAuthFailed");
            goto error_login_auth_failed;
        }

        account_hash = password_hash;
    }

    /* here we check if the user successfully entered his password or if he failed */
    if (account_hash != ccp.user_password_hash) {
        goto error_login_auth_failed;
    }

    /* Check if we already have a client online and if we do disconnect it
     * @note we should send GPSTransportClosed with reason "The user's connection has been usurped on the proxy"
     */
    if (account_info.online) {
        Client* client = sEntityList.FindAccount(account_info.id);
        if (client != NULL)
            client->DisconnectClient();
    }

    mNet->QueueRep( rsp );
    PyDecRef( rsp );

    /* update account information, increase login count, last login timestamp and mark account as online */
    m_services.serviceDB().UpdateAccountInformation( account_info.name.c_str(), true );

    /* marshaled Python string "None" */
    static const uint8 handshakeFunc[] = { 0x74, 0x04, 0x00, 0x00, 0x00, 0x4E, 0x6F, 0x6E, 0x65 };

    /* send our handshake */

    server_shake.serverChallenge = "";
    server_shake.func_marshaled_code = new PyBuffer( handshakeFunc, handshakeFunc + sizeof( handshakeFunc ) );
    server_shake.verification = new PyBool( false );
    server_shake.cluster_usercount = _GetUserCount();
    server_shake.proxy_nodeid = 0xFFAA;
    server_shake.user_logonqueueposition = _GetQueuePosition();
    // binascii.crc_hqx of marshaled single-element tuple containing 64 zero-bytes string
    server_shake.challenge_responsehash = "55087";

    // the image server used by the client to download images
    server_shake.imageserverurl = sImageServer.url();

    server_shake.macho_version = MachoNetVersion;
    server_shake.boot_version = EVEVersionNumber;
    server_shake.boot_build = EVEBuildVersion;
    server_shake.boot_codename = EVEProjectCodename;
    server_shake.boot_region = EVEProjectRegion;

    rsp = server_shake.Encode();
    mNet->QueueRep( rsp );
    PyDecRef( rsp );

    // Setup session, but don't send the change yet.
    mSession.SetString( "address", EVEClientSession::GetAddress().c_str() );
    mSession.SetString( "languageID", ccp.user_languageid.c_str() );

    //user type 1 is normal user, type 23 is a trial account user.
    mSession.SetInt( "userType", 1 );
    mSession.SetInt( "userid", account_info.id );
    mSession.SetLong( "role", account_info.role );

    sLog.Success("Client::Login()","Account \"%s\" logged in from IP %s", account_info.name.c_str() ,EVEClientSession::GetAddress().c_str() );

    return true;

error_login_auth_failed:

    GPSTransportClosed* except = new GPSTransportClosed( transport_closed_msg );
    mNet->QueueRep( except );
    PyDecRef( except );

    return false;
}

bool Client::_VerifyFuncResult( CryptoHandshakeResult& result )
{
    _log(NET__PRES_DEBUG, "%s: Handshake result received.", GetAddress().c_str());

    //send this before session change
    CryptoHandshakeAck ack;
    ack.jit = GetLanguageID();
    ack.userid = GetAccountID();
    ack.maxSessionTime = new PyNone;
    ack.userType = 1;
    ack.role = GetAccountRole();
    ack.address = GetAddress();
    ack.inDetention = new PyNone;
    // no client update available
    ack.client_hash = new PyNone;
    ack.user_clientid = GetAccountID();
    ack.live_updates = sLiveUpdateDB.GetUpdates();

    PyRep* r = ack.Encode();
    mNet->QueueRep( r );
    PyDecRef( r );

    // Send out the session change
    _SendSessionChange();

    return true;
}

/************************************************************************/
/* EVEPacketDispatcher interface                                        */
/************************************************************************/
bool Client::Handle_CallReq( PyPacket* packet, PyCallStream& req )
{
    PyCallable* dest;
    if( packet->dest.service.empty() )
    {
        //bound object
        uint32 nodeID, bindID;
        if( sscanf( req.remoteObjectStr.c_str(), "N=%u:%u", &nodeID, &bindID ) != 2 )
        {
            sLog.Error("Client","Failed to parse bind string '%s'.", req.remoteObjectStr.c_str());
            return false;
        }

        if( nodeID != m_services.GetNodeID() )
        {
            sLog.Error("Client","Unknown nodeID %u received (expected %u).", nodeID, m_services.GetNodeID());
            return false;
        }

        dest = services().FindBoundObject( bindID );
        if( dest == NULL )
        {
            sLog.Error("Client", "Failed to find bound object %u.", bindID);
            return false;
        }
    }
    else
    {
        //service
        dest = services().LookupService( packet->dest.service );
        if( dest == NULL )
        {
            sLog.Error("Client","Unable to find service to handle call to: %s", packet->dest.service.c_str());
            packet->dest.Dump(CLIENT__ERROR, "    ");

//#pragma message( "TODO: throw proper exception to client (exceptions.ServiceNotFound)." )
            throw PyException( new PyNone );
        }
    }

    //Debug code
    if( req.method == "BeanCount" )
        sLog.Warning("Client::BeanCount","BeanCount error reporting and handling is not implemented yet.");
    else
        sLog.Log("Server", "%s call made to %s",req.method.c_str(),packet->dest.service.c_str());

    //build arguments
    PyCallArgs args( this, req.arg_tuple, req.arg_dict );

    //parts of call may be consumed here
    PyResult result = dest->Call( req.method, args );

    _SendSessionChange();  //send out the session change before the return.
    _SendCallReturn( packet->dest, packet->source.callID, &result.ssResult );

    return true;
}

bool Client::Handle_Notify( PyPacket* packet )
{
    //turn this thing into a notify stream:
    ServerNotification notify;
    if( !notify.Decode( packet->payload ) )
    {
        sLog.Error("Client","Failed to convert rep into a notify stream");
        return false;
    }

    if(notify.method == "ClientHasReleasedTheseObjects")
    {
        ServerNotification_ReleaseObj element;

        PyList::const_iterator cur, end;
        cur = notify.elements->begin();
        end = notify.elements->end();
        for(; cur != end; cur++)
        {
            if(!element.Decode( *cur )) {
                sLog.Error("Client","Notification '%s' from %s: Failed to decode element. Skipping.", notify.method.c_str(), GetName());
                continue;
            }

            uint32 nodeID, bindID;
            if(sscanf(element.boundID.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
                sLog.Error("Client","Notification '%s' from %s: Failed to parse bind string '%s'. Skipping.",
                    notify.method.c_str(), GetName(), element.boundID.c_str());
                continue;
            }

            if(nodeID != m_services.GetNodeID()) {
                sLog.Error("Client","Notification '%s' from %s: Unknown nodeID %u received (expected %u). Skipping.",
                    notify.method.c_str(), GetName(), nodeID, m_services.GetNodeID());
                continue;
            }

            m_services.ClearBoundObject(bindID);
        }
    }
    else
    {
        sLog.Error("Client","Unhandled notification from %s: unknown method '%s'", GetName(), notify.method.c_str());
        return false;
    }

    _SendSessionChange();  //just for good measure...
    return true;
}

void Client::UpdateSession(const char *sessionType, int value)
{
    mSession.SetInt(sessionType, value);
}

