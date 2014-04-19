-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               5.6.13-log - MySQL Community Server (GPL)
-- Server OS:                    Win64
-- HeidiSQL Version:             8.2.0.4675
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Dumping structure for table crucible.insurance
CREATE TABLE IF NOT EXISTS `insurance` (
  `shipID` int(10) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `price` double NOT NULL,
  `ownerID` int(10) unsigned NOT NULL,
  `isCorpItem` tinyint(3) unsigned NOT NULL,
  `startDate` bigint(20) NOT NULL,
  `endDate` bigint(20) NOT NULL,
  `fraction` int(11) NOT NULL,
  PRIMARY KEY (`shipID`),
  KEY `shipID` (`shipID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table crucible.insurance: ~0 rows (approximately)
/*!40000 ALTER TABLE `insurance` DISABLE KEYS */;
/*!40000 ALTER TABLE `insurance` ENABLE KEYS */;


-- Dumping structure for table crucible.insurance_price
CREATE TABLE IF NOT EXISTS `insurance_price` (
  `typeID` int(10) unsigned NOT NULL,
  `price` double NOT NULL,
  PRIMARY KEY (`typeID`),
  KEY `typeID` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table crucible.insurance_price: ~0 rows (approximately)
/*!40000 ALTER TABLE `insurance_price` DISABLE KEYS */;
INSERT IGNORE INTO `insurance_price` (`typeID`, `price`) VALUES
	(600, 25000),
	(601, 25000),
	(602, 25000),
	(603, 25000),
	(604, 25000),
	(605, 25000),
	(606, 25000);
/*!40000 ALTER TABLE `insurance_price` ENABLE KEYS */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;