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

#include "system/SystemDB.h"

bool SystemDB::LoadSystemEntities(uint32 systemID, std::vector<DBSystemEntity> &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   itemID,typeID,groupID,orbitID,"
        "   x,y,z,radius,security,itemName"
        " FROM mapDenormalize"
        " WHERE solarSystemID=%u", systemID )) {
            codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
            sLog.Warning("SystemDB::LoadSystemEntities", "  Loading system entities for system %u failed", systemID);
            return false;
        }

    DBResultRow row;
    DBSystemEntity entry;
    while(res.GetRow(row)) {
        entry.itemID = row.GetInt(0);
        entry.typeID = row.GetInt(1);
        entry.groupID = row.GetInt(2);
        entry.orbitID = (row.IsNull(3) ? 0 : row.GetInt(3));
        entry.position.x = row.GetInt(4);
        entry.position.y = row.GetInt(5);
        entry.position.z = row.GetInt(6);
        entry.radius = (row.IsNull(7) ? 1 : row.GetDouble(7));
        entry.security = (row.IsNull(8) ? 0.0 : row.GetDouble(8));
        entry.itemName = row.GetText(9);
        into.push_back(entry);
    }

    sLog.Success("SystemDB::LoadSystemEntities", "  System entities for system %u loaded", systemID);
    return true;
}

bool SystemDB::LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity> &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   e.itemID,"
        "   e.itemName,"
        "   e.typeID,"
        "   e.ownerID,"
        "   e.locationID,"
        "   e.flag,"
        "   t.groupID,"
        "   g.categoryID,"
        "   IFNULL(c.corporationID, e.ownerID),"
        "   IFNULL(co.allianceID, 0),"
        "   e.x, e.y, e.z"
        " FROM entity AS e"
        "  LEFT JOIN invTypes AS t ON t.typeID = e.typeID"
        "  LEFT JOIN invGroups AS g ON g.groupID = t.groupID"
        "  LEFT JOIN character_ AS c ON e.ownerID = c.characterID"
        "  LEFT JOIN corporation AS co ON c.corporationID = co.corporationID"
        " WHERE locationID = %u"
        "  AND g.categoryID NOT IN (%d, %d)",
        systemID,
        //excluded categories:
        //celestials:            0                             3
        EVEDB::invCategories::_System, EVEDB::invCategories::Station
        //NPCs:                   11
        //EVEDB::invCategories::Entity,
        )) {
            codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
            return false;
        }

    DBResultRow row;
    DBSystemDynamicEntity entry;
    while(res.GetRow(row)) {
        entry.itemID = row.GetInt(0);
        entry.itemName = row.GetText(1);
        entry.typeID = row.GetInt(2);
        entry.ownerID = row.GetInt(3);
        entry.locationID = row.GetInt(4);
        entry.flag = row.GetInt(5);
        entry.groupID = row.GetInt(6);
        entry.categoryID = row.GetInt(7);
        entry.corporationID = row.GetInt(8);
        entry.allianceID = row.GetInt(9);
        entry.x = row.GetDouble(10);
        entry.y = row.GetDouble(11);
        entry.z = row.GetDouble(12);
        into.push_back(entry);
    }

    return true;
}

uint32 SystemDB::GetObjectLocationID( uint32 itemID ) {

    //TODO: test database logic and query       -allan 22Jan14
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT locationID FROM entity WHERE itemID=%u", itemID )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row)) return (row.GetUInt(0)); else return 0;
}

bool SystemDB::GetWrecksToTypes(DBQueryResult &res) {
    if(!sDatabase.RunQuery(res, "SELECT * FROM invTypesToWrecks")) {
       sLog.Error("SystemDB::GetWrecksToTypes()", "Error in query: %s", res.error.c_str());
        return false;
    }
    return true;
}

PyObject *SystemDB::ListFactions() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res, "SELECT factionID FROM chrFactions")) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *SystemDB::ListJumps(uint32 stargateID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   celestialID AS toCelestialID,"
        "   solarSystemID AS locationID"
        " FROM mapJumps "
        "  LEFT JOIN mapDenormalize ON celestialID=itemID"
        " WHERE stargateID=%u", stargateID)) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}
