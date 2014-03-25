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

#include "EntityList.h"
#include "PyServiceCD.h"
#include "station/StationService.h"

PyCallable_Make_InnerDispatcher(StationService)

StationService::StationService(PyServiceMgr *mgr)
: PyService(mgr, "station"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(StationService, GetGuests)
    PyCallable_REG_CALL(StationService, GetSolarSystem)
}

StationService::~StationService() {
    delete m_dispatch;
}

PyResult StationService::Handle_GetSolarSystem(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "%s: Failed to decode GetSolarSystem arguments.", call.client->GetName());
        return NULL;
    }

    int system = arg.arg;

    // this needs to return some cache status?
    return new PyObject("util.CachedObject", new PyInt(system));
}

// station guest list shows ALL docked players no matter what station they're in.
PyResult StationService::Handle_GetGuests(PyCallArgs &call) {
  /**
00:26:27 L StationService::Handle_GetGuests(): size= 0
00:26:27 [SvcCall]   Call Arguments:
00:26:27 [SvcCall]       Tuple: Empty
00:26:27 [SvcCall]   Call Named Arguments:
00:26:27 [SvcCall]     Argument 'machoVersion':
00:26:27 [SvcCall]         Integer field: 1

  sLog.Log( "StationService::Handle_GetGuests()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
*/
    PyList *res = new PyList();

    std::vector<Client *> clients;
    m_manager->entity_list.FindByStationID(call.client->GetStationID(), clients);  /// oh!  THIS isnt working right....check later
    std::vector<Client *>::iterator cur, end;
    cur = clients.begin();
    end = clients.end();
    for(; cur != end; cur++) {
        PyTuple *t = new PyTuple(4);
        t->items[0] = new PyInt((*cur)->GetCharacterID());
        t->items[1] = new PyInt((*cur)->GetCorporationID());
        t->items[2] = new PyInt((*cur)->GetAllianceID());
        t->items[3] = new PyInt((*cur)->GetStationID());    //unknown - trying StationID to see if this corrects station's "guest list"
        res->AddItem(t);
    }

    return res;

}
