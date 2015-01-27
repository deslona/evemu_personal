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
    Author:        Zhur
*/

/*
Dynamic Bodies:
    - shape
    - `mass`
    - `radius`
    - `volume`???
    - `Inertia`???
    - inertia information
    - position
    - velocity
    - angular velocity
    - collide with things
        - approximate eve "run into and stop/turn around" collisions

Ship: extends Dynamic
    - `maxVelocity`
    - thrust (propulsion + speed ratio)
    - angular thrust
    - "stopping" algorithm
    - "orbit" algorithm
    - "turning" algorithm

Static Bodies:
    - shape
    - position

detect clients moving into agro radius
*/

#ifndef EVE_CLIENT_H
#define EVE_CLIENT_H

#include "ClientSession.h"

#include "inventory/InventoryItem.h"
#include "character/Character.h"
#include "ship/Ship.h"

#include "system/SystemEntity.h"
#include "system/SystemGPoint.h"
#include "ship/ModuleManager.h"

class CryptoChallengePacket;
class EVENotificationStream;
class PySubStream;
class InventoryItem;
class SystemManager;
class PyServiceMgr;
class PyCallStream;
class PyTuple;
class LSCChannel;
class PyAddress;
class PyList;
class PyDict;
class PyPacket;
//class Client;
class PyRep;

//DO NOT INHERIT THIS OBJECT!
class Client
: public DynamicSystemEntity,
  protected EVEClientSession,
  protected EVEPacketDispatcher
{
public:
    Client(PyServiceMgr &services, EVETCPConnection** con);
    virtual ~Client();

    bool            ProcessNet();
    virtual void    Process();

    PyServiceMgr& services() const { return m_services; }

    /********************************************************************/
    /* Session values                                                   */
    /********************************************************************/
    std::string GetAddress() const                  { return mSession.GetCurrentString( "address" ); }
    std::string GetLanguageID() const               { return mSession.GetCurrentString( "languageID" ); }

    uint32 GetAccountType() const                   { return mSession.GetCurrentInt( "userType" ); }
    uint32 GetAccountID() const                     { return mSession.GetCurrentInt( "userid" ); }
    uint64 GetAccountRole() const                   { return mSession.GetCurrentLong( "role" ); }

    uint32 GetCharacterID() const                   { return mSession.GetCurrentInt( "charid" ); }
    std::string GetCharacterName() const            { return mSession.GetCurrentString( "charname" ); }
    uint32 GetCorporationID() const                 { return mSession.GetCurrentInt( "corpid" ); }
    uint32 GetLocationID() const                    { return mSession.GetCurrentInt( "locationid" ); }
    uint32 GetStationID() const                     { return mSession.GetCurrentInt( "stationid" ); }
    uint32 GetSystemID() const                      { return mSession.GetCurrentInt( "solarsystemid2" ); }
    uint32 GetConstellationID() const               { return mSession.GetCurrentInt( "constellationid" ); }
    uint32 GetRegionID() const                      { return mSession.GetCurrentInt( "regionid" ); }
    uint32 GetCloneLocationID() const               { return mSession.GetCurrentInt( "cloneLocationID" ); }

    uint32 GetCorpHQ() const                        { return mSession.GetCurrentInt( "hqID" ); }
    int32 GetCorpAccountKey() const                 { return mSession.GetCurrentInt( "corpAccountKey" ); }
    int64 GetCorpRole() const                       { return mSession.GetCurrentLong( "corpRole" ); }
    int64 GetRolesAtAll() const                     { return mSession.GetCurrentLong( "rolesAtAll" ); }
    int64 GetRolesAtBase() const                    { return mSession.GetCurrentLong( "rolesAtBase" ); }
    int64 GetRolesAtHQ() const                      { return mSession.GetCurrentLong( "rolesAtHQ" ); }
    int64 GetRolesAtOther() const                   { return mSession.GetCurrentLong( "rolesAtOther" ); }

    uint32 GetShipID() const                        { return m_shipId; }
    uint32 GetGangRole() const                      { return mSession.GetCurrentInt( "gangrole" ); }

    uint8 GetFleetRole() const                      { return mSession.GetCurrentInt( "fleetrole" ); }

    //  public functions to update client session when char's roles are changed
    void UpdateCorpSession( const CharacterConstRef& character );
    void UpdateFleetSession( const CharacterConstRef& character );

    // character data
    CharacterRef GetChar() const                    { return m_char; }
    ShipRef GetShip() const                         { return ShipRef::StaticCast( Item() ); }
    bool InPod() const                              { return ( GetShip()->groupID() == EVEDB::invGroups::Capsule ); }

    bool IsInSpace() const                          { return ( GetStationID() == 0 ); }
    double x() const                                { return GetPosition().x; }    //this is terribly inefficient.
    double y() const                                { return GetPosition().y; }    //this is terribly inefficient.
    double z() const                                { return GetPosition().z; }    //this is terribly inefficient.

    uint32 GetAllianceID() const                    { return GetChar()->allianceID(); }
    uint32 GetWarFactionID() const                  { return GetChar()->warFactionID(); }
    double GetBounty() const                        { return GetChar()->bounty(); }
    double GetSecurityRating() const                { return GetChar()->securityRating(); }
    double GetBalance() const                       { return GetChar()->balance(); }
    double GetAurBalance() const                    { return GetChar()->aurBalance(); }

    std::string GetSystemName() const               { return(m_systemName); }

    bool AddBalance(double amount);

    void BoardShip(ShipRef new_ship);
    void UndockFromStation( uint32, uint32, uint32, uint32, GPoint, GPoint );
    void MoveToLocation(uint32 location, const GPoint &pt);
    void MoveToPosition(const GPoint &pt);
    void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);
    bool EnterSystem(bool login = false);
    bool UpdateLocation(bool login = false);
    bool SelectCharacter( uint32 char_id );
    void JoinCorporationUpdate(uint32 corp_id);
    void SavePosition();
    void SaveAllToDatabase();
    void UpdateSkillTraining();
	void SpawnNewRookieShip();

    double GetPropulsionStrength() const;

    bool LaunchDrone(InventoryItemRef drone);

    void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

    //destiny stuff...
    void StargateJump(uint32 fromGate, uint32 toGate);
    void SetDockStationID(uint32 stationID) { m_dockStationID = stationID; };
    uint32 GetDockStationID() { return m_dockStationID; };

    bool GetPendingDockOperation() { return m_needToDock; };
    void SetPendingDockOperation(bool needToDock) { m_needToDock = needToDock; }
	void SetAutoPilot(bool);

    //messages and LSC
    void SendErrorMsg(const char *fmt, ...);
    void SendErrorMsg(const char *fmt, va_list args);
    void SendNotifyMsg(const char *fmt, ...);
    void SendNotifyMsg(const char *fmt, va_list args);
    void SendInfoModalMsg(const char *fmt, ...);
    void SelfChatMessage(const char *fmt, ...);
    void SelfEveMail(const char *subject, const char *fmt, ...);
    void ChannelJoined(LSCChannel *chan);
    void ChannelLeft(LSCChannel *chan);
    void UpdateSession( const char *sessionType, int value );
    bool IsKennyTranslatorEnabled() { return bKennyfied; };
    void EnableKennyTranslator() { bKennyfied = true; };
    void DisableKennyTranslator() { bKennyfied = false; };

    // character notification messages
    void OnCharNowInStation();
    void OnCharNoLongerInStation();

    /********************************************************************/
    /* DynamicSystemEntity interface                                    */
    /********************************************************************/
    virtual EntityClass GetClass() const { return(ecClient); }
    virtual bool IsClient() const { return true; }
    virtual Client *CastToClient() { return(this); }
    virtual const Client *CastToClient() const { return(this); }

    virtual const char *GetName() const { return GetChar() ? GetChar()->itemName().c_str() : "(null)"; }
    virtual PyDict *MakeSlimItem() const;
    virtual PyRep *GetAggressors() const;
    virtual void QueueDestinyUpdate(PyTuple** du);
    virtual void QueueDestinyEvent(PyTuple** multiEvent);

    virtual void TargetAdded(SystemEntity *who);
    virtual void TargetLost(SystemEntity *who);
    virtual void TargetedAdd(SystemEntity *who);
    virtual void TargetedLost(SystemEntity *who);
    virtual void TargetsCleared();

    virtual void ApplyDamageModifiers(Damage &d, SystemEntity *target);
    virtual bool ApplyDamage(Damage &d);
    virtual void Killed(Damage &fatal_blow);
    virtual void _DropLoot(uint32 groupID, uint32 owner, uint32 locationID);
    virtual SystemManager *System() const { return (m_system); }    //may yeild NULL

    /********************************************************************/
    /* Server Administration Interface                                  */
    /********************************************************************/
    void DisconnectClient();
    void BanClient();

protected:
    void _ReduceDamage(Damage &d);
    void _UpdateSession( const CharacterConstRef& character );
    void _UpdateSession2( uint32 characterID  );

    // Packet stuff
    void _SendCallReturn( const PyAddress& source, uint64 callID, PyRep** return_value, const char* channel = NULL );
    void _SendException( const PyAddress& source, uint64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload );
    void _SendSessionChange();
    void _SendPingRequest();
    void _SendPingResponse( const PyAddress& source, uint64 callID );

    PyServiceMgr& m_services;
    Timer m_pingTimer;
    ClientSession mSession;

    SystemManager *m_system;    //we do not own this
    CharacterRef m_char;

	SystemGPoint m_SGP;

    std::set<LSCChannel *> m_channels;    //we do not own these.

    //this whole move system is a piece of crap:
    typedef enum {
        msIdle,
        msJump
    } _MoveState;
    void _postMove(_MoveState type, uint32 wait_ms=500);
    _MoveState m_moveState;
    Timer m_moveTimer;
    uint32 m_moveSystemID;
    GPoint m_movePoint;
    uint32 m_dockStationID;
    void _ExecuteJump();
    bool m_needToDock;

    // set true for using autopilot.
    bool m_autoPilot = false;

    uint32 m_shipId;

    bool m_justUndocked;
    int m_justUndockedCount;
    GPoint m_undockAlignToPoint;

    EvilNumber m_timeEndTrain;

    /********************************************************************/
    /* EVEClientSession interface                                       */
    /********************************************************************/
    void _GetVersion( VersionExchangeServer& version );
    uint32 _GetUserCount();
    uint32 _GetQueuePosition() { /* hack */ return 1; }

    /********************************************************************/
    /* EVEClientLogin statemachine                                      */
    /********************************************************************/
    bool _VerifyVersion( VersionExchangeClient& version );
    bool _VerifyCrypto( CryptoRequestPacket& cr );
    bool _VerifyLogin( CryptoChallengePacket& ccp );
    bool _VerifyVIPKey( const std::string& vipKey ) { /* do nothing */ return true; }
    bool _VerifyFuncResult( CryptoHandshakeResult& result );

    /********************************************************************/
    /* EVEPacketDispatcher interface                                    */
    /********************************************************************/
    bool Handle_CallReq( PyPacket* packet, PyCallStream& req );
    bool Handle_Notify( PyPacket* packet );
    bool Handle_PingReq( PyPacket* packet ) { _SendPingResponse( packet->dest, packet->source.callID ); return true; }
    bool Handle_PingRsp( PyPacket* packet ) { /* do nothing */ return true; }

private:
    //queues for destiny updates:
    PyList* m_destinyEventQueue;    //we own these. These are events as used in OnMultiEvent
    PyList* m_destinyUpdateQueue;    //we own these. They are the `update` which go into DoDestinyAction
    void _SendQueuedUpdates();

    uint32 m_nextNotifySequence;

    bool bKennyfied;

    /************************************************************************/
    /* new system for MultiEvents                                           */
    /************************************************************************/
    bool ScatterEvent(const char* event_name, PyRep* packet);

    bool DoDestinyUpdate();
    std::list<PyTuple*> mDogmaMessages;
    std::string m_systemName;

};

#endif
