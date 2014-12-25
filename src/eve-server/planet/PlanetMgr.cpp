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

//work in progress

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "planet/PlanetMgr.h"

class PlanetMgrBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetMgrBound)

    PlanetMgrBound(PyServiceMgr *mgr)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PlanetMgrBound";

        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetInfo);
        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetResourceInfo);
    }
    virtual ~PlanetMgrBound() { delete m_dispatch; }
    virtual void Release() {
        //He hates this statement
        delete this;
    }

        PyCallable_DECL_CALL(GetPlanetInfo);
        PyCallable_DECL_CALL(GetPlanetResourceInfo);

protected:
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(PlanetMgrService)

PlanetMgrService::PlanetMgrService(PyServiceMgr *mgr)
: PyService(mgr, "planetMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PlanetMgrService, GetPlanetsForChar);
    PyCallable_REG_CALL(PlanetMgrService, GetMyLaunchesDetails);
}

PlanetMgrService::~PlanetMgrService() {
    delete m_dispatch;
}

PyBoundObject *PlanetMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return NULL;
    }
    return new PlanetMgrBound(m_manager);
}

//02:49:10 PlanetMgrBound: Handle_GetPlanetInfo() size=0
PyResult PlanetMgrBound::Handle_GetPlanetInfo(PyCallArgs &call) {
    /*
    /common/lib/bluepy.py(396) Wrapper
    /client/script/environment/planet/clientplanet.py(109) PreparePlanet
    self = <planet.ClientPlanet instance at 0x4AEC3918>
    planetInfo = None
    AttributeError: 'NoneType' object has no attribute 'solarSystemID'
    */

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetPlanetResourceInfo(PyCallArgs &call) {
    sLog.Log("PlanetMgrBound", "Handle_GetPlanetResourceInfo() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

//05:44:57 PlanetMgrBound: Handle_GetPlanetsForChar() size=0
PyResult PlanetMgrService::Handle_GetPlanetsForChar(PyCallArgs &call) {
  /**
            self.colonizationData = sm.RemoteSvc('planetMgr').GetPlanetsForChar()
            returns  PlanetID, numberOfPins
            */

    return NULL;
}

//05:51:11 PlanetMgrBound: Handle_GetMyLaunchesDetails() size=0
PyResult PlanetMgrService::Handle_GetMyLaunchesDetails(PyCallArgs &call) {
    sLog.Log("PlanetMgrBound", "Handle_GetMyLaunchesDetails() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}


/*
 * /common/lib/bluepy.py(86) CallWrapper
 * /client/script/ui/shared/planet/planetnavigation.py(301) OnMouseMove
 *        self = uicls.PlanetLayer object at 0xdca8330, name=l_planet, destroyed=False>
 *        args = ()
 * AttributeError: 'NoneType' object has no attribute 'ManualRotate'
 *
 * /client/script/ui/shared/planet/planetnavigation.py(394) OnMouseUp
 *        self = uicls.PlanetLayer object at 0xdca8330, name=l_planet, destroyed=False>
 *        btnNum = 0
 * AttributeError: 'PlanetLayer' object has no attribute 'eventManager'
 *
 * /../carbon/client/script/ui/services/registry.py(277) SetFocus
 * /client/script/ui/shared/planet/planetnavigation.py(292) OnKillFocus
 *        self = uicls.PlanetLayer object at 0xdca8330, name=l_planet, destroyed=False>
 *        args = ()
 * AttributeError: 'PlanetLayer' object has no attribute 'eventManager'
 *
 * /client/script/ui/shared/planet/planetuisvc.py(1162) LogPlanetAccess
 *        planetAccessed = 1
 *        myPlanets = None
 *        self = <svc.PlanetUISvc instance at 0x30838800>
 *        colonized = 0
 * TypeError: 'NoneType' object is not iterable
 *
 */