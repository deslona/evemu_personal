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
#include "station/JumpCloneService.h"

class JumpCloneBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(JumpCloneBound)

    JumpCloneBound(PyServiceMgr *mgr, StationDB *db)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "JumpCloneBound";

        PyCallable_REG_CALL(JumpCloneBound, GetCloneState);
        PyCallable_REG_CALL(JumpCloneBound, InstallCloneInStation);
        PyCallable_REG_CALL(JumpCloneBound, GetPriceForClone);
        PyCallable_REG_CALL(JumpCloneBound, GetShipCloneState);
    }
    virtual ~JumpCloneBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetCloneState);
    PyCallable_DECL_CALL(InstallCloneInStation);
    PyCallable_DECL_CALL(GetPriceForClone);
    PyCallable_DECL_CALL(GetShipCloneState);

protected:
    StationDB *const m_db;        //we do not own this
    Dispatcher *const m_dispatch;    //we own this
};

PyCallable_Make_InnerDispatcher(JumpCloneService)

JumpCloneService::JumpCloneService(PyServiceMgr *mgr)
: PyService(mgr, "jumpCloneSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(JumpCloneService, GetShipCloneState);
}

JumpCloneService::~JumpCloneService() {
    delete m_dispatch;
}

PyBoundObject* JumpCloneService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
    _log( CLIENT__MESSAGE, "JumpCloneService bind request for:" );
    bind_args->Dump( CLIENT__MESSAGE, "    " );

    return new JumpCloneBound( m_manager, &m_db );
}

PyResult JumpCloneBound::Handle_InstallCloneInStation( PyCallArgs &call ) {
  sLog.Log( "JumpCloneBound::Handle_InstallCloneInStation()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
    PyRep *result = NULL;

    return result;
}

//11:46:15 JumpCloneBound::Handle_GetCloneState(): size= 0
PyResult JumpCloneBound::Handle_GetCloneState(PyCallArgs &call) {
    /*
  sLog.Log( "JumpCloneBound::Handle_GetCloneState()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
  */

    //returns (clones, implants, timeLastJump)
    //where jumpClones is a rowset? with at least columns: jumpCloneID, locationID

    PyDict* d = new PyDict;
    d->SetItemString( "clones", new PyNone );
    d->SetItemString( "implants", new PyNone );
    d->SetItemString( "timeLastJump", new PyNone );

    return new PyObject( "util.KeyVal", d );
}

//11:51:52 JumpCloneBound::Handle_GetPriceForClone(): size= 0
PyResult JumpCloneBound::Handle_GetPriceForClone(PyCallArgs &call) {
    /*        kwargs = {'amount': None, 'player': 140000038}
     * TypeError: Numeric Formatter expects floating point or signed integer types.
     */
    PyRep *result = new PyInt(1000000);

    return result;
}

PyResult JumpCloneBound::Handle_GetShipCloneState(PyCallArgs &call) {
    /* 00:24:21 [SvcCall] Service jumpCloneSvc: handling MachoBindObject request directly
     * 00:24:21 Server: Unknown call to 'GetShipCloneState' by 'Lee'
     */
  sLog.Log( "JumpCloneService::Handle_GetShipCloneState()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    PyRep *result = NULL;
    return result;
}
