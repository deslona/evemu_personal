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
    Author:     Zhur, Allan
*/

#include "eve-server.h"

#include "config/ConfigDB.h"

PyRep *ConfigDB::GetMultiOwnersEx(const std::vector<int32> &entityIDs) {
//#   pragma message( "we need to deal with corporations!" )
    //we only get called for items which are not already sent in the
    // eveStaticOwners cachable object.
    /*
23:14:21 L ConfigService: Handle_GetMultiOwnersEx
23:14:21 [SvcCall]   Call Arguments:
23:14:21 [SvcCall]       Tuple: 1 elements
23:14:21 [SvcCall]         [ 0] List: 1 elements
23:14:21 [SvcCall]         [ 0]   [ 0] Integer field: 140000053
*/
    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;
    DBResultRow row;

    //first we check to see if there is such ids in the entity tables
    if(!sDatabase.RunQuery(res,
        "SELECT "
        " itemID as ownerID,"
        " itemName as ownerName,"
        " typeID,"
        " NULL as ownerNameID,"
        " 1 as gender"
        " FROM entity "
        " WHERE itemID in (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    //second: we check to see if the id points to a static entity (Agents, NPC Corps, etc.)
    if(!res.GetRow(row)) {
        if(!sDatabase.RunQuery(res,
            "SELECT "
            " ownerID,ownerName,typeID,"
            " NULL as ownerNameID,"
            " 1 as gender"
            " FROM eveStaticOwners "
            " WHERE ownerID in (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return new PyInt(0);
        }
    } else {
        res.Reset();
    }

    //third: we check to see if the id points to a player's character
    if(!res.GetRow(row)) {
        if(!sDatabase.RunQuery(res,
            "SELECT "
            " characterID as ownerID,"
            " itemName as ownerName,"
            " typeID,"
            " characterID as ownerNameID,"
            " gender"
            " FROM character_ "
            " LEFT JOIN entity ON characterID = itemID"
            " WHERE characterID in (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return NULL;
        }
    } else {
        res.Reset();
    }

    return(DBResultToTupleSet(res));
}

PyRep *ConfigDB::GetMultiAllianceShortNamesEx(const std::vector<int32> &entityIDs) {
    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   itemID as allianceID,"
        "   itemName as shortName" //we likely need to use customInfo or something for this.
        " FROM entity "
        " WHERE typeID = %d"
        "  AND itemID in (%s)",
        AllianceTypeID,
        ids.c_str()
        ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return(DBResultToTupleSet(res));
}


PyRep *ConfigDB::GetMultiLocationsEx(const std::vector<int32> &entityIDs) {     // now working correctly  -allan  5May
    bool use_map = false;
    use_map = IsStaticMapItem(entityIDs[0]);

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(use_map) {
        if(!sDatabase.RunQuery(res,
            "SELECT "
            " itemID AS locationID,"
            " itemName AS locationName,"
            " x, y, z,"
            " regionID AS locationNameID"
            " FROM mapDenormalize "
            " WHERE itemID in (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return new PyNone;
        }
    } else {
        if(!sDatabase.RunQuery(res,
            "SELECT "
            " itemID AS locationID,"
            " itemName AS locationName,"
            " x, y, z,"
            " locationID AS locationNameID"
            " FROM entity "
            " WHERE itemID in (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return new PyNone;
        }
    }

    return(DBResultToTupleSet(res));
}

PyRep *ConfigDB::GetMultiCorpTickerNamesEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationID, tickerName, "
        "   shape1, shape2, shape3,"
        "   color1, color2, color3 "
        " FROM corporation "
        " WHERE corporationID in (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return(DBResultToRowList(res));
}


PyRep *ConfigDB::GetMultiGraphicsEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   graphicID, url3D, urlWeb, icon, urlSound, explosionID"
        " FROM eveGraphics "
        " WHERE graphicID in (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return(DBResultToRowList(res));
}

PyObject *ConfigDB::GetUnits() {

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        " unitID, unitName, displayName"
        " FROM eveUnits "))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToIndexRowset(res, "unitID"));
}

PyObjectEx *ConfigDB::GetMapObjects(uint32 entityID, bool wantRegions,
    bool wantConstellations, bool wantSystems, bool wantStations)
{
    const char *key = "solarSystemID";
    if(wantRegions) {
        entityID = 3;   /* a little hackish... */
        key = "typeID";
    } else if(wantConstellations) {
        key = "regionID";
    } else if(wantSystems) {
        key = "constellationID";
    } else if(wantStations) {
        key = "solarSystemID";
    }

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   groupID, typeID, itemID, itemName, solarSystemID AS locationID, IFNULL(orbitID, 0) AS orbitID,"
        "   x, y, z"
        " FROM mapDenormalize"
        " WHERE %s=%u", key, entityID
    )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
//    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMap(uint32 solarSystemID) {
    DBQueryResult res;

    //how in the world do they get a list in the freakin rowset for destinations???
    //   .....  like this..    -allan
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   s.solarSystemID AS locationID,"
        "   s.xMin, s.xMax, s.yMin,"
        "   s.yMax, s.zMin, s.zMax,"
        "   s.luminosity,"
        "   d.x, d.y, d.z,"
        "   d.itemID,"
        "   d.itemName,"
        "   d.typeID,"
        "   d.groupID,"
        "   d.orbitID,"
        "   j.celestialID AS destinations"
        " FROM mapSolarSystems AS s"
        "  LEFT JOIN mapDenormalize AS d USING (solarSystemID)"
        "  LEFT JOIN mapJumps AS j ON j.stargateID = d.itemID"
        " WHERE solarSystemID=%u", solarSystemID
    )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *ConfigDB::ListLanguages() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   languageID,languageName,translatedLanguageName"
        " FROM languages"
    )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}


PyRep *ConfigDB::GetMultiInvTypesEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   typeID,groupID,typeName,description,graphicID,radius,"
        "   mass,volume,capacity,portionSize,raceID,basePrice,"
        "   published,marketGroupID,chanceOfDuplicating "
        " FROM invTypes "
        " WHERE typeID in (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return(DBResultToRowList(res));
}

PyRep *ConfigDB::GetStationSolarSystemsByOwner(uint32 ownerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " stationID, solarSystemID "
        " FROM staStations "
        " WHERE corporationID = %u ", ownerID
        ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToRowset(res);
}

PyRep *ConfigDB::GetCelestialStatistic(uint32 celestialID) {
    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   groupID"
        " FROM eveNames"
        " WHERE itemID = %u", celestialID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Unable to find celestial object %u", celestialID);
        return new PyInt(0);
    }
    uint32 groupID = row.GetUInt(0);

    std::string query = "";

    switch (groupID) {
    case EVEDB::invGroups::Sun:
            query = "SELECT"
                    "   temperature,"
                    "   spectralClass,"
                    "   luminosity,"
                    "   age,"
                    "   radius"
                    " FROM mapCelestialStatistics"
                    " WHERE celestialID = %u";
            break;
    case EVEDB::invGroups::Planet:
            query = "SELECT"
                    "   temperature,"
                    "   orbitRadius,"
                    "   eccentricity,"
                    "   massDust,"
                    "   density,"
                    "   surfaceGravity,"
                    "   escapeVelocity,"
                    "   orbitPeriod,"
                    "   pressure,"
                    "   radius"
                    " FROM mapCelestialStatistics"
                    " WHERE celestialID = %u";
            break;
    case EVEDB::invGroups::Moon:
            query = "SELECT"
                    "   temperature,"
                    "   orbitRadius,"
                    "   massDust,"
                    "   density,"
                    "   surfaceGravity,"
                    "   escapeVelocity,"
                    "   orbitPeriod,"
                    "   pressure,"
                    "   radius"
                    " FROM mapCelestialStatistics"
                    " WHERE celestialID = %u";
            break;
    case EVEDB::invGroups::Asteroid_Belt:
            query = "SELECT"
                    "   orbitRadius,"
                    "   eccentricity,"
                    "   massDust,"
                    "   density,"
                    "   orbitPeriod"
                    " FROM mapCelestialStatistics"
                    " WHERE celestialID = %u";
            break;

    default:
            codelog(DATABASE__ERROR, "Invalid object groupID (%u) for %u", groupID, celestialID);
            return new PyInt(0);
    }

    if (!sDatabase.RunQuery(res, query.c_str(), celestialID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToCRowset(res);
}

PyRep *ConfigDB::GetDynamicCelestials(uint32 solarSystemID) {
  /**  this return broke the rclick in space menu.  disable for now  9July14*/
  // updated to show only POS.  -allan 9Jul14
  //  need to remove all items except stations

    DBQueryResult result;
/**
  /*<DBRow object [7, 2016, 40176433, u'Halaima II', 30002781, 40176430, False, 68268179028.0, 8474470920.0, 20992190960.0]>,
        "SELECT "
        "   groupID, typeID, itemID, itemName, solarSystemID AS locationID, IFNULL(orbitID, 0) AS orbitID,"
        **THEN SOMETHING ELSE (BOOLEAN)**
        "   x, y, z"
        " FROM entity"  // this is to push ONLY player owned stations (from what i can tell in client code)
        */
    if (!sDatabase.RunQuery(result,
        "SELECT"
        "   g.groupID,"
        "   t.typeID,"
        "   e.itemID,"
        "   e.itemName,"
        "   e.locationID,"
        "   0 AS orbitID,"   // field doesnt exist on those tables....not sure if this is needed or not.  hack for now.
        "   False,"     //i still dont know what this is.....
        "   e.x, e.y, e.z"
        //"   CAST(e.z AS UNSIGNED INTEGER) AS z"
        " FROM entity AS e"
        "  LEFT JOIN invTypes AS t ON t.typeID = e.typeID"
        "  LEFT JOIN invGroups AS g ON g.groupID = t.groupID"
        " WHERE e.locationID = %u"
        " AND g.categoryID = %d"
        " AND e.itemID > 140000000",
        solarSystemID, EVEDB::invCategories::Station )) {
            codelog(DATABASE__ERROR, "GetDynamicCelestials Error in query: %s", result.error.c_str());
            return new PyInt(0);
    }

    return DBResultToRowset(result);
}

PyRep *ConfigDB::GetTextsForGroup(const std::string & langID, uint32 textgroup) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT textLabel, `text` FROM intro WHERE langID = '%s' AND textgroup = %u", langID.c_str(), textgroup))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMapOffices(uint32 solarSystemID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "  corporationID,"
        "  description,"
        "  iconID"
        " FROM crpNPCCorporations"
        " WHERE solarSystemID=%u", solarSystemID
    ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
//  AttributeError: stationID     need to find this in db.....????
    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMapConnections(uint32 id, bool sol, bool reg, bool con, uint16 cel, uint16 _c) {
  sLog.Warning ("ConfigDB::GetMapConnections", "DB query:%u, B1:%u, B2:%u, B3:%u, I2:%u, I3:%u", id, sol, reg, con, cel, _c);

    const char *key = "fromsol";
    if(sol) key = "fromsol";
    else if(reg) key = "fromreg";
    else if(con) key = "fromcon";
    else sLog.Error ("ConfigDB::GetMapConnections()", "Bad argument passed to key.");

    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT ctype, fromreg, fromcon, fromsol, stargateID, celestialID, tosol, tocon, toreg"
        " FROM mapConnections"
        " WHERE %s = %u", key, id )) {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            sLog.Error ("ConfigDB::GetMapConnections()", "No Data for key: %s, id: %u.", key, id);
            return NULL;
    }
    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMapLandmarks() {    // working 29June14
    DBQueryResult res;

      if(!sDatabase.RunQuery(res,
          "SELECT"
          "   landmarkID,"
          "   landmarkName,"
          "   0 AS landmarkNameID,"
          "   x, y, z,"
          "   radius,"
          " FROM mapLandmarks" ))
      {
          codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
          return NULL;
      }
    return DBResultToRowset(res);
}

