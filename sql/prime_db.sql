/*
 * This file is here to deal with some tables needing to contain both
 * static and dynamic information. The solution for ease of release is
 * to keep all the static info in a seperate copy of the tables such
 * that it can be loaded into the non-static table.
 */

/*
 * Truncate all the dynamic tables
 */
TRUNCATE TABLE alliance_ShortNames;
TRUNCATE TABLE billsPayable;
TRUNCATE TABLE billsReceivable;
TRUNCATE TABLE bookmarks;
TRUNCATE TABLE cacheLocations;
TRUNCATE TABLE cacheOwners;
TRUNCATE TABLE channelChars;
TRUNCATE TABLE channelMods;
TRUNCATE TABLE character_;
TRUNCATE TABLE chrApplications;
TRUNCATE TABLE chrEmployment;
TRUNCATE TABLE chrMissionState;
TRUNCATE TABLE chrNotes;
TRUNCATE TABLE chrNPCStandings;
TRUNCATE TABLE chrOffers;
TRUNCATE TABLE chrOwnerNote;
TRUNCATE TABLE chrSkillQueue;
TRUNCATE TABLE chrStandings;
TRUNCATE TABLE corporation;
TRUNCATE TABLE courierMissions;
TRUNCATE TABLE crpCharShares;
TRUNCATE TABLE crpOffices;
TRUNCATE TABLE droneState;
TRUNCATE TABLE entity;
TRUNCATE TABLE entity_attributes;
TRUNCATE TABLE eveMail;
TRUNCATE TABLE eveMailDetails;
TRUNCATE TABLE invBlueprints;
TRUNCATE TABLE market_history_old;
TRUNCATE TABLE market_journal;
TRUNCATE TABLE market_orders;
TRUNCATE TABLE market_transactions;
TRUNCATE TABLE ramAssemblyLineStationCostLogs;
TRUNCATE TABLE ramJobs;
TRUNCATE TABLE rentalInfo;

/*
 * Copy over the static entities:
 * Static record of EVE System
 */
INSERT INTO entity (itemID, itemName, singleton, quantity)
 VALUES (1, 'EVE System', 1, 1);
/*
 * Insert solar systems
 */
INSERT INTO entity (itemID, itemName, typeID, ownerID, singleton, quantity, x, y, z)
 SELECT solarSystemID, solarSystemName, 5, 1, 1, 1, x, y, z
 FROM mapSolarSystems;
/*
 * Insert stations
 */
INSERT INTO entity (itemID, itemName, typeID, ownerID, locationID, singleton, quantity, x, y, z)
 SELECT stationID, stationName, stationTypeID, corporationID, solarSystemID, 1, 1, x, y, z
 FROM staStations;
/*
 * Insert characters
 */
INSERT INTO entity (itemID, itemName, typeID, ownerID, locationID, singleton, quantity)
 SELECT characterID, characterName, typeID, 1, stationID, 1, 1
  FROM characterStatic;

/*
 * Copy over the static corporation info
 */
INSERT INTO corporation
 SELECT * FROM corporationStatic;

/*
 * Copy over the static owner info.
 * This is a bit hacky: we rebuild this table although it's static but it
 * allows us not to include its data in dump.
 */
TRUNCATE TABLE eveStaticOwners;
/*
 * Static record of EVE System
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 VALUES (1, 'EVE System', 0);
/*
 * Insert agents
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT characterID, characterName, typeID
 FROM characterStatic;
/*
 * Insert factions
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT factionID, factionName, 30 AS typeID
 FROM chrFactions;
/*
 * Insert corporations
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT corporationID, corporationName, 2 AS typeID
 FROM corporationStatic;


INSERT INTO cacheLocations(locationID, locationName, x, y, z)
 SELECT e.itemID, e.itemName, e.x, e.y, e.z
 FROM entity AS e
 LEFT JOIN invTypes USING (typeID)
 LEFT JOIN invGroups AS g USING (groupID)
 WHERE g.categoryID IN (0, 2, 3, 6, 22, 23);


INSERT INTO cacheOwners(ownerID, ownerName, typeID)
 SELECT e.itemID, e.itemName, e.typeID
 FROM entity AS e
 LEFT JOIN invTypes USING (typeID)
 WHERE invTypes.groupID IN ( /*1,*/ 2, 19, 32 );      /*char, corp, faction, alliance*/