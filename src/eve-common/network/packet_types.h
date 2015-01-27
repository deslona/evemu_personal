/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Zhur
 *    Updates:       Allan
 */


#ifndef EVE_PACKET_TYPES_H
#define EVE_PACKET_TYPES_H

/*
 *
 * SERVICE_STOPPED = 1
 * SERVICE_START_PENDING = 2
 * SERVICE_STOP_PENDING = 3
 * SERVICE_RUNNING = 4
 * SERVICE_CONTINUE_PENDING = 5
 * SERVICE_PAUSE_PENDING = 6
 * SERVICE_PAUSED = 7
 * SERVICETYPE_NORMAL = 1
 * SERVICETYPE_BUILTIN = 2
 * SERVICETYPE_EXPORT_CONSTANTS = 4
 * SERVICE_CONTROL_STOP = 1
 * SERVICE_CONTROL_PAUSE = 2
 * SERVICE_CONTROL_CONTINUE = 3
 * SERVICE_CONTROL_INTERROGATE = 4
 * SERVICE_CONTROL_SHUTDOWN = 5
 * SERVICE_CHECK_NONE = 0
 * SERVICE_CHECK_CALL = 1
 * SERVICE_CHECK_INIT = 2
 * SERVICE_WANT_SESSIONS = 1
 * PRE_NONE = 0
 * PRE_AUTH = 1
 * PRE_HASCHAR = 2
 * PRE_HASSHIP = 4
 * PRE_INSTATION = 8
 * PRE_INFLIGHT = 16
 */

enum MACHONETMSG_TYPE
{
    AUTHENTICATION_REQ                = 0,
    AUTHENTICATION_RSP                = 1,
    IDENTIFICATION_REQ                = 2,
    IDENTIFICATION_RSP                = 3,
    __Fake_Invalid_Type                = 4,
    CALL_REQ                        = 6,
    CALL_RSP                        = 7,
    TRANSPORTCLOSED                    = 8,
    RESOLVE_REQ                        = 10,
    RESOLVE_RSP                        = 11,
    NOTIFICATION                    = 12,
    ERRORRESPONSE                    = 15,
    SESSIONCHANGENOTIFICATION        = 16,
    SESSIONINITIALSTATENOTIFICATION    = 18,
    PING_REQ                        = 20,
    PING_RSP                        = 21,
    MOVEMENTNOTIFICATION            = 100,
    MACHONETMSG_TYPE_COUNT
};

enum MACHONETERR_TYPE
{
    UNMACHODESTINATION = 0,
    UNMACHOCHANNEL = 1,
    WRAPPEDEXCEPTION = 2
};
//see PyPacket.cpp
extern const char* MACHONETMSG_TYPE_NAMES[MACHONETMSG_TYPE_COUNT];

//item types which we need to know about:
enum EVEItemType {
    AllianceTypeID = 16159
};

//these came from the 'constants' object:
enum EVEItemChangeType {
    ixItemID = 0,    //also ixLauncherCapacity?
    ixTypeID = 1,    //also ixLauncherUsed = 1,
    ixOwnerID = 2,    //also ixLauncherChargeItem?
    ixLocationID = 3,
    ixFlag = 4,
    ixContraband = 5,
    ixSingleton = 6,
    ixGroupID = 8,
    ixQuantity = 7,
    ixCategoryID = 9,
    ixCustomInfo = 10,
    ixSubitems = 11
};

enum EVEContainerTypes {
    containerWallet             = 10001,
    containerGlobal             = 10002,
    containerSolarSystem         = 10003,
    containerHangar             = 10004,
    containerScrapHeap            = 10005,
    containerFactory            = 10006,
    containerBank                 = 10007,
    containerRecycler            = 10008,
    containerOffices            = 10009,
    containerStationCharacters    = 10010,
    containerCharacter             = 10011,
    containerCorpMarket            = 10012
};

enum EVERookieShipTypes {
    amarrRookie                        = 596,
    caldariRookie                    = 601,
    gallenteRookie                    = 606,
    minmatarRookie                    = 588,
};

enum EVESkillEvent {
    skillEventCharCreation = 33,
    skillEventClonePenalty = 34,
    skillEventGMGive = 39,
    skillEventHaltedAccountLapsed = 260,
    skillEventTaskMaster = 35,
    skillEventTrainingCancelled = 38,
    skillEventTrainingComplete = 37,
    skillEventTrainingStarted = 36,
    skillEventQueueTrainingCompleted = 53,
    skillEventSkillInjected = 56,
    skillEventFreeSkillPointsUsed = 307,
    skillEventGMReverseFreeSkillPointsUsed = 309
};

#include "tables/invCategories.h"
typedef EVEDB::invCategories::invCategories EVEItemCategories;

//from invFlags DB table
typedef enum EVEItemFlags
{
    flagAutoFit                        = 0,
    flagWallet                        = 1,
    flagFactory                        = 2,
    flagHangar                        = 4,
    flagCargoHold                    = 5,
    flagBriefcase                    = 6,
    flagSkill                        = 7,
    flagReward                        = 8,
    flagConnected                    = 9,    //Character in station connected
    flagDisconnected                = 10,    //Character in station offline

    //ship fittings:
    flagLowSlot0                    = 11,    //Low power slot 1
    flagLowSlot1                    = 12,
    flagLowSlot2                    = 13,
    flagLowSlot3                    = 14,
    flagLowSlot4                    = 15,
    flagLowSlot5                    = 16,
    flagLowSlot6                    = 17,
    flagLowSlot7                    = 18,    //Low power slot 8

    flagMedSlot0                    = 19,    //Medium power slot 1
    flagMedSlot1                    = 20,
    flagMedSlot2                    = 21,
    flagMedSlot3                    = 22,
    flagMedSlot4                    = 23,
    flagMedSlot5                    = 24,
    flagMedSlot6                    = 25,
    flagMedSlot7                    = 26,    //Medium power slot 8

    flagHiSlot0                        = 27,    //High power slot 1
    flagHiSlot1                        = 28,
    flagHiSlot2                        = 29,
    flagHiSlot3                        = 30,
    flagHiSlot4                        = 31,
    flagHiSlot5                        = 32,
    flagHiSlot6                        = 33,
    flagHiSlot7                        = 34,    //High power slot 8
    flagFixedSlot                    = 35,

    //factory stuff:
    flagFactoryBlueprint            = 36,
    flagFactoryMinerals                = 37,
    flagFactoryOutput                = 38,
    flagFactoryActive                = 39,
    flagFactory_SlotFirst            = 40,
    flagFactory_SlotLast            = 55,

    flagCapsule                        = 56,    //Capsule item in space
    flagPilot                        = 57,
    flagPassenger                    = 58,
    flagBoardingGate                = 59,
    flagCrew                        = 60,
    flagSkillInTraining                = 61,
    flagCorpMarket                    = 62,    //Corporation Market Deliveries / Returns
    flagLocked                        = 63,    //Locked item, can not be moved unless unlocked
    flagUnlocked                    = 64,

    flagOfficeSlotFirst                = 70,
    flagOfficeSlotLast                = 85,

    flagBonus                        = 86,    //Char bonus/penalty
    flagDroneBay                    = 87,
    flagBooster                        = 88,
    flagImplant                        = 89,
    flagShipHangar                    = 90,
    flagShipOffline                    = 91,

    flagRigSlot0                    = 92,    //Rig power slot 1
    flagRigSlot1                    = 93,    //Rig power slot 2
    flagRigSlot2                    = 94,    //Rig power slot 3
    flagRigSlot3                    = 95,    //Rig power slot 4
    flagRigSlot4                    = 96,    //Rig power slot 5
    flagRigSlot5                    = 97,    //Rig power slot 6
    flagRigSlot6                    = 98,    //Rig power slot 7
    flagRigSlot7                    = 99,    //Rig power slot 8

    flagFactoryOperation            = 100,

    flagCorpSecurityAccessGroup2    = 116,
    flagCorpSecurityAccessGroup3    = 117,
    flagCorpSecurityAccessGroup4    = 118,
    flagCorpSecurityAccessGroup5    = 119,
    flagCorpSecurityAccessGroup6    = 120,
    flagCorpSecurityAccessGroup7    = 121,

    flagSecondaryStorage            = 122,    //Secondary Storage  (strontium bay on POS)
    flagCaptainsQuarters            = 123,    //Captains Quarters
    flagWisPromenade                = 124,    //Wis Promenade

    flagSubSystem0                    = 125,    //Sub system slot 0
    flagSubSystem1                    = 126,    //Sub system slot 1
    flagSubSystem2                    = 127,    //Sub system slot 2
    flagSubSystem3                    = 128,    //Sub system slot 3
    flagSubSystem4                    = 129,    //Sub system slot 4
    flagSubSystem5                    = 130,    //Sub system slot 5
    flagSubSystem6                    = 131,    //Sub system slot 6
    flagSubSystem7                    = 132,    //Sub system slot 7

    flagSpecializedFuelBay          = 133,
    flagSpecializedOreHold          = 134,
    flagSpecializedGasHold          = 135,
    flagSpecializedMineralHold      = 136,
    flagSpecializedSalvageHold      = 137,
    flagSpecializedShipHold         = 138,
    flagSpecializedSmallShipHold    = 139,

    flagFactorySlotFirst            = 140,
    //flagSpecializedMediumShipHold   = 140,

    flagSpecializedLargeShipHold    = 141,
    flagSpecializedIndustrialShipHold = 142,
    flagSpecializedAmmoHold         = 143,
    flagStructureActive             = 144,
    flagStructureInactive           = 145,
    flagJunkyardReprocessed         = 146,
    flagJunkyardTrashed             = 147,
    flagSpecializedCommandCenterHold = 148,
    flagSpecializedPlanetaryCommoditiesHold = 149,

    flagFactorySlotLast                = 195,

    flagResearchFacilitySlotFirst    = 200,
    flagResearchFacilitySlotLast    = 255,

    flagClone                        = 400,

    flagIllegal                     = 9999
} EVEItemFlags;

//for use in the new module manager
typedef enum {
    NaT                                = 0,
    slotTypeSubSystem                = 1,
    slotTypeRig                        = 2,
    slotTypeLowPower                = 3,
    slotTypeMedPower                = 4,
    slotTypeHiPower                    = 5

} EVEItemSlotType;

//some alternative names for entries above.
static const EVEItemFlags flagSlotFirst = flagLowSlot0;    //duplicate values
static const EVEItemFlags flagSlotLast = flagFixedSlot;
static const EVEItemFlags flagNone = flagAutoFit;

static const EVEItemFlags flagAnywhere = flagAutoFit;
static const uint8 MAX_MODULE_COUNT = flagSlotLast - flagSlotFirst + 1;
static const uint8 MAX_HIGH_SLOT_COUNT = flagHiSlot7 - flagHiSlot0 + 1;
static const uint8 MAX_MEDIUM_SLOT_COUNT = flagMedSlot7 - flagMedSlot0 + 1;
static const uint8 MAX_LOW_SLOT_COUNT = flagLowSlot7 - flagLowSlot0 + 1;
static const uint8 MAX_RIG_COUNT = flagRigSlot7 - flagRigSlot0 + 1;
static const uint8 MAX_ASSEMBLY_COUNT = flagSubSystem7 - flagSubSystem0 + 1;

#define FlagToSlot(flag) \
(flag - flagSlotFirst)
#define SlotToFlag(slot) \
((EVEItemFlags)(flagSlotFirst + slot))

/** Created with MYSQL query:
 * SELECT concat("\tskill", Replace(t.typeName, ' ', ''), "\t\t= ", t.typeID, ',\t\t// group = ', g.groupName)
 * FROM invTypes t, invGroups g, invCategories c
 * WHERE g.groupID = t.groupID AND c.categoryID = g.categoryID AND c.categoryID = 16
 * ORDER BY g.groupName
 */
//  -allan 21Mar14
typedef enum {  //thanks positron96 for this query
    skillCorporationManagement      = 3363,     // group = Corporation Management
    skillStationManagement      = 3364,     // group = Corporation Management
    skillStarbaseManagement     = 3365,     // group = Corporation Management
    skillFactoryManagement      = 3366,     // group = Corporation Management
    skillRefineryManagement     = 3367,     // group = Corporation Management
    skillEthnicRelations        = 3368,     // group = Corporation Management
    skillCFOTraining        = 3369,     // group = Corporation Management
    skillChiefScienceOfficer        = 3370,     // group = Corporation Management
    skillPublicRelations        = 3371,     // group = Corporation Management
    skillIntelligenceAnalyst        = 3372,     // group = Corporation Management
    skillStarbaseDefenseManagement      = 3373,     // group = Corporation Management
    skillMegacorpManagement     = 3731,     // group = Corporation Management
    skillEmpireControl      = 3732,     // group = Corporation Management
    skillAnchoring      = 11584,        // group = Corporation Management
    skillSovereignty        = 12241,        // group = Corporation Management
    skillDrones     = 3436,     // group = Drones
    skillScoutDroneOperation        = 3437,     // group = Drones
    skillMiningDroneOperation       = 3438,     // group = Drones
    skillRepairDroneOperation       = 3439,     // group = Drones
    skillSalvageDroneOperation      = 3440,     // group = Drones
    skillHeavyDroneOperation        = 3441,     // group = Drones
    skillDroneInterfacing       = 3442,     // group = Drones
    skillDroneNavigation        = 12305,        // group = Drones
    skillAmarrDroneSpecialization       = 12484,        // group = Drones
    skillMinmatarDroneSpecialization        = 12485,        // group = Drones
    skillGallenteDroneSpecialization        = 12486,        // group = Drones
    skillCaldariDroneSpecialization     = 12487,        // group = Drones
    skillTESTDroneSkill     = 22172,        // group = Drones
    skillMiningDroneSpecialization      = 22541,        // group = Drones
    skillFighters       = 23069,        // group = Drones
    skillElectronicWarfareDroneInterfacing      = 23566,        // group = Drones
    skillSentryDroneInterfacing     = 23594,        // group = Drones
    skillPropulsionJammingDroneInterfacing      = 23599,        // group = Drones
    skillDroneSharpshooting     = 23606,        // group = Drones
    skillDroneDurability        = 23618,        // group = Drones
    skillCombatDroneOperation       = 24241,        // group = Drones
    skillAdvancedDroneInterfacing       = 24613,        // group = Drones
    skillFighterBombers     = 32339,        // group = Drones
    skillElectronics        = 3426,     // group = Electronics
    skillElectronicWarfare      = 3427,     // group = Electronics
    skillLongRangeTargeting     = 3428,     // group = Electronics
    skillTargeting      = 3429,     // group = Electronics
    skillMultitasking       = 3430,     // group = Electronics
    skillSignatureAnalysis      = 3431,     // group = Electronics
    skillElectronicsUpgrades        = 3432,     // group = Electronics
    skillSensorLinking      = 3433,     // group = Electronics
    skillWeaponDisruption       = 3434,     // group = Electronics
    skillPropulsionJamming      = 3435,     // group = Electronics
    skillSurvey     = 3551,     // group = Electronics
    skillAdvancedSensorUpgrades     = 11208,        // group = Electronics
    skillCloaking       = 11579,        // group = Electronics
    skillHypereuclideanNavigation       = 12368,        // group = Electronics
    skillLongDistanceJamming        = 19759,        // group = Electronics
    skillFrequencyModulation        = 19760,        // group = Electronics
    skillSignalDispersion       = 19761,        // group = Electronics
    skillSignalSuppression      = 19766,        // group = Electronics
    skillTurretDestabilization      = 19767,        // group = Electronics
    skillTargetPainting     = 19921,        // group = Electronics
    skillSignatureFocusing      = 19922,        // group = Electronics
    skillCynosuralFieldTheory       = 21603,        // group = Electronics
    skillProjectedElectronicCounterMeasures     = 27911,        // group = Electronics
    skillTournamentObservation      = 28604,        // group = Electronics
    skillImperialNavySecurityClearance      = 28631,        // group = Electronics
    skillEngineering        = 3413,     // group = Engineering
    skillShieldOperation        = 3416,     // group = Engineering
    skillEnergySystemsOperation     = 3417,     // group = Engineering
    skillEnergyManagement       = 3418,     // group = Engineering
    skillShieldManagement       = 3419,     // group = Engineering
    skillTacticalShieldManipulation     = 3420,     // group = Engineering
    skillEnergyPulseWeapons     = 3421,     // group = Engineering
    skillShieldEmissionSystems      = 3422,     // group = Engineering
    skillEnergyEmissionSystems      = 3423,     // group = Engineering
    skillEnergyGridUpgrades     = 3424,     // group = Engineering
    skillShieldUpgrades     = 3425,     // group = Engineering
    skillAdvancedEnergyGridUpgrades     = 11204,        // group = Engineering
    skillAdvancedShieldUpgrades     = 11206,        // group = Engineering
    skillThermicShieldCompensation      = 11566,        // group = Engineering
    skillEMShieldCompensation       = 12365,        // group = Engineering
    skillKineticShieldCompensation      = 12366,        // group = Engineering
    skillExplosiveShieldCompensation        = 12367,        // group = Engineering
    skillShieldCompensation     = 21059,        // group = Engineering
    skillCapitalShieldOperation     = 21802,        // group = Engineering
    skillCapitalShieldEmissionSystems       = 24571,        // group = Engineering
    skillCapitalEnergyEmissionSystems       = 24572,        // group = Engineering
    skillStealthBombersFakeSkill        = 20127,        // group = Fake Skills
    skillGunnery        = 3300,     // group = Gunnery
    skillSmallHybridTurret      = 3301,     // group = Gunnery
    skillSmallProjectileTurret      = 3302,     // group = Gunnery
    skillSmallEnergyTurret      = 3303,     // group = Gunnery
    skillMediumHybridTurret     = 3304,     // group = Gunnery
    skillMediumProjectileTurret     = 3305,     // group = Gunnery
    skillMediumEnergyTurret     = 3306,     // group = Gunnery
    skillLargeHybridTurret      = 3307,     // group = Gunnery
    skillLargeProjectileTurret      = 3308,     // group = Gunnery
    skillLargeEnergyTurret      = 3309,     // group = Gunnery
    skillRapidFiring        = 3310,     // group = Gunnery
    skillSharpshooter       = 3311,     // group = Gunnery
    skillMotionPrediction       = 3312,     // group = Gunnery
    skillSurgicalStrike     = 3315,     // group = Gunnery
    skillControlledBursts       = 3316,     // group = Gunnery
    skillTrajectoryAnalysis     = 3317,     // group = Gunnery
    skillWeaponUpgrades     = 3318,     // group = Gunnery
    skillSmallRailgunSpecialization     = 11082,        // group = Gunnery
    skillSmallBeamLaserSpecialization       = 11083,        // group = Gunnery
    skillSmallAutocannonSpecialization      = 11084,        // group = Gunnery
    skillAdvancedWeaponUpgrades     = 11207,        // group = Gunnery
    skillSmallArtillerySpecialization       = 12201,        // group = Gunnery
    skillMediumArtillerySpecialization      = 12202,        // group = Gunnery
    skillLargeArtillerySpecialization       = 12203,        // group = Gunnery
    skillMediumBeamLaserSpecialization      = 12204,        // group = Gunnery
    skillLargeBeamLaserSpecialization       = 12205,        // group = Gunnery
    skillMediumRailgunSpecialization        = 12206,        // group = Gunnery
    skillLargeRailgunSpecialization     = 12207,        // group = Gunnery
    skillMediumAutocannonSpecialization     = 12208,        // group = Gunnery
    skillLargeAutocannonSpecialization      = 12209,        // group = Gunnery
    skillSmallBlasterSpecialization     = 12210,        // group = Gunnery
    skillMediumBlasterSpecialization        = 12211,        // group = Gunnery
    skillLargeBlasterSpecialization     = 12212,        // group = Gunnery
    skillSmallPulseLaserSpecialization      = 12213,        // group = Gunnery
    skillMediumPulseLaserSpecialization     = 12214,        // group = Gunnery
    skillLargePulseLaserSpecialization      = 12215,        // group = Gunnery
    skillCapitalEnergyTurret        = 20327,        // group = Gunnery
    skillCapitalHybridTurret        = 21666,        // group = Gunnery
    skillCapitalProjectileTurret        = 21667,        // group = Gunnery
    skillTacticalWeaponReconfiguration      = 22043,        // group = Gunnery
    skillIndustry       = 3380,     // group = Industry
    skillAmarrTech      = 3381,     // group = Industry
    skillCaldariTech        = 3382,     // group = Industry
    skillGallenteTech       = 3383,     // group = Industry
    skillMinmatarTech       = 3384,     // group = Industry
    skillRefining       = 3385,     // group = Industry
    skillMining     = 3386,     // group = Industry
    skillMassProduction     = 3387,     // group = Industry
    skillProductionEfficiency       = 3388,     // group = Industry
    skillRefineryEfficiency     = 3389,     // group = Industry
    skillMobileRefineryOperation        = 3390,     // group = Industry
    skillMobileFactoryOperation     = 3391,     // group = Industry
    skillDeepCoreMining     = 11395,        // group = Industry
    skillArkonorProcessing      = 12180,        // group = Industry
    skillBistotProcessing       = 12181,        // group = Industry
    skillCrokiteProcessing      = 12182,        // group = Industry
    skillDarkOchreProcessing        = 12183,        // group = Industry
    skillGneissProcessing       = 12184,        // group = Industry
    skillHedbergiteProcessing       = 12185,        // group = Industry
    skillHemorphiteProcessing       = 12186,        // group = Industry
    skillJaspetProcessing       = 12187,        // group = Industry
    skillKerniteProcessing      = 12188,        // group = Industry
    skillMercoxitProcessing     = 12189,        // group = Industry
    skillOmberProcessing        = 12190,        // group = Industry
    skillPlagioclaseProcessing      = 12191,        // group = Industry
    skillPyroxeresProcessing        = 12192,        // group = Industry
    skillScorditeProcessing     = 12193,        // group = Industry
    skillSpodumainProcessing        = 12194,        // group = Industry
    skillVeldsparProcessing     = 12195,        // group = Industry
    skillScrapmetalProcessing       = 12196,        // group = Industry
    skillIceHarvesting      = 16281,        // group = Industry
    skillIceProcessing      = 18025,        // group = Industry
    skillMiningUpgrades     = 22578,        // group = Industry
    skillSupplyChainManagement      = 24268,        // group = Industry
    skillAdvancedMassProduction     = 24625,        // group = Industry
    skillGasCloudHarvesting     = 25544,        // group = Industry
    skillDrugManufacturing      = 26224,        // group = Industry
    skillOreCompression     = 28373,        // group = Industry
    skillIndustrialReconfiguration      = 28585,        // group = Industry
    skillLeadership     = 3348,     // group = Leadership
    skillSkirmishWarfare        = 3349,     // group = Leadership
    skillSiegeWarfare       = 3350,     // group = Leadership
    skillSiegeWarfareSpecialist     = 3351,     // group = Leadership
    skillInformationWarfareSpecialist       = 3352,     // group = Leadership
    skillWarfareLinkSpecialist      = 3354,     // group = Leadership
    skillArmoredWarfareSpecialist       = 11569,        // group = Leadership
    skillSkirmishWarfareSpecialist      = 11572,        // group = Leadership
    skillWingCommand        = 11574,        // group = Leadership
    skillArmoredWarfare     = 20494,        // group = Leadership
    skillInformationWarfare     = 20495,        // group = Leadership
    skillMiningForeman      = 22536,        // group = Leadership
    skillMiningDirector     = 22552,        // group = Leadership
    skillFleetCommand       = 24764,        // group = Leadership
    skillMechanics      = 3392,     // group = Mechanics
    skillRepairSystems      = 3393,     // group = Mechanics
    skillHullUpgrades       = 3394,     // group = Mechanics
    skillFrigateConstruction        = 3395,     // group = Mechanics
    skillIndustrialConstruction     = 3396,     // group = Mechanics
    skillCruiserConstruction        = 3397,     // group = Mechanics
    skillBattleshipConstruction     = 3398,     // group = Mechanics
    skillOutpostConstruction        = 3400,     // group = Mechanics
    skillRemoteArmorRepairSystems       = 16069,        // group = Mechanics
    skillCapitalRepairSystems       = 21803,        // group = Mechanics
    skillCapitalShipConstruction        = 22242,        // group = Mechanics
    skillEMArmorCompensation        = 22806,        // group = Mechanics
    skillExplosiveArmorCompensation     = 22807,        // group = Mechanics
    skillKineticArmorCompensation       = 22808,        // group = Mechanics
    skillThermicArmorCompensation       = 22809,        // group = Mechanics
    skillCapitalRemoteArmorRepairSystems        = 24568,        // group = Mechanics
    skillSalvaging      = 25863,        // group = Mechanics
    skillJuryRigging        = 26252,        // group = Mechanics
    skillArmorRigging       = 26253,        // group = Mechanics
    skillAstronauticsRigging        = 26254,        // group = Mechanics
    skillDronesRigging      = 26255,        // group = Mechanics
    skillElectronicSuperiorityRigging       = 26256,        // group = Mechanics
    skillProjectileWeaponRigging        = 26257,        // group = Mechanics
    skillEnergyWeaponRigging        = 26258,        // group = Mechanics
    skillHybridWeaponRigging        = 26259,        // group = Mechanics
    skillLauncherRigging        = 26260,        // group = Mechanics
    skillShieldRigging      = 26261,        // group = Mechanics
    skillRemoteHullRepairSystems        = 27902,        // group = Mechanics
    skillTacticalLogisticsReconfiguration       = 27906,        // group = Mechanics
    skillCapitalRemoteHullRepairSystems     = 27936,        // group = Mechanics
    skillNaniteOperation        = 28879,        // group = Mechanics
    skillNaniteInterfacing      = 28880,        // group = Mechanics
    skillMissileLauncherOperation       = 3319,     // group = Missile Launcher Operation
    skillRockets        = 3320,     // group = Missile Launcher Operation
    skillStandardMissiles       = 3321,     // group = Missile Launcher Operation
    skillFoFMissiles        = 3322,     // group = Missile Launcher Operation
    skillDefenderMissiles       = 3323,     // group = Missile Launcher Operation
    skillHeavyMissiles      = 3324,     // group = Missile Launcher Operation
    skillTorpedoes      = 3325,     // group = Missile Launcher Operation
    skillCruiseMissiles     = 3326,     // group = Missile Launcher Operation
    skillMissileBombardment     = 12441,        // group = Missile Launcher Operation
    skillMissileProjection      = 12442,        // group = Missile Launcher Operation
    skillRocketSpecialization       = 20209,        // group = Missile Launcher Operation
    skillStandardMissileSpecialization      = 20210,        // group = Missile Launcher Operation
    skillHeavyMissileSpecialization     = 20211,        // group = Missile Launcher Operation
    skillCruiseMissileSpecialization        = 20212,        // group = Missile Launcher Operation
    skillTorpedoSpecialization      = 20213,        // group = Missile Launcher Operation
    skillGuidedMissilePrecision     = 20312,        // group = Missile Launcher Operation
    skillTargetNavigationPrediction     = 20314,        // group = Missile Launcher Operation
    skillWarheadUpgrades        = 20315,        // group = Missile Launcher Operation
    skillRapidLaunch        = 21071,        // group = Missile Launcher Operation
    skillCitadelTorpedoes       = 21668,        // group = Missile Launcher Operation
    skillHeavyAssaultMissileSpecialization      = 25718,        // group = Missile Launcher Operation
    skillHeavyAssaultMissiles       = 25719,        // group = Missile Launcher Operation
    skillBombDeployment     = 28073,        // group = Missile Launcher Operation
    skillCitadelCruiseMissiles      = 32435,        // group = Missile Launcher Operation
    skillNavigation     = 3449,     // group = Navigation
    skillAfterburner        = 3450,     // group = Navigation
    skillFuelConservation       = 3451,     // group = Navigation
    skillAccelerationControl        = 3452,     // group = Navigation
    skillEvasiveManeuvering     = 3453,     // group = Navigation
    skillHighSpeedManeuvering       = 3454,     // group = Navigation
    skillWarpDriveOperation     = 3455,     // group = Navigation
    skillJumpDriveOperation     = 3456,     // group = Navigation
    skillJumpFuelConservation       = 21610,        // group = Navigation
    skillJumpDriveCalibration       = 21611,        // group = Navigation
    skillAdvancedPlanetology        = 2403,     // group = Planet Management
    skillPlanetology        = 2406,     // group = Planet Management
    skillInterplanetaryConsolidation        = 2495,     // group = Planet Management
    skillCommandCenterUpgrades      = 2505,     // group = Planet Management
    skillRemoteSensing      = 13279,        // group = Planet Management
    skillScience        = 3402,     // group = Science
    skillResearch       = 3403,     // group = Science
    skillBiology        = 3405,     // group = Science
    skillLaboratoryOperation        = 3406,     // group = Science
    skillReverseEngineering     = 3408,     // group = Science
    skillMetallurgy     = 3409,     // group = Science
    skillAstrogeology       = 3410,     // group = Science
    skillCybernetics        = 3411,     // group = Science
    skillAstrometrics       = 3412,     // group = Science
    skillHighEnergyPhysics      = 11433,        // group = Science
    skillPlasmaPhysics      = 11441,        // group = Science
    skillNaniteEngineering      = 11442,        // group = Science
    skillHydromagneticPhysics       = 11443,        // group = Science
    skillAmarrianStarshipEngineering        = 11444,        // group = Science
    skillMinmatarStarshipEngineering        = 11445,        // group = Science
    skillGravitonPhysics        = 11446,        // group = Science
    skillLaserPhysics       = 11447,        // group = Science
    skillElectromagneticPhysics     = 11448,        // group = Science
    skillRocketScience      = 11449,        // group = Science
    skillGallenteanStarshipEngineering      = 11450,        // group = Science
    skillNuclearPhysics     = 11451,        // group = Science
    skillMechanicalEngineering      = 11452,        // group = Science
    skillElectronicEngineering      = 11453,        // group = Science
    skillCaldariStarshipEngineering     = 11454,        // group = Science
    skillQuantumPhysics     = 11455,        // group = Science
    skillAstronauticEngineering     = 11487,        // group = Science
    skillMolecularEngineering       = 11529,        // group = Science
    skillHypernetScience        = 11858,        // group = Science
    skillResearchProjectManagement      = 12179,        // group = Science
    skillArchaeology        = 13278,        // group = Science
    skillTalocanTechnology      = 20433,        // group = Science
    skillHacking        = 21718,        // group = Science
    skillSleeperTechnology      = 21789,        // group = Science
    skillCaldariEncryptionMethods       = 21790,        // group = Science
    skillMinmatarEncryptionMethods      = 21791,        // group = Science
    skillAmarrEncryptionMethods     = 23087,        // group = Science
    skillGallenteEncryptionMethods      = 23121,        // group = Science
    skillTakmahlTechnology      = 23123,        // group = Science
    skillYanJungTechnology      = 23124,        // group = Science
    skillInfomorphPsychology        = 24242,        // group = Science
    skillScientificNetworking       = 24270,        // group = Science
    skillJumpPortalGeneration       = 24562,        // group = Science
    skillDoomsdayOperation      = 24563,        // group = Science
    skillCloningFacilityOperation       = 24606,        // group = Science
    skillAdvancedLaboratoryOperation        = 24624,        // group = Science
    skillNeurotoxinRecovery     = 25530,        // group = Science
    skillNaniteControl      = 25538,        // group = Science
    skillAstrometricRangefinding        = 25739,        // group = Science
    skillAstrometricPinpointing     = 25810,        // group = Science
    skillAstrometricAcquisition     = 25811,        // group = Science
    skillThermodynamics     = 28164,        // group = Science
    skillDefensiveSubsystemTechnology       = 30324,        // group = Science
    skillEngineeringSubsystemTechnology     = 30325,        // group = Science
    skillElectronicSubsystemTechnology      = 30326,        // group = Science
    skillOffensiveSubsystemTechnology       = 30327,        // group = Science
    skillPropulsionSubsystemTechnology      = 30788,        // group = Science
    skillSocial     = 3355,     // group = Social
    skillNegotiation        = 3356,     // group = Social
    skillDiplomacy      = 3357,     // group = Social
    skillFastTalk       = 3358,     // group = Social
    skillConnections        = 3359,     // group = Social
    skillCriminalConnections        = 3361,     // group = Social
    skillDEDConnections     = 3362,     // group = Social
    skillMiningConnections      = 3893,     // group = Social
    skillDistributionConnections        = 3894,     // group = Social
    skillSecurityConnections        = 3895,     // group = Social
    skillOREIndustrial      = 3184,     // group = Spaceship Command
    skillSpaceshipCommand       = 3327,     // group = Spaceship Command
    skillGallenteFrigate        = 3328,     // group = Spaceship Command
    skillMinmatarFrigate        = 3329,     // group = Spaceship Command
    skillCaldariFrigate     = 3330,     // group = Spaceship Command
    skillAmarrFrigate       = 3331,     // group = Spaceship Command
    skillGallenteCruiser        = 3332,     // group = Spaceship Command
    skillMinmatarCruiser        = 3333,     // group = Spaceship Command
    skillCaldariCruiser     = 3334,     // group = Spaceship Command
    skillAmarrCruiser       = 3335,     // group = Spaceship Command
    skillGallenteBattleship     = 3336,     // group = Spaceship Command
    skillMinmatarBattleship     = 3337,     // group = Spaceship Command
    skillCaldariBattleship      = 3338,     // group = Spaceship Command
    skillAmarrBattleship        = 3339,     // group = Spaceship Command
    skillGallenteIndustrial     = 3340,     // group = Spaceship Command
    skillMinmatarIndustrial     = 3341,     // group = Spaceship Command
    skillCaldariIndustrial      = 3342,     // group = Spaceship Command
    skillAmarrIndustrial        = 3343,     // group = Spaceship Command
    skillGallenteTitan      = 3344,     // group = Spaceship Command
    skillMinmatarTitan      = 3345,     // group = Spaceship Command
    skillCaldariTitan       = 3346,     // group = Spaceship Command
    skillAmarrTitan     = 3347,     // group = Spaceship Command
    skillJoveFrigate        = 3755,     // group = Spaceship Command
    skillJoveCruiser        = 3758,     // group = Spaceship Command
    skillPolaris        = 9955,     // group = Spaceship Command
    skillConcord        = 10264,        // group = Spaceship Command
    skillJoveIndustrial     = 11075,        // group = Spaceship Command
    skillJoveBattleship     = 11078,        // group = Spaceship Command
    skillInterceptors       = 12092,        // group = Spaceship Command
    skillCovertOps      = 12093,        // group = Spaceship Command
    skillAssaultShips       = 12095,        // group = Spaceship Command
    skillLogistics      = 12096,        // group = Spaceship Command
    skillDestroyers     = 12097,        // group = Spaceship Command
    skillInterdictors       = 12098,        // group = Spaceship Command
    skillBattlecruisers     = 12099,        // group = Spaceship Command
    skillHeavyAssaultShips      = 16591,        // group = Spaceship Command
    skillMiningBarge        = 17940,        // group = Spaceship Command
    skillOmnipotent     = 19430,        // group = Spaceship Command
    skillTransportShips     = 19719,        // group = Spaceship Command
    skillAdvancedSpaceshipCommand       = 20342,        // group = Spaceship Command
    skillAmarrFreighter     = 20524,        // group = Spaceship Command
    skillAmarrDreadnought       = 20525,        // group = Spaceship Command
    skillCaldariFreighter       = 20526,        // group = Spaceship Command
    skillGallenteFreighter      = 20527,        // group = Spaceship Command
    skillMinmatarFreighter      = 20528,        // group = Spaceship Command
    skillCaldariDreadnought     = 20530,        // group = Spaceship Command
    skillGallenteDreadnought        = 20531,        // group = Spaceship Command
    skillMinmatarDreadnought        = 20532,        // group = Spaceship Command
    skillCapitalShips       = 20533,        // group = Spaceship Command
    skillExhumers       = 22551,        // group = Spaceship Command
    skillReconShips     = 22761,        // group = Spaceship Command
    skillCommandShips       = 23950,        // group = Spaceship Command
    skillAmarrCarrier       = 24311,        // group = Spaceship Command
    skillCaldariCarrier     = 24312,        // group = Spaceship Command
    skillGallenteCarrier        = 24313,        // group = Spaceship Command
    skillMinmatarCarrier        = 24314,        // group = Spaceship Command
    skillCapitalIndustrialShips     = 28374,        // group = Spaceship Command
    skillHeavyInterdictors      = 28609,        // group = Spaceship Command
    skillElectronicAttackShips      = 28615,        // group = Spaceship Command
    skillBlackOps       = 28656,        // group = Spaceship Command
    skillMarauders      = 28667,        // group = Spaceship Command
    skillJumpFreighters     = 29029,        // group = Spaceship Command
    skillIndustrialCommandShips     = 29637,        // group = Spaceship Command
    skillAmarrStrategicCruiser      = 30650,        // group = Spaceship Command
    skillCaldariStrategicCruiser        = 30651,        // group = Spaceship Command
    skillGallenteStrategicCruiser       = 30652,        // group = Spaceship Command
    skillMinmatarStrategicCruiser       = 30653,        // group = Spaceship Command
    skillAmarrDefensiveSystems      = 30532,        // group = Subsystems
    skillAmarrElectronicSystems     = 30536,        // group = Subsystems
    skillAmarrOffensiveSystems      = 30537,        // group = Subsystems
    skillAmarrPropulsionSystems     = 30538,        // group = Subsystems
    skillAmarrEngineeringSystems        = 30539,        // group = Subsystems
    skillGallenteDefensiveSystems       = 30540,        // group = Subsystems
    skillGallenteElectronicSystems      = 30541,        // group = Subsystems
    skillCaldariElectronicSystems       = 30542,        // group = Subsystems
    skillMinmatarElectronicSystems      = 30543,        // group = Subsystems
    skillCaldariDefensiveSystems        = 30544,        // group = Subsystems
    skillMinmatarDefensiveSystems       = 30545,        // group = Subsystems
    skillGallenteEngineeringSystems     = 30546,        // group = Subsystems
    skillMinmatarEngineeringSystems     = 30547,        // group = Subsystems
    skillCaldariEngineeringSystems      = 30548,        // group = Subsystems
    skillCaldariOffensiveSystems        = 30549,        // group = Subsystems
    skillGallenteOffensiveSystems       = 30550,        // group = Subsystems
    skillMinmatarOffensiveSystems       = 30551,        // group = Subsystems
    skillCaldariPropulsionSystems       = 30552,        // group = Subsystems
    skillGallentePropulsionSystems      = 30553,        // group = Subsystems
    skillMinmatarPropulsionSystems      = 30554,        // group = Subsystems
    skillTrade      = 3443,     // group = Trade
    skillRetail     = 3444,     // group = Trade
    skillBlackMarketTrading     = 3445,     // group = Trade
    skillBrokerRelations        = 3446,     // group = Trade
    skillVisibility     = 3447,     // group = Trade
    skillSmuggling      = 3448,     // group = Trade
    skillTest       = 11015,        // group = Trade
    skillGeneralFreight     = 12834,        // group = Trade
    skillStarshipFreight        = 13069,        // group = Trade
    skillMineralFreight     = 13070,        // group = Trade
    skillMunitionsFreight       = 13071,        // group = Trade
    skillDroneFreight       = 13072,        // group = Trade
    skillRawMaterialFreight     = 13073,        // group = Trade
    skillConsumableFreight      = 13074,        // group = Trade
    skillHazardousMaterialFreight       = 13075,        // group = Trade
    skillProcurement        = 16594,        // group = Trade
    skillDaytrading     = 16595,        // group = Trade
    skillWholesale      = 16596,        // group = Trade
    skillMarginTrading      = 16597,        // group = Trade
    skillMarketing      = 16598,        // group = Trade
    skillAccounting     = 16622,        // group = Trade
    skillTycoon     = 18580,        // group = Trade
    skillCorporationContracting     = 25233,        // group = Trade
    skillContracting        = 25235,        // group = Trade
    skillTaxEvasion     = 28261     // group = Trade
} EVESkillID;

//List of eve item types which have special purposes in the game.
//try to keep this list as short as possible, most things should be accomplish able
//by looking at the attributes of an item, not its type.
typedef enum {
    itemTypeCapsule = 670
} EVEItemTypeID;

//paperdoll state.  may be used later
typedef enum {
    paperdollStateNoRecustomization = 0,
    paperdollStateResculpting = 1,
    paperdollStateNoExistingCustomization = 2,
    paperdollStateFullRecustomizing = 3,
    paperdollStateForceRecustomize = 4
} paperDollState;

//userType id's used for accounts
typedef enum {
    userTypePlayer              = 1,        // i added this....not sure if it's right.
    userTypeCCP                 = 13,
    userTypePBC                 = 20,
    userTypeETC                 = 21,
    userTypeTrial               = 23,
    userTypeMammon              = 30,
    userTypeMedia               = 31,
    userTypeCDKey               = 33,
    userTypeIA                  = 34,

    userTypeDustPlayer          = 101,
    userTypeDustCCP             = 102,
    userTypeDustBattleServer    = 103
} EVEUserType;


//raceID as in table 'entity'
enum EVERace {
    raceCaldari     = 1,
    raceMinmatar    = 2,
    raceAmarr       = 4,
    raceGallente    = 8,
    raceJove        = 16,
    racePirate      = 32
};

//eve standing change messages
//If oFromID and oToID != fromID and toID, the following message is added (except for those marked with x):
//This standing change was initiated by a change from _oFromID towards _oToID
typedef enum {    //chrStandingChanges.eventTypeID
    standingGMInterventionReset             = 25,   //Reset by a GM.
    standingDecay                           = 49,   //All standing decays except when user isn't logged in
    standingPlayerSet                       = 65,   //Set by player him/herself. Reason: _msg
    standingCorpSet                         = 68,   //Corp stand set by _int1. Reason: _msg
    standingMissionCompleted                = 73,   //_msg: name of mission
    standingMissionFailure                  = 74,   //_msg: name of mission
    standingMissionDeclined                 = 75,   //_msg: name of mission
    standingCombatAggression                = 76,   //Combat - Aggression
    standingCombatShipKill                  = 77,   //Combat - Ship Kill
    standingCombatPodKill                   = 78,   //Combat - Pod Kill
    standingDerivedModificationPleased      = 82,   //_fromID Corp was pleased
    standingDerivedModificationDispleased   = 83,   //_fromID Corp was displeased
    standingGMInterventionDirect            = 84,   //Mod directly by _int1. Reason: _msg
    standingLawEnforcement                  = 89,   //Granted by Concord for actions against _int1
    standingMissionOfferExpired             = 90,   //Mission Offer Expired - _msg
    standingCombatAssistance                = 112,  //Combat - Assistance
    standingPropertyDamage                  = 154   //Property Damage
    //anything up until 500 is 'Standing Change'
} EVEStandingEventTypeID;

enum:uint64 {
    ROLE_DUST               = 1,
    ROLE_BANNING            = 2,
    ROLE_MARKET             = 4,
    ROLE_MARKETH            = 8,
    ROLE_CSMADMIN           = 16,
    ROLE_CSMDELEGATE        = 32,
    ROLE_EXPOPLAYER         = 64,
    ROLE_PETITIONEE         = 256,
    ROLE_CENTURION          = 2048,
    ROLE_WORLDMOD           = 4096,
    ROLE_DBA                = 16384,
    ROLE_REMOTESERVICE      = 131072,
    ROLE_LEGIONEER          = 262144,
    ROLE_TRANSLATION        = 524288,
    ROLE_CHTINVISIBLE       = 1048576,
    ROLE_CHTADMINISTRATOR   = 2097152,
    ROLE_HEALSELF           = 4194304,
    ROLE_HEALOTHERS         = 8388608,
    ROLE_NEWSREPORTER       = 16777216,
    ROLE_TRANSLATIONADMIN   = 134217728,
    ROLE_ACCOUNTMANAGEMENT  = 536870912,
    ROLE_SPAWN              = 8589934592,
    ROLE_IGB                = 2147483648,
    ROLE_TRANSLATIONEDITOR  = 4294967296,
    ROLE_BATTLESERVER       = 17179869184,
    ROLE_TRANSLATIONTESTER  = 34359738368,
    ROLE_WIKIEDITOR         = 68719476736,
    ROLE_TRANSFER           = 137438953472L,
    ROLE_GMS                = 274877906944L,
    ROLE_CL                 = 549755813888L,
    ROLE_CR                 = 1099511627776L,
    ROLE_CM                 = 2199023255552L,
    ROLE_BSDADMIN           = 35184372088832L,
    ROLE_PROGRAMMER         = 2251799813685248L,
    ROLE_QA                 = 4503599627370496L,
    ROLE_GMH                = 9007199254740992L,
    ROLE_GML                = 18014398509481984L,
    ROLE_CONTENT            = 36028797018963968L,
    ROLE_ADMIN              = 72057594037927936L,
    ROLE_VIPLOGIN           = 144115188075855872L,
    ROLE_ROLEADMIN          = 288230376151711744L,
    ROLE_NEWBIE             = 576460752303423488L,
    ROLE_SERVICE            = 1152921504606846976L,
    ROLE_PLAYER             = 2305843009213693952L,
    ROLE_LOGIN              = 4611686018427387904L,

    ROLE_ANY                = 18446744073709551615UL & ~ROLE_IGB,   // do NOT use.  PyLong is signed.
    ROLEMASK_ELEVATEDPLAYER = ROLE_ANY & ~(ROLE_LOGIN | ROLE_PLAYER | ROLE_NEWBIE | ROLE_VIPLOGIN),
    ROLEMASK_VIEW = ROLE_ADMIN | ROLE_CONTENT | ROLE_GML | ROLE_GMH | ROLE_QA,
    ROLE_SLASH = ROLE_GML | ROLE_LEGIONEER,
    ROLE_TRANSAM = ROLE_TRANSLATION | ROLE_TRANSLATIONADMIN | ROLE_TRANSLATIONEDITOR,
    ROLE_DEV = ROLE_CONTENT | ROLE_GMH | ROLE_QA | ROLE_PROGRAMMER | ROLE_SPAWN | ROLE_WORLDMOD
};

enum {
    corpRoleLocationTypeHQ = 1,
    corpRoleLocationTypeBase = 2,
    corpRoleLocationTypeOther = 3
};

typedef enum:uint64 {
    corpRoleDirector                        = 1,
    corpRolePersonnelManager                = 128,
    corpRoleAccountant                      = 256,
    corpRoleSecurityOfficer                 = 512,
    corpRoleFactoryManager                  = 1024,
    corpRoleStationManager                  = 2048,
    corpRoleAuditor                         = 4096,
    corpRoleHangarCanTake1                  = 8192,
    corpRoleHangarCanTake2                  = 16384,
    corpRoleHangarCanTake3                  = 32768,
    corpRoleHangarCanTake4                  = 65536,
    corpRoleHangarCanTake5                  = 131072,
    corpRoleHangarCanTake6                  = 262144,
    corpRoleHangarCanTake7                  = 524288,
    corpRoleHangarCanQuery1                 = 1048576,
    corpRoleHangarCanQuery2                 = 2097152,
    corpRoleHangarCanQuery3                 = 4194304,
    corpRoleHangarCanQuery4                 = 8388608,
    corpRoleHangarCanQuery5                 = 16777216,
    corpRoleHangarCanQuery6                 = 33554432,
    corpRoleHangarCanQuery7                 = 67108864,
    corpRoleAccountCanTake1                 = 134217728,
    corpRoleAccountCanTake2                 = 268435456,
    corpRoleAccountCanTake3                 = 536870912,
    corpRoleAccountCanTake4                 = 1073741824,
    corpRoleAccountCanTake5                 = 2147483648L,
    corpRoleAccountCanTake6                 = 4294967296L,
    corpRoleAccountCanTake7                 = 8589934592L,
    corpRoleDiplomat                        = 17179869184L,
    corpRoleEquipmentConfig                 = 2199023255552L,
    corpRoleContainerCanTake1               = 4398046511104L,
    corpRoleContainerCanTake2               = 8796093022208L,
    corpRoleContainerCanTake3               = 17592186044416L,
    corpRoleContainerCanTake4               = 35184372088832L,
    corpRoleContainerCanTake5               = 70368744177664L,
    corpRoleContainerCanTake6               = 140737488355328L,
    corpRoleContainerCanTake7               = 281474976710656L,
    corpRoleCanRentOffice                   = 562949953421312L,
    corpRoleCanRentFactorySlot              = 1125899906842624L,
    corpRoleCanRentResearchSlot             = 2251799813685248L,
    corpRoleJuniorAccountant                = 4503599627370496L,
    corpRoleStarbaseConfig                  = 9007199254740992L,
    corpRoleTrader                          = 18014398509481984L,
    corpRoleChatManager                     = 36028797018963968L,
    corpRoleContractManager                 = 72057594037927936L,
    corpRoleInfrastructureTacticalOfficer   = 144115188075855872L,
    corpRoleStarbaseCaretaker               = 288230376151711744L,
    corpRoleFittingManager                  = 576460752303423488L,
    corpRoleAll                             = 1152921504606846975L,

    //Some Combos
    corpRoleAllHangar   = corpRoleHangarCanTake1&corpRoleHangarCanTake2&corpRoleHangarCanTake3&corpRoleHangarCanTake4&corpRoleHangarCanTake5&corpRoleHangarCanTake6&corpRoleHangarCanTake7&corpRoleHangarCanQuery1&corpRoleHangarCanQuery2&corpRoleHangarCanQuery3&corpRoleHangarCanQuery4&corpRoleHangarCanQuery5&corpRoleHangarCanQuery6&corpRoleHangarCanQuery7,
    corpRoleAllAccount  = corpRoleJuniorAccountant&corpRoleAccountCanTake1&corpRoleAccountCanTake2&corpRoleAccountCanTake3&corpRoleAccountCanTake4&corpRoleAccountCanTake5&corpRoleAccountCanTake6&corpRoleAccountCanTake7,
    corpRoleAllContainer= corpRoleContainerCanTake1&corpRoleContainerCanTake2&corpRoleContainerCanTake3&corpRoleContainerCanTake4&corpRoleContainerCanTake5&corpRoleContainerCanTake6&corpRoleContainerCanTake7,
    corpRoleAllOffice   = corpRoleCanRentOffice&corpRoleCanRentFactorySlot&corpRoleCanRentResearchSlot,
    corpRoleAllStarbase = corpRoleStarbaseCaretaker&corpRoleStarbaseConfig
} CorpRoleFlags;

/*
 * corpactivityEducation = 18
 * corpactivityEntertainment = 8
 * corpactivityMilitary = 5
 * corpactivitySecurity = 16
 * corpactivityTrading = 12
 * corpactivityWarehouse = 10
 * corpDivisionDistribution = 22
 * corpDivisionMining = 23
 * corpDivisionSecurity = 24
 */

//from market_keyMap
typedef enum {
    accountingKeyCash           = 1000,
    accountingKeyCash2          = 1001,     //walletDivision2...
    accountingKeyCash3          = 1002,
    accountingKeyCash4          = 1003,
    accountingKeyCash5          = 1004,
    accountingKeyCash6          = 1005,
    accountingKeyCash7          = 1006,
    accountingKeyProperty       = 1100,
    accountingKeyAUR            = 1200,
    accountingKeyAUR2           = 1201,     //walletDivision2...
    accountingKeyAUR3           = 1202,
    accountingKeyAUR4           = 1203,
    accountingKeyAUR5           = 1204,
    accountingKeyAUR6           = 1205,
    accountingKeyAUR7           = 1206,
    accountingKeyEscrow         = 1500,
    accountingKeyReceivables    = 1800,
    accountingKeyPayables       = 2000,
    accountingKeyGold           = 2010,
    accountingKeyEquity         = 2900,
    accountingKeySales          = 3000,
    accountingKeyPurchases      = 4000
} EVEAccountKeys;

//these come from dgmEffects.
//  -allan 18Aug14
//  incomplete  3jan15
typedef enum {
    effectShieldBoosting                = 4,    //effects.ShieldBoosting
    effectSpeedBoost                    = 7,    //effects.SpeedBoost
    effectMissileLaunching              = 9,    //effects.MissileDeployment
    effectTargetAttack                  = 10,   //effects.Laser
    effectLoPower                       = 11,
    effectHiPower                       = 12,
    effectMedPower                      = 13,
    effectOnline                        = 16,
    effectMining                        = 17,   //effects.Mining
    effectShieldTransfer                = 18,   //effects.ShieldTransfer
    effectStructureRepair               = 26,   //effects.StructureRepair
    effectArmorRepair                   = 27,   //effects.ArmorRepair
    effectModifyTargetSpeed             = 29,   //effects.ModifyTargetSpeed
    effectEnergyTransfer                = 31,   //effects.EnergyTransfer
    effectEnergyVampire                 = 32,   //effects.EnergyDestabilization
    effectProjectileFired               = 34,   //effects.ProjectileFired
    effectEnergyDestabilization         = 36,   //effects.EnergyDestabilization
    effectEMPWave                       = 38,   //effects.EMPWave
    effectWarpScramble                  = 39,   //effects.WarpScramble
    effectCargoScan                     = 47,   //effects.CargoScan
    effectECMBurst                      = 53,   //effects.ECMBurst
    effectMiningLaser                   = 67,   //effects.Laser
    effectEmpFieldRange                 = 99,
    effectSurveyScan                    = 81,   //effects.SurveyScan
    effectFoFMissileLaunching           = 104,  //effects.MissileDeployment
    effectTurretWeaponRangeTrackingSpeedMultiplyActivate        = 123,  //effects.TurretWeaponRangeTrackingSpeedMultiplyActi...
    effectScanStrengthBonusTarget       = 124,  //effects.ScanStrengthBonusTarget
    effectTurretWeaponRangeTrackingSpeedMultiplyTarget          = 126,  //effects.TurretWeaponRangeTrackingSpeedMultiplyTarg...
    effectTorpedoLaunching              = 127,  //effects.TorpedoDeployment
    effectSkillEffect                   = 132,
    effectBarrage                       = 263,  //effects.Barrage
    effectWarpScrambleForEntity         = 563,  //effects.WarpScramble
    effectModifyTargetSpeed2            = 575,  //effects.ModifyTargetSpeed
    effectDecreaseTargetSpeed           = 586,  //effects.ModifyTargetSpeed
    effectTargetArmorRepair             = 592,  //effects.RemoteArmourRepair
    effectTargetedEMResonanceMultiply   = 597,  //effects.EnergyDestabilization
    effectCloaking                      = 607,  //effects.Cloaking
    effectTurretWeaponRangeTrackingSpeedMultiplyTargetHostile   = 609,  //effects.ElectronicAttributeModifyTarget
    effectTargetedKineticResonanceMultiply          = 615,  //effects.EnergyDestabilization
    effectTargetedThermalResonanceMultiply          = 616,  //effects.EnergyDestabilization
    effectTargetedExplosiveResonanceMultiply        = 617,  //effects.EnergyDestabilization
    effectAnchorDrop                    = 649,  //effects.AnchorDrop
    effectAnchorLift                    = 650,  //effects.AnchorLift
    effectStealthActive                 = 713,  //effects.ElectronicAttributeModifyActivate
    effectSensorBoostTargeted           = 716,  //effects.ElectronicAttributeModifyTarget
    effectSensorBoosterActive           = 720,  //effects.ElectronicAttributeModifyActivate
    effectSensorBoostTargetedHostile    = 837,  //effects.ElectronicAttributeModifyTarget
    effectShieldBoostingForEntities     = 876,  //effects.ShieldBoosting
    effectArmorRepairForEntities        = 878,  //effects.ArmorRepair
    effectSuicideBomb                   = 885,  //effects.EMPWave
    effectOnlineForStructures           = 901,  //effects.StructureOnline
    effectDecloakWave                   = 902,  //effects.DecloakWave
    effectCloakingWarpSafe              = 980,  //effects.Cloaking
    effectAnchorDropForStructures       = 1022, //effects.AnchorDrop
    effectAnchorLiftForStructures       = 1023, //effects.AnchorLift
    effectProjectileFiredForEntities    = 1086, //effects.ProjectileFiredForEntities
    effectTargetAttackForStructures     = 1199, //effects.ProjectileFiredForEntities
    effectNewEwTestscanStrengthBonusTargetHostile   = 1271, //effects.ScanStrengthBonusTarget
    effectNewEwTestswarpScramble        = 1272, //effects.WarpScramble
    effectNewEwTestssensorBoostTargetedHostile      = 1273, //effects.ElectronicAttributeModifyTarget
    effectNewEwTeststurretWeaponRangeTrackingSpeedMultiplyTargetHostile         = 1274, //effects.Target_paint
    effectNewEwTestsdecreaseTargetSpeed = 1275, //effects.ModifyTargetSpeed
    effectEWTestEffectRsd               = 1354, //effects.ElectronicAttributeModifyTarget
    effectEWTestEffectWs                = 1355, //effects.WarpScramble
    effectGangBonusSignature            = 1411, //effects.ElectronicAttributeModifyActivate
    effectGangArmorHardening            = 1510, //effects.ElectronicAttributeModifyActivate
    effectGangPropulsionJammingBoost    = 1546, //effects.ElectronicAttributeModifyActivate
    effectGangShieldHardening           = 1548, //effects.ElectronicAttributeModifyActivate
    effectEWTargetPaint                 = 1549, //effects.TargetPaint
    effectGangECCMfixed                 = 1648, //effects.ElectronicAttributeModifyActivate
    effectOpenSpawnContainer            = 1738, //effects.TargetScan
    effectSiegeModeEffectOld            = 1745, //effects.SiegeMode
    effectGangArmorRepairSpeedAmplifier = 1746, //effects.ElectronicAttributeModifyActivate
    effectGangArmorRepairCapReducer     = 1747, //effects.ElectronicAttributeModifyActivate
    effectEntityCapacitorDrain          = 1872, //effects.EnergyVampire
    effectEntityTrackingDisrupt         = 1877, //effects.ElectronicAttributeModifyTarget
    effectEntitySensorDampen            = 1878, //effects.ElectronicAttributeModifyTarget
    effectSetActiveDamageResonanceMultiplier    = 1938, //effects.ModifyShieldResonance
    effecteEMPWaveGrid                  = 2071, //effects.EMPWaveGrid
    effectModifyActiveArmorResonanceAndNullifyPassiveResonance  = 2098, //effects.ArmorHardening
    effectModifyActiveShieldResonanceAndNullifyPassiveResonance = 2118, //effects.ModifyShieldResonance
    effectJumpPortalGeneration          = 2152, //effects.JumpPortal
    effectEntityShieldBoostingSmall     = 2192, //effects.ShieldBoosting
    effectEntityShieldBoostingMedium    = 2193, //effects.ShieldBoosting
    effectEntityShieldBoostingLarge     = 2194, //effects.ShieldBoosting
    effectEntityArmorRepairingSmall     = 2195, //effects.ArmorRepair
    effectEntityArmorRepairingMedium    = 2196, //effects.ArmorRepair
    effectEntityArmorRepairingLarge     = 2197, //effects.ArmorRepair
    effectScanStrengthBonusPercentActivate      = 2231, //effects.ScanStrengthBonusActivate
    effectTractorBeam                   = 2255, //effects.TractorBeam
    effectScanStrengthBonusPercentPassive       = 2298, //effects.ScanStrengthBonusActivate
    effectEnergyDestabilizationNew      = 2303, //effects.EnergyDestabilization
    effectEnergyNosferatu               = 2304, //effects.EnergyVampire
    effectSnowBallLaunching             = 2413, //effects.MissileDeployment
    effectDecreaseTargetSpeedForStructures      = 2480, //effects.ModifyTargetSpeed
    effectWarpScrambleForStructure      = 2481, //effects.WarpScramble
    effectTorpedoLaunchingIsOffensive   = 2576, //effects.TorpedoDeployment
    effectEntityEnvironmentalEffectDamageTest   = 2662, //effects.EMPWave
    effectSensorBoosterActivePercentage = 2670, //effects.ElectronicAttributeModifyActivate
    effectMiningClouds                  = 2726, //effects.CloudMining
    effectSalvaging                     = 2757, //effects.Salvaging
    effectEnergyDestabilizationNewForStructure  = 2912, //effects.EnergyDestabilization
    effectRemoteEcmBurst                = 2913, //effects.RemoteECM
    effectBombLaunching                 = 2971, //effects.MissileDeployment
    effectEnergyDestabilizationForStructure     = 3003, //effects.EnergyDestabilization
    effectRemoteHullRepair              = 3041, //effects.RemoteArmourRepair
    effectTriageModeEffectWithoutECMBurst       = 3045, //effects.TriageMode
    effectSiegeModeEffect               = 3062, //effects.SiegeMode
    effectSensorBoostTargetedHostileKali2Test   = 3161, //effects.ElectronicAttributeModifyTarget
    effectTriageModeEffect              = 3162, //effects.TriageMode
    effectGangArmorRepairCapReducerSelfAndProjected         = 3165, //effects.ElectronicAttributeModifyActivate
    effectGangArmorRepairSpeedAmplifierSelfAndProjected     = 3167, //effects.ElectronicAttributeModifyActivate
    effectLeech                         = 3250, //effects.EnergyVampire
    effectIndustrialCoreEffectOLD       = 3282, //effects.SiegeMode
    effectLeechNpc                      = 3332, //effects.EnergyVampire
    effectWarpDisruptSphere             = 3380, //effects.WarpDisruptFieldGenerating
    effectIndustrialCoreEffect          = 3492, //effects.SiegeMode
    effectTargetTurretWeaponMaxRangeAndTrackingSpeedBonusHostile    = 3552, //effects.ElectronicAttributeModifyTarget
    effectTargetGunneryMaxRangeAndTrackingSpeedBonusHostile         = 3555, //effects.ElectronicAttributeModifyTarget
    effectTargetGunneryMaxRangeAndTrackingSpeedBonusAssistance      = 3556, //effects.TurretWeaponRangeTrackingSpeedMultiplyTarg...
    effectGunneryMaxRangeAndTrackingSpeedBonus                      = 3559, //effects.TurretWeaponRangeTrackingSpeedMultiplyActi...
    effectTargetMaxTargetRangeAndScanResolutionBonusAssistance      = 3583, //effects.ElectronicAttributeModifyTarget
    effectTargetMaxTargetRangeAndScanResolutionBonusHostile         = 3584, //effects.ElectronicAttributeModifyTarget
    effectTargetSetWarpScrambleStatusHidden                         = 3604, //effects.WarpScramble
    effectJumpPortalGenerationBO        = 3674, //effects.JumpPortalBO
    effectTargetGunneryMaxRangeAndTrackingSpeedAndFalloffBonusHostile       = 3690, //effects.ElectronicAttributeModifyTarget
    effectTurretWeaponRangeFalloffTrackingSpeedMultiplyTargetHostile        = 3697, //effects.ElectronicAttributeModifyTarget
    effectConcordWarpScramble           = 3713, //effects.WarpScramble
    effectConcordModifyTargetSpeed      = 3714, //effects.ModifyTargetSpeed
    effectWarpScrambleTargetMWDBlockActivation                      = 3725, //effects.WarpScramble
    effectProbeLaunching                = 3793, //effects.MissileDeployment
    effectNPCRemoteArmorRepair          = 3852, //effects.RemoteArmourRepair
    effectNPCRemoteShieldBoost          = 3855, //effects.ShieldTransfer
    effectSuperWeaponTurret             = 4481, //effects.AnchorLift
    effectSuperWeaponAmarr              = 4489, //effects.SuperWeaponAmarr
    effectSuperWeaponCaldari            = 4490, //effects.SuperWeaponCaldari
    effectSuperWeaponGallente           = 4491, //effects.SuperWeaponGallente
    effectSuperWeaponMinmatar           = 4492, //effects.SuperWeaponMinmatar
    effectGunneryMaxRangeFalloffTrackingSpeedBonus                  = 4559, //effects.TurretWeaponRangeTrackingSpeedMultiplyActi...
    effectTargetGunneryMaxRangeFalloffTrackingSpeedBonusAssistance  = 4560, //effects.TurretWeaponRangeTrackingSpeedMultiplyTarg...
    effectSiegeModeEffect3              = 4568, //effects.SiegeMode
    effectSiegeModeEffect4              = 4573, //effects.SiegeMode
    effectTriageModeEffect2             = 4574, //effects.TriageMode
    effectIndustrialCoreEffect2         = 4575, //effects.SiegeMode
    effectNPCGroupShieldAssist          = 4686, //effects.ElectronicAttributeModifyActivate
    effectNPCGroupSpeedAssist           = 4687, //effects.ElectronicAttributeModifyActivate
    effectNPCGroupPropJamAssist         = 4688, //effects.ElectronicAttributeModifyActivate
    effectNPCGroupArmorAssist           = 4689, //effects.ElectronicAttributeModifyActivate
    effectFighterMissile                = 4729, //effects.Lasereffect
} EVEEffectID;

//  -allan 20Dec14
typedef enum {
    refSkipLog = -1,
    refUndefined = 0,
    refPlayerTrading = 1,
    refMarketTransaction = 2,
    refGMCashTransfer = 3,
    refATMWithdraw = 4,
    refATMDeposit = 5,
    refBackwardCompatible = 6,
    refMissionReward = 7,
    refCloneActivation = 8,
    refInheritance = 9,
    refPlayerDonation = 10,
    refCorporationPayment = 11,
    refDockingFee = 12,
    refOfficeRentalFee = 13,
    refFactorySlotRentalFee = 14,
    refRepairBill = 15,
    refBounty = 16,
    refBountyPrize = 17,
    refInsurance = 19,
    refMissionExpiration = 20,
    refMissionCompletion = 21,
    refShares = 22,
    refCourierMissionEscrow = 23,
    refMissionCost = 24,
    refAgentMiscellaneous = 25,
    refPaymentToLPStore = 26,
    refAgentLocationServices = 27,
    refAgentDonation = 28,
    refAgentSecurityServices = 29,
    refAgentMissionCollateralPaid = 30,
    refAgentMissionCollateralRefunded = 31,
    refAgentMissionReward = 33,
    refAgentMissionTimeBonusReward = 34,
    refCSPA = 35,
    refCSPAOfflineRefund = 36,
    refCorporationAccountWithdrawal = 37,
    refCorporationDividendPayment = 38,
    refCorporationRegistrationFee = 39,
    refCorporationLogoChangeCost = 40,
    refReleaseOfImpoundedProperty = 41,
    refMarketEscrow = 42,
    refMarketFinePaid = 44,
    refBrokerfee = 46,
    refAllianceRegistrationFee = 48,
    refWarFee = 49,
    refAllianceMaintainanceFee = 50,
    refContrabandFine = 51,
    refCloneTransfer = 52,
    refAccelerationGateFee = 53,
    refTransactionTax = 54,
    refJumpCloneInstallatio,nFee = 55,
    refManufacturing = 56,
    refResearchingTechnology = 57,
    refResearchingTimeProductivity = 58,
    refResearchingMaterialProductivity = 59,
    refCopying = 60,
    refDuplicating = 61,
    refReverseEngineering = 62,
    refContractAuctionBid = 63,
    refContractAuctionBidRefund = 64,
    refContractCollateral = 65,
    refContractRewardRefund = 66,
    refContractAuctionSold = 67,
    refContractReward = 68,
    refContractCollateralRefund = 69,
    refContractCollateralPayout = 70,
    refContractPrice = 71,
    refContractBrokersFee = 72,
    refContractSalesTax = 73,
    refContractDeposit = 74,
    refContractDepositSalesTax = 75,
    refSecureEVETimeCodeExchange = 76,
    refContractAuctionBidCorp = 77,
    refContractCollateralCorp = 78,
    refContractPriceCorp = 79,
    refContractBrokersFeeCorp = 80,
    refContractDepositCorp = 81,
    refContractDepositRefund = 82,
    refContractRewardAdded = 83,
    refContractRewardAddedCorp = 84,
    refBountyPrizes = 85,
    refCorporationAdvertisementFee = 86,
    refMedalCreation = 87,
    refMedalIssuing = 88,
    refAttributeRespecification = 90,
    refSovereignityRegistrarFee = 91,
    refCorporationTaxNpcBounties = 92,
    refCorporationTaxAgentRewards = 93,
    refCorporationTaxAgentBonusRewards = 94,
    refSovereignityUpkeepAdjustment = 95,
    refPlanetaryImportTax = 96,
    refPlanetaryExportTax = 97,
    refPlanetaryConstruction = 98,
    refRewardManager = 99,
    refBountySurcharge = 101,
    refContractReversal = 102,
    refCorporationTaxRewards = 103,
    refStorePurchase = 106,
    refStoreRefund = 107,
    refPlexConversion = 108,
    refAurumGiveAway = 109,
    refAurumTokenConversion = 111,
    refMaxEve = 10000
} JournalRefType;

//  -allan 7Jul14
typedef enum {
    Scrap           = 1,
    Signatures      = 4,
    Ships           = 8,
    Structures      = 16,
    DronesAndProbes = 32,
    Celestials      = 64,
    Anomalies       = 128
} ProbeScanGroup;

/*
 * //  -allan 11Jan15
 * typedef enum {
 *    probeResultPerfect = 1.0f,
 *    probeResultInformative = 0.75f,
 *    probeResultGood = 0.25f,
 *    probeResultUnusable = 0.001f
 * } ProbeResult;
 */
//  -allan 7Jul14
typedef enum {
    Inactive    = 0,
    Idle        = 1,
    Moving      = 2,
    Warping     = 3,
    Scanning    = 4,
    Returning   = 5
} ProbeState;

//  -allan 7Jul14
typedef enum {
    RegionMin           = 11000000,
    RegionMax           = 11999999,
    ConstellationMin    = 21000000,
    ConstellationMax    = 21999999,
    SystemMin           = 31000000,
    SystemMax           = 31999999
} mapWormholeValues;

//  -allan 7Jul14
typedef enum {
    MissionAllocated    = 0,
    MissionOffered      = 1,
    MissionAccepted     = 2,
    MissionFailed       = 3
} MissionState;

//  -allan 7Jul14
typedef enum {
    DungeonStarted      = 0,
    DungeonCompleted    = 1,
    DungeonFailed       = 2
} DungeonState;

/** misc costs
 * costCloneContract = 5600
 * costJumpClone = 100000
 */

//  -allan 7Jul14
typedef enum {
    Docking     = 3000,
    Jumping     = 5000,
    WarpingOut  = 5000,
    WarpingIn   = 10000,
    Undocking   = 30000,
    Restoring   = 60000
} InvulTimer;
//  -allan 11Jan15

typedef enum {
    STRUCTURE_UNANCHORED        = 0,
    STRUCTURE_ANCHORED          = 1,
    STRUCTURE_ONLINING          = 2,
    STRUCTURE_REINFORCED        = 3,
    STRUCTURE_ONLINE            = 4,
    STRUCTURE_OPERATING         = 5,
    STRUCTURE_VULNERABLE        = 6,
    STRUCTURE_SHIELD_REINFORCE  = 7,
    STRUCTURE_ARMOR_REINFORCE   = 8,
    STRUCTURE_INVULNERABLE      = 9
    /*
     *    pwnStructureStateAnchored = 'anchored',
     *    pwnStructureStateAnchoring = 'anchoring',
     *    pwnStructureStateOnline = 'online',
     *    pwnStructureStateOnlining = 'onlining',
     *    pwnStructureStateUnanchored = 'unanchored',
     *    pwnStructureStateUnanchoring = 'unanchoring',
     *    pwnStructureStateVulnerable = 'vulnerable',
     *    pwnStructureStateInvulnerable = 'invulnerable',
     *    pwnStructureStateReinforced = 'reinforced',
     *    pwnStructureStateOperating = 'operating',
     *    pwnStructureStateIncapacitated = 'incapacitated',
     *    pwnStructureStateAnchor = 'anchor',
     *    pwnStructureStateUnanchor = 'unanchor',
     *    pwnStructureStateOffline = 'offline',
     *    pwnStructureStateOnlineActive = 'online - active',
     *    pwnStructureStateOnlineStartingUp = 'online - starting up'
     */
} POSState;

typedef enum {
    STATE_OFFLINING             = -7,
    STATE_ANCHORING             = -6,
    STATE_ONLINING              = -5,
    STATE_ANCHORED              = -4,
    STATE_UNANCHORING           = -3,
    STATE_UNANCHORED            = -2,
    STATE_INCAPACITATED         = -1,
    STATE_IDLE                  = 0,
    STATE_COMBAT                = 1,
    STATE_MINING                = 2,
    STATE_APPROACHING           = 3,
    STATE_DEPARTING             = 4,
    STATE_DEPARTING_2           = 5,
    STATE_PURSUIT               = 6,
    STATE_FLEEING               = 7,
    STATE_REINFORCED            = 8,
    STATE_OPERATING             = 9,
    STATE_ENGAGE                = 10,
    STATE_VULNERABLE            = 11,
    STATE_SHIELD_REINFORCE      = 12,
    STATE_ARMOR_REINFORCE       = 13,
    STATE_INVULNERABLE          = 14,
    STATE_WARPAWAYANDDIE        = 15,
    STATE_WARPAWAYANDCOMEBACK   = 16,
    STATE_WARPTOPOSITION        = 17
} StructureState;

/*
 * planetResourceScanDistance = 1000000000
 * planetResourceProximityDistant = 0
 * planetResourceProximityRegion = 1
 * planetResourceProximityConstellation = 2
 * planetResourceProximitySystem = 3
 * planetResourceProximityPlanet = 4
 * planetResourceProximityLimits = [(2, 6),
 * (4, 10),
 * (6, 15),
 * (10, 20),
 * (15, 30)]
 * planetResourceScanningRanges = [9.0,
 * 7.0,
 * 5.0,
 * 3.0,
 * 1.0]
 * planetResourceUpdateTime = 1 * HOUR
 * planetResourceMaxValue = 1.21
 */

/*
 * mailingListBlocked = 0
 * mailingListAllowed = 1
 * mailingListMemberMuted = 0
 * mailingListMemberDefault = 1
 * mailingListMemberOperator = 2
 * mailingListMemberOwner = 3
 * ALLIANCE_SERVICE_MOD = 200
 * CHARNODE_MOD = 64
 * PLANETARYMGR_MOD = 128
 * mailTypeMail = 1
 * mailTypeNotifications = 2
 * mailStatusMaskRead = 1
 * mailStatusMaskReplied = 2
 * mailStatusMaskForwarded = 4
 * mailStatusMaskTrashed = 8
 * mailStatusMaskDraft = 16
 * mailStatusMaskAutomated = 32
 * mailLabelInbox = 1
 * mailLabelSent = 2
 * mailLabelCorporation = 4
 * mailLabelAlliance = 8
 * mailLabelsSystem = mailLabelInbox + mailLabelSent + mailLabelCorporation + mailLabelAlliance
 * mailMaxRecipients = 50
 * mailMaxGroups = 1
 * mailMaxSubjectSize = 150
 * mailMaxBodySize = 8000
 * mailMaxTaggedBodySize = 10000
 * mailMaxLabelSize = 40
 * mailMaxNumLabels = 25
 * mailMaxPerPage = 100
 * mailTrialAccountTimer = 1
 * mailMaxMessagePerMinute = 5
 * mailinglistMaxMembers = 3000
 * mailinglistMaxMembersUpdated = 1000
 * mailingListMaxNameSize = 60
 */


/*
 * typedef enum {
 * 3018681,
 * 3018821,
 * 3018822,
 * 3018823,
 * 3018824,
 * 3018680,
 * 3018817,
 * 3018818,
 * 3018819,
 * 3018820,
 * 3018682,
 * 3018809,
 * 3018810,
 * 3018811,
 * 3018812,
 * 3018678,
 * 3018837,
 * 3018838,
 * 3018839,
 * 3018840,
 * 3018679,
 * 3018841,
 * 3018842,
 * 3018843,
 * 3018844,
 * 3018677,
 * 3018845,
 * 3018846,
 * 3018847,
 * 3018848,
 * 3018676,
 * 3018825,
 * 3018826,
 * 3018827,
 * 3018828,
 * 3018675,
 * 3018805,
 * 3018806,
 * 3018807,
 * 3018808,
 * 3018672,
 * 3018801,
 * 3018802,
 * 3018803,
 * 3018804,
 * 3018684,
 * 3018829,
 * 3018830,
 * 3018831,
 * 3018832,
 * 3018685,
 * 3018813,
 * 3018814,
 * 3018815,
 * 3018816,
 * 3018683,
 * 3018833,
 * 3018834,
 * 3018835,
 * 3018836]
 * }rookieAgentList;
 */
/*
auraAgentIDs = [
 3019499,
 3019493,
 3019495,
 3019490,
 3019497,
 3019496,
 3019486,
 3019498,
 3019492,
 3019500,
 3019489,
 3019494]
 */

/*
agentRangeSameSystem = 1
agentRangeSameOrNeighboringSystemSameConstellation = 2
agentRangeSameOrNeighboringSystem = 3
agentRangeNeighboringSystemSameConstellation = 4
agentRangeNeighboringSystem = 5
agentRangeSameConstellation = 6
agentRangeSameOrNeighboringConstellationSameRegion = 7
agentRangeSameOrNeighboringConstellation = 8
agentRangeNeighboringConstellationSameRegion = 9
agentRangeNeighboringConstellation = 10
agentRangeNearestEnemyCombatZone = 11
agentRangeNearestCareerHub = 12
agentIskMultiplierLevel1 = 1
agentIskMultiplierLevel2 = 2
agentIskMultiplierLevel3 = 4
agentIskMultiplierLevel4 = 8
agentIskMultiplierLevel5 = 16
agentIskMultipliers = (agentIskMultiplierLevel1,
 agentIskMultiplierLevel2,
 agentIskMultiplierLevel3,
 agentIskMultiplierLevel4,
 agentIskMultiplierLevel5)
agentLpMultiplierLevel1 = 20
agentLpMultiplierLevel2 = 60
agentLpMultiplierLevel3 = 180
agentLpMultiplierLevel4 = 540
agentLpMultiplierLevel5 = 4860
agentLpMultipliers = (agentLpMultiplierLevel1,
 agentLpMultiplierLevel2,
 agentLpMultiplierLevel3,
 agentLpMultiplierLevel4,
 agentLpMultiplierLevel5)
agentIskRandomLowValue = 11000
agentIskRandomHighValue = 16500
agentCareerTypeIndustry = 1
agentCareerTypeBusiness = 2
agentCareerTypeMilitary = 3
agentCareerTypeExploration = 4
agentCareerTypeAdvMilitary = 5
agentDialogueButtonViewMission = 1
agentDialogueButtonRequestMission = 2
agentDialogueButtonAccept = 3
agentDialogueButtonAcceptChoice = 4
agentDialogueButtonAcceptRemotely = 5
agentDialogueButtonComplete = 6
agentDialogueButtonCompleteRemotely = 7
agentDialogueButtonContinue = 8
agentDialogueButtonDecline = 9
agentDialogueButtonDefer = 10
agentDialogueButtonQuit = 11
agentDialogueButtonStartResearch = 12
agentDialogueButtonCancelResearch = 13
agentDialogueButtonBuyDatacores = 14
agentDialogueButtonLocateCharacter = 15
agentDialogueButtonLocateAccept = 16
agentDialogueButtonLocateReject = 17
agentDialogueButtonYes = 18
agentDialogueButtonNo = 19
*/

/*
allianceApplicationAccepted = 2
allianceApplicationEffective = 3
allianceApplicationNew = 1
allianceApplicationRejected = 4
allianceCreationCost = 1000000000
allianceMembershipCost = 2000000
allianceRelationshipCompetitor = 3
allianceRelationshipEnemy = 4
allianceRelationshipFriend = 2
allianceRelationshipNAP = 1
*/

/*factionNoFaction = 0
factionAmarrEmpire = 500003
factionAmmatar = 500007
factionAngelCartel = 500011
factionCONCORDAssembly = 500006
factionCaldariState = 500001
factionGallenteFederation = 500004
factionGuristasPirates = 500010
factionInterBus = 500013
factionJoveEmpire = 500005
factionKhanidKingdom = 500008
factionMinmatarRepublic = 500002
factionMordusLegion = 500018
factionORE = 500014
factionOuterRingExcavations = 500014
factionSanshasNation = 500019
factionSerpentis = 500020
factionSistersOfEVE = 500016
factionSocietyOfConsciousThought = 500017
factionTheBloodRaiderCovenant = 500012
factionTheServantSistersofEVE = 500016
factionTheSyndicate = 500009
factionThukkerTribe = 500015
factionUnknown = 500021
factionMordusLegionCommand = 500018
factionTheInterBus = 500013
factionAmmatarMandate = 500007
factionTheSociety = 500017
*/

/*
facwarCorporationJoining = 0
facwarCorporationActive = 1
facwarCorporationLeaving = 2
facwarStandingPerVictoryPoint = 0.0015
facwarWarningStandingCharacter = 0
facwarWarningStandingCorporation = 1
facwarOccupierVictoryPointBonus = 0.1
facwarMinStandingsToJoin = 0.5
facwarStatTypeKill = 0
facwarStatTypeLoss = 1
*/


//  -allan 7Jul14
typedef enum {
    calendarMonday = 0,
    calendarTuesday = 1,
    calendarWednesday = 2,
    calendarThursday = 3,
    calendarFriday = 4,
    calendarSaturday = 5,
    calendarSunday = 6,
    calendarJanuary = 1,
    calendarFebruary = 2,
    calendarMarch = 3,
    calendarApril = 4,
    calendarMay = 5,
    calendarJune = 6,
    calendarJuly = 7,
    calendarAugust = 8,
    calendarSeptember = 9,
    calendarOctober = 10,
    calendarNovember = 11,
    calendarDecember = 12,
    calendarNumDaysInWeek = 7,
    calendarTagPersonal = 1,
    calendarTagCorp = 2,
    calendarTagAlliance = 4,
    calendarTagCCP = 8,
    calendarTagAutomated = 16,
    calendarViewRangeInMonths = 12,
    calendarMaxTitleSize = 40,
    calendarMaxDescrSize = 500,
    calendarMaxInvitees = 50,
    calendarMaxInviteeDisplayed = 100,
    calendarAutoEventPosFuel = 1,
    calendarStartYear = 2003,
    eventResponseUninvited = 0,
    eventResponseDeleted = 1,
    eventResponseDeclined = 2,
    eventResponseUndecided = 3,
    eventResponseAccepted = 4,
    eventResponseMaybe = 5
} CalendarDef;

//the constants are made up of:
//  prefix     -> cachedObject
//                config.BulkData.constants
//     category   -> config.BulkData.categories
//     group      -> config.BulkData.groups
//     metaGreoup -> config.BulkData.metagroups
//     attribute  -> config.BulkData.dgmattribs
//     effect     -> config.BulkData.dgmeffects
//    billType   -> config.BulkData.billtypes
//     role       -> config.Roles
//     flag       -> config.Flags
//     race       -> config.Races
//     bloodline  -> config.Bloodlines
//     statistic  -> config.Statistics
//     unit       -> config.Units
//     channelType -> config.ChannelTypes
//     encyclopediaType -> config.EncyclopediaTypes
//     activity   -> config.BulkData.ramactivities
//     completedStatus -> config.BulkData.ramcompletedstatuses
//
// First letter of `Name` field if capitalized when prefixed.
// see InsertConstantsFromRowset


//message format argument types:
typedef enum {
    fmtMapping_OWNERID2 = 1,    //not used? owner name
    fmtMapping_OWNERID = 2,    //owner name
    fmtMapping_LOCID = 3,    //locations
    fmtMapping_itemTypeName = 4,    //TYPEID: takes the item ID
    fmtMapping_itemTypeDescription = 5,    //TYPEID2: takes the item ID
    fmtMapping_blueprintTypeName = 6,    //from invBlueprints
    fmtMapping_itemGroupName = 7,    //GROUPID: takes the item group ID
    fmtMapping_itemGroupDescription = 8,    //GROUPID2: takes the item group ID
    fmtMapping_itemCategoryName = 9,    //CATID: takes the item category ID
    fmtMapping_itemCategoryDescription = 10,    //CATID2: takes the item category ID
    fmtMapping_DGMATTR = 11,    //not used...
    fmtMapping_DGMFX = 12,        //not used...
    fmtMapping_DGMTYPEFX = 13,    //not used...
    fmtMapping_dateTime = 14,    //DATETIME: formatted date and time
    fmtMapping_date = 15,        //DATE: formatted date
    fmtMapping_time = 16,        //TIME: formatted time
    fmtMapping_shortTime = 17,    //TIMESHRT: formatted time, short format
    fmtMapping_long = 18,        //AMT: regular number format
    fmtMapping_ISK2 = 19,        //AMT2: ISK format
    fmtMapping_ISK3 = 20,        //AMT3: ISK format
    fmtMapping_distance = 21,        //DIST: distance format
    fmtMapping_message = 22,    //MSGARGS: nested message
    fmtMapping_ADD_THE = 22,    //ADD_THE: prefix argument with 'the '
    fmtMapping_ADD_A = 23,        //ADD_A: prefix argument with 'a ' or 'an ' as appropriate
    fmtMapping_typeQuantity = 24,    //TYPEIDANDQUANTITY: human readable representation of the two arguments: typeID and quantity
    fmtMapping_ownerNickname = 25,    //OWNERIDNICK: first part of owner name (before space)
    fmtMapping_station = 26,    //SESSIONSENSITIVESTATIONID: human readable, fully qualified station name (includes system, constellation and region)
    fmtMapping_system = 27,    //SESSIONSENSITIVELOCID: human readable, fully qualified system name (includes constellation and region)
    fmtMapping_ISK = 28,        //ISK: ISK format
    fmtMapping_TYPEIDL = 29
} fmtMappingType;

typedef enum {
    dgmEffPassive = 0,
    dgmEffActivation = 1,
    dgmEffTarget = 2,
    dgmEffArea = 3,
    dgmEffOnline = 4,
} EffectCategories;

//  -allan 5Aug14
typedef enum {
    fleetJobNone = 0,
    fleetJobScout = 1,
    fleetJobCreator = 2
} FleetJobs;

//  -allan 5Aug14
typedef enum {
    fleetRoleLeader = 1,
    fleetRoleWingCmdr = 2,
    fleetRoleSquadCmdr = 3,
    fleetRoleMember = 4
} FleetRoles;

//  -allan 5Aug14
typedef enum {
    fleetBoosterNone = 0,
    fleetBoosterFleet = 1,
    fleetBoosterWing = 2,
    fleetBoosterSquad = 3
} FleetBoosters;

//  -allan 5Aug14
typedef enum {
    searchResultAgent = 1,
    searchResultCharacter = 2,
    searchResultCorporation = 3,
    searchResultAlliance = 4,
    searchResultFaction = 5,
    searchResultConstellation = 6,
    searchResultSolarSystem = 7,
    searchResultRegion = 8,
    searchResultStation = 9,
    searchResultInventoryType = 10,
    //searchResultAllOwners = [1, 2, 3, 4, 5],
    //searchResultAllLocations = [6, 7, 8, 9],
    searchMaxResults = 500,
    searchMinWildcardLength = 3
} SearchTypes;

typedef enum EVETutorialTypes {
    advchannelsTutorial = 50,
    cloningTutorial = 42,
    cloningWhenPoddedTutorial = 27,
    podriskTutorial = 43,
    skillfittingTutorial = 51,
    insuranceTutorial = 41,
    tutorial = 109,
    tutorialAuraIntroduction = 5,
    tutorialBoarding = 6,
    tutorialCharacterSheet = 7,
    tutorialControlConsole = 18,
    tutorialItems = 8,
    tutorialNavigation = 19,
    tutorialOverview = 21,
    tutorialSelling = 10,
    tutorialShips = 6,
    tutorialSpace = 17,
    tutorialTargeting = 20,
    tutorialUndock = 16,
    tutorialWallet = 11,
    tutorialWarpingDC = 23,
    tutorialCombatChooseTheVenue = 103,
    tutorialCombatConcepts = 105,
    tutorialCombatKnowYourEquipment = 104,
    tutorialCombatStudyTheOpponent = 102,
    tutorialInformativeCareerPlanning = 99,
    tutorialInformativeCharacterSheetAdvancedInformation = 100,
    tutorialInformativeContracts = 54,
    tutorialInformativeCorporations = 33,
    tutorialInformativeCosmosComplexes = 101,
    tutorialInformativeCrimeAndPunishment = 97,
    tutorialInformativeDrones = 65,
    tutorialInformativeExploration = 124,
    tutorialInformativeFittingStationService = 13,
    tutorialInformativeHeat = 123,
    tutorialInformativeMap = 14,
    tutorialInformativeMarket = 12,
    tutorialInformativePeopleAndPlaces = 15,
    tutorialInformativePoliticsAndmanagement = 98,
    tutorialInformativeRepairshop = 46,
    tutorialInformativeReprocessingPlant = 9,
    tutorialInformativeSalvaging = 122,
    tutorialInformativeScanning = 63,
    tutorialInformativeScienceIndustry = 52,
    tutorialWorldspaceNavigation = 235,
    tutorialTutorials = 215,
    tutorialCertificates = 134
} TutorialTypes;

//  -allan 20Dec14
typedef enum {
    agentTypeNonAgent = 1,
    agentTypeBasicAgent = 2,
    agentTypeTutorialAgent = 3,
    agentTypeResearchAgent = 4,
    agentTypeGenericStorylineMissionAgent = 6,
    agentTypeStorylineMissionAgent = 7,
    agentTypeEventMissionAgent = 8,
    agentTypeFactionalWarfareAgent = 9,
    agentTypeEpicArcAgent = 10,
    agentTypeAura = 11
} agentTypes;

/*
 * typedef enum {
 *    posShieldStartLevel = 0.505f,
 *    posMaxShieldPercentageForWatch = 0.95f,
 *    posMinDamageDiffToPersist = 0.05f
 * };
 */

typedef enum {
    npcDivisionAccounting = 1,
    npcDivisionAdministration = 2,
    npcDivisionAdvisory = 3,
    npcDivisionArchives = 4,
    npcDivisionAstrosurveying = 5,
    npcDivisionCommand = 6,
    npcDivisionDistribution = 7,
    npcDivisionFinancial = 8,
    npcDivisionIntelligence = 9,
    npcDivisionInternalSecurity = 10,
    npcDivisionLegal = 11,
    npcDivisionManufacturing = 12,
    npcDivisionMarketing = 13,
    npcDivisionMining = 14,
    npcDivisionPersonnel = 15,
    npcDivisionProduction = 16,
    npcDivisionPublicRelations = 17,
    npcDivisionRD = 18,
    npcDivisionSecurity = 19,
    npcDivisionStorage = 20,
    npcDivisionSurveillance = 21
} npcDivisions;

/*
 * cacheSystemIntervals = 2000109999
 * cacheSystemSettings = 2000100001
 * cacheSystemSchemas = 2000100003
 * cacheSystemTables = 2000100004
 * cacheSystemProcedures = 2000100006
 * cacheSystemEventTypes = 2000100013
 * cacheUserEventTypes = 2000209999
 * cacheUserColumns = 2000209998
 * cacheUserRegions = 2000209997
 * cacheUserTimeZones = 2000209996
 * cacheUserCountries = 2000209995
 * cacheUserTypes = 2000209994
 * cacheUserStatuses = 2000209993
 * cacheUserRoles = 2000209992
 * cacheUserConnectTypes = 2000209991
 * cacheUserOperatingSystems = 2000209990
 * cacheStaticSettings = 2000309999
 * cacheStaticBranches = 2000300001
 * cacheStaticReleases = 2000300006
 * cacheStaticIntegrateOptions = 2000300008
 * cacheMlsLanguages = 2000409999
 * cacheMlsTranslationStatuses = 2000409998
 * cacheMlsTextGroupTypes = 2000409997
 * cacheMlsTextStatuses = 2000409996
 * cacheMlsTaskStatuses = 2000409995
 * cacheClusterServices = 2000909999
 * cacheClusterMachines = 2000909998
 * cacheClusterProxies = 2000909997
 * cacheClientBrowserSiteFlags = 2003009999
 * cacheAccountingKeys = 2001100001
 * cacheAccountingEntryTypes = 2001100002
 * cacheInventoryCategories = 2001300001
 * cacheInventoryGroups = 2001300002
 * cacheInventoryTypes = 2001300003
 * cacheInventoryFlags = 2001300012
 * cacheEventGroups = 2001500002
 * cacheEventTypes = 2001500003
 * cacheWorldSpaces = 2001700035
 * cacheWorldSpaceDistricts = 2001700001
 * cacheResGraphics = 2001800001
 * cacheResSounds = 2001800002
 * cacheResDirectories = 2001800003
 * cacheResIcons = 2001800004
 * cacheResDetailMeshes = 2001800005
 * cacheActionTreeSteps = 2001900002
 * cacheActionTreeProcs = 2001900003
 * cacheEntityIngredients = 2002200001
 * cacheEntityIngredientInitialValues = 2002200002
 * cacheEntitySpawns = 2002200006
 * cacheEntityRecipes = 2002200009
 * cacheEntitySpawnGroups = 2002200010
 * cacheEntitySpawnGroupLinks = 2002200011
 * cacheActionObjects = 2002400001
 * cacheActionStations = 2002400002
 * cacheActionStationActions = 2002400003
 * cacheActionObjectStations = 2002400004
 * cacheActionObjectExits = 2002400005
 * cacheTreeNodes = 2002500001
 * cacheTreeLinks = 2002500002
 * cacheTreeProperties = 2002500005
 * cachePerceptionSenses = 2002600001
 * cachePerceptionStimTypes = 2002600002
 * cachePerceptionSubjects = 2002600004
 * cachePerceptionTargets = 2002600005
 * cachePerceptionBehaviorSenses = 2002600010
 * cachePerceptionBehaviorFilters = 2002600011
 * cachePerceptionBehaviorDecays = 2002600012
 * cachePaperdollModifierLocations = 2001600002
 * cachePaperdollResources = 2001600003
 * cachePaperdollSculptingLocations = 2001600004
 * cachePaperdollColors = 2001600005
 * cachePaperdollColorNames = 2001600006
 * cachePaperdollColorRestrictions = 2001600007
 * cacheEncounterEncounters = 2003100001
 * cacheEncounterCoordinates = 2003100002
 * cacheEncounterCoordinateSets = 2003100003
 * cacheStaticUsers = 2000000001
 * cacheUsersDataset = 2000000002
 * cacheCharactersDataset = 2000000003
 * cacheNameNames = 2000000004
 */


/*
 *
 * ENV_IDX_SELF = 0
 * ENV_IDX_CHAR = 1
 * ENV_IDX_SHIP = 2
 * ENV_IDX_TARGET = 3
 * ENV_IDX_OTHER = 4
 * ENV_IDX_AREA = 5
 * ENV_IDX_EFFECT = 6
 *
 *
 *
 *
 */

/*
 * service.ROLE_CHTADMINISTRATOR | service.ROLE_GMH
 * CHTMODE_CREATOR = (((8 + 4) + 2) + 1)
 * CHTMODE_OPERATOR = ((4 + 2) + 1)
 * CHTMODE_CONVERSATIONALIST = (2 + 1)
 * CHTMODE_SPEAKER = 2
 * CHTMODE_LISTENER = 1
 * CHTMODE_NOTSPECIFIED = -1
 * CHTMODE_DISALLOWED = -2
 * CHTERR_NOSUCHCHANNEL = -3
 * CHTERR_ACCESSDENIED = -4
 * CHTERR_INCORRECTPASSWORD = -5
 * CHTERR_ALREADYEXISTS = -6
 * CHTERR_TOOMANYCHANNELS = -7
 * CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50
 *
 * CHANNEL_CUSTOM = 0
 * CHANNEL_GANG = 3
 *
 *
 */

//  defines based on itemID, per client
#define maxNonCapitalModuleSize 500

#define minEveMarketGroup       0
#define maxEveMarketGroup       350000
#define minDustMarketGroup      350001
#define maxDustMarketGroup      999999
#define minFaction              500000
#define maxFaction              599999
#define minNPCCorporation       1000000
#define maxNPCCorporation       1000999
#define maxCorporation          1999999
#define minAgent                3000000
#define maxAgent                3999999
#define minRegion               10000000
#define maxRegion               19999999
#define minConstellation        20000000
#define maxConstellation        29999999
#define minSolarSystem          30000000
#define maxSolarSystem          39999999
#define minValidLocation        30000000
#define minValidShipLocation    30000000
#define minUniverseCelestial    40000000
#define maxUniverseCelestial    49999999
#define minStargate             50000000
#define maxStargate             59999999
#define minValidCharLocation    60000000
#define minStation              60000000
#define maxNPCStation           60999999
#define maxStation              69999999
#define minUniverseAsteroid     70000000
#define maxUniverseAsteroid     79999999
#define minAlliance             99000000
#define maxAlliance             99900000
#define minPlayerItem           100000000
#define maxEveItem              2147483647

#define EVEMU_MINIMUM_ID minPlayerItem
#define EVEMU_MINIMUM_ENTITY_ID 90000000
#define EVEMU_MAXIMUM_ENTITY_ID (EVEMU_MINIMUM_ID-1)
#define STATION_HANGAR_MAX_CAPACITY 9000000000000000.0  //per client
#define MAX_MARKET_PRICE 9223372036854

/* there needs to be more to this check.....
 * #define IsChar(charID) \
 *    (charID > 140000000)
 */

#define IsCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID <= maxCorporation))

#define IsNPCCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID < maxNPCCorporation))

#define IsPlayerCorp(itemID) \
((itemID >= maxNPCCorporation) && (itemID < maxCorporation))

#define IsAlliance(itemID) \
((itemID >= minAlliance) && (itemID < maxAlliance))

#define IsAgent(itemID) \
((itemID >= 3008416) && (itemID < 3020000))

#define IsFaction(itemID) \
((itemID >= minFaction) && (itemID < maxFaction))

// this covers ALL static celestial-type items
#define IsStaticMapItem(itemID) \
((itemID >= minUniverseCelestial) && (itemID < maxStation))

#define IsRegion(itemID) \
((itemID >= 10000000) && (itemID < 20000000))

#define IsConstellation(itemID) \
((itemID >= 20000000) && (itemID < 30000000))

#define IsSolarSystem(itemID) \
((itemID >= 30000000) && (itemID < 40000000))

#define IsUniverseCelestial(itemID) \
((itemID >= 40000000) && (itemID < 50000000))

#define IsStargate(itemID) \
((itemID >= 50000000) && (itemID < 60000000))

#define IsStation(itemID) \
((itemID >= minStation) && (itemID < maxStation))

#define IsNPCStation(itemID) \
((itemID >= minStation) && (itemID <= maxNPCStation))

#define IsOutpost(itemID) \
((itemID > maxNPCStation) && (itemID < maxStation))

#define IsTrading(itemID) \
((itemID >= 64000000) && (itemID < 66000000))

#define IsOfficeFolder(itemID) \
((itemID >= 66000000) && (itemID < 68000000))

#define IsFactoryFolder(itemID) \
((itemID >= 68000000) && (itemID < 70000000))

#define IsUniverseAsteroid(itemID) \
((itemID >= 70000000) && (itemID < 80000000))

#define IsScenarioItem(itemID) \
((itemID >= 90000000) && (itemID < EVEMU_MINIMUM_ID))

#define IsPlayerItem(itemID) \
(itemID >= EVEMU_MINIMUM_ID)

#endif

/*
def IsSystem(ownerID):
    return ownerID <= 10000


def IsNPC(ownerID):
    return ownerID < 90000000 and ownerID > 10000


def IsNPCCorporation(ownerID):
    return ownerID < 2000000 and ownerID >= 1000000


def IsNPCCharacter(ownerID):
    return ownerID < 4000000 and ownerID >= 3000000


def IsSystemOrNPC(ownerID):
    return ownerID < 90000000


def IsFaction(ownerID):
    if ownerID >= 500000 and ownerID < 1000000:
        return 1
    else:
        return 0


def IsCorporation(ownerID):
    if ownerID >= 1000000 and ownerID < 2000000:
        return 1
    if ownerID < 98000000 or ownerID > 2147483647:
        return 0
    if ownerID < 99000000:
        return 1
    if ownerID < 100000000:
        return 0
    if boot.role == 'server' and sm.GetService('standing2').IsKnownToBeAPlayerCorp(ownerID):
        return 1
    try:
        return cfg.eveowners.Get(ownerID).IsCorporation()
    except KeyError:
        return 0


def IsCharacter(ownerID):
    if ownerID >= 3000000 and ownerID < 4000000:
        return 1
    if ownerID < 90000000 or ownerID > 2147483647:
        return 0
    if ownerID < 98000000:
        return 1
    if ownerID < 100000000:
        return 0
    if boot.role == 'server' and sm.GetService('standing2').IsKnownToBeAPlayerCorp(ownerID):
        return 0
    try:
        return cfg.eveowners.Get(ownerID).IsCharacter()
    except KeyError:
        return 0


def IsPlayerAvatar(itemID):
    return IsCharacter(itemID)


def IsOwner(ownerID, fetch = 1):
    if ownerID >= 500000 and ownerID < 1000000 or ownerID >= 1000000 and ownerID < 2000000 or ownerID >= 3000000 and ownerID < 4000000:
        return 1
    if IsNPC(ownerID):
        return 0
    if ownerID < 90000000 or ownerID > 2147483647:
        return 0
    if ownerID < 100000000:
        return 1
    if fetch:
        try:
            oi = cfg.eveowners.Get(ownerID)
        except KeyError:
            return 0

        if oi.groupID in (const.groupCharacter, const.groupCorporation):
            return 1
        else:
            return 0
    else:
        return 0


def IsAlliance(ownerID):
    if ownerID < 99000000 or ownerID > 2147483647:
        return 0
    if ownerID < 100000000:
        return 1
    if boot.role == 'server' and sm.GetService('standing2').IsKnownToBeAPlayerCorp(ownerID):
        return 0
    try:
        return cfg.eveowners.Get(ownerID).IsAlliance()
    except KeyError:
        return 0


def IsRegion(itemID):
    return itemID >= 10000000 and itemID < 20000000


def IsConstellation(itemID):
    return itemID >= 20000000 and itemID < 30000000


def IsSolarSystem(itemID):
    return itemID >= 30000000 and itemID < 40000000


def IsCelestial(itemID):
    return itemID >= 40000000 and itemID < 50000000


def IsWormholeSystem(itemID):
    return itemID >= const.mapWormholeSystemMin and itemID < const.mapWormholeSystemMax


def IsWormholeConstellation(constellationID):
    return constellationID >= const.mapWormholeConstellationMin and constellationID < const.mapWormholeConstellationMax


def IsWormholeRegion(regionID):
    return regionID >= const.mapWormholeRegionMin and regionID < const.mapWormholeRegionMax


def IsUniverseCelestial(itemID):
    return itemID >= const.minUniverseCelestial and itemID <= const.maxUniverseCelestial


def IsStargate(itemID):
    return itemID >= 50000000 and itemID < 60000000


def IsStation(itemID):
    return itemID >= 60000000 and itemID < 64000000


def IsWorldSpace(itemID):
    return itemID >= const.mapWorldSpaceMin and itemID < const.mapWorldSpaceMax


def IsOutpost(itemID):
    return itemID >= 61000000 and itemID < 64000000


def IsTrading(itemID):
    return itemID >= 64000000 and itemID < 66000000


def IsOfficeFolder(itemID):
    return itemID >= 66000000 and itemID < 68000000


def IsFactoryFolder(itemID):
    return itemID >= 68000000 and itemID < 70000000


def IsUniverseAsteroid(itemID):
    return itemID >= 70000000 and itemID < 80000000


def IsJunkLocation(locationID):
    if locationID >= 2000:
        return 0
    elif locationID in (6, 8, 10, 23, 25):
        return 1
    elif locationID > 1000 and locationID < 2000:
        return 1
    else:
        return 0


def IsControlBunker(itemID):
    return itemID >= 80000000 and itemID < 80100000


def IsPlayerItem(itemID):
    return itemID >= const.minPlayerItem and itemID < const.minFakeItem


def IsFakeItem(itemID):
    return itemID > const.minFakeItem


def IsNewbieSystem(itemID):
    default = [30002547,
     30001392,
     30002715,
     30003489,
     30005305,
     30004971,
     30001672,
     30002505,
     30000141,
     30003410,
     30005042,
     30001407]
    optional = [30001722,
     30002518,
     30003388,
     30003524,
     30005015,
     30010141,
     30011392,
     30011407,
     30011672,
     30012505,
     30012547,
     30012715,
     30013410,
     30013489,
     30014971,
     30015042,
     30015305,
     30020141,
     30021392,
     30021407,
     30021672,
     30022505,
     30022547,
     30022715,
     30023410,
     30023489,
     30024971,
     30025042,
     30025305,
     30030141,
     30031392,
     30031407,
     30031672,
     30032505,
     30032547,
     30032715,
     30033410,
     30033489,
     30034971,
     30035042,
     30035305,
     30040141,
     30041392,
     30041407,
     30041672,
     30042505,
     30042547,
     30042715,
     30043410,
     30043489,
     30044971,
     30045042,
     30045305]
    if boot.region == 'optic':
        return itemID in default + optional
    return itemID in default


def IsStructure(categoryID):
    return categoryID in (const.categorySovereigntyStructure, const.categoryStructure)


def IsOrbital(categoryID):
    return categoryID == const.categoryOrbital


def IsPreviewable(typeID):
    type = cfg.invtypes.GetIfExists(typeID)
    if type is None:
        return False
    groupID = type.groupID
    categoryID = type.categoryID
    return categoryID in const.previewCategories or groupID in const.previewGroups


def IsPlaceable(typeID):
    type = cfg.invtypes.GetIfExists(typeID)
    if type is None:
        return False
    return const.categoryPlaceables == type.categoryID


def IsEveUser(userID):
    if userID < const.minDustUser:
        return True
    return False


def IsDustUser(userID):
    if userID > const.minDustUser:
        return True
    return False


def IsDustCharacter(characterID):
    if characterID > const.minDustCharacter and characterID < const.maxDustCharacter:
        return True
    return False
*/