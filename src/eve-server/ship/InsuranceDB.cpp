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
	Author:		Nerey
*/

#include "eve-server.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "ship/InsuranceService.h"
#include "ship/InsuranceDB.h"
#include "ServiceDB.h"

PyRep *InsuranceDB::GetContracts(uint32 ownerID)
{
	DBQueryResult res;
	if(!sDatabase.RunQuery(res,
		"SELECT"
		"	shipID, typeID, price, isCorpItem, ownerID, startDate, endDate, fraction"
		" FROM insurance "
        " WHERE ownerID=%u"
		" GROUP BY typeID", ownerID, ownerID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	PyObject* result = DBResultToRowset(res);
	return(result);
}

PyRep *InsuranceDB::GetInsurancePrice(uint32 typeID)
{
	DBQueryResult res;
	if(!sDatabase.RunQuery(res,
		"SELECT"
		"	price"
		" FROM insurance_price "
        " WHERE typeID=%u"
		" GROUP BY typeID", typeID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	DBResultRow row;
	if(res.GetRow(row))
	{
		PyFloat* result = new PyFloat(row.GetDouble(0));
		return(result);
	}
	else
	{
		return new PyFloat(75000.0);
	}
}

PyRep *InsuranceDB::InsureShip(uint32 shipID, uint32 ownerID, uint32 insuranceType, double price, uint32 isCorpItem, uint64 startDate, uint64 endDate)
{
	DBerror err;
	if(!sDatabase.RunQuery(err,
		" INSERT"
		" INTO insurance"
		" VALUES (%u, %u, %f, %u, %u, %u, %u, %u)", shipID, insuranceType, price, ownerID, isCorpItem, startDate, endDate, 1))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.GetError());
		return new PyBool(false);
	};
	return new PyBool(true);
}

PyRep *InsuranceDB::GetContractForShip(uint32 shipID, uint32 ownerID)
{
	DBQueryResult res;
	if(!sDatabase.RunQuery(res,
		"SELECT"
		"	shipID, typeID, price, isCorpItem, ownerID, startDate, endDate, fraction"
		" FROM insurance "
        " WHERE ownerID=%u"
		" GROUP BY typeID"
		" LIMIT 1", ownerID, ownerID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	if(res.GetRowCount() == 0) { return new PyNone(); }
	DBResultRow row;
	res.GetRow(row);
	PyObject* result = DBRowToRow(row);
	return(result);
}

PyRep *InsuranceDB::UnInsureShip(uint32 shipID, uint32 ownerID)
{
	DBerror err;
	if(!sDatabase.RunQuery(err,
		" DELETE"
		" FROM insurance"
		" WHERE shipID='%u' AND ownerID='%u'"
		" LIMIT 1", shipID, ownerID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.GetError());
		return new PyBool(false);
	};
	return new PyBool(true);
}

uint64 Win32TimeOffset(int days, int hours)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
	ULONGLONG qwResult;
	qwResult = (((ULONGLONG) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
	qwResult += days * _DAY;
	qwResult += hours * _HOUR;
	ft.dwLowDateTime  = (DWORD) (qwResult & 0xFFFFFFFF );
	ft.dwHighDateTime = (DWORD) (qwResult >> 32 );
    return((uint64(ft.dwHighDateTime) << 32) | uint64(ft.dwLowDateTime));
}
