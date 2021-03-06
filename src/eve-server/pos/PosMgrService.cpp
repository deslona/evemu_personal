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

        PyCallable_REG_CALL(PosMgrBound, GetMoonForTower);
        PyCallable_REG_CALL(PosMgrBound, SetTowerPassword);
        PyCallable_REG_CALL(PosMgrBound, SetShipPassword);
        PyCallable_REG_CALL(PosMgrBound, GetSiloCapacityByItemID);
    }

    virtual ~PosMgrBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(GetMoonForTower);
    PyCallable_DECL_CALL(SetTowerPassword);
    PyCallable_DECL_CALL(SetShipPassword);
    PyCallable_DECL_CALL(GetSiloCapacityByItemID);

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

    PyCallable_REG_CALL(PosMgrService, GetControlTowerFuelRequirements);
    PyCallable_REG_CALL(PosMgrService, InstallJumpBridgeLink);
    PyCallable_REG_CALL(PosMgrService, UninstallJumpBridgeLink);
    PyCallable_REG_CALL(PosMgrService, GetJumpArrays);
    PyCallable_REG_CALL(PosMgrService, GetControlTowers);
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

    uint32 itemID = 0;

    return m_db->GetSiloCapacityByItemID(itemID);
}

PyResult PosMgrService::Handle_GetControlTowerFuelRequirements(PyCallArgs &call) {
  sLog.Log( "PosMgrService::Handle_GetControlTowerFuelRequirements()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    return m_db.GetControlTowerFuelRequirements();
}

PyResult PosMgrService::Handle_InstallJumpBridgeLink(PyCallArgs &call) {
  /**
    def BridgePortals(self, localItemID, remoteSolarSystemID, remoteItemID):
        posLocation = util.Moniker('posMgr', session.solarsystemid)
        posLocation.InstallJumpBridgeLink(localItemID, remoteSolarSystemID, remoteItemID)

  */
  sLog.Log( "PosMgrService::Handle_InstallJumpBridgeLink()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult PosMgrService::Handle_UninstallJumpBridgeLink(PyCallArgs &call) {
  /**
    def UnbridgePortal(self, itemID):
        posLocation = util.Moniker('posMgr', session.solarsystemid)
        posLocation.UninstallJumpBridgeLink(itemID)

  */
  sLog.Log( "PosMgrService::Handle_UninstallJumpBridgeLink()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult PosMgrService::Handle_GetJumpArrays(PyCallArgs &call) {
    //        jb = sm.RemoteSvc('posMgr').GetJumpArrays()
    sLog.Log( "PosMgrService::Handle_GetJumpArrays()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult PosMgrService::Handle_GetControlTowers(PyCallArgs &call) {
    //    ct = sm.RemoteSvc('posMgr').GetControlTowers()

    sLog.Log( "PosMgrService::Handle_GetControlTowers()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    return NULL;
}


/*
    def AnchorOrbital(self, itemID):
        posMgr = util.Moniker('posMgr', session.solarsystemid)
        posMgr.AnchorOrbital(itemID)

    def UnanchorOrbital(self, itemID):
        posMgr = util.Moniker('posMgr', session.solarsystemid)
        posMgr.UnanchorOrbital(itemID)

    def CompleteOrbitalStateChange(self, itemID):
        posMgr = util.Moniker('posMgr', session.solarsystemid)
        posMgr.CompleteOrbitalStateChange(itemID)

    def GMUpgradeOrbital(self, itemID):
        posMgr = util.Moniker('posMgr', session.solarsystemid)
        posMgr.GMUpgradeOrbital(itemID)

*/
/*
                state = slimItem.orbitalState
                if state in (entities.STATE_UNANCHORING,
                 entities.STATE_ONLINING,
                 entities.STATE_ANCHORING,
                 entities.STATE_OPERATING,
                 entities.STATE_OFFLINING,
                 entities.STATE_SHIELD_REINFORCE):
                    stateText = pos.DISPLAY_NAMES[pos.Entity2DB(state)]
                    gm.append(('End orbital state change (%s)' % stateText, self.CompleteOrbitalStateChange, (itemID,)))
                elif state == entities.STATE_ANCHORED:
                    upgradeType = sm.GetService('godma').GetTypeAttribute2(slimItem.typeID, const.attributeConstructionType)
                    if upgradeType is not None:
                        gm.append(('Upgrade to %s' % cfg.invtypes.Get(upgradeType).typeName, self.GMUpgradeOrbital, (itemID,)))
                gm.append(('GM: Take Control', self.TakeOrbitalOwnership, (itemID, slimItem.planetID)))
                */

/*
    def GetGMStructureStateMenu(self, itemID = None, slimItem = None, charID = None, invItem = None, mapItem = None):
        subMenu = []
        if hasattr(slimItem, 'posState') and slimItem.posState is not None:
            currentState = slimItem.posState
            if currentState not in pos.ONLINE_STABLE_STATES:
                if currentState == pos.STRUCTURE_ANCHORED:
                    subMenu.append(('Online', sm.RemoteSvc('slash').SlashCmd, ('/pos online ' + str(itemID),)))
                    subMenu.append(('Unanchor', sm.RemoteSvc('slash').SlashCmd, ('/pos unanchor ' + str(itemID),)))
                elif currentState == pos.STRUCTURE_UNANCHORED:
                    subMenu.append(('Anchor', sm.RemoteSvc('slash').SlashCmd, ('/pos anchor ' + str(itemID),)))
            else:
                if getattr(slimItem, 'posTimestamp', None) is not None:
                    subMenu.append(('Complete State', sm.RemoteSvc('slash').SlashCmd, ('/sov complete ' + str(itemID),)))
                subMenu.append(('Offline', sm.RemoteSvc('slash').SlashCmd, ('/pos offline ' + str(itemID),)))
        if hasattr(slimItem, 'structureState') and slimItem.structureState != None and slimItem.structureState in [pos.STRUCTURE_SHIELD_REINFORCE, pos.STRUCTURE_ARMOR_REINFORCE]:
            subMenu.append(('Complete State', sm.RemoteSvc('slash').SlashCmd, ('/sov complete ' + str(itemID),)))
        return subMenu
*/
/*
 * class BasicOrbital(spaceObject.PlayerOwnedStructure):
 *
 *    def Assemble(self):
 *        self.SetStaticRotation()
 *
 *    def OnSlimItemUpdated(self, slimItem):
 *        orbitalState = getattr(slimItem, 'orbitalState', None)
 *        orbitalTimestamp = getattr(slimItem, 'orbitalTimestamp', blue.os.GetSimTime())
 *        fxSequencer = sm.GetService('FxSequencer')
 *        if not hasattr(self, 'orbitalState'):
 *            if orbitalState in (entities.STATE_ANCHORING, entities.STATE_ANCHORED):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorDrop', 0, 1, 0)
 *            elif orbitalState in (entities.STATE_IDLE, entities.STATE_OPERATING):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnlined', 0, 1, 0)
 *        else:
 *            if orbitalState == entities.STATE_ANCHORING and self.orbitalState == entities.STATE_UNANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorDrop', 0, 1, 0)
 *            if orbitalState == entities.STATE_ONLINING and self.orbitalState == entities.STATE_ANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnline', 0, 1, 0)
 *            if orbitalState == entities.STATE_IDLE and self.orbitalState == entities.STATE_ONLINING:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnlined', 0, 1, 0)
 *            if orbitalState == entities.STATE_ANCHORED and self.orbitalState in (entities.STATE_OFFLINING, entities.STATE_IDLE, entities.STATE_OPERATING):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOffline', 0, 1, 0)
 *            if orbitalState == entities.STATE_UNANCHORING and self.orbitalState == entities.STATE_ANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorLift', 0, 1, 0)
 *        self.orbitalState = orbitalState
 *        self.orbitalTimestamp = orbitalTimestamp
 *
 *    def IsAnchored(self):
 *        self.LogInfo('Anchor State = ', not self.isFree)
 *        return not self.isFree
 *
 *    def IsOnline(self):
 *        slimItem = sm.StartService('michelle').GetBallpark().GetInvItem(self.id)
 *        return slimItem.orbitalState is not None and slimItem.orbitalState in (entities.STATE_OPERATING, entities.STATE_IDLE)
 *
 */