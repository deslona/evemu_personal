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
    Author:      Allan
*/

#include "eve-server.h"

#include "system/Search.h"
#include "system/SearchDB.h"

SearchDB::SearchDB() { }

//  starting basic search querys.....

PyObject *SearchDB::Query(std::string string, int32 searchID, uint32 charID) {
   sLog.Warning("SearchDB::Query", "Search : String = %s, ID = %u", string.c_str(), searchID );
    std::string query = "";

    switch (searchID) {
      case 1:           //  agent
      case 2:           //  char
            query = "SELECT"
                    "   itemID,"
                    "   itemName,"
                    "   locationID"
                    " FROM entity"
                    " WHERE itemName LIKE '%s' ";
            break;
      case 3:           //  corp            AttributeError: Rowset instance has no attribute 'get'
            query = "SELECT"
                    "   corporationID,"
                    "   corporationName,"
                    "   stationID,"
                    "   graphicID"
                    " FROM corporation"
                    " WHERE corporationName LIKE '%s' ";
            break;
      case 4:           //  alliance
            query = "SELECT"
                    " FROM "
                    " WHERE  LIKE '%s' ";
            break;
      case 5:           //  faction
            query = "SELECT"
                    " FROM "
                    " WHERE  LIKE '%s' ";
            break;
      case 6:           //  constellation
            query = "SELECT"
                    "   regionID,"
                    "   constellationID,"
                    "   constellationName"
                    " FROM mapConstellations"
                    " WHERE constellationName LIKE '%s' ";
            break;
      case 7:           //  solar system
            query = "SELECT "
                    "   regionID,"
                    "   constellationID,"
                    "   solarSystemID,"
                    "   solarSystemName"
                    " FROM mapSolarSystems "
                    " WHERE solarSystemName LIKE '%s' ";
            break;
      case 8:           //  region
            query = "SELECT "
                    "   regionID,"
                    "   regionName"
                    " FROM mapRegions"
                    " WHERE regionName LIKE '%s' ";
            break;
      case 9:           //  station
            query = "SELECT "
                    "   regionID,"
                    "   constellationID,"
                    "   solarSystemID,"
                    "   stationID,"
                    "   stationName"
                    " FROM staStations "
                    " WHERE stationName LIKE '%s' ";
            break;
      default:
            codelog(SERVICE__ERROR, "Invalid query '%s' on search %u for %u", query.c_str(), searchID, charID);
            return NULL;
    }

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, query.c_str(), string.c_str() )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    DBResultRow row;
    if(res.GetRow(row)) return(DBResultToRowset(res)); else { sLog.Error("SearchDB::Query", "res = NULL : query = %s : string = %s", query.c_str(), string.c_str() ); return NULL; }

    //return DBResultToRowset(res);
}


//  this needs work....use nested querys like ConfigDB::GetMultiOwnersEx to search till matched or end.
PyObject *SearchDB::QueryAll(std::string string, uint32 charID) {
   sLog.Warning("SearchDB::QueryAll", "Search String = %s", string.c_str() );
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT regionID, constellationID, solarSystemID, solarSystemName FROM mapSolarSystems WHERE solarSystemName LIKE '%s'", string.c_str() )) {
            sLog.Error("SearchDB::QueryAll","%u: Query Failed: %s", charID, res.error.c_str() );
            return NULL;
    }

    DBResultRow row;
    if(res.GetRow(row)) return(DBResultToRowset(res)); else { sLog.Error("SearchDB::QueryAll","res = NULL : string = %s", string.c_str() ); return NULL; }
    //return DBResultToRowset(res);
}

//PyObject *SearchDB::QuickQuery(std::string string, int32 int1, int32 int2, int32 int3, int32 hideNPC, int32 onlyAltName, uint32 charID) {
PyObject *SearchDB::QuickQuery(std::string string, uint32 charID) {
   sLog.Warning("SearchDB::QuickQuery", "Search String = %s", string.c_str() );
    DBQueryResult res;
    DBResultRow row;

    //first we check to see if the string is in the solar system tables
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   regionID,"
        "   constellationID,"
        "   solarSystemID,"
        "   solarSystemName"
        " FROM mapSolarSystems"
        " WHERE solarSystemName LIKE '%s' ", string.c_str() ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    if(res.GetRow(row)) return(DBResultToRowset(res));
    else res.Reset();

    //second: we check to see if the string is a sun, moon, or planet
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   regionID,"
        "   constellationID,"
        "   solarSystemID,"
        "   itemID,"
        "   itemName"
        " FROM mapDenormalize"
        " WHERE itemName LIKE '%s' ", string.c_str() ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    if(res.GetRow(row)) return(DBResultToRowset(res)); else { sLog.Error("SearchDB::QuickQuery", "res = NULL : string = %s",  string.c_str() ); return NULL; }
}