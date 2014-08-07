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

//work in progress

#include "eve-server.h"

#include "PyServiceCD.h"
#include "ship/FleetObject.h"

PyCallable_Make_InnerDispatcher(FleetObjectHandler)

FleetObjectHandler::FleetObjectHandler(PyServiceMgr *mgr)
: PyService(mgr, "fleetObjectHandler"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FleetObjectHandler, CreateFleet);
}

FleetObjectHandler::~FleetObjectHandler()
{
    delete m_dispatch;
}

PyResult FleetObjectHandler::Handle_CreateFleet(PyCallArgs &call) {
    sLog.Log("FleetObjectHandler", "Handle_CreateFleet() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

	/**  returns fleetID */
    return NULL;
}
/**
typedef enum {
fleetJobNone = 0,
fleetJobScout = 1,
fleetJobCreator = 2
} FleetJobs;

typedef enum {
fleetRoleLeader = 1,
fleetRoleWingCmdr = 2,
fleetRoleSquadCmdr = 3,
fleetRoleMember = 4
} FleetRoles;

typedef enum {
fleetBoosterNone = 0,
fleetBoosterFleet = 1,
fleetBoosterWing = 2,
fleetBoosterSquad = 3
} FleetBoosters;

fleetGroupingRange = 300
rejectFleetInviteTimeout = 1
rejectFleetInviteAlreadyInFleet = 2
*/
/*
MoveMember(charID, wingID, squadID, role, roleBooster):


    def SetBooster(self, charID, roleBooster):
        self.CheckIsInFleet()
        if self.fleet.SetBooster(charID, roleBooster):
            sm.ScatterEvent('OnFleetMemberChanging', charID)

    def CreateWing(self):
        self.CheckIsInFleet()
        wingID = self.fleet.CreateWing()
        if wingID:
            self.CreateSquad(wingID)

    def DeleteWing(self, wingID):
        self.CheckIsInFleet()
        self.fleet.DeleteWing(wingID)

    def CreateSquad(self, wingID):
        self.CheckIsInFleet()
        self.fleet.CreateSquad(wingID)

    def DeleteSquad(self, wingID):
        self.CheckIsInFleet()
        self.fleet.DeleteSquad(wingID)

    def MakeLeader(self, charID):
        self.fleet.MakeLeader(charID)

    def KickMember(self, charID):
        if charID == eve.session.charid:
            self.LeaveFleet()
        else:
            self.fleet.KickMember(charID)

self.fleet.SetMotdEx(motd)
self.fleet.GetMotd()
*/