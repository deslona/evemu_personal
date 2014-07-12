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
#include "scanning/ScanMgrService.h"

PyCallable_Make_InnerDispatcher(ScanMgrService)

ScanMgrService::ScanMgrService(PyServiceMgr *mgr)
: PyService(mgr, "scanMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ScanMgrService, GetSystemScanMgr)

}

ScanMgrService::~ScanMgrService() {
    delete m_dispatch;
}

//02:17:50 L ScanMgrService::Handle_GetSystemScanMgr(): size= 0
PyResult ScanMgrService::Handle_GetSystemScanMgr( PyCallArgs& call ) {
  /*
01:49:51 L ScanMgrService::Handle_GetSystemScanMgr(): size= 0
01:49:51 [SvcCall]   Call Arguments:
01:49:51 [SvcCall]       Tuple: Empty
01:49:51 [SvcCall]   Call Named Arguments:
01:49:51 [SvcCall]     Argument 'machoVersion':
01:49:51 [SvcCall]         Integer field: 1

/client/script/parklife/scansvc.py(194) RequestScans
        self = <svc.ScanSvc instance at 0x06556AD0>
        scanMan = None
        probes = None
AttributeError: 'NoneType' object has no attribute 'RequestScans'
*/
  sLog.Log( "ScanMgrService::Handle_GetSystemScanMgr()", "size= %u", call.tuple->size() );
  /**
   ConeScan  -AttributeError: 'NoneType' object has no attribute 'ConeScan'
   RequestScans  -AttributeError: 'NoneType' object has no attribute 'RequestScans'
  */
  call.Dump(SERVICE__CALLS);

    PyDict *res = new PyDict();

    res->SetItem("ConeScan", new PyInt(1) ) ;
    res->SetItem("RequestScans", new PyInt(2) ) ;

    return res;
}

/**
        return sm.RemoteSvc('scanMgr').GetSystemScanMgr().ConeScan(scanangle, scanRange, x, y, z)

            sm.RemoteSvc('scanMgr').GetSystemScanMgr().ReconnectToLostProbes()

        successProbeIDs = sm.RemoteSvc('scanMgr').GetSystemScanMgr().RecoverProbes(probeIDs)

        scanMan = sm.RemoteSvc('scanMgr').GetSystemScanMgr()
        scanMan.RequestScans(probes)

            scanMan = sm.RemoteSvc('scanMgr').GetSystemScanMgr()
            scanMan.DestroyProbe(probeID)

            */