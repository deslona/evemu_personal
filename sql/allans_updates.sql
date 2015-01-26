/*  this file is missing some updates.  not sure which ones yet...
        -allan 25jan15
 */


 /* Table structure for table `shipInsurance` */

CREATE TABLE IF NOT EXISTS `shipInsurance` (
  `shipID` int(11) NOT NULL,
  `shipName` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `ownerID` int(11) NOT NULL,
  `startDate` bigint(20) NOT NULL,
  `endDate` bigint(20) NOT NULL,
  `fraction` float(4,3) NOT NULL,
  `isCorpItem` tinyint(1) NOT NULL,
  PRIMARY KEY (`shipID`),
  UNIQUE KEY `shipID` (`shipID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/* Table structure for table `chrPausedSkillQueue` */

CREATE TABLE IF NOT EXISTS `chrPausedSkillQueue` (
  `characterID` int(10) unsigned NOT NULL,
  `orderIndex` int(10) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL
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

/* Table structure for table `webBounties` */

CREATE TABLE IF NOT EXISTS `webBounties` (
  `characterID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `bounty` bigint(20) NOT NULL,
  `timePlaced` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`timePlaced`),
  KEY `ownerID` (`ownerID`),
  KEY `timePlaced` (`timePlaced`),
  KEY `bounty` (`bounty`),
  KEY `characterID` (`characterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

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
  `idx` int(11) NOT NULL AUTO_INCREMENT,
  `characterID` int(20) NOT NULL,
  `solarSystemID` int(10) NOT NULL,
  `visits` int(10) NOT NULL DEFAULT '0',
  `lastDateTime` bigint(20) unsigned NOT NULL,
  PRIMARY KEY `idx` (`idx`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*  Table structure for table `chrSkillHistory`  */

CREATE TABLE IF NOT EXISTS `chrSkillHistory` (
  `eventTypeID` smallint(6) NOT NULL,
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
  `groupID` tinyint(3) unsigned NOT NULL,
  `dropChance` float(6,4) unsigned NOT NULL DEFAULT '0.0000',
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
  UNIQUE KEY `stargateID` (`stargateID`),
  UNIQUE KEY `celestialID` (`celestialID`),
  KEY `stargateID_2` (`stargateID`),
  KEY `celestialID_2` (`celestialID`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci ;

/* Table structure for table `asteroidDistribution`  */

CREATE TABLE IF NOT EXISTS `asteroidDistribution` (
  `systemSec` varchar(2) NOT NULL,
  `asteroidID` int(10) unsigned NOT NULL,
  `percent` double NOT NULL,
  KEY `systemSec` (`systemSec`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `srvStatus`;

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

/* Table structure for table `sklBaseSkills` */

CREATE TABLE IF NOT EXISTS `sklBaseSkills` (
  `ID` tinyint(4) NOT NULL AUTO_INCREMENT,
  `skillTypeID` smallint(6) NOT NULL,
  `level` tinyint(4) NOT NULL,
  PRIMARY KEY (`ID`)
  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Basic Skills for All Races' AUTO_INCREMENT=10 ;

/* Table structure for table `sklCareerSkills` */

CREATE TABLE IF NOT EXISTS `sklCareerSkills` (
  `careerID` int(11) NOT NULL DEFAULT '0',
  `skillTypeID` int(11) NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`careerID`,`skillTypeID`)
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by careerID';

/* Table structure for table `sklRaceSkills` */

CREATE TABLE IF NOT EXISTS `sklRaceSkills` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `raceID` int(11) DEFAULT NULL,
  `skillTypeID` int(11) DEFAULT NULL,
  `level` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `skillTypeID` (`skillTypeID`)
  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by raceID' AUTO_INCREMENT=33 ;

CREATE TABLE IF NOT EXISTS `mapSystemSovInfo` (
  `solarSystemID` int(11) NOT NULL,
  `corporationID` int(11) NOT NULL,
  `allianceID` int(11) NOT NULL,
  `claimStructureID` int(11) NOT NULL,
  `claimTime` int(20) NOT NULL,
  `hubID` int(11) NOT NULL,
  `contested` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='SystemSovereigntyInfo';

  /*  hack for minor client error...we dont have the real data for this yet  */
ALTER TABLE `staOperations` ADD `descriptionID` INT(3) NOT NULL DEFAULT '0' AFTER `description`;

/* set skill level from float to int */
UPDATE `dgmTypeAttributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;
UPDATE `entity_attributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;
UPDATE `entity_default_attributes` SET `valueInt`=0,`valueFloat`=NULL WHERE `attributeID`=280 AND `valueFloat`=0;

/* update beacon type 10124 IsGlobal attribute to true */
INSERT INTO `dgmTypeAttributes` (`typeID`, `attributeID`, `valueInt`, `valueFloat`) VALUES ('10124', '1207', '1', NULL);

/* reset rifter cap recharge rate */
UPDATE `dgmTypeAttributes` SET `valueFloat` = '125000' WHERE `typeID` = 587 AND `attributeID` = 55;

/* fix radius in mapDenormalize */
UPDATE mapDenormalize AS md INNER JOIN invTypes AS it USING (typeID) SET md.radius = it.radius WHERE md.groupID = 10;

ALTER TABLE `corporation` ADD `allianceMemberStartDate` BIGINT(20) NOT NULL DEFAULT '0' AFTER `allianceID`;

ALTER TABLE `character_` ADD `paperDollState` TINYINT(2) NOT NULL DEFAULT '0' ;
ALTER TABLE `character_` ADD `corpAccountKey` SMALLINT(4) NOT NULL DEFAULT '0' AFTER `corporationID`;
ALTER TABLE corporation AUTO_INCREMENT=2000000;
ALTER TABLE entity AUTO_INCREMENT=100000000;

ALTER TABLE `market_orders` CHANGE `range` `orderRange` INT(10) UNSIGNED NOT NULL DEFAULT '0';

/*  polaris legerion updates */
UPDATE `dgmTypeAttributes` SET `valueInt` = '8' WHERE `typeID` = 9860 AND `attributeID` = 12;
UPDATE `dgmTypeAttributes` SET `valueInt` = '8' WHERE `typeID` = 9860 AND `attributeID` = 13;
UPDATE `dgmTypeAttributes` SET `valueInt` = '8' WHERE `typeID` = 9860 AND `attributeID` = 14;
UPDATE `dgmTypeAttributes` SET `valueFloat` = '3' WHERE `typeID` = 9860 AND `attributeID` = 600;
UPDATE `dgmTypeAttributes` SET `valueFloat` = '5' WHERE `typeID` = 9860 AND `attributeID` = 1281;
