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

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "pos/PosMgrService.h"

class PosMgrBound
    : public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PosMgrBound);

    PosMgrBound(PyServiceMgr *mgr, PosMgrDB *db)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PosMgrBound";

        PyCallable_REG_CALL(PosMgrBound, GetMoonForTower)
        PyCallable_REG_CALL(PosMgrBound, SetTowerPassword)
        PyCallable_REG_CALL(PosMgrBound, SetShipPassword)
        PyCallable_REG_CALL(PosMgrBound, GetSiloCapacityByItemID)
    }

    virtual ~PosMgrBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(GetMoonForTower)
    PyCallable_DECL_CALL(SetTowerPassword)
    PyCallable_DECL_CALL(SetShipPassword)
    PyCallable_DECL_CALL(GetSiloCapacityByItemID)

protected:
    Dispatcher *const m_dispatch;
    PosMgrDB  *const m_db;        //we do not own this

};

PyCallable_Make_InnerDispatcher(PosMgrService)

PosMgrService::PosMgrService(PyServiceMgr *mgr)
: PyService(mgr, "posMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PosMgrService, GetControlTowerFuelRequirements)
    //PyCallable_REG_CALL(PosMgrService, )
}

PosMgrService::~PosMgrService() {
    delete m_dispatch;
}

PyBoundObject* PosMgrService::_CreateBoundObject( Client* c, const PyRep* bind_args ) {
    _log( CLIENT__MESSAGE, "PosMgrService bind request for:" );
    bind_args->Dump( CLIENT__MESSAGE, "    " );

    return new PosMgrBound( m_manager, &m_db );
}

PyResult PosMgrBound::Handle_GetMoonForTower( PyCallArgs &call ) {
  /*
13:13:06 L PosMgrBound::Handle_GetMoonForTower(): size= 1
13:13:06 [SvcCall]   Call Arguments:
13:13:06 [SvcCall]       Tuple: 1 elements
13:13:06 [SvcCall]         [ 0] Integer field: 140001260

  sLog.Log( "PosMgrBound::Handle_GetMoonForTower()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
*/
  PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_SetTowerPassword( PyCallArgs &call ) {
  /*
13:10:09 L PosMgrBound::Handle_SetTowerPassword(): size= 2
13:10:09 [SvcCall]   Call Arguments:
13:10:09 [SvcCall]       Tuple: 2 elements
13:10:09 [SvcCall]         [ 0] Integer field: 140001260
13:10:09 [SvcCall]         [ 1] WString: 'test'

  sLog.Log( "PosMgrBound::Handle_SetTowerPassword()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
*/
  PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_SetShipPassword( PyCallArgs &call ) {
  /*
13:16:17 L PosMgrBound::Handle_SetShipPassword(): size= 1
13:16:17 [SvcCall]   Call Arguments:
13:16:17 [SvcCall]       Tuple: 1 elements
13:16:17 [SvcCall]         [ 0] WString: 'test'

  sLog.Log( "PosMgrBound::Handle_SetShipPassword()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
*/

  PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_GetSiloCapacityByItemID(PyCallArgs &call) {
  sLog.Log( "PosMgrBound::Handle_GetSiloCapacityByItemID()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    uint32 itemID;

    return m_db->GetSiloCapacityByItemID(itemID);
}

PyResult PosMgrService::Handle_GetControlTowerFuelRequirements(PyCallArgs &call) {
  sLog.Log( "PosMgrService::Handle_GetControlTowerFuelRequirements()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    return m_db.GetControlTowerFuelRequirements();
}
