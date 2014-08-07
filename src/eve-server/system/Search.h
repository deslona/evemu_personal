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
    Author:        Allan
*/


#ifndef __SEARCH_SERVICE_H_INCL__
#define __SEARCH_SERVICE_H_INCL__

#include "PyService.h"
#include "system/SearchDB.h"

class Search : public PyService {
public:
    Search(PyServiceMgr *mgr);
    virtual ~Search();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    SearchDB m_db;

    PyCallable_DECL_CALL(Query);
    PyCallable_DECL_CALL(QuickQuery);
};

#endif


/**
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
searchResultAllOwners = [1,
 2,
 3,
 4,
 5]
searchResultAllLocations = [6,
 7,
 8,
 9]
searchMaxResults = 500
searchMinWildcardLength = 3
*/