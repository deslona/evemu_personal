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

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "character/CharMgrService.h"

class CharMgrBound
	: public PyBoundObject
{
public:
	PyCallable_Make_Dispatcher(CharMgrBound);

    CharMgrBound(PyServiceMgr *mgr, uint32 characterID)
		: PyBoundObject(mgr),
		m_dispatch(new Dispatcher(this)),
		m_characterID(characterID)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "CharMgrBound";

		PyCallable_REG_CALL(CharMgrBound, ListStations);
		PyCallable_REG_CALL(CharMgrBound, ListStationItems);
	}

    virtual ~CharMgrBound() {delete m_dispatch;}

	virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(ListStations)
    PyCallable_DECL_CALL(ListStationItems)

protected:
    Dispatcher *const m_dispatch;

    uint32 m_characterID;
};

PyResult CharMgrBound::Handle_ListStations( PyCallArgs& call )
{
	// TODO: really query the database directly?
    DBQueryResult res;
	if(!sDatabase.RunQuery(res, "SELECT locationID AS stationID, COUNT(itemID) as itemCount FROM entity WHERE ownerID=%d AND flag=4 GROUP BY locationID", m_characterID))
	{
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	return(DBResultToCRowset(res));
}

PyResult CharMgrBound::Handle_ListStationItems( PyCallArgs& call )
{       // this is the assets window
	uint32 locationID = call.tuple->GetItem(0)->AsInt()->value();

	// TODO: really query the database directly?
	DBQueryResult res;
	if(!sDatabase.RunQuery(res,
		"SELECT "
		"  e.itemID, "
		"  e.itemName, "
		"  e.typeID, "
		"  e.ownerID, "
		"  e.locationID, "
		"  e.flag as flagID, "
		"  e.quantity as stacksize, "
		"  e.customInfo, "
		"  e.singleton, "
		"  g.categoryID, "
		"  g.groupID "
		"FROM "
		"  (entity e LEFT JOIN invTypes t ON e.typeID=t.typeID) LEFT JOIN invGroups g ON t.groupID=g.groupID "
		"WHERE "
		"  e.ownerID=%d AND "
		"  e.locationID=%d AND "
		"  e.flag=4"
		, m_characterID, locationID))
	{
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	return(DBResultToCRowset(res));
}

PyCallable_Make_InnerDispatcher(CharMgrService)

CharMgrService::CharMgrService(PyServiceMgr *mgr)
: PyService(mgr, "charMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CharMgrService, GetPublicInfo)
    PyCallable_REG_CALL(CharMgrService, GetPublicInfo3)
    PyCallable_REG_CALL(CharMgrService, GetTopBounties)
    PyCallable_REG_CALL(CharMgrService, AddToBounty)
    PyCallable_REG_CALL(CharMgrService, GetOwnerNoteLabels)
    PyCallable_REG_CALL(CharMgrService, AddOwnerNote)
    PyCallable_REG_CALL(CharMgrService, GetContactList)
    PyCallable_REG_CALL(CharMgrService, GetCloneTypeID)
    PyCallable_REG_CALL(CharMgrService, GetHomeStation)
    PyCallable_REG_CALL(CharMgrService, GetFactions)
    PyCallable_REG_CALL(CharMgrService, SetActivityStatus)
    PyCallable_REG_CALL(CharMgrService, GetSettingsInfo)
    PyCallable_REG_CALL(CharMgrService, GetCharacterDescription)
    PyCallable_REG_CALL(CharMgrService, SetCharacterDescription)
    PyCallable_REG_CALL(CharMgrService, GetNote)
    PyCallable_REG_CALL(CharMgrService, SetNote)
    PyCallable_REG_CALL(CharMgrService, AddContact)
    PyCallable_REG_CALL(CharMgrService, EditContact)
    PyCallable_REG_CALL(CharMgrService, GetRecentShipKillsAndLosses)

    //these 2 are for labels in PnP window
    PyCallable_REG_CALL(CharMgrService, GetLabels)
    PyCallable_REG_CALL(CharMgrService, CreateLabel)
}

CharMgrService::~CharMgrService() {
    delete m_dispatch;
}

PyBoundObject *CharMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    Call_TwoIntegerArgs args;
    //crap
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        _log(SERVICE__ERROR, "%s: Failed to decode bind object params.", GetName());
        return NULL;
    }
	//bind_args->Dump(SERVICE__ERROR, "    ");
    //arg1 = characterID
    //arg2 = ??? (i.e.: 10002)

	return(new CharMgrBound(m_manager, args.arg1));
}

PyResult CharMgrService::Handle_GetContactList(PyCallArgs &call) {
  /*
19:52:53 [SvcCall] Service charMgr: calling GetContactList
19:52:53 [SvcCall]   Call Arguments:
19:52:53 [SvcCall]       Tuple: Empty
19:52:53 [SvcCall]   Call Named Arguments:
19:52:53 [SvcCall]     Argument 'machoVersion':
19:52:53 [SvcCall]         Integer field: 1
    call.Dump(SERVICE__CALLS);
    */
    // another dummy
    DBRowDescriptor *header = new DBRowDescriptor();
    header->AddColumn("contactID", DBTYPE_I4);
    header->AddColumn("inWatchList", DBTYPE_BOOL);
    header->AddColumn("relationshipID", DBTYPE_R8);
    header->AddColumn("labelMask", DBTYPE_I8);
    CRowSet *rowset = new CRowSet( &header );

    PyDict* dict = new PyDict();
    dict->SetItemString("addresses", rowset);
    dict->SetItemString("blocked", rowset);
    PyObject *keyVal = new PyObject( "util.KeyVal", dict);

    return keyVal;
}

PyResult CharMgrService::Handle_AddOwnerNote( PyCallArgs& call )
{
  uint8 size = call.tuple->size();
  sLog.Log( "CharMgrService::Handle_AddOwnerNote()", "size=%u ", size );
  call.Dump(SERVICE__CALLS);

  return NULL;
}

PyResult CharMgrService::Handle_GetOwnerNoteLabels(PyCallArgs &call)
{
  sLog.Log( "CharMgrService::Handle_GetOwnerNoteLabels()", "size= %u", call.tuple->size() );
    // just a dummy for now
    DBRowDescriptor *header = new DBRowDescriptor();
    header->AddColumn("noteID", DBTYPE_I4);
    header->AddColumn("label", DBTYPE_WSTR);
    CRowSet *rowset = new CRowSet( &header );

    return rowset;
}

PyResult CharMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
  /*
17:01:37 [SvcCall] Service charMgr: calling GetPublicInfo
17:01:37 [SvcCall]   Call Arguments:
17:01:37 [SvcCall]       Tuple: 1 elements
17:01:37 [SvcCall]         [ 0] Integer field: 140000000
17:01:37 [SvcCall]   Call Named Arguments:
17:01:37 [SvcCall]     Argument 'machoVersion':
17:01:37 [SvcCall]         Integer field: 1

    call.Dump(SERVICE__CALLS);
*/
    //takes a single int arg: char id or corp id
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return NULL;
    }

    /*if(IsAgent(args.arg)) {
        //handle agents special right now...
        PyRep *result = m_db.GetAgentPublicInfo(args.arg);
        if(result == NULL) {
            codelog(CLIENT__ERROR, "%s: Failed to find agent %u", call.client->GetName(), args.arg);
            return NULL;
        }
        return result;
    }*/

    PyRep *result = m_db.GetCharPublicInfo(args.arg);
    if(result == NULL) {
        codelog(CLIENT__ERROR, "%s: Failed to find char %u", call.client->GetName(), args.arg);
        return NULL;
    }

    return result;
}

PyResult CharMgrService::Handle_GetPublicInfo3(PyCallArgs &call) {
    //takes a single int arg: char id
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return NULL;
    }

    PyRep *result = m_db.GetCharPublicInfo3(args.arg);
    if(result == NULL) {
        codelog(CLIENT__ERROR, "%s: Failed to find char %u", call.client->GetName(), args.arg);
        return NULL;
    }

    return result;
}

PyResult CharMgrService::Handle_AddToBounty( PyCallArgs& call )
{/**
23:02:28 [SvcCall] Service charMgr: calling AddToBounty
23:02:28 [SvcCallTrace]   Call Arguments:
23:02:28 [SvcCallTrace]       Tuple: 2 elements
23:02:28 [SvcCallTrace]         [ 0] Integer field: 140000524       <- char to put bounty on
23:02:28 [SvcCallTrace]         [ 1] Integer field: 5000            <- amount of bounty
23:02:28 [SvcCallTrace]   Call Named Arguments:
23:02:28 [SvcCallTrace]     Argument 'machoVersion':
23:02:28 [SvcCallTrace]         Integer field: 1
*/
  sLog.Log( "CharMgrService::Handle_AddToBounty()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    Call_TwoIntegerArgs args;
    if( !args.Decode( &call.tuple ) )  {
        codelog( SERVICE__ERROR, "Unable to decode arguments for CharMgrService::Handle_AddToBounty from '%s'", call.client->GetName() );
        return NULL;
    }

    //AddBounty(uint32 charID, uint32 ownerID, uint32 amount)
    m_db.AddBounty(args.arg1, call.client->GetCharacterID(), args.arg2);

    return new PyNone;
}

//02:17:26 L CharMgrService::Handle_GetTopBounties(): size= 0
PyResult CharMgrService::Handle_GetTopBounties( PyCallArgs& call ) {
    return(m_db.GetTopBounties());
}

PyResult CharMgrService::Handle_GetCloneTypeID( PyCallArgs& call )
{
	uint32 typeID;
	if( !m_db.GetActiveCloneType(call.client->GetCharacterID(), typeID ) )
	{
		// This should not happen, because a clone is created at char creation.
		// We don't have a clone, so return a basic one. cloneTypeID = 9917 (Clone Grade Delta)
		typeID = 9917;
		sLog.Debug( "CharMgrService", "Returning a basic clone for Char %u of type %u", call.client->GetCharacterID(), typeID );
	}
    return new PyInt(typeID);
}

PyResult CharMgrService::Handle_GetHomeStation( PyCallArgs& call )
{
	uint32 stationID;
	if( !m_db.GetCharHomeStation(call.client->GetCharacterID(), stationID) )
	{
		sLog.Debug( "CharMgrService", "Could't get the home station for Char %u", call.client->GetCharacterID() );
		return new PyNone;
	}
    return new PyInt(stationID);
}

PyResult CharMgrService::Handle_GetFactions( PyCallArgs& call )
{
  sLog.Log( "CharMgrService::Handle_GetFactions()", "size= %u", call.tuple->size() );

    return NULL;
}

PyResult CharMgrService::Handle_SetActivityStatus( PyCallArgs& call ) {
  uint8 size = call.tuple->size();
  uint16 int1 = call.tuple->GetItem(0)->AsInt()->value();
  uint16 int2 = call.tuple->GetItem(1)->AsInt()->value();
  sLog.Log( "CharMgrService::Handle_SetActivityStatus()", "size=%u, 0=Int(%u), 1=Int(%u) ", size, int1, int2   );
  //call.Dump(SERVICE__CALLS);

    return new PyInt( 0 );
}

PyResult CharMgrService::Handle_GetSettingsInfo( PyCallArgs& call ) {
  /*
22:07:36 L CharMgrService::Handle_GetSettingsInfo(): size= 0
22:07:36 [SvcCall]   Call Arguments:
22:07:36 [SvcCall]       Tuple: Empty
22:07:36 [SvcCall]   Call Named Arguments:
22:07:36 [SvcCall]     Argument 'machoVersion':
22:07:36 [SvcCall]         Integer field: 1
  sLog.Log( "CharMgrService::Handle_GetSettingsInfo()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
    */
    //return new PyInt( 0 );
    return NULL;
}

PyResult CharMgrService::Handle_GetCharacterDescription(PyCallArgs &call)
{
    //takes characterID
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    m_manager->item_factory.SetUsingClient(call.client);
    CharacterRef c = m_manager->item_factory.GetCharacter(args.arg);
    if( !c ) {
        _log(CLIENT__ERROR, "GetCharacterDescription failed to load character %u.", args.arg);
        return NULL;
    }

    return new PyString(c->description());
}

PyResult CharMgrService::Handle_SetCharacterDescription(PyCallArgs &call)
{
    //takes WString of bio
    Call_SingleWStringSoftArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    CharacterRef c = call.client->GetChar();
    if( !c ) {
        _log(CLIENT__ERROR, "SetCharacterDescription called with no char!");
        return NULL;
    }
    c->SetDescription(args.arg.c_str());

    return NULL;
}

//17:09:10 L CharMgrService::Handle_GetNote(): size= 1
PyResult CharMgrService::Handle_GetNote( PyCallArgs& call )
{
    uint32 ownerID = call.client->GetCharacterID();
    uint32 itemID = call.tuple->GetItem(0)->AsInt()->value();

	PyString *str = m_db.GetNote(ownerID, itemID);
    if(!str)
        str = new PyString("");

    return str;
}

PyResult CharMgrService::Handle_SetNote(PyCallArgs &call)
{
    Call_SetNote args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    m_db.SetNote(call.client->GetCharacterID(), args.itemID, args.note.c_str());

    return new PyNone;
}

//18:07:30 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(2784)
//18:07:35 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(63177)
PyResult CharMgrService::Handle_AddContact( PyCallArgs& call )
{
  /**
00:26:29 [SvcCall] Service charMgr: calling AddContact      *** adding agent
00:26:29 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(3017440)
00:26:29 [SvcCall]   Call Arguments:
00:26:29 [SvcCall]       Tuple: 1 elements
00:26:29 [SvcCall]         [ 0] Integer field: 3017440
00:26:29 [SvcCall]   Call Named Arguments:
00:26:29 [SvcCall]     Argument 'machoVersion':
00:26:29 [SvcCall]         Integer field: 1

15:48:32 [SvcCall] Service charMgr: calling AddContact      *** adding player
15:48:32 L CharMgrService::Handle_AddContact(): size=5, 0=Integer(140000212)
15:48:32 [SvcCall]   Call Arguments:
15:48:32 [SvcCall]       Tuple: 5 elements
15:48:32 [SvcCall]         [ 0] Integer field: 140000212
15:48:32 [SvcCall]         [ 1] Integer field: 10
15:48:32 [SvcCall]         [ 2] Integer field: 1
15:48:32 [SvcCall]         [ 3] Integer field: 1
15:48:32 [SvcCall]         [ 4] String: ''
15:48:32 [SvcCall]   Call Named Arguments:
15:48:32 [SvcCall]     Argument 'machoVersion':
15:48:32 [SvcCall]         Integer field: 1
*/
  uint8 size = call.tuple->size();
  uint32 int0 = call.tuple->GetItem(0)->AsInt()->value();
  sLog.Log( "CharMgrService::Handle_AddContact()", "size=%u, 0=%s(%u) ", size, call.tuple->GetItem( 0 )->TypeString(), int0 );
  call.Dump(SERVICE__CALLS);

  // make db call to save contact.  will have to find the call to get contact list....
  return NULL;
}

PyResult CharMgrService::Handle_EditContact( PyCallArgs& call )
{
  uint8 size = call.tuple->size();
  sLog.Log( "CharMgrService::Handle_EditContact()", "size=%u ", size );
  call.Dump(SERVICE__CALLS);

  return NULL;
}

PyResult CharMgrService::Handle_GetRecentShipKillsAndLosses( PyCallArgs& call )
{
  uint8 size = call.tuple->size();
  sLog.Log( "CharMgrService::Handle_GetRecentShipKillsAndLosses()", "size=%u ", size );
  call.Dump(SERVICE__CALLS);

  return NULL;
}

PyResult CharMgrService::Handle_GetLabels( PyCallArgs& call )
{
  uint8 size = call.tuple->size();
  sLog.Log( "CharMgrService::Handle_GetLabels()", "size=%u ", size );
  call.Dump(SERVICE__CALLS);

//AttributeError: 'NoneType' object has no attribute 'values'
//AttributeError: 'NoneType' object has no attribute 'itervalues'

  return NULL;
}

PyResult CharMgrService::Handle_CreateLabel( PyCallArgs& call )
{
  uint8 size = call.tuple->size();
  sLog.Log( "CharMgrService::Handle_CreateLabel()", "size=%u ", size );
  call.Dump(SERVICE__CALLS);

  return NULL;
}
