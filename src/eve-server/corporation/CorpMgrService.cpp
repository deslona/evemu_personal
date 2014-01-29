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
    Author:        Zhur
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "corporation/CorpMgrService.h"

PyCallable_Make_InnerDispatcher(CorpMgrService)

CorpMgrService::CorpMgrService(PyServiceMgr *mgr)
: PyService(mgr, "corpmgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpMgrService, GetPublicInfo)
    PyCallable_REG_CALL(CorpMgrService, GetAssetInventory)
}

CorpMgrService::~CorpMgrService() {
    delete m_dispatch;
}


PyResult CorpMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
    Call_SingleIntegerArg corpID;
    if (!corpID.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Bad param");
        return NULL;
    }

    return m_db.GetCorporation(corpID.arg);
}


//22:33:36 L CorpMgrService::Handle_GetAssetInventory(): size= 2, 0 = int (1001000), 1 = string (varies)
PyResult CorpMgrService::Handle_GetAssetInventory(PyCallArgs &call)
{
  uint32 size = call.tuple->size();
  uint32 int1 = call.tuple->GetItem(0)->AsInt()->value();   // corpID
  std::string string = call.tuple->GetItem(1)->AsString()->content();  // tab in corp asset window...offices, impounded, in space,
                                                                       //      deliveries, lockdown, search.  also called from map.

  sLog.Log( "CorpMgrService::Handle_GetAssetInventory()", "size= %u, 0 = int (%u), 1 = string (%s)", size, int1, string.c_str() );

    return NULL;
}



















