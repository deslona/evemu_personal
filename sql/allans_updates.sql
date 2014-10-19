
ALTER TABLE `character_`
  ADD
  `logonDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `logonMinutes` int(10) unsigned NOT NULL DEFAULT '0';

/* Table structure for table `mapDynamicData` */

CREATE TABLE IF NOT EXISTS `mapDynamicData` (
  `solarSystemID` int(10) NOT NULL,
  `moduleCnt` int(10) NOT NULL DEFAULT '0',
  `structureCnt` int(10) NOT NULL DEFAULT '0',
  `jumpsHour` int(10) NOT NULL DEFAULT '0',
  `killsHour` int(10) NOT NULL DEFAULT '0',
  `kills24Hours` int(10) NOT NULL DEFAULT '0',
  `pilotsDocked` int(10) NOT NULL DEFAULT '0',
  `pilotsInSpace` int(10) NOT NULL DEFAULT '0',
  `podKillsHour` int(10) NOT NULL DEFAULT '0',
  `podKills24Hour` int(10) NOT NULL DEFAULT '0',
  `factionKills` int(11) NOT NULL DEFAULT '0',
  `factionKills24Hour` int(11) NOT NULL DEFAULT '0',
  `jumpsDateTime` bigint(20) DEFAULT NULL,
  `killsDateTime` bigint(20) DEFAULT NULL,
  `kills24DateTime` bigint(20) DEFAULT NULL,
  `podDateTime` bigint(20) DEFAULT NULL,
  `pod24DateTime` bigint(20) DEFAULT NULL,
  `factionDateTime` bigint(20) DEFAULT NULL,
  `faction24DateTime` bigint(20) DEFAULT NULL,
  `pilotsDateTime` bigint(20) DEFAULT NULL,
  UNIQUE KEY `solarSystemID` (`solarSystemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/* Table structure for table `chrPausedSkillQueue` */

CREATE TABLE IF NOT EXISTS `chrPausedSkillQueue` (
  `characterID` int(10) unsigned NOT NULL,
  `orderIndex` int(10) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`characterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `chrPausedSkillQueue` */

/* Table structure for table `bounties` */

CREATE TABLE IF NOT EXISTS `chrBounties` (
  `characterID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `bounty` bigint(20) NOT NULL,
  `timePlaced` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`timePlaced`),
  KEY `ownerID` (`ownerID`),
  KEY `timePlaced` (`timePlaced`),
  KEY `bounty` (`bounty`),
  KEY `characterID` (`characterID`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci ;

/* Table structure for table `mapOreBySystemSecurityClass` */

CREATE TABLE IF NOT EXISTS `mapOreBySystemSecurityClass` (
  `systemSecurityClass` char(2) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Veldspar` bit(1) DEFAULT NULL,
  `Scordite` bit(1) DEFAULT NULL,
  `Pyroxeres` bit(1) DEFAULT NULL,
  `Plagioclase` bit(1) DEFAULT NULL,
  `Omber` bit(1) DEFAULT NULL,
  `Kernite` bit(1) DEFAULT NULL,
  `Jaspet` bit(1) DEFAULT NULL,
  `Hemorphite` bit(1) DEFAULT NULL,
  `Hedbergite` bit(1) DEFAULT NULL,
  `Gneiss` bit(1) DEFAULT NULL,
  `DarkOchre` bit(1) DEFAULT NULL,
  `Spodumain` bit(1) DEFAULT NULL,
  `Crokite` bit(1) DEFAULT NULL,
  `Bistot` bit(1) DEFAULT NULL,
  `Arkanor` bit(1) DEFAULT NULL,
  `Mercoxit` bit(1) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/* Table structure for table `mapDynamicData`  */

CREATE TABLE IF NOT EXISTS `mapDynamicData` (
  `solarSystemID` int(10) NOT NULL,
  `moduleCnt` tinyint(4) NOT NULL DEFAULT '0',
  `structureCnt` tinyint(4) NOT NULL DEFAULT '0',
  `pilotsDocked` smallint(6) NOT NULL DEFAULT '0',
  `pilotsInSpace` smallint(6) NOT NULL DEFAULT '0',
  `jumpsHour` smallint(6) NOT NULL DEFAULT '0',
  `killsHour` smallint(6) NOT NULL DEFAULT '0',
  `kills24Hour` smallint(6) NOT NULL DEFAULT '0',
  `factionKills` smallint(6) NOT NULL DEFAULT '0',
  `factionKills24Hour` smallint(6) NOT NULL DEFAULT '0',
  `podKillsHour` smallint(6) NOT NULL DEFAULT '0',
  `podKills24Hour` smallint(6) NOT NULL DEFAULT '0',
  `pilotsDateTime` bigint(20) DEFAULT NULL,
  `jumpsDateTime` bigint(20) DEFAULT NULL,
  `killsDateTime` bigint(20) DEFAULT NULL,
  `kills24DateTime` bigint(20) DEFAULT NULL,
  `podDateTime` bigint(20) DEFAULT NULL,
  `pod24DateTime` bigint(20) DEFAULT NULL,
  `factionDateTime` bigint(20) DEFAULT NULL,
  `faction24DateTime` bigint(20) DEFAULT NULL,
  UNIQUE KEY `solarSystemID` (`solarSystemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*  Table structure for table `chrVisitedSystems` */

CREATE TABLE IF NOT EXISTS `chrVisitedSystems` (
  `characterID` int(20) NOT NULL,
  `solSystemID` int(10) NOT NULL,
  `visits` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`characterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*  Table structure for table `chrSkillHistory`  */

CREATE TABLE IF NOT EXISTS `chrSkillHistory` (
  `eventID` smallint(6) NOT NULL,
  `characterID` int(11) NOT NULL,
  `logDate` bigint(20) NOT NULL,
  `skillTypeID` int(8) NOT NULL,
  `skillLevel` tinyint(4) NOT NULL,
  `relativePoints` bigint(20) NOT NULL,
  `absolutePoints` bigint(20) NOT NULL,
  `AI` int(11) NOT NULL AUTO_INCREMENT,
  UNIQUE KEY `AI` (`AI`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COMMENT='Char Skill History' AUTO_INCREMENT=25 ;

/* Table structure for table `invWrecksToSalvage`  */

CREATE TABLE IF NOT EXISTS `invWrecksToSalvage` (
  `wreckTypeID` int(10) unsigned NOT NULL,
  `salvageItemID` int(10) unsigned NOT NULL,
  `group` tinyint(3) unsigned NOT NULL,
  `dropChance` decimal(6,4) unsigned NOT NULL DEFAULT '0.0000',
  `minDrop` tinyint(4) NOT NULL DEFAULT '0',
  `maxDrop` tinyint(4) NOT NULL DEFAULT '0',
  `WreckName` varchar(45) COLLATE utf8_unicode_ci NOT NULL,
  `SalvageName` varchar(45) COLLATE utf8_unicode_ci NOT NULL,
  KEY `wreckTypeID` (`wreckTypeID`,`salvageItemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


/* Table structure for table `mapConnections`  */

CREATE TABLE IF NOT EXISTS `mapConnections` (
  `ctype` int(10) unsigned NOT NULL,
  `fromreg` int(10) unsigned NOT NULL,
  `fromcon` int(10) unsigned DEFAULT NULL,
  `fromsol` int(10) unsigned DEFAULT NULL,
  `stargateID` int(10) unsigned DEFAULT NULL,
  `celestialID` int(10) unsigned DEFAULT NULL,
  `tosol` int(10) unsigned DEFAULT NULL,
  `tocon` int(10) unsigned DEFAULT NULL,
  `toreg` int(10) unsigned NOT NULL,
  PRIMARY KEY (`AI`),
  UNIQUE KEY `stargateID` (`stargateID`),
  UNIQUE KEY `celestialID` (`celestialID`),
  KEY `stargateID_2` (`stargateID`),
  KEY `celestialID_2` (`celestialID`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=14575 ;

/* Table structure for table `roidDistribution`  */

CREATE TABLE IF NOT EXISTS `asteroidDistribution` (
  `systemSec` varchar(2) NOT NULL,
  `asteroidID` int(10) unsigned NOT NULL,
  `percent` double NOT NULL,
  KEY `systemSec` (`systemSec`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/* Table structure for table `srvStatus */

CREATE TABLE IF NOT EXISTS `srvStatus` (
  `AI` int(11) NOT NULL AUTO_INCREMENT,
  `srvName` varchar(60) NOT NULL,
  `Online` tinyint(1) NOT NULL,
  `startTime` bigint(20) NOT NULL,
  `Connections` smallint(6) NOT NULL,
  `threads` tinyint(4) NOT NULL,
  `rss` decimal(6,3) NOT NULL,
  `vm` decimal(6,3) NOT NULL,
  `user` decimal(4,2) NOT NULL,
  `kernel` decimal(4,2) NOT NULL,
  `items` int(11) NOT NULL,
  `systems` int(11) NOT NULL,
  `bubbles` int(11) NOT NULL,
  PRIMARY KEY (`AI`),
  UNIQUE KEY `AI` (`AI`)
  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Table structure for table `sklBaseSkills`
--

CREATE TABLE IF NOT EXISTS `sklBaseSkills` (
  `ID` tinyint(4) NOT NULL AUTO_INCREMENT,
  `skillTypeID` smallint(6) NOT NULL,
  `level` tinyint(4) NOT NULL,
  PRIMARY KEY (`ID`)
  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Basic Skills for All Races' AUTO_INCREMENT=10 ;

--
-- Dumping data for table `sklBaseSkills`
--

INSERT INTO `sklBaseSkills` (`ID`, `skillTypeID`, `level`) VALUES
(1, 3300, 2),
(2, 3327, 2),
(3, 3386, 1),
(4, 3392, 2),
(5, 3402, 1),
(6, 3413, 2),
(7, 3416, 1),
(8, 3426, 2),
(9, 3449, 2);

-- --------------------------------------------------------

--
-- Table structure for table `sklCareerSkills`
--

CREATE TABLE IF NOT EXISTS `sklCareerSkills` (
  `careerID` int(11) NOT NULL DEFAULT '0',
  `skillTypeID` int(11) NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`careerID`,`skillTypeID`)
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by careerID';

--
-- Dumping data for table `sklCareerSkills`
--

INSERT INTO `sklCareerSkills` (`careerID`, `skillTypeID`, `level`) VALUES
(11, 3300, 3),
(11, 3301, 3),
(11, 3311, 3),
(11, 3319, 3),
(11, 3321, 2),
(11, 3327, 2),
(11, 3330, 1),
(11, 3425, 1),
(11, 3432, 2),
(11, 12441, 2),
(11, 21059, 2),
(14, 3348, 4),
(14, 3349, 1),
(14, 3350, 3),
(14, 3355, 4),
(14, 3357, 5),
(14, 3363, 5),
(14, 3368, 2),
(14, 3443, 5),
(14, 16595, 3),
(14, 20494, 4),
(14, 20495, 2),
(17, 3348, 4),
(17, 3380, 3),
(17, 3385, 3),
(17, 3386, 4),
(17, 3392, 1),
(17, 3402, 2),
(17, 3406, 3),
(17, 3425, 1),
(17, 3432, 2),
(17, 21059, 1),
(17, 22536, 1),
(21, 3300, 2),
(21, 3302, 2),
(21, 3310, 4),
(21, 3311, 3),
(21, 3319, 3),
(21, 3320, 2),
(21, 3321, 3),
(21, 3329, 2),
(21, 3392, 1),
(21, 3413, 1),
(21, 3416, 2),
(21, 3424, 2),
(21, 3425, 3),
(21, 3426, 1),
(21, 3431, 3),
(21, 3436, 2),
(21, 3437, 2),
(21, 3449, 1),
(21, 3450, 2),
(21, 3455, 2),
(24, 3348, 4),
(24, 3349, 2),
(24, 3350, 1),
(24, 3355, 3),
(24, 3357, 3),
(24, 3363, 4),
(24, 3368, 3),
(24, 3431, 2),
(24, 3443, 3),
(24, 3449, 1),
(24, 3450, 2),
(24, 3455, 2),
(24, 16595, 3),
(24, 20494, 3),
(24, 20495, 3),
(27, 3319, 2),
(27, 3320, 1),
(27, 3321, 2),
(27, 3348, 3),
(27, 3380, 4),
(27, 3385, 4),
(27, 3386, 4),
(27, 3387, 3),
(27, 3392, 1),
(27, 3402, 4),
(27, 3403, 3),
(27, 3406, 3),
(27, 3413, 1),
(27, 3443, 1),
(27, 22536, 1),
(41, 3300, 2),
(41, 3303, 2),
(41, 3310, 4),
(41, 3311, 3),
(41, 3312, 3),
(41, 3316, 3),
(41, 3317, 2),
(41, 3331, 2),
(41, 3392, 1),
(41, 3393, 1),
(41, 3394, 1),
(41, 3413, 2),
(41, 3417, 2),
(41, 3418, 2),
(44, 3348, 4),
(44, 3349, 2),
(44, 3350, 1),
(44, 3355, 4),
(44, 3356, 3),
(44, 3357, 5),
(44, 3363, 5),
(44, 3368, 4),
(44, 3443, 5),
(44, 3446, 4),
(44, 16595, 3),
(44, 20494, 3),
(44, 20495, 3),
(47, 3348, 3),
(47, 3380, 3),
(47, 3385, 3),
(47, 3386, 4),
(47, 3387, 2),
(47, 3392, 1),
(47, 3393, 1),
(47, 3394, 1),
(47, 3402, 3),
(47, 3403, 1),
(47, 3406, 1),
(47, 3443, 2),
(47, 22536, 1),
(81, 3300, 2),
(81, 3301, 2),
(81, 3312, 1),
(81, 3316, 2),
(81, 3317, 2),
(81, 3319, 3),
(81, 3323, 3),
(81, 3328, 2),
(81, 3393, 3),
(81, 3426, 2),
(81, 3449, 1),
(81, 3450, 2),
(81, 3455, 2),
(84, 3348, 4),
(84, 3349, 2),
(84, 3350, 1),
(84, 3355, 5),
(84, 3356, 3),
(84, 3357, 3),
(84, 3363, 4),
(84, 3368, 3),
(84, 3443, 5),
(84, 3444, 3),
(84, 3446, 4),
(84, 16595, 4),
(84, 20494, 3),
(84, 20495, 3),
(87, 3348, 3),
(87, 3380, 4),
(87, 3385, 4),
(87, 3386, 3),
(87, 3402, 3),
(87, 3403, 1),
(87, 3406, 1),
(87, 3413, 3),
(87, 3417, 3),
(87, 3418, 3),
(87, 3443, 2),
(87, 22536, 1);

-- --------------------------------------------------------

--
-- Table structure for table `sklRaceSkills`
--

CREATE TABLE IF NOT EXISTS `sklRaceSkills` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `raceID` int(11) DEFAULT NULL,
  `skillTypeID` int(11) DEFAULT NULL,
  `level` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `skillTypeID` (`skillTypeID`)
  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by raceID' AUTO_INCREMENT=33 ;

/* Dumping data for table `sklRaceSkills`  */

INSERT INTO `sklRaceSkills` (`id`, `raceID`, `skillTypeID`, `level`) VALUES
(1, 1, 3301, 2),
(2, 1, 3330, 2),
(3, 1, 3319, 2),
(4, 1, 3321, 3),
(5, 1, 3413, 2),
(6, 1, 13432, 1),
(7, 1, 3416, 1),
(8, 1, 3426, 2),
(9, 1, 21059, 2),
(10, 1, 3425, 1),
(11, 2, 3302, 2),
(12, 2, 3329, 2),
(13, 2, 3416, 2),
(14, 2, 3413, 1),
(15, 2, 3426, 1),
(16, 2, 3300, 1),
(17, 2, 3393, 3),
(18, 2, 3394, 4),
(19, 4, 3303, 2),
(20, 4, 3331, 2),
(21, 4, 3392, 1),
(22, 4, 3393, 3),
(23, 4, 3394, 3),
(24, 4, 3417, 2),
(25, 4, 3418, 2),
(26, 8, 3301, 2),
(27, 8, 3328, 2),
(28, 8, 3436, 4),
(29, 8, 3437, 3),
(30, 8, 3442, 2),
(31, 8, 12305, 2),
(32, 8, 3392, 2);

-- --------------------------------------------------------


UPDATE `dgmTypeAttributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;
UPDATE `entity_attributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;
UPDATE `entity_default_attributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;


