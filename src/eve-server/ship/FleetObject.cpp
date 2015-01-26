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
    PyCallable_REG_CALL(FleetObjectHandler, CreateWing);
    PyCallable_REG_CALL(FleetObjectHandler, CreateSquad);
    PyCallable_REG_CALL(FleetObjectHandler, SetMotdEx);
    PyCallable_REG_CALL(FleetObjectHandler, GetMotd);
    PyCallable_REG_CALL(FleetObjectHandler, CheckIsInFleet);
    PyCallable_REG_CALL(FleetObjectHandler, MakeLeader);
    PyCallable_REG_CALL(FleetObjectHandler, SetBooster);
    PyCallable_REG_CALL(FleetObjectHandler, MoveMember);
    PyCallable_REG_CALL(FleetObjectHandler, KickMember);
    PyCallable_REG_CALL(FleetObjectHandler, DeleteWing);
    PyCallable_REG_CALL(FleetObjectHandler, DeleteSquad);
    PyCallable_REG_CALL(FleetObjectHandler, LeaveFleet);
}

FleetObjectHandler::~FleetObjectHandler()
{
    delete m_dispatch;
}

PyResult FleetObjectHandler::Handle_CreateFleet(PyCallArgs &call) {
    /*  object manager here sets fleet IDs (squad, wing, fleet)
     *   and tracks player movement within the fleet.
     *
     *   will also call updates on fleet session data
     */

    uint16 fleetID = 20;
    FleetMemberInfo fleet;

    fleet.fleetID = fleetID;
    fleet.wingID = 0;
    fleet.squadID = 0;
    fleet.fleetRole = fleetRoleLeader;
    fleet.fleetBooster = fleetBoosterNone;
    fleet.fleetJob = fleetJobCreator;

    call.client->GetChar().get()->SetFleetData(fleet);

    /**  returns fleetID */
    return new PyInt(fleetID);
}

PyResult FleetObjectHandler::Handle_CreateWing(PyCallArgs &call) {
    /*  wingID = self.fleet.CreateWing()  */

    sLog.Log("FleetObjectHandler", "Handle_CreateWing() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_CreateSquad(PyCallArgs &call) {
    /* self.fleet.CreateSquad(wingID)  */

    sLog.Log("FleetObjectHandler", "Handle_CreateSquad() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_SetMotdEx(PyCallArgs &call) {
    /*  self.fleet.SetMotdEx(motd)  */

    sLog.Log("FleetObjectHandler", "Handle_SetMotdEx() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_GetMotd(PyCallArgs &call) {
    /*  self.fleet.GetMotd()  */

    sLog.Log("FleetObjectHandler", "Handle_GetMotd() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_CheckIsInFleet(PyCallArgs &call) {
    /* self.CheckIsInFleet()  */

    sLog.Log("FleetObjectHandler", "Handle_CheckIsInFleet() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_MakeLeader(PyCallArgs &call) {
    /* self.fleet.MakeLeader(charID)  */

    sLog.Log("FleetObjectHandler", "Handle_MakeLeader() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_SetBooster(PyCallArgs &call) {
    /*self.fleet.SetBooster(charID, roleBooster):
     *sm.ScatterEvent('OnFleetMemberChanging', charID)  */

    sLog.Log("FleetObjectHandler", "Handle_SetBooster() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_MoveMember(PyCallArgs &call) {
    /*  MoveMember(charID, wingID, squadID, role, roleBooster):  */

    sLog.Log("FleetObjectHandler", "Handle_MoveMember() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_KickMember(PyCallArgs &call) {
    /*
     *        if charID == eve.session.charid:
     *            self.LeaveFleet()
     *        else:
     *            self.fleet.KickMember(charID)
     */

    sLog.Log("FleetObjectHandler", "Handle_KickMember() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_DeleteWing(PyCallArgs &call) {
    /*    self.fleet.DeleteWing(wingID)  */

    sLog.Log("FleetObjectHandler", "Handle_DeleteWing() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_DeleteSquad(PyCallArgs &call) {
    /* self.fleet.DeleteSquad(wingID)  */

    sLog.Log("FleetObjectHandler", "Handle_DeleteSquad() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObjectHandler::Handle_LeaveFleet(PyCallArgs &call) {
    /*
        if charID == eve.session.charid:
            self.LeaveFleet()
        else:
            self.fleet.KickMember(charID)
    */

    sLog.Log("FleetObjectHandler", "Handle_LeaveFleet() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

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