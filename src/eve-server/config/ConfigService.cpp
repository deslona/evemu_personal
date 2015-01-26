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
#include "config/ConfigService.h"

PyCallable_Make_InnerDispatcher(ConfigService)

ConfigService::ConfigService(PyServiceMgr *mgr)
: PyService(mgr, "config"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ConfigService, GetMultiOwnersEx);
    PyCallable_REG_CALL(ConfigService, GetMultiLocationsEx);
    PyCallable_REG_CALL(ConfigService, GetMultiAllianceShortNamesEx);
    PyCallable_REG_CALL(ConfigService, GetMultiCorpTickerNamesEx);
    PyCallable_REG_CALL(ConfigService, GetUnits);
    PyCallable_REG_CALL(ConfigService, GetMap);
    PyCallable_REG_CALL(ConfigService, GetMapOffices);
    PyCallable_REG_CALL(ConfigService, GetMapObjects);
    PyCallable_REG_CALL(ConfigService, GetMapConnections);
    PyCallable_REG_CALL(ConfigService, GetMultiGraphicsEx);
    PyCallable_REG_CALL(ConfigService, GetMultiInvTypesEx);
    PyCallable_REG_CALL(ConfigService, GetStationSolarSystemsByOwner);
    PyCallable_REG_CALL(ConfigService, GetCelestialStatistic);
    PyCallable_REG_CALL(ConfigService, GetDynamicCelestials);
    PyCallable_REG_CALL(ConfigService, GetMapLandmarks);
    PyCallable_REG_CALL(ConfigService, SetMapLandmarks);
}

ConfigService::~ConfigService() {
    delete m_dispatch;
}

PyResult ConfigService::Handle_GetMultiOwnersEx(PyCallArgs &call) {
  /*
23:14:21 L ConfigService: Handle_GetMultiOwnersEx
23:14:21 [SvcCall]   Call Arguments:
23:14:21 [SvcCall]       Tuple: 1 elements
23:14:21 [SvcCall]         [ 0] List: 1 elements
23:14:21 [SvcCall]         [ 0]   [ 0] Integer field: 140000053
  sLog.Log( "ConfigService", "Handle_GetMultiOwnersEx" );
  call.Dump(SERVICE__CALLS);
  */

    Call_SingleIntList arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments.");
        return NULL;
    }

    return(m_db.GetMultiOwnersEx(arg.ints));
}

PyResult ConfigService::Handle_GetMultiAllianceShortNamesEx(PyCallArgs &call) {
    Call_SingleIntList arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments.");
        return NULL;
    }

    return(m_db.GetMultiAllianceShortNamesEx(arg.ints));
}


PyResult ConfigService::Handle_GetMultiLocationsEx(PyCallArgs &call) {      // now working correctly  -allan  25April
    // the PyRep is passed from client as a pylist....
/**
23:23:57 L ConfigService: Handle_GetMultiLocationsEx
23:23:57 [SvcCall]   Call Arguments:
23:23:57 [SvcCall]       Tuple: 1 elements
23:23:57 [SvcCall]         [ 0] List: 2 elements
23:23:57 [SvcCall]         [ 0]   [ 0] Integer field: 140000725
23:23:57 [SvcCall]         [ 0]   [ 1] Integer field: 140001260

  sLog.Log( "ConfigService", "Handle_GetMultiLocationsEx" );
  call.Dump(SERVICE__CALLS);
*/
    Call_SingleIntList arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments.");
        return NULL;
    }

    return(m_db.GetMultiLocationsEx( arg.ints ));
}

PyResult ConfigService::Handle_GetMultiCorpTickerNamesEx(PyCallArgs &call) {
    //parse the PyRep to get the list of IDs to query.
    Call_SingleIntList arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments.");
        return NULL;
    }

    return(m_db.GetMultiCorpTickerNamesEx(arg.ints));
}

PyResult ConfigService::Handle_GetMultiGraphicsEx(PyCallArgs &call) {
    //parse the PyRep to get the list of IDs to query.
    Call_SingleIntList arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments.");
        return NULL;
    }

    return(m_db.GetMultiGraphicsEx(arg.ints));
}

PyResult ConfigService::Handle_GetUnits(PyCallArgs &call) {
    return(m_db.GetUnits());
}

PyResult ConfigService::Handle_GetMap(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return NULL;
    }

    return(m_db.GetMap(args.arg));
}

PyResult ConfigService::Handle_GetMapOffices(PyCallArgs &call) {
  /*
22:38:58 [SvcCall] Service config: calling GetMapOffices
22:38:58 [SvcCall]   Call Arguments:
22:38:58 [SvcCall]       Tuple: 1 elements
22:38:58 [SvcCall]         [ 0] Integer field: 30002507     -solarSystemID
22:38:58 [SvcCall]   Call Named Arguments:
22:38:58 [SvcCall]     Argument 'machoVersion':
22:38:58 [SvcCall]         Integer field: 1
  call.Dump(SERVICE__CALLS);
  */
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return NULL;
    }

    return(m_db.GetMapOffices(args.arg));
}

PyResult ConfigService::Handle_GetMapObjects(PyCallArgs &call) {
  Call_GetMapObjects args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return NULL;
    }

    return m_db.GetMapObjects( args.systemID, args.reg, args.con, args.sys, args.sta);
}

PyResult ConfigService::Handle_GetMultiInvTypesEx(PyCallArgs &call) {
    sLog.Log( "ConfigService", "Handle_GetMultiInvTypesEx" );
    call.Dump(SERVICE__CALLS);

    //parse the PyRep to get the list of IDs to query.
    Call_SingleIntList arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments.");
        return NULL;
    }

    return(m_db.GetMultiInvTypesEx(arg.ints));
}


//02:10:35 L ConfigService::Handle_GetMapConnections(): size= 6
PyResult ConfigService::Handle_GetMapConnections(PyCallArgs &call) {
/**
        this is cached on clientside.  only called if not in client cache
*/
    Call_GetMapConnections args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return new PyInt(0);
    }

    if(args.id == 9 || args.sol) {
        sLog.Warning( "ConfigService::Handle_GetMapConnections()", "args.id = 9 | args.sol");
        return m_db.GetMapConnections(call.client->GetSystemID(), args.sol, args.reg, args.con, args.cel, args._c);
    } else
        return m_db.GetMapConnections(args.id, args.sol, args.reg, args.con, args.cel, args._c);
}

PyResult ConfigService::Handle_GetStationSolarSystemsByOwner(PyCallArgs &call) {
  /*
18:22:36 L ConfigService::Handle_GetStationSolarSystemsByOwner(): size= 1
18:22:36 [SvcCall]   Call Arguments:
18:22:36 [SvcCall]       Tuple: 1 elements
18:22:36 [SvcCall]         [ 0] Integer field: 1000084
18:22:36 [SvcCallTrace] Call GetStationSolarSystemsByOwner returned:
18:22:36 [SvcCallTrace]       Object:
18:22:36 [SvcCallTrace]         Type: String: 'util.Rowset'
18:22:36 [SvcCallTrace]         Args: Dictionary: 3 entries
18:22:36 [SvcCallTrace]         Args:   [ 0] Key: String: 'lines'
18:22:36 [SvcCallTrace]         Args:   [ 0] Value: List: 42 elements
18:22:36 [SvcCallTrace]         Args:   [ 0] Value:   [ 0] List: 2 elements
18:22:36 [SvcCallTrace]         Args:   [ 0] Value:   [ 0]   [ 0] Integer field: 1000084
18:22:36 [SvcCallTrace]         Args:   [ 0] Value:   [ 0]   [ 1] Integer field: 30002279
18:22:36 [SvcCallTrace]         Args:   [ 1] Key: String: 'RowClass'
18:22:36 [SvcCallTrace]         Args:   [ 1] Value: Token: 'util.Row'
18:22:36 [SvcCallTrace]         Args:   [ 2] Key: String: 'header'
18:22:36 [SvcCallTrace]         Args:   [ 2] Value: List: 2 elements
18:22:36 [SvcCallTrace]         Args:   [ 2] Value:   [ 0] String: 'corporationID'
18:22:36 [SvcCallTrace]         Args:   [ 2] Value:   [ 1] String: 'solarSystemID'
*/
  sLog.Log( "ConfigService::Handle_GetStationSolarSystemsByOwner()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Bad arguments");
        return NULL;
    }

    // No idea what to return... yet...
    // Similar to GetCorpInfo(corpID) / corpSvc
    return m_db.GetStationSolarSystemsByOwner(arg.arg);
}

PyResult ConfigService::Handle_GetCelestialStatistic(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Bad arguments");
        return NULL;
    }

    return m_db.GetCelestialStatistic(arg.arg);
}

PyResult ConfigService::Handle_GetDynamicCelestials(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Bad arguments");
        return NULL;
    }

    if(IsSolarSystem(arg.arg)) {
        //sLog.Success("GetDynamicCelesitals", " IsSolarSystem %u", arg.arg);
        return m_db.GetDynamicCelestials(arg.arg);
    } else {
        sLog.Error("GetDynamicCelesitals", "  !IsSolarSystem %u", arg.arg);
        return new PyInt( 0 );
    }
}

PyResult ConfigService::Handle_GetMapLandmarks(PyCallArgs &call) {
    return m_db.GetMapLandmarks();
}

PyResult ConfigService::Handle_SetMapLandmarks(PyCallArgs &call) {
  /**
            x, y, z = landmark.translation
            data = (landmark.landmarkID,
             x / STARMAP_SCALE,
             y / STARMAP_SCALE,
             z / STARMAP_SCALE,
             landmark.GetRadius())

        sm.RemoteSvc('config').SetMapLandmarks(landmarkData)
             */
  sLog.Log( "MapService::Handle_SetMapLandmarks()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

        return NULL;
}

