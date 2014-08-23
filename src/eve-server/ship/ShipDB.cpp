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
Author: Zhur, Allan
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "ship/ShipDB.h"

PyTuple* ShipDB::GetFormations()
{
    //vicious crap... but this is gunna be a bit of work to load from the DB (nested tuples)
    PyTuple* res = new PyTuple( 2 );

    Beyonce_Formation f;

    //Diamond formation
    f.name = "Diamond";

    f.pos1.x = 100;
    f.pos1.y = 0;
    f.pos1.z = 0;

    f.pos2.x = 0;
    f.pos2.y = 100;
    f.pos2.z = 0;

    f.pos3.x = -100;
    f.pos3.y = 0;
    f.pos3.z = 0;

    f.pos4.x = 0;
    f.pos4.y = -100;
    f.pos4.z = 0;

    res->SetItem( 0, f.Encode() );

    //Arrow formation
    f.name = "Arrow";

    f.pos1.x = 100;
    f.pos1.y = 0;
    f.pos1.z = -50;

    f.pos2.x = 50;
    f.pos2.y = 0;
    f.pos2.z = 0;

    f.pos3.x = -100;
    f.pos3.y = 0;
    f.pos3.z = -50;

    f.pos4.x = -50;
    f.pos4.y = 0;
    f.pos4.z = 0;

    res->SetItem( 1, f.Encode() );

    return res;
}

PyResult ShipDB::GetInsuranceByShipID(uint32 shipID) {
    DBQueryResult res;
    sDatabase.RunQuery(res,
        "SELECT shipID, startDate, endDate, fraction"
        " FROM chrInsurance"
        " WHERE shipID = %d", shipID );

    DBResultRow row;
    if(!res.GetRow(row)) return NULL;

    Rsp_GetInsuranceContracts rsp;
    rsp.shipID = row.GetUInt( 0 );
    rsp.startDate = row.GetUInt64( 1 );
    rsp.endDate = row.GetUInt64( 2 );
    rsp.fraction = row.GetUInt( 3 );

    return(rsp.Encode());
}

PyResult ShipDB::GetInsuranceByOwnerID(uint32 ownerID) {
    DBQueryResult res;
    sDatabase.RunQuery(res,
        "SELECT i.shipID, i.startDate, i.endDate, i.fraction"
        " FROM chrInsurance AS i"
        "  LEFT JOIN entity AS e ON e.itemID = i.shipID"
        " WHERE e.ownerID = %d", ownerID );

    DBResultRow row;
    if(!res.GetRow(row)) return NULL;

    Rsp_GetInsuranceContracts rsp;
    rsp.shipID = row.GetUInt( 0 );
    rsp.startDate = row.GetUInt64( 1 );
    rsp.endDate = row.GetUInt64( 2 );
    rsp.fraction = row.GetUInt( 3 );

    return(rsp.Encode());
}

bool ShipDB::InsertInsuranceByShipID(uint32 shipID, float fraction, bool isCorpItem) {
    uint64 endDate = Win32TimeNow() + (Win32Time_Day * 90);

    DBerror err;
    if(!sDatabase.RunQuery(err,
      "INSERT INTO "
      "  chrInsurance (shipID, isCorpItem, startDate, endDate, fraction)"
      " VALUES (%d, %u, %" PRIu64 ", %" PRIu64 ", %.3f)",
      shipID, isCorpItem, Win32TimeNow(), endDate, fraction ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}

void ShipDB::DeleteInsuranceByShipID(uint32 shipID) {
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM chrInsurance WHERE shipID=%d", shipID);
}
