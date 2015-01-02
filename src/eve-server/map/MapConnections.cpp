/*
 *    File:  MapConnections.cpp
 *  Author:  Allan
 * Purpose:  This file was created to hold functions specific to the mapConnections DB table.
 *      The table is used to supply map connection data to the client for drawing maps, and is not supplied by CCP.
 *      The functions in this file will gather approprate data from the DB, and populate mapConnections with the data needed.
 *
 *      The mapDenormalize table does not contain radius information for stargates.
 *      I have added a function to populate the radius information to correct that.
 *
 * Created on June 29, 2014, 12:40
 * Updated on December 27, 2014, 12:01
 */

#include "eve-server.h"
#include "MapConnections.h"


MapCon::MapCon()
{
}

void MapCon::PopulateConnections() {
    sLog.Warning("PopulateConnections()", "Populating mapConnections.");
    uint16 loops = 14335;   //14334
    uint16 count = 1;
    uint32 fromreg, fromcon, fromsol, stargateID, celestialID;

    DBQueryResult res;
    DBResultRow row;

    while ( count < loops ) {
        sDatabase.RunQuery(res,
                           "SELECT"
                           "   ssj.fromRegionID,"
                           "   ssj.fromConstellationID,"
                           "   ssj.fromSolarSystemID,"
                           "   c.stargateID"
                           " FROM mapConnections AS c"
                           "  LEFT JOIN mapDenormalize AS d ON d.itemID = c.stargateID"
                           "  LEFT JOIN mapSolarSystemJumps AS ssj ON ssj.fromSolarSystemID = d.solarSystemID"
                           " WHERE c.AI = %u", count );

        res.GetRow(row);
        fromreg = row.GetUInt(0);
        fromcon = row.GetUInt(1);
        fromsol = row.GetUInt(2);

        Populate2(count, fromreg, fromcon, fromsol);

        res.Reset();
        count++;
    }

    sLog.Success("PopulateConnections()", "mapConnections Populated.  Please disable this function.");
}

void MapCon::Populate2(uint16 count, uint32 fromreg, uint32 fromcon, uint32 fromsol) {

    uint32 tosol, tocon, toreg;
    DBQueryResult res;

    sDatabase.RunQuery(res,
                       "SELECT"
                       "   ssj.toSolarSystemID,"
                       "   ssj.toConstellationID,"
                       "   ssj.toRegionID,"
                       "   c.celestialID"
                       " FROM mapConnections AS c"
                       "  LEFT JOIN mapDenormalize AS d ON d.itemID = c.celestialID"
                       "  LEFT JOIN mapSolarSystemJumps AS ssj ON ssj.toSolarSystemID = d.solarSystemID"
                       " WHERE c.AI = %u", count );

    DBResultRow row;
    res.GetRow(row);
    tosol = row.GetUInt(0);
    tocon = row.GetUInt(1);
    toreg = row.GetUInt(2);


    Populate3(fromreg, fromcon, fromsol, tosol, tocon, toreg, count);
}

void MapCon::Populate3(uint32 fromreg, uint32 fromcon, uint32 fromsol, uint32 tosol, uint32 tocon, uint32 toreg, uint16 count) {
    uint16 ctype = 3;
    if(( fromreg == toreg ) && ( fromcon == tocon )) ctype = 2;
    else if( fromreg == toreg ) ctype = 1;
    else ctype = 0;

    DBerror err;
    sDatabase.RunQuery(err,
                       "UPDATE mapConnections"
                       " SET ctype = %u, fromreg = %u, fromcon = %u, fromsol = %u, tosol = %u, tocon = %u, toreg = %u"
                       " WHERE AI = %u ", ctype, fromreg, fromcon, fromsol, tosol, tocon, toreg, count);
}
