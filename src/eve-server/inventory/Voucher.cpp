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
#include "inventory/Voucher.h"

PyCallable_Make_InnerDispatcher(VoucherService)

VoucherService::VoucherService(PyServiceMgr *mgr)
: PyService(mgr, "voucher"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(VoucherService, GetObject)

}

VoucherService::~VoucherService() {
    delete m_dispatch;
}

PyResult VoucherService::Handle_GetObject( PyCallArgs& call ) {
  /*t
23:33:00 L VoucherService::Handle_GetObject(): size= 1
23:33:00 [SvcCall]   Call Arguments:
23:33:00 [SvcCall]       Tuple: 1 elements
23:33:00 [SvcCall]         [ 0] Integer field: 140000575

NOTE:  this function sends  bookmark voucher itemID.  will need to figure out how to save BM with copied original bmID, then get info from original via this voucher using db calls..
//not sure what the return is yet.
  sLog.Log( "VoucherService::Handle_GetObject()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
  */

    return NULL;
}
