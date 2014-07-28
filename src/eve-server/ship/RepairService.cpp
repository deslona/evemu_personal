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
    Author:        Reve
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "ship/RepairService.h"

PyCallable_Make_InnerDispatcher(RepairService)

RepairService::RepairService(PyServiceMgr *mgr)
: PyService(mgr, "repairSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(RepairService, UnasembleItems);
}

RepairService::~RepairService() {
    delete m_dispatch;
}

PyResult RepairService::Handle_UnasembleItems(PyCallArgs &call) {
  /*
19:49:29 L RepairService::Handle_UnasembleItems: Called UnasembleItems stub.
19:49:29 [SvcCall]   Call Arguments:
19:49:29 [SvcCall]       Tuple: 2 elements
19:49:29 [SvcCall]         [ 0] Dictionary: 1 entries
19:49:29 [SvcCall]         [ 0]   [ 0] Key: Integer field: 60004450
19:49:29 [SvcCall]         [ 0]   [ 0] Value: List: 1 elements
19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0] Tuple: 2 elements
19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0]   [ 0] Integer field: 140001999
19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0]   [ 1] Integer field: 60004450
19:49:29 [SvcCall]         [ 1] List: Empty

    sLog.Warning("RepairService::Handle_UnasembleItems", "Called UnasembleItems stub.");
  call.Dump(SERVICE__CALLS);
*/
call.client->SendNotifyMsg("This Service is Not Implemented at this time.");
    return NULL;
}

/*
            damageReports = self.repairSvc.GetDamageReports(itemIDs)

                    self.repairSvc.RepairItems(itemIDs, amount['qty'])


                itemIDAndAmountOfDamageList.append((item.itemID, amount))
                self.repairSvc.DamageModules(itemIDAndAmountOfDamageList)
*/

/**
                sm.RemoteSvc('repairSvc').UnasembleItems(dict(validIDsByStationID), skipChecks)

repackableCategorys = (categoryStructure,
 categoryShip,
 categoryDrone,
 categoryModule,
 categorySubSystem,
 categorySovereigntyStructure)
repackableGroups = (groupCargoContainer,
 groupSecureCargoContainer,
 groupAuditLogSecureContainer,
 groupFreightContainer,
 groupTool,
 groupMobileWarpDisruptor)


 */
