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

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/Search.h"

PyCallable_Make_InnerDispatcher(Search)

Search::Search(PyServiceMgr *mgr)
: PyService(mgr, "search"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(Search, Query)
    PyCallable_REG_CALL(Search, QuickQuery)
}

Search::~Search() {
    delete m_dispatch;
}

PyResult Search::Handle_Query( PyCallArgs& call )   //called from people/places window
{/*
06:01:35 L search::Handle_Query(): size=2, 0=WString(rens), 1=List()
06:01:35 [SvcCall]   Call Arguments:
06:01:35 [SvcCall]       Tuple: 2 elements
06:01:35 [SvcCall]         [ 0] WString: 'rens'            // search string
06:01:35 [SvcCall]         [ 1] List: 1 elements           // unknown yet
06:01:35 [SvcCall]         [ 1]   [ 0] Integer field: 5
06:01:35 [SvcCall]   Call Named Arguments:
06:01:35 [SvcCall]     Argument 'machoVersion':
06:01:35 [SvcCall]         Integer field: 1
*/

    return NULL;
}

PyResult Search::Handle_QuickQuery( PyCallArgs& call )  //called from starmap
{/*
05:56:52 L search::Handle_QuickQuery(): size=2, 0=WString(rens*), 1=List()
05:56:52 [SvcCall]   Call Arguments:
05:56:52 [SvcCall]       Tuple: 2 elements
05:56:52 [SvcCall]         [ 0] WString: 'rens*'            // search string
05:56:52 [SvcCall]         [ 1] List: 3 elements            // unknown yet
05:56:52 [SvcCall]         [ 1]   [ 0] Integer field: 6
05:56:52 [SvcCall]         [ 1]   [ 1] Integer field: 7
05:56:52 [SvcCall]         [ 1]   [ 2] Integer field: 8
05:56:52 [SvcCall]   Call Named Arguments:
05:56:52 [SvcCall]     Argument 'hideNPC':
05:56:52 [SvcCall]         Integer field: 0
05:56:52 [SvcCall]     Argument 'machoVersion':
05:56:52 [SvcCall]         Integer field: 1
05:56:52 [SvcCall]     Argument 'onlyAltName':
05:56:52 [SvcCall]         Integer field: 0
*/
    return NULL;
}