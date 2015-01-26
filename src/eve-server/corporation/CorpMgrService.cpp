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
    Author:        Zhur, Allan
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

    PyCallable_REG_CALL(CorpMgrService, GetPublicInfo);
    PyCallable_REG_CALL(CorpMgrService, GetCorporations);
    PyCallable_REG_CALL(CorpMgrService, GetAssetInventory);
    PyCallable_REG_CALL(CorpMgrService, GetCorporationStations);
    PyCallable_REG_CALL(CorpMgrService, GetCorporationIDForCharacter);
    PyCallable_REG_CALL(CorpMgrService, GetAssetInventoryForLocation);
    PyCallable_REG_CALL(CorpMgrService, AuditMember);
}

CorpMgrService::~CorpMgrService() {
    delete m_dispatch;
}


PyResult CorpMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
  /*
14:12:47 [SvcCall] Service corpmgr: calling GetPublicInfo
14:12:47 W Client::BeanCount: BeanCount error reporting and handling is not implemented yet.
14:12:47 [SvcCall] Service alert: calling BeanCount
14:12:47 [SvcCall] Service alert: calling SendClientStackTraceAlert
EXCEPTION #13 logged at  07/27/2014 14:12:47 Unhandled exception in <TaskletExt object at 2f7882f0, abps=1001, ctxt="<NO CONTEXT>^<bound method InfoWindow.LoadData of form.infowindow object at (snip)f3d6090, name=('infowindow', 130509619634265564L), destroyed=False>>">
Caught at:
/common/lib/bluepy.py(98) CallWrapper
Thrown at:
/common/lib/bluepy.py(86) CallWrapper
/client/script/ui/services/infosvc.py(3231) LoadData
/client/script/ui/services/infosvc.py(3296) _LoadInfoWindow
/client/script/ui/services/infosvc.py(4109) GetNameAndDescription
        itemID = 1000123
        typeID = 2
        invtype = <Instance of class sys.InvType>
                       typeID:                 2
                       groupID:                2
                       typeName:               Corporation
                       ...
        capt = u'Corporation'
        self = form.infowindow object at 0x2f3d6090, name=('infowindow', 130509619634265564L), destroyed=False>
        label = ''
        desc = ''
AttributeError: Rowset instance has no attribute 'corporationID'
*/
    Call_SingleIntegerArg corpID;
    if (!corpID.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Bad param");
        return NULL;
    }

    //if(corpID.arg < 1001000)    //is NPCCorp
    //    return m_db.GetCorpInfo(corpID.arg);
    //else
        return m_db.GetCorporation(corpID.arg);
}

PyResult CorpMgrService::Handle_GetCorporations(PyCallArgs &call) {
  Call_SingleIntegerArg corpID;
  if (!corpID.Decode(&call.tuple)) {
      codelog(SERVICE__ERROR, "Bad param");
      return NULL;
  }

  return m_db.GetCorporations(corpID.arg);
}

//  started...still needs work
PyResult CorpMgrService::Handle_GetAssetInventory(PyCallArgs &call) {
    /* 21:34:13 L CorpMgrService::Handle_GetAssetInventory(): size= 2
     * 21:34:13 [SvcCall]   Call Arguments:
     * 21:34:13 [SvcCall]       Tuple: 2 elements
     * 21:34:13 [SvcCall]         [ 0] Integer field: 2000000
     * 21:34:13 [SvcCall]         [ 1] String: 'offices'
     *
     *
     *  sLog.Log( "CorpMgrService::Handle_GetAssetInventory()", "size= %u", call.tuple->size() );
     *  call.Dump(SERVICE__CALLS);
     */
    Call_GetAssetInventory args;

    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "CorpMgrService::Handle_GetAssetInventory() - Error Decoding Arguments.");
        return NULL;
    }

    // corpID = args.corpID;
    // std::string assetKey = args.assetKey;

    //  assetKey tells what inventory they're looking for.
    //        tab in corp asset window...offices, impounded, in space, deliveries, lockdown, search.
    // also called from map... -ColorStarsByCorpAssets

    //  i havent thought much about how to do this one yet.  will need items based on location and status(assetKey) for corp
    PyTuple *res = NULL;
    return res;
}

PyResult CorpMgrService::Handle_GetCorporationStations(PyCallArgs &call) {
  /**           this is called from trademgr.py
        stations = sm.RemoteSvc('corpmgr').GetCorporationStations()
        for station in stations:
            if station.itemID in self.shell.GetStationIDs():
                continue
            stationListing.append([localization.GetByLabel('UI/PVPTrade/StationInSolarsystem', station=station.itemID, solarsystem=station.locationID), station.itemID, station.typeID])
*/

  sLog.Log( "CorpMgrService::Handle_GetCorporationStations()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult CorpMgrService::Handle_GetCorporationIDForCharacter(PyCallArgs &call) {
/**        returns corpID for given charID  */

  sLog.Log( "CorpMgrService::Handle_GetCorporationIDForCharacter()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult CorpMgrService::Handle_GetAssetInventoryForLocation(PyCallArgs &call) {
/**
    items = sm.RemoteSvc('corpmgr').GetAssetInventoryForLocation(eve.session.corpid, stationID, which)
    */

sLog.Log( "CorpMgrService::Handle_GetAssetInventoryForLocation()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult CorpMgrService::Handle_AuditMember(PyCallArgs &call) {
    /**
     * logItemEventRows, crpRoleHistroyRows = sm.RemoteSvc('corpmgr').AuditMember(memberID, fromDate, toDate, rowsPerPage)
     */

    sLog.Log( "CorpMgrService::Handle_AuditMember()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}