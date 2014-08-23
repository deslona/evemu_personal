
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


UPDATE `dgmTypeAttributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;
UPDATE `entity_attributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;
UPDATE `entity_default_attributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;


