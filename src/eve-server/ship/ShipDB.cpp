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
Author: Zhur
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

PyResult ShipDB::GetInsuranceInfoByShipID(uint32 shipID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT e.ownerID, i.fraction, i.startDate, i.endDate"
        " FROM chrInsurance AS i"
        "  LEFT JOIN entity e ON i.shipID=e.itemID"
        " WHERE shipID=%d", shipID ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
/*
    CRowSet *rowSet = (CRowSet*)DBResultToCRowset(res);

    // do we have an Insurance?
    if(rowSet->GetRowCount()<1)
      return NULL;

    PyPackedRow *row = rowSet->GetRow(0);

    return row;
    */
    return(DBResultToRowset(res));
}

PyResult ShipDB::GetInsuranceContractsByOwnerID(uint32 ownerID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT i.shipID, i.startDate, i.endDate, i.fraction"
        " FROM chrInsurance AS i"
        "  LEFT JOIN entity AS e ON i.shipID = e.itemID "
        " WHERE e.ownerID=%d", ownerID ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyNone;
    }
/*
    DBResultRow row;
    if(!res.GetRow(row)) return NULL;

    Rsp_GetInsuranceContracts rsp;
    rsp.shipID = row.GetUInt( 0 );
    rsp.startDate = row.GetUInt( 1 );
    rsp.endDate = row.GetUInt( 2 );
    rsp.fraction = row.GetUInt( 3 );

    return(rsp.Encode());
*/
    return(DBResultToRowset(res));      // this gives "Assertion `DBTYPE_ERROR != result' failed." when there are active contracts.
}

bool ShipDB::InsertInsuranceByShipID(uint32 shipID, double fraction) {
    uint64 startDate = Win32TimeNow();
    uint64 endDate = startDate + (Win32Time_Day * 84);

    DBerror err;
    if(!sDatabase.RunQuery(err,
      "INSERT INTO "
      "  chrInsurance (shipID, startDate, endDate, fraction)"
      " VALUES (%d, %" PRIu64 ", %" PRIu64 ", %.4f)",
      shipID, startDate, endDate, fraction ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}

bool ShipDB::DeleteInsuranceByShipID(uint32 shipID) {
    DBerror err;

    if(!sDatabase.RunQuery(err, "DELETE FROM chrInsurance WHERE shipID=%d", shipID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}
