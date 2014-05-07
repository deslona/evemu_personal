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
#include "map/MapService.h"

PyCallable_Make_InnerDispatcher(MapService)

MapService::MapService(PyServiceMgr *mgr)
: PyService(mgr, "map"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MapService, GetStationExtraInfo)
    PyCallable_REG_CALL(MapService, GetSolarSystemPseudoSecurities)
    PyCallable_REG_CALL(MapService, GetSolarSystemVisits)
    PyCallable_REG_CALL(MapService, GetBeaconCount)
    PyCallable_REG_CALL(MapService, GetHistory)
    PyCallable_REG_CALL(MapService, GetStationCount)    //ColorStarsByStationCount

    /**  not handled yet...these are empty calls  */
    PyCallable_REG_CALL(MapService, GetStuckSystems)
    PyCallable_REG_CALL(MapService, GetRecentSovActivity)
    PyCallable_REG_CALL(MapService, GetDeadspaceAgentsMap)
    PyCallable_REG_CALL(MapService, GetDeadspaceComplexMap)
    PyCallable_REG_CALL(MapService, GetIncursionGlobalReport)
    PyCallable_REG_CALL(MapService, GetSystemsInIncursions)
    PyCallable_REG_CALL(MapService, GetSystemsInIncursionsGM)
    PyCallable_REG_CALL(MapService, GetAllianceSystems)    // wrong place
    PyCallable_REG_CALL(MapService, GetMyExtraMapInfoAgents)  //ColorStarsByMyAgents
    PyCallable_REG_CALL(MapService, GetVictoryPoints)

}

MapService::~MapService() {
    delete m_dispatch;
}

PyResult MapService::Handle_GetStationExtraInfo(PyCallArgs &call) {
    //takes no arguments
    //returns tuple(
    //     stations: rowset stationID,solarSystemID,operationID,stationTypeID,ownerID
    //     opservices: rowset: (operationID, serviceID) (from staOperationServices)
    //     services: rowset: (serviceID,serviceName) (from staServices)
    // )

    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetStationExtraInfo");

    //uint32 systemID = call.client->GetSystemID();

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.

        PyTuple *resultt = new PyTuple(3);

        resultt->items[0] = m_db.GetStationExtraInfo();
        if(resultt->items[0] == NULL) {
            codelog(SERVICE__ERROR, "Failed to query station info");
            return NULL;
        }

        resultt->items[1] = m_db.GetStationOpServices();
        if(resultt->items[1] == NULL) {
            codelog(SERVICE__ERROR, "Failed to query op services");
            return NULL;
        }

        resultt->items[2] = m_db.GetStationServiceInfo();
        if(resultt->items[2] == NULL) {
            codelog(SERVICE__ERROR, "Failed to query service info");
            return NULL;
        }

        result = resultt;
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}


PyResult MapService::Handle_GetSolarSystemPseudoSecurities(PyCallArgs &call) {
    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetSolarSystemPseudoSecurities");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetPseudoSecurities();
        if(result == NULL)
            result = new PyNone();
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}

PyResult MapService::Handle_GetSolarSystemVisits(PyCallArgs &call)
{       // passes no args
    uint32 charID = call.client->GetCharacterID();
    return (m_db.GetSolSystemVisits(charID));
}

PyResult MapService::Handle_GetHistory(PyCallArgs &call) {
    uint32 int1 = call.tuple->GetItem(0)->AsInt()->value();
    uint32 int2 = call.tuple->GetItem(1)->AsInt()->value();
      //sLog.Log( "MapService::Handle_GetHistory()", "size= %u, 0 = Interger (%u), 1 = Interger (%u)", call.tuple->size(), int1, int2 );

    return (m_db.GetDynamicData(int1, int2));
}

//02:38:07 L MapService::Handle_GetBeaconCount(): size= 0
PyResult MapService::Handle_GetBeaconCount(PyCallArgs &call)
{
    /**
    ColorStarsByCynosuralFields
    */

    return (m_db.GetDynamicData(10, 0));
}

//02:51:49 L MapService::Handle_GetStationCount(): size= 0
PyResult MapService::Handle_GetStationCount(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetStationCount()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
  /**
  ColorStarsByStationCount
  - not real sure how to do this one.....see MapDB.cpp
  ValueError: need more than 1 value to unpack
  */
    return (m_db.GetStationCount());
}

/** not handled */

PyResult MapService::Handle_GetStuckSystems(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetStuckSystems()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    uint8 none = 0;

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}

//22:49:23 L MapService::Handle_GetRecentSovActivity(): size= 0
PyResult MapService::Handle_GetRecentSovActivity(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetRecentSovActivity()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

//   DED Agent Site Report
PyResult MapService::Handle_GetDeadspaceAgentsMap(PyCallArgs &call)
{/*
23:25:52 [SvcCall]   Call Arguments:
23:25:52 [SvcCall]       Tuple: 1 elements
23:25:52 [SvcCall]         [ 0] String: 'EN'
  sLog.Log( "MapService::Handle_GetDeadspaceAgentsMap()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
*/
    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

//  DED Deadspace Report
//22:37:54 L MapService::Handle_GetDeadspaceComplexMap(): size= 1
PyResult MapService::Handle_GetDeadspaceComplexMap(PyCallArgs &call)
{/*
23:25:52 [SvcCall]   Call Arguments:
23:25:52 [SvcCall]       Tuple: 1 elements
23:25:52 [SvcCall]         [ 0] String: 'EN'

  sLog.Log( "MapService::Handle_GetDeadspaceComplexMap()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
*/
    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

PyResult MapService::Handle_GetIncursionGlobalReport(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetIncursionGlobalReport()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult MapService::Handle_GetSystemsInIncursions(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetSystemsInIncursions()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

//20:38:53 L MapService::Handle_GetSystemsInIncursionsGM(): size= 0
PyResult MapService::Handle_GetSystemsInIncursionsGM(PyCallArgs &call)
{
    call.Dump(SERVICE__CALLS);
    return NULL;
}

PyResult MapService::Handle_GetAllianceSystems(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetAllianceSystems()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

//   factional warfare shit
//https://wiki.eveonline.com/en/wiki/Victory_Points_and_Command_Bunker
PyResult MapService::Handle_GetVictoryPoints(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetVictoryPoints()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

//06:04:50 L MapService::Handle_GetMyExtraMapInfoAgents(): size= 0
PyResult MapService::Handle_GetMyExtraMapInfoAgents(PyCallArgs &call)  //ColorStarsByMyAgents
{
     /**
     ColorStarsByMyAgents  -AttributeError: 'NoneType' object has no attribute 'Index'

     --  this one will have to work in multiple steps....
     1 - get calling char standing for each faction
     2 - query agents from DB
     3 - check char standing against needed standing for agent
     4 - drop agents with standing > char standing
     5 - list and return agents and systemID
     */

    uint8 none = 0;

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}
