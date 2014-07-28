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
    Author:        Zhur, Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "standing/Standing.h"

PyCallable_Make_InnerDispatcher(Standing)

Standing::Standing(PyServiceMgr *mgr)
: PyService(mgr, "standing2"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(Standing, GetMyKillRights);
    PyCallable_REG_CALL(Standing, GetNPCNPCStandings);
    PyCallable_REG_CALL(Standing, GetMyStandings);
    PyCallable_REG_CALL(Standing, GetSecurityRating);
    PyCallable_REG_CALL(Standing, GetStandingTransactions);
    PyCallable_REG_CALL(Standing, GetCharStandings);
    PyCallable_REG_CALL(Standing, GetCorpStandings);

}

Standing::~Standing() {
    delete m_dispatch;
}


PyResult Standing::Handle_GetMyKillRights(PyCallArgs &call) {
  /*
20:12:41 L Standing::Handle_GetMyKillRights(): size= 0
20:12:41 [SvcCall]   Call Arguments:
20:12:41 [SvcCall]       Tuple: Empty
20:12:41 [SvcCall]   Call Named Arguments:
20:12:41 [SvcCall]     Argument 'machoVersion':
20:12:41 [SvcCall]         Integer field: 1
*/
    PyRep *result = NULL;

    PyTuple *tu = new PyTuple(2);
    PyDict *u1 = new PyDict();
    PyDict *u2 = new PyDict();
    tu->items[0] = u1;
    tu->items[1] = u2;
    result = tu;

    return result;
}

PyResult Standing::Handle_GetMyStandings(PyCallArgs &call) {
  sLog.Log( "Standing::Handle_GetMyStandings()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
   /*
      Args:   [ 4]   [ 0]       Args:   [ 1]     Tuple: 6 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0] Object:
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Type: String: 'util.Rowset'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args: Dictionary: 3 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Key: String: 'header'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Value: List: 2 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Value:   [ 0] String: 'toID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Value:   [ 1] String: 'standing'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 1] Key: String: 'RowClass'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 1] Value: String (Type1): 'util.Row'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 2] Key: String: 'lines'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 2] Value: List: Empty
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1] Object:
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Type: String: 'util.Rowset'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args: Dictionary: 3 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Key: String: 'header'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value: List: 3 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value:   [ 0] String: 'ownerID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value:   [ 1] String: 'ownerName'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value:   [ 2] String: 'typeID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 1] Key: String: 'RowClass'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 1] Value: String (Type1): 'util.Row'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 2] Key: String: 'lines'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 2] Value: List: Empty
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2] Object:
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Type: String: 'util.Rowset'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args: Dictionary: 3 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Key: String: 'header'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Value: List: 2 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Value:   [ 0] String: 'fromID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Value:   [ 1] String: 'standing'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 1] Key: String: 'RowClass'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 1] Value: String (Type1): 'util.Row'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Key: String: 'lines'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value: List: 1 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value:   [ 0] List: 2 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value:   [ 0]   [ 0] Integer field: 3012239
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value:   [ 0]   [ 1] Real Field: 10.000000
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 3] Dictionary: 0 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 4] Dictionary: 0 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 5] Dictionary: 0 entries
    */
    PyRep *result = NULL;

    PyRep *charstandings;
    PyRep *charprime;
    PyRep *npccharstandings;

    charstandings = m_db.GetCharStandings(call.client->GetCharacterID());
    charprime = m_db.GetCharPrimeStandings(call.client->GetCharacterID());
    npccharstandings = m_db.GetCharNPCStandings(call.client->GetCharacterID());

    PyDict *corpstandings = new PyDict();
    PyDict *corpprime = new PyDict();
    PyDict *npccorpstandings = new PyDict();

    PyTuple *tu = new PyTuple(6);
    tu->items[0] = charstandings;
    tu->items[1] = charprime;
    tu->items[2] = npccharstandings;
    tu->items[3] = corpstandings;
    tu->items[4] = corpprime;
    tu->items[5] = npccorpstandings;
    result = tu;

    return result;
}


PyResult Standing::Handle_GetNPCNPCStandings(PyCallArgs &call) {
  /*
20:12:42 L Standing::Handle_GetNPCNPCStandings(): size= 0
20:12:42 [SvcCall]   Call Arguments:
20:12:42 [SvcCall]       Tuple: Empty
20:12:42 [SvcCall]   Call Named Arguments:
20:12:42 [SvcCall]     Argument 'machoVersion':
20:12:42 [SvcCall]         List: 2 elements
20:12:42 [SvcCall]           [ 0] Integer field: 130385792280000000
20:12:42 [SvcCall]           [ 1] Integer field: 1389841652
  sLog.Log( "Standing::Handle_GetNPCNPCStandings()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
*/
    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetNPCNPCStandings");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetNPCStandings();
        if(result == NULL)
            result = new PyNone();
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}

PyResult Standing::Handle_GetSecurityRating(PyCallArgs &call) {
  /*
16:08:58 L Standing::Handle_GetSecurityRating(): size= 1
16:08:58 [SvcCall]   Call Arguments:
16:08:58 [SvcCall]       Tuple: 1 elements
16:08:58 [SvcCall]         [ 0] Integer field: 140000212
16:08:58 [SvcCall]   Call Named Arguments:
16:08:58 [SvcCall]     Argument 'machoVersion':
16:08:58 [SvcCall]         Integer field: 1
  sLog.Log( "Standing::Handle_GetSecurityRating()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
*/
    //takes an integer: characterID
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return NULL;
    }

    CharacterRef c = m_manager->item_factory.GetCharacter( arg.arg );
    if( !c ) {
        _log(SERVICE__ERROR, "Character %u not found.", arg.arg);
        return NULL;
    }

    return new PyFloat( c->securityRating() );
}

PyResult Standing::Handle_GetStandingTransactions(PyCallArgs &call) {
  /*-
21:50:56 L Standing::Handle_GetStandingTransactions(): size= 6
21:50:56 [SvcCall]   Call Arguments:
21:50:56 [SvcCall]       Tuple: 6 elements
21:50:56 [SvcCall]         [ 0] Integer field: 1000125
21:50:56 [SvcCall]         [ 1] Integer field: 140000000
21:50:56 [SvcCall]         [ 2] Integer field: 1
21:50:56 [SvcCall]         [ 3] (None)
21:50:56 [SvcCall]         [ 4] (None)
21:50:56 [SvcCall]         [ 5] (None)
21:50:56 [SvcCall]   Call Named Arguments:
21:50:56 [SvcCall]     Argument 'machoVersion':
21:50:56 [SvcCall]         Integer field: 1
  sLog.Log( "Standing::Handle_GetStandingTransactions()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
*/

    Call_GetStandingTransactions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    PyObject * result = m_db.GetStandingTransactions(args.toID);

    return (result);
}

PyResult Standing::Handle_GetCharStandings(PyCallArgs &call) {
  /*
20:12:44 L Standing::Handle_GetCharStandings(): size= 0
20:12:44 [SvcCall]   Call Arguments:
20:12:44 [SvcCall]       Tuple: Empty
20:12:44 [SvcCall]   Call Named Arguments:
20:12:44 [SvcCall]     Argument 'machoVersion':
20:12:44 [SvcCall]         List: 2 elements
20:12:44 [SvcCall]           [ 0] Integer field: 130385792290000000     //time
20:12:44 [SvcCall]           [ 1] Integer field: 1305240809
  sLog.Log( "Standing::Handle_GetCharStandings()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
*/
/*
    ObjectCachedSessionMethodID method_id(GetName(), "GetCharStandings", call.client->GetCharacterID());

    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyObjectEx *t = m_db.GetCharStandings(call.client->GetCharacterID());

        m_manager->cache_service->GiveCache(method_id, (PyRep **)&t);
    }

    return(m_manager->cache_service->MakeObjectCachedSessionMethodCallResult(method_id, "charID"));
    */
    return(m_db.GetCharStandings(call.client->GetCharacterID()));
}
PyResult Standing::Handle_GetCorpStandings(PyCallArgs &call) {
  sLog.Log( "Standing::Handle_GetCorpStandings()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

    ObjectCachedSessionMethodID method_id(GetName(), "GetCorpStandings", call.client->GetCorporationID());

    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyObjectEx *t = m_db.GetCorpStandings(call.client->GetCorporationID());

        m_manager->cache_service->GiveCache(method_id, (PyRep **)&t);
    }

    return(m_manager->cache_service->MakeObjectCachedSessionMethodCallResult(method_id, "corpID"));
}


/**
  standing.CanUseAgent



*/