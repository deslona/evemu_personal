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

/**   search runs thru these types...even inventory (type 10)
searchResultAgent = 1
searchResultCharacter = 2
searchResultCorporation = 3
searchResultAlliance = 4
searchResultFaction = 5
searchResultConstellation = 6
searchResultSolarSystem = 7
searchResultRegion = 8
searchResultStation = 9
searchResultInventoryType = 10
searchResultAllOwners = [1, 2, 3, 4, 5]
searchResultAllLocations = [6, 7, 8, 9]
searchMaxResults = 500
searchMinWildcardLength = 3
*/

//  starting basic search querys.....

PyDict *SearchDB::Query(std::string string, int32 searchID, uint32 charID) {
   sLog.Warning("SearchDB::Query", "Search : String = %s, ID = %u", string.c_str(), searchID );

    DBQueryResult res;
	std::string id;
	switch(searchID) {
	  case 1:	//searchResultAgent = 1
        sDatabase.RunQuery(res,
			"SELECT"
            "   itemID AS agentID"
            " FROM entity"
            " WHERE itemName LIKE '%s' "
            " AND itemID BETWEEN 2999999 AND 4000000 "
            " LIMIT 0, 10", string.c_str() );
		id = "agentID";
        break;
	  case 2:	//searchResultCharacter = 2
        sDatabase.RunQuery(res,
			"SELECT"
            "   itemID AS ownerID"
            " FROM entity"
            " WHERE itemName LIKE '%s' "
            " AND ownerID = 1", string.c_str() );
		id = "ownerID";
        break;
	  case 3:	//searchResultCorporation = 3
        sDatabase.RunQuery(res,
			"SELECT"
            "   corporationID AS ownerID"
            " FROM corporation"
            " WHERE corporationName LIKE '%s' "
            " LIMIT 0, 10", string.c_str() );
		id = "ownerID";
        break;
	  case 4:	//searchResultAlliance = 4
        sDatabase.RunQuery(res,
			"SELECT"
            " FROM "
            " WHERE  LIKE '%s' "
            " LIMIT 0, 10", string.c_str() );
		id = "ownerID";
        break;
	  case 5:	//searchResultFaction = 5
        sDatabase.RunQuery(res,
			"SELECT"
            " FROM "
            " WHERE  LIKE '%s' "
            " LIMIT 0, 10", string.c_str() );
		id = "ownerID";
        break;
	  case 6:	//searchResultConstellation = 6
        sDatabase.RunQuery(res,
			"SELECT"
            "   constellationID AS itemID"
            " FROM mapConstellations"
            " WHERE constellationName LIKE '%s' "
            " LIMIT 0, 10", string.c_str() );
		id = "itemID";
        break;
	  case 7:	//searchResultSolarSystem = 7
        sDatabase.RunQuery(res,
			"SELECT "
            "   solarSystemID AS itemID"
            " FROM mapSolarSystems "
            " WHERE solarSystemName LIKE '%s' "
            " LIMIT 0, 10", string.c_str() );
		id = "itemID";
        break;
	  case 8:	//searchResultRegion = 8
        sDatabase.RunQuery(res,
			"SELECT "
            "   regionID AS itemID"
            " FROM mapRegions"
            " WHERE regionName LIKE '%s' "
            " LIMIT 0, 10", string.c_str() );
		id = "itemID";
        break;
	  case 9:	//searchResultStation = 9
        sDatabase.RunQuery(res,
			"SELECT "
            "   stationID AS itemID"
            " FROM staStations "
            " WHERE stationName LIKE '%s' "
            " LIMIT 0, 10", string.c_str() );
		id = "itemID";
        break;
	  case 10:	//searchResultInventoryType = 10
	    sDatabase.RunQuery(res,
			"SELECT"
            "   typeID"
            " FROM entity"
            " WHERE itemName LIKE '%s'"
            " AND ownerID = %u", string.c_str(), charID );
		id = "typeID";
        break;
	}
	
	return DBResultToPackedRowDict(res, searchID);
}

//  this needs work....
PyDict *SearchDB::QueryAll(std::string string, uint32 charID) {
    sLog.Warning("SearchDB::QueryAll", "Search String = %s", string.c_str() );
    DBQueryResult res;
    sDatabase.RunQuery(res,
	    "SELECT"
        "   itemID"
        " FROM entity"
        " WHERE itemName LIKE '%s' "
        " AND ownerID = 1"
        " LIMIT 0, 10", string.c_str() );

}

//PyObject *SearchDB::QuickQuery(std::string string, int32 int1, int32 int2, int32 int3, int32 hideNPC, int32 onlyAltName, uint32 charID) {
PyDict *SearchDB::QuickQuery(std::string string, uint32 charID) {
   sLog.Warning("SearchDB::QuickQuery", "Search String = %s", string.c_str() );
    DBQueryResult res;
    DBResultRow row;

    //first we check to see if the string is in the solar system tables
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   solarSystemID"
        " FROM mapSolarSystems"
        " WHERE solarSystemName = '%s' ", string.c_str() ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    if(res.GetRow(row)) return DBResultToPackedRowDict(res, 7);
    else res.Reset();

    //second:  we check entity to see if string is char
    if(!sDatabase.RunQuery(res,
		"SELECT"
        "   itemID"
        " FROM entity"
        " WHERE itemName = '%s' ", string.c_str() ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    if(res.GetRow(row)) return DBResultToPackedRowDict(res, 2);  else { sLog.Error("SearchDB::QuickQuery", "res = NULL : string = %s", string.c_str() ); return NULL; }

}

