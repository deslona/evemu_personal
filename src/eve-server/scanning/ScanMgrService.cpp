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

    PyCallable_REG_CALL(ScanMgrService, GetSystemScanMgr);

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

/*

	probeResultPerfect = 1.0
	probeResultInformative = 0.75
	probeResultGood = 0.25
	probeResultUnusable = 0.001


probeScanGroupScrap = 1
probeScanGroupSignatures = 4
probeScanGroupShips = 8
probeScanGroupStructures = 16
probeScanGroupDronesAndProbes = 32
probeScanGroupCelestials = 64
probeScanGroupAnomalies = 128
probeScanGroups = {}
probeScanGroups[probeScanGroupScrap] = set([groupBiomass,
 groupCargoContainer,
 groupWreck,
 groupSecureCargoContainer,
 groupAuditLogSecureContainer])
probeScanGroups[probeScanGroupSignatures] = set([groupCosmicSignature])
probeScanGroups[probeScanGroupAnomalies] = set([groupCosmicAnomaly])
probeScanGroups[probeScanGroupShips] = set([groupAssaultShip,
 groupBattlecruiser,
 groupBattleship,
 groupBlackOps,
 groupCapitalIndustrialShip,
 groupCapsule,
 groupCarrier,
 groupCombatReconShip,
 groupCommandShip,
 groupCovertOps,
 groupCruiser,
 groupDestroyer,
 groupDreadnought,
 groupElectronicAttackShips,
 groupEliteBattleship,
 groupExhumer,
 groupForceReconShip,
 groupFreighter,
 groupFrigate,
 groupHeavyAssaultShip,
 groupHeavyInterdictors,
 groupIndustrial,
 groupIndustrialCommandShip,
 groupInterceptor,
 groupInterdictor,
 groupJumpFreighter,
 groupLogistics,
 groupMarauders,
 groupMiningBarge,
 groupSupercarrier,
 groupPrototypeExplorationShip,
 groupRookieship,
 groupShuttle,
 groupStealthBomber,
 groupTitan,
 groupTransportShip,
 groupStrategicCruiser])
probeScanGroups[probeScanGroupStructures] = set([groupConstructionPlatform,
 groupStationUpgradePlatform,
 groupStationImprovementPlatform,
 groupMobileWarpDisruptor,
 groupAssemblyArray,
 groupControlTower,
 groupCorporateHangarArray,
 groupElectronicWarfareBattery,
 groupEnergyNeutralizingBattery,
 groupForceFieldArray,
 groupJumpPortalArray,
 groupLogisticsArray,
 groupMobileHybridSentry,
 groupMobileLaboratory,
 groupMobileLaserSentry,
 groupMobileMissileSentry,
 groupMobilePowerCore,
 groupMobileProjectileSentry,
 groupMobileReactor,
 groupMobileShieldGenerator,
 groupMobileStorage,
 groupMoonMining,
 groupRefiningArray,
 groupScannerArray,
 groupSensorDampeningBattery,
 groupShieldHardeningArray,
 groupShipMaintenanceArray,
 groupSilo,
 groupStasisWebificationBattery,
 groupStealthEmitterArray,
 groupTrackingArray,
 groupWarpScramblingBattery,
 groupCynosuralSystemJammer,
 groupCynosuralGeneratorArray,
 groupInfrastructureHub,
 groupSovereigntyClaimMarkers,
 groupSovereigntyDisruptionStructures,
 groupOrbitalConstructionPlatforms,
 groupPlanetaryCustomsOffices])
probeScanGroups[probeScanGroupDronesAndProbes] = set([groupCapDrainDrone,
 groupCombatDrone,
 groupElectronicWarfareDrone,
 groupFighterDrone,
 groupFighterBomber,
 groupLogisticDrone,
 groupMiningDrone,
 groupProximityDrone,
 groupRepairDrone,
 groupStasisWebifyingDrone,
 groupUnanchoringDrone,
 groupWarpScramblingDrone,
 groupScannerProbe,
 groupSurveyProbe,
 groupWarpDisruptionProbe])
probeScanGroups[probeScanGroupCelestials] = set([groupAsteroidBelt,
 groupForceField,
 groupMoon,
 groupPlanet,
 groupStargate,
 groupSun,
 groupStation])
 */