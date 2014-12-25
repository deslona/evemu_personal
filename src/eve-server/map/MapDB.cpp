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
/**
        starmap.stationCountCache = sm.RemoteSvc('map').GetStationCount()
    history = starmap.stationCountCache
    maxCount = 0
    for solarSystemID, amount in history:
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
///  NOTE: DB has fields for timing the *Hour and *24Hour parts. need to write checks for that once everything else is working.
///    NOTE:   use averages for *Hour based on current data and serverUpTime.   may be able to do 24Hour same way.
PyRep *MapDB::GetDynamicData(uint32 int1, uint32 int2) {
  /*   object#  0 = type   1 = timeframe
solarSystemID
moduleCnt
structureCnt
pilotsDocked
pilotsInSpace
jumpsHour
killsHour
kills24Hour
factionKills
factionKills24Hour
podKillsHour
podKills24Hour
pilotsDateTime
jumpsDateTime
killsDateTime
kills24DateTime
podDateTime
pod24DateTime
factionDateTime
faction24DateTime
    */
    DBQueryResult res;
    if( (int1 == 1) && (int2 == 1) ) sDatabase.RunQuery(res, "SELECT solarSystemID, jumpsHour AS value1 FROM mapDynamicData" );
    else if (int1 == 2) {
        DBResultRow row;
        sDatabase.RunQuery(res, "SELECT solarSystemID, moduleCnt AS moduleCount, structureCnt AS structureCount FROM mapDynamicData" );
        //res.GetRow(row);
        return DBResultToCRowset(res);
    }
    else if (int1 == 3) {
      if (int2 == 1) sDatabase.RunQuery(res, "SELECT solarSystemID, killsHour AS value1, factionKills AS value2, podKillsHour AS value3 FROM mapDynamicData" );
      else if (int2 == 24) sDatabase.RunQuery(res, "SELECT solarSystemID, kills24Hour AS value1, factionKills24Hour AS value2, podKills24Hour AS value3 FROM mapDynamicData" );
      }
    else if (int1 == 5) sDatabase.RunQuery(res, "SELECT solarSystemID, killsHour AS value1, factionKills AS value2, kills24Hour AS value3, factionKills24Hour AS value4, podKills24Hour AS value5 FROM mapDynamicData" );
    else return NULL;

    return DBResultToRowset(res);
}

