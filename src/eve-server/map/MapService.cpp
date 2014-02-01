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

    /**  not handled yet...these are empty calls  */
    PyCallable_REG_CALL(MapService, GetStuckSystems)
    PyCallable_REG_CALL(MapService, GetRecentSovActivity)
    PyCallable_REG_CALL(MapService, GetDeadspaceAgentsMap)
    PyCallable_REG_CALL(MapService, GetDeadspaceComplexMap)
    PyCallable_REG_CALL(MapService, GetIncursionGlobalReport)
    PyCallable_REG_CALL(MapService, GetSystemsInIncursions)
    PyCallable_REG_CALL(MapService, GetSystemsInIncursionsGM)
    PyCallable_REG_CALL(MapService, GetStationCount)    //ColorStarsByStationCount
    PyCallable_REG_CALL(MapService, GetAllianceSystems)    // wrong place
    PyCallable_REG_CALL(MapService, GetMapLandmarks)
    PyCallable_REG_CALL(MapService, GetMyExtraMapInfoAgents)  //ColorStarsByMyAgents
    PyCallable_REG_CALL(MapService, GetClusterSessionStatistics)    // wrong place
    PyCallable_REG_CALL(MapService, GetHistory)
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

//02:38:07 L MapService::Handle_GetSolarSystemVisits(): size= 0
//AttributeError: 'int' object has no attribute 'lastDateTime'
PyResult MapService::Handle_GetSolarSystemVisits(PyCallArgs &call)
{
  uint32 systemID = 0;
  uint16 visits = 0;
  uint32 charID = call.client->GetCharacterID();
/**
        PyTuple *result = new PyTuple(1);

        result->items[0] = m_db.GetSolSystemVisits(charID);
        */
  //m_db.

    PyTuple* res = NULL;

    return res;
}

//02:38:07 L MapService::Handle_GetBeaconCount(): size= 0
PyResult MapService::Handle_GetBeaconCount(PyCallArgs &call)
{
    /**
    ColorStarsByCynosuralFields  -AttributeError: 'tuple' object has no attribute 'iteritems'
    */
    uint8 none = 0;
    //m_db.GetDynamicData(0, 0)
    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}

/** not handled */
PyResult MapService::Handle_GetStuckSystems(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetStuckSystems()", "size= %u, 0 = %s", call.tuple->size(),
            call.tuple->GetItem(0)->TypeString() );

    uint8 none = 0;

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}

PyResult MapService::Handle_GetRecentSovActivity(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetRecentSovActivity()", "size= %u", call.tuple->size() );

    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

PyResult MapService::Handle_GetDeadspaceAgentsMap(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetDeadspaceAgentsMap()", "size= %u", call.tuple->size() );

    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

PyResult MapService::Handle_GetDeadspaceComplexMap(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetDeadspaceComplexMap()", "size= %u", call.tuple->size() );

    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

PyResult MapService::Handle_GetIncursionGlobalReport(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetIncursionGlobalReport()", "size= %u", call.tuple->size() );

    return NULL;
}

PyResult MapService::Handle_GetSystemsInIncursions(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetSystemsInIncursions()", "size= %u", call.tuple->size() );

    return NULL;
}

//20:38:53 L MapService::Handle_GetSystemsInIncursionsGM(): size= 0
PyResult MapService::Handle_GetSystemsInIncursionsGM(PyCallArgs &call)
{
    return NULL;
}

//02:51:49 L MapService::Handle_GetStationCount(): size= 0
PyResult MapService::Handle_GetStationCount(PyCallArgs &call)
{
  /**
  ColorStarsByStationCount
  */
    return NULL;
}

PyResult MapService::Handle_GetAllianceSystems(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetAllianceSystems()", "size= %u", call.tuple->size() );

    return NULL;
}

PyResult MapService::Handle_GetMapLandmarks(PyCallArgs &call)
{
  sLog.Log( "MapService::Handle_GetMapLandmarks()", "size= %u", call.tuple->size() );

    return NULL;
}

PyResult MapService::Handle_GetMyExtraMapInfoAgents(PyCallArgs &call)  //ColorStarsByMyAgents
{
  sLog.Log( "MapService::Handle_GetMyExtraMapInfoAgents()", "size= %u", call.tuple->size() );
     /**
     ColorStarsByMyAgents  -AttributeError: 'NoneType' object has no attribute 'Index'
     */

    uint8 none = 0;

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}

PyResult MapService::Handle_GetClusterSessionStatistics(PyCallArgs &call)   //ColorStarsByNumPilots
{
  sLog.Log( "MapService::Handle_GetMyExtraMapInfoAgents()", "size= %u", call.tuple->size() );
    /**
    ColorStarsByNumPilots
    */
    //m_db.GetDynamicData(0, 0)
    uint8 none = 0;

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}

//02:52:11 L CorpMgrService::Handle_GetAssetInventory(): size= 2, 0 = Integer (3), 1 = Integer (24)    -ColorStarsByCynosuralFields - ColorStarsByFactionKills - ColorStarsByKills - GetKillLast24H
//02:52:13 L CorpMgrService::Handle_GetAssetInventory(): size= 2, 0 = Integer (1), 1 = Integer (1)     -ColorStarsByJumps1Hour
//02:52:14 L CorpMgrService::Handle_GetAssetInventory(): size= 2, 0 = Integer (3), 1 = Integer (1)     -ColorStarsByPodKills
//13:37:09 L MapService::Handle_GetHistory(): size= 2, 0 = Interger (3), 1 = Interger (24)

PyResult MapService::Handle_GetHistory(PyCallArgs &call)
{
    /**
    ColorStarsByJumps1Hour  -AttributeError: 'int' object has no attribute 'value1'
    ColorStarsByPodKills  -AttributeError: 'int' object has no attribute 'value3'
    ColorStarsByFactionKills  -AttributeError: 'int' object has no attribute 'value2'
    ColorStarsByKills  -AttributeError: 'int' object has no attribute 'value1'
    GetKillLast24H  -AttributeError: 'int' object has no attribute 'solarSystemID'
    */
    uint32 int1 = call.tuple->GetItem(0)->AsInt()->value();
    uint32 int2 = call.tuple->GetItem(1)->AsInt()->value();
      sLog.Log( "MapService::Handle_GetHistory()", "size= %u, 0 = Interger (%u), 1 = Interger (%u)", call.tuple->size(), int1, int2 );
    //m_db.GetDynamicData(int1, int2)
    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( int1 );

    res = tuple0;

    return res;
}
