/*
 *    File:  MapConnections.cpp
 *  Author:  Allan
 * Purpose:  This file was created to hold functions specific to the mapConnections DB table.
 *      The table is used to supply map connection data to the client for drawing maps, and is not supplied by CCP.
 *      The functions in this file will gather approprate data from the DB, and populate mapConnections with the data needed.
 *
 * Created on June 29, 2014, 12:40 PM
 */

#include "eve-server.h"
#include "MapConnections.h"


MapCon::MapCon()
{
}

void MapCon::PopulateConnections() {
ctype, fromreg, fromcon, fromsol, stargateID, celestialID, tosol, tocon, toreg

    DBQueryResult res;
      if(!sDatabase.RunQuery(res,
          SELECT
          ssj.fromRegionID,
          ssj.fromConstellationID,
          ssj.fromSolarSystemID,
          j.stargateID,
          j.celestialID
          FROM mapJumps AS j
          LEFT JOIN mapDenormalize AS d ON d.itemID = j.stargateID
          LEFT JOIN mapSolarSystemJumps AS ssj ON ssj.fromSolarSystemID = d.solarSystemID
          WHERE %s = %u, place.c_str(), queryID ))



    sLog.Success("server init", "mapConnections Populated.  Please comment out the init line.");
}
