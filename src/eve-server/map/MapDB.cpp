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

#include "map/MapDB.h"

PyObject *MapDB::GetPseudoSecurities() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res, "SELECT solarSystemID, security FROM mapSolarSystems")) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationExtraInfo() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   stationID,"
        "   solarSystemID,"
        "   operationID,"
        "   stationTypeID,"
        "   corporationID AS ownerID"
        " FROM staStations" )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationOpServices() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT operationID, serviceID FROM staOperationServices")) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationServiceInfo() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT serviceID, serviceName FROM staServices ")) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationCount() {
    //DBQueryResult res;
/**     stations are listed as unique StationID's.  will have to get, combine, then count for each system....*sigh*
    if(!sDatabase.RunQuery(res, "SELECT solarSystemID, COUNT(stationID) FROM staStations" )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
    */

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        " SELECT "
        "    COUNT(stationID) "
        " FROM staStations "
        " WHERE solarSystemID "
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetSolSystemVisits(uint32 charID)
{
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        " SELECT"
        "   solarSystemID,"
        "   visits,"
        "   lastDateTime"
        " FROM chrVisitedSystems"
        " WHERE characterID = %u", charID ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);  //DBRowToKeyVal
}

//  called from MapService by multiple functions based on passed values.
///  added jumpsHour and numPilots data inserts.  16Mar14
///  added killsHour, factionKills, podKillsHour  24Mar14
///  NOTE: DB has fields for timing the *Hour and *24Hour parts. need to write checks for that when everything else starts working.
PyObject *MapDB::GetDynamicData(uint32 int1, uint32 int2) {
  /*   object#  0 = # returns expected    1 = type of info requested
//02:52:13 L MapService::Handle_GetHistory(): size= 2, 0 = Integer (1), 1 = Integer (1)     -ColorStarsByJumps1Hour
    ColorStarsByJumps1Hour  -AttributeError: 'int' object has no attribute 'value1'

//02:52:14 L MapService::Handle_GetHistory(): size= 2, 0 = Integer (3), 1 = Integer (1)     -ColorStarsByKills
//02:52:11 L MapService::Handle_GetHistory(): size= 2, 0 = Integer (3), 1 = Integer (24)
//  -ColorStarsByFactionKills - ColorStarsByKills - GetKillLast24H
    GetKillsLast24H  -AttributeError: 'int' object has no attribute 'solarSystemID'
    ColorStarsByKills  -AttributeError: 'int' object has no attribute 'value1'
    ColorStarsByFactionKills  -AttributeError: 'int' object has no attribute 'value2'
//21:20:25 L MapService::Handle_GetHistory(): size= 2, 0 = Interger (5), 1 = Interger (24)  -ColorStarsByKills (factionKills?)
22:30:32 L MapService::Handle_GetHistory(): size= 2, 0 = Interger (5), 1 = Interger (1)

solarSystemID
beaconCount
cynoFields
jumpsHour
killsHour
kills24Hours
pilotsDocked
pilotsInSpace
podKillsHour
podKills24Hour
factionKills
    */
    DBQueryResult res;
    if( (int1 == 1) && (int2 == 1) ) {
      sDatabase.RunQuery(res, "SELECT solarSystemID, jumpsHour AS value1 FROM mapDynamicData" );
    } else if (int1 == 3) {
      if (int2 == 1) {
        sDatabase.RunQuery(res, "SELECT solarSystemID, KillsHour AS value1, factionKills AS value2, podKillsHour AS value3 FROM mapDynamicData" );
      } else if (int2 == 24) {
          sDatabase.RunQuery(res, "SELECT solarSystemID, kills24Hours AS value1, factionKills AS value2, kills24Hours AS value3 FROM mapDynamicData" );
      }
    } else if (int1 == 5) {
          sDatabase.RunQuery(res, "SELECT solarSystemID, killsHour AS value1, factionKills AS value2, kills24Hours AS value3, podKillsHour AS value4, podKills24Hour AS value5 FROM mapDynamicData" );
    } else if (int1 == 10) {
          sDatabase.RunQuery(res, "SELECT solarSystemID, beaconCount FROM mapDynamicData WHERE beaconCount != 0" );
    } else {
       return NULL;
    }

    return DBResultToRowset(res);
}
