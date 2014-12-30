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

#include "EVEServerConfig.h"
#include "character/Character.h"
#include "character/CharacterDB.h"

CharacterDB::CharacterDB()
{
    load_name_validation_set();
}

bool CharacterDB::ReportRespec(uint32 characterId)
{
    DBerror error;
    if (!sDatabase.RunQuery(error, "UPDATE character_ SET freeRespecs = freeRespecs - 1, lastRespecDateTime = %" PRIu64 ", nextRespecDateTime = %" PRIu64 " WHERE characterId = %u AND freeRespecs > 0",
        Win32TimeNow(), Win32TimeNow() + Win32Time_Year, characterId))
        return false;
    return true;
}

bool CharacterDB::GetRespecInfo(uint32 characterId, uint32& out_freeRespecs, uint64& out_lastRespec, uint64& out_nextRespec)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT freeRespecs, lastRespecDateTime, nextRespecDateTime FROM character_ WHERE characterID = %u", characterId))
        return false;
    if (res.GetRowCount() < 1)
        return false;
    DBResultRow row;
    res.GetRow(row);
    out_freeRespecs = row.GetUInt(0);
    out_lastRespec = row.GetUInt64(1);
    out_nextRespec = row.GetUInt64(2);

    // can't have more than two
    if (out_freeRespecs == 2)
        out_nextRespec = 0;
    else if (out_freeRespecs < 2 && out_nextRespec < Win32TimeNow())
    {
        // you may get another
        out_freeRespecs++;
        if (out_freeRespecs == 1)
            out_nextRespec = Win32TimeNow() + Win32Time_Year;
        else
            out_nextRespec = 0;

        // reflect this in the database, too
        DBerror err;
        sDatabase.RunQuery(err, "UPDATE character_ SET freeRespecs = %u, nextRespecDateTime = %" PRIu64 " WHERE characterId = %u",
            out_freeRespecs, out_nextRespec, characterId);
    }

    return true;
}

uint64 CharacterDB::PrepareCharacterForDelete(uint32 accountID, uint32 charID)
{
    // calculate the point in time from which this character may be deleted
    uint64 deleteTime = Win32TimeNow() + (Win32Time_Second * sConfig.character.terminationDelay);

    // note: the queries relating to character deletion have been specifically designed to avoid wreaking havoc when used by a malicious client
    // the client can't lie to us about accountID, only charID

    DBerror error;
    uint32 affectedRows;
    sDatabase.RunQuery(error, affectedRows, "UPDATE character_ SET deletePrepareDateTime = %" PRIu64 " WHERE accountID = %u AND characterID = %u", deleteTime, accountID, charID);
    if (affectedRows != 1)
        return 0;

    return deleteTime;
}

void CharacterDB::CancelCharacterDeletePrepare(uint32 accountID, uint32 charID)
{
    DBerror error;
    uint32 affectedRows;
    sDatabase.RunQuery(error, affectedRows, "UPDATE character_ SET deletePrepareDateTime = 0 WHERE accountID = %u AND characterID = %u", accountID, charID);
    if (affectedRows != 1)
        codelog(CLIENT__ERROR, "Failed to cancel character deletion, affected rows: %u", affectedRows);
}

PyRep* CharacterDB::DeleteCharacter(uint32 accountID, uint32 charID)
{
    DBerror error;
    uint32 affectedRows;
    sDatabase.RunQuery(error, affectedRows, "DELETE FROM character_ WHERE deletePrepareDateTime > 0 AND deletePrepareDateTime <= %" PRIu64 " AND accountID = %u AND characterID = %u", Win32TimeNow(), accountID, charID);

    if (affectedRows == 1)
    {
        // valid request; this means we may use charID safely here
        sDatabase.RunQuery(error, "DELETE FROM entity WHERE ownerID = %u", charID);

		//  just a small list of possible locations to delete char references from....
		/**
		6 matches in avatar_colors
		15 matches in avatar_modifiers
		13 matches in avatar_sculpts
		1 match in avatars
		0 matches in bookmarkFolders
		0 matches in bookmarkVouchers
		1 match in bookmarks
		0 matches in bounties
		1 match in cacheOwners
		4 matches in channelChars
		0 matches in chrApplications
		1 match in chrCertificates
		1 match in chrEmployment
		31 matches in chrSkillHistory
		0 matches in chrSkillQueue
		0 matches in chrOwnerNote
		0 matches in chrPausedSkillQueue
		1 match in entity
		61 matches in entity_attributes
		0 matches in entity_attributesStatic
		60 matches in entity_default_attributes
		0 matches in ramJobs
		*/

        // indicates 'no error' to the client
        return NULL;
    }
    else
        return new PyString("Invalid delete request");
}

PyRep *CharacterDB::GetCharacterList(uint32 accountID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  characterID,"
        "  itemName AS characterName,"
        "  deletePrepareDateTime,"
        "  gender,"
        "  typeID"
        " FROM character_ "
        "  LEFT JOIN entity ON characterID = itemID"
        " WHERE accountID=%u", accountID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

bool CharacterDB::ValidateCharName(const char *name)
{
    if (name == NULL || *name == '\0')
        return false;

    /* hash the name */
    uint32 hash = djb2_hash(name);

    /* check if its in our std::set */
    CharValidationSetItr itr = mNameValidation.find(hash);

    /* if itr is not equal to the end of the set it means that the same hash has been found */
    if (itr != mNameValidation.end())
        return false;

    /* if we got here the name is "new" */
    return true;
}

PyRep *CharacterDB::GetCharSelectInfo(uint32 characterID) {
  /**  this shows char on select screen....added "SkillQueueEndTime" from here for 'current' data, if applicable...  -allan 31Mar14*/
    DBQueryResult res;

    uint32 worldSpaceID = 0;

    std::string shipName = "My Ship";
    uint32 shipTypeID = 606;  /// why is this hardcoded to the velator???

    DBQueryResult res2;
    if(!sDatabase.RunQuery(res2, "SELECT itemName, typeID FROM entity WHERE itemID = (SELECT shipID FROM character_ WHERE characterID = %u)", characterID)) {
        codelog(SERVICE__WARNING, "Unable to get current ship: %s", res.error.c_str());
    } else {
        DBResultRow row;
        while(res2.GetRow(row)) {
            sDatabase.DoEscapeString(shipName, row.GetText(0));
            shipTypeID = row.GetUInt(1);
        }
    }

    uint32 unreadMailCount = 0;
    uint32 upcomingEventCount = 0;
    uint32 unprocessedNotifications = 0;
    uint32 daysLeft = 14;
    uint32 userType = 23;
    uint64 allianceMemberStartDate = Win32TimeNow() - 15*Win32Time_Day;
    uint64 startDate = Win32TimeNow() - 24*Win32Time_Day;

    if(!sDatabase.RunQuery(res,
        "SELECT " // fixed DB query -allan 11Jan14
        "  entity.itemName AS shortName, "
        "  bloodlineTypes.bloodlineID, "
        "  ch.gender, "
        "  ch.bounty, "
        "  ch.corporationID, "
        "  corporation.allianceID, "
        "  ch.title, "
        "  ch.startDateTime, "
        "  ch.createDateTime, "
        "  ch.securityRating, "
        "  ch.balance, "
        "  ch.aurBalance, "
        "  ch.stationID, "
        "  ch.solarSystemID, "
        "  ch.constellationID, "
        "  ch.regionID, "
        "  ch.petitionMessage, "
        "  ch.logonMinutes, "
        "  corporation.tickerName, "
        "  %u AS worldSpaceID, "
        "  '%s' AS shipName, "
        "  %u AS shipTypeID, "
        "  %u AS unreadMailCount,"
        "  %u AS upcomingEventCount, "
        "  %u AS unprocessedNotifications, "
        "  %u AS daysLeft, "
        "  %u AS userType, "
        "  0 AS paperDollState, "
        "  0 AS newPaperdollState,"
        "  0 AS oldPaperdollState, "
        "  ch.skillPoints, "
        "  ch.skillQueueEndTime, "
        "  %" PRIu64 " AS allianceMemberStartDate, "
        "  %" PRIu64 " AS startDate, "
        "  0 AS locationSecurity "
        " FROM character_ AS ch"
        "    LEFT JOIN entity ON entity.itemID = ch.characterID "
        "    LEFT JOIN corporation USING (corporationID) "
        "    LEFT JOIN bloodlineTypes USING (typeID) "
        " WHERE characterID=%u", worldSpaceID, shipName.c_str(), shipTypeID, unreadMailCount, upcomingEventCount, unprocessedNotifications, daysLeft, userType, allianceMemberStartDate, startDate, characterID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToCRowset(res);
}

PyObject *CharacterDB::GetCharPublicInfo(uint32 characterID) {
    if(characterID < 140000000) {
        sLog.Error("CharacterDB::GetCharPublicInfo()", "Character %u is NPC.", characterID);
        return NULL;
    }
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "       // fixed DB Query   -allan 11Jan14
        "  entity.typeID,"
        "  entity.itemName AS characterName,"
        "  ch.corporationID,"
        "  chrBloodlines.raceID,"
        "  bloodlineTypes.bloodlineID,"
        "  ch.ancestryID,"
        "  ch.careerID,"
        "  ch.schoolID,"
        "  ch.careerSpecialityID,"
        "  ch.age,"
        "  ch.createDateTime,"
        "  ch.gender,"
        "  ch.characterID,"
        "  ch.description,"
        "  ch.corporationDateTime"
        " FROM character_ AS ch"
        "    LEFT JOIN entity ON entity.itemID = ch.characterID "
        "    LEFT JOIN bloodlineTypes USING (typeID)"
        "    LEFT JOIN chrBloodlines USING (bloodlineID)"
        " WHERE characterID=%u", characterID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(SERVICE__ERROR, "Error in GetCharPublicInfo query: no data for char %d", characterID);
        return NULL;
    }
    return(DBRowToKeyVal(row));

}

//void CharacterDB::GetCharacterData(uint32 characterID, std::vector<uint32> &characterDataVector) {
void CharacterDB::GetCharacterData(uint32 characterID, std::map<std::string, uint32> &characterDataMap) {

    DBQueryResult res;
    DBResultRow row;

    if(!sDatabase.RunQuery(res,
        "SELECT "       // fixed DB Query   -allan 11Jan14
        "  ch.corporationID, "
        "  ch.stationID, "
        "  ch.solarSystemID, "
        "  ch.constellationID, "
        "  ch.regionID, "
        "  corporation.stationID, "
        "  ch.corpRole, "
        "  ch.rolesAtAll, "
        "  ch.rolesAtBase, "
        "  ch.rolesAtHQ, "
        "  ch.rolesAtOther, "
        "  ch.shipID, "
        "  ch.gender, "
        "  entity.locationID "
        " FROM character_ AS ch"
        "    LEFT JOIN corporation USING (corporationID) "
        "    LEFT JOIN entity ON entity.itemID = ch.characterID "
        " WHERE characterID = %u",
        characterID))
    {
        sLog.Error("CharacterDB::GetCharPublicInfo2()", "Failed to query HQ of character's %u corporation: %s.", characterID, res.error.c_str());
    }

    if(!res.GetRow(row))
    {
        sLog.Error("CharacterDB::GetCharacterData()", "No valid rows were returned by the database query.");
        return;
    }

//    std::map<std::string,uint32> characterDataMap;
//    for( uint32 i=0; i<=characterDataVector.size(); i++ )
//        characterDataVector.push_back( row.GetUInt(i) );

    characterDataMap["corporationID"] = row.GetUInt(0);
    characterDataMap["stationID"] = row.GetUInt(1);
    characterDataMap["solarSystemID"] = row.GetUInt(2);
    characterDataMap["constellationID"] = row.GetUInt(3);
    characterDataMap["regionID"] = row.GetUInt(4);
    characterDataMap["corporationHQ"] = row.GetUInt(5);
    characterDataMap["corpRole"] = row.GetUInt64(6);
    characterDataMap["rolesAtAll"] = row.GetUInt64(7);
    characterDataMap["rolesAtBase"] = row.GetUInt64(8);
    characterDataMap["rolesAtHQ"] = row.GetUInt64(9);
    characterDataMap["rolesAtOther"] = row.GetUInt64(10);
    characterDataMap["shipID"] = row.GetUInt(11);
    characterDataMap["gender"] = row.GetUInt(12);
    characterDataMap["locationID"] = row.GetUInt(13);
}

PyObject *CharacterDB::GetCharPublicInfo3(uint32 characterID) {

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "  bounty"
        " FROM character_ "
        " WHERE characterID=%u", characterID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

//just return all itemIDs which has ownerID set to characterID
bool CharacterDB::GetCharItems(uint32 characterID, std::vector<uint32> &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID"
        " FROM entity"
        " WHERE ownerID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query items of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row))
        into.push_back(row.GetUInt(0));

    return true;
}

//returns a list of the itemID for all the clones belonging to the character
bool CharacterDB::GetCharClones(uint32 characterID, std::vector<uint32> &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID"
        " FROM entity"
        " WHERE ownerID = %u"
        "   and flag='400'",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query clones of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row))
        into.push_back(row.GetUInt(0));

    return true;
}

//returns the itemID of the active clone
//if you want to get the typeID of the clone, please use GetActiveCloneType
bool CharacterDB::GetActiveClone(uint32 characterID, uint32 &itemID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID"
        " FROM entity"
        " WHERE ownerID = %u"
        "   and flag='400'"
        "   and customInfo='active'",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    res.GetRow(row);
    itemID=row.GetUInt(0);

    return true;
}

//we use this function because, when we change the clone type,
//the cached item type doesn't change, so we need to read it
//directly from the db
bool CharacterDB::GetActiveCloneType(uint32 characterID, uint32 &typeID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  typeID"
        " FROM entity"
        " WHERE ownerID = %u"
        "   and flag='400'"
        "   and customInfo='active'",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    res.GetRow(row);
    typeID=row.GetUInt(0);

    return true;
}

// Return the Home station of the char based on the active clone
bool CharacterDB::GetCharHomeStation(uint32 characterID, uint32 &stationID) {
	uint32 activeCloneID;
	if( !GetActiveClone(characterID, activeCloneID) )
	{
		_log( DATABASE__ERROR, "Could't get the active clone for char %u", characterID );
		return false;
	}

	DBQueryResult res;
	if( !sDatabase.RunQuery(res,
		"SELECT locationID "
		"FROM entity "
		"WHERE itemID = %u",
		activeCloneID ))
	{
		_log(DATABASE__ERROR, "Could't get the location of the clone for char %u", characterID );
		return false;
	}

	DBResultRow row;
    if(res.GetRow(row)) stationID = row.GetUInt(0);
	return true;
}

bool CharacterDB::GetAttributesFromAncestry(uint32 ancestryID, uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  intelligence, charisma, perception, memory, willpower "
        " FROM chrAncestries "
        " WHERE ancestryID = %u ", ancestryID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return (false);
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Failed to find ancestry information for ancestry %u", ancestryID);
        return false;
    }

    intelligence += row.GetUInt(0);
    charisma += row.GetUInt(1);
    perception += row.GetUInt(2);
    memory += row.GetUInt(3);
    willpower += row.GetUInt(4);

    return (true);
}

bool CharacterDB::GetCareerBySchool(uint32 schoolID, uint32 &careerID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT careerID FROM careers WHERE schoolID = %u", schoolID))  {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return (false);
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Failed to find matching career for school %u", schoolID);
        return false;
    }
    careerID = row.GetInt(0);
    return (true);
}

bool CharacterDB::GetCorporationBySchool(uint32 schoolID, uint32 &corporationID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res, "SELECT corporationID FROM chrSchools WHERE schoolID = %u", schoolID)) {
        codelog(SERVICE__ERROR, "Error in query: %S", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Failed to find matching corporation for school %u", schoolID);
        return false;
    }
    corporationID = row.GetInt(0);
    return true;
}

/**
  * @todo Here should come a call to Corp??::CharacterJoinToCorp or what the heck... for now we only put it there
  */
bool CharacterDB::GetLocationCorporationByCareer(CharacterData &cdata) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT "      // fixed DB Query   -allan 01/02/14
     "  chrSchools.corporationID, "
     "  chrSchools.schoolID, "
     "  corporation.allianceID, "
     "  corporation.stationID, "
     "  staStations.solarSystemID, "
     "  staStations.constellationID, "
     "  staStations.regionID "
     " FROM staStations"
     "    LEFT JOIN corporation ON corporation.stationID=staStations.stationID"
     "    LEFT JOIN chrSchools ON corporation.corporationID=chrSchools.corporationID"
     "    LEFT JOIN careers ON chrSchools.schoolID=careers.schoolID"
     " WHERE careers.careerID = %u", cdata.careerID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return (false);
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Failed to find career %u", cdata.careerID);
        return false;
    }

    cdata.corporationID = row.GetUInt(0);
    cdata.schoolID = row.GetUInt(1);
    cdata.allianceID = row.GetUInt(2);
    cdata.stationID = row.GetUInt(3);
    cdata.solarSystemID = row.GetUInt(4);
    cdata.constellationID = row.GetUInt(5);
    cdata.regionID = row.GetUInt(6);

    return (true);
}

bool CharacterDB::GetLocationByStation(uint32 staID, CharacterData &cdata) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT "
     "  stationID, "
     "  solarSystemID, "
     "  constellationID, "
     "  regionID "
     " FROM staStations"
     " WHERE stationID = %u", staID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return (false);
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Failed to find station %u", staID);
        return false;
    }

    cdata.stationID = staID;
    cdata.solarSystemID = row.GetUInt(1);
    cdata.constellationID = row.GetUInt(2);
    cdata.regionID = row.GetUInt(3);

    return (true);

}

bool CharacterDB::GetCareerStationByCorporation(uint32 corporationID, uint32 &stationID)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT stationID FROM corporation WHERE corporationID = %u", corporationID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row))
    {
        codelog(SERVICE__ERROR, "Failed to find corporation %u", corporationID);
        return false;
    }

    stationID = row.GetUInt(0);
    return true;
}

bool CharacterDB::DoesCorporationExist(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT "
     "  corporationID, "
     "  corporationName "
     " FROM corporation"
     " WHERE corporationID = %u", corpID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Failed to find corporation %u", corpID);
        return false;
    }
    return true;
}

void CharacterDB::SetAvatar(uint32 charID, PyRep* hairDarkness) {
	//populate the DB with avatar information
	DBerror err;
	if(!sDatabase.RunQuery(err,
		"INSERT INTO avatars ("
		"  charID, hairDarkness)"
		" VALUES (%u, %f)",
		charID, hairDarkness->AsFloat()->value()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarColors(uint32 charID, uint32 colorID, uint32 colorNameA, uint32 colorNameBC, double weight, double gloss) {
	//add avatar colors to the DB
	DBerror err;
	if(!sDatabase.RunQuery(err,
		"INSERT INTO avatar_colors ("
		"  charID, colorID, colorNameA, colorNameBC, weight, gloss)"
		" VALUES (%u, %u, %u, %u, %f, %f)",
		charID, colorID, colorNameA, colorNameBC, weight, gloss))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarModifiers(uint32 charID, PyRep* modifierLocationID,  PyRep* paperdollResourceID, PyRep* paperdollResourceVariation) {
	//add avatar modifiers to the DB
	DBerror err;
	if(!sDatabase.RunQuery(err,
		"INSERT INTO avatar_modifiers ("
		"  charID, modifierLocationID, paperdollResourceID, paperdollResourceVariation)"
		" VALUES (%u, %u, %u, %u)",
		charID,
		modifierLocationID->AsInt()->value(),
		paperdollResourceID->AsInt()->value(),
		paperdollResourceVariation->IsInt() ? paperdollResourceVariation->AsInt()->value() : 0 ))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarSculpts(uint32 charID, PyRep* sculptLocationID, PyRep* weightUpDown, PyRep* weightLeftRight, PyRep* weightForwardBack) {
	//add avatar sculpts to the DB
	DBerror err;
	if(!sDatabase.RunQuery(err,
		"INSERT INTO avatar_sculpts ("
		"  charID, sculptLocationID, weightUpDown, weightLeftRight, weightForwardBack)"
		" VALUES (%u, %u, %f, %f, %f)",
		charID,
		sculptLocationID->AsInt()->value(),
		weightUpDown->IsFloat() ? weightUpDown->AsFloat()->value() : 0.0,
		weightLeftRight->IsFloat() ? weightLeftRight->AsFloat()->value() : 0.0,
		weightForwardBack->IsFloat() ? weightForwardBack->AsFloat()->value() : 0.0))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
	}
}

bool CharacterDB::GetBaseSkills(std::map<uint32, uint32> &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  skillTypeID, level"
        " FROM sklBaseSkills "))
    {
        _log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row)) {
        into[row.GetUInt(0)] == row.GetUInt(1);
    }
    return true;
}

bool CharacterDB::GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  skillTypeID, level"
        " FROM sklRaceSkills "
        " WHERE raceID = %u ", raceID))
    {
        _log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row)) {
        if(into.find(row.GetUInt(0)) == into.end())
            into[row.GetUInt(0)] = row.GetUInt(1);
        else
            into[row.GetUInt(0)] += row.GetUInt(1);
        //check to avoid more than 5 levels of a skill
        if(into[row.GetUInt(0)] > 5)
            into[row.GetUInt(0)] = 5;
    }

    return true;
}

bool CharacterDB::GetSkillsByCareer(uint32 careerID, std::map<uint32, uint32> &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  skillTypeID, level"
        " FROM sklCareerSkills"
        " WHERE careerID = %u", careerID))
    {
        _log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row)) {
        if(into.find(row.GetUInt(0)) == into.end())
            into[row.GetUInt(0)] = row.GetUInt(1);
        else
            into[row.GetUInt(0)] += row.GetUInt(1);
        //check to avoid more than 5 levels of a skill
        if(into[row.GetUInt(0)] > 5)
            into[row.GetUInt(0)] = 5;
    }

    return true;
}

PyString *CharacterDB::GetNote(uint32 ownerID, uint32 itemID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
            "SELECT `note` FROM `chrNotes` WHERE ownerID = %u AND itemID = %u",
            ownerID, itemID)
        )
    {
        codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if(!res.GetRow(row))
        return NULL;

    return(new PyString(row.GetText(0)));
}

bool CharacterDB::SetNote(uint32 ownerID, uint32 itemID, const char *str) {
    DBerror err;

    if (str[0] == '\0') {
        // str is empty
        if (!sDatabase.RunQuery(err,
            "DELETE FROM `chrNotes` "
            " WHERE itemID = %u AND ownerID = %u LIMIT 1",
             itemID, ownerID)
            )
        {
            codelog(CLIENT__ERROR, "Error on query: %s", err.c_str());
            return false;
        }
    } else {
        // escape it before insertion
        std::string escaped;
        sDatabase.DoEscapeString(escaped, str);

        if (!sDatabase.RunQuery(err,
            "REPLACE INTO `chrNotes` ( ownerID, itemID, note)"
            " VALUES (%u, %u, '%s')",
            ownerID, itemID, escaped.c_str())
            )
        {
            codelog(CLIENT__ERROR, "Error on query: %s", err.c_str());
            return false;
        }
    }

    return true;
}

uint32 CharacterDB::AddOwnerNote(uint32 charID, const std::string & label, const std::string & content) {
    DBerror err;
    uint32 id;

    std::string lblS;
    sDatabase.DoEscapeString(lblS, label);

    std::string contS;
    sDatabase.DoEscapeString(contS, content);

    if (!sDatabase.RunQueryLID(err, id,
        "INSERT INTO chrOwnerNote (ownerID, label, note) VALUES (%u, '%s', '%s');",
        charID, lblS.c_str(), contS.c_str()))
    {
        codelog(SERVICE__ERROR, "Error on query: %s", err.c_str());
        return 0;
    }

    return id;
}

bool CharacterDB::EditOwnerNote(uint32 charID, uint32 noteID, const std::string & label, const std::string & content) {
    DBerror err;

    std::string contS;
    sDatabase.DoEscapeString(contS, content);

    if (!sDatabase.RunQuery(err,
        "UPDATE chrOwnerNote SET note = '%s' WHERE ownerID = %u AND noteID = %u;",
        contS.c_str(), charID, noteID))
    {
        codelog(SERVICE__ERROR, "Error on query: %s", err.c_str());
        return false;
    }

    return true;
}

PyObjectEx *CharacterDB::GetOwnerNoteLabels(uint32 charID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT noteID, label FROM chrOwnerNote WHERE ownerID = %u", charID))
    {
        codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
        return (NULL);
    }

    return DBResultToCRowset(res);
}

PyObjectEx *CharacterDB::GetOwnerNote(uint32 charID, uint32 noteID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT note FROM chrOwnerNote WHERE ownerID = %u AND noteID = %u", charID, noteID))
    {
        codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
        return (NULL);
    }

    return DBResultToCRowset(res);
}

uint32 CharacterDB::djb2_hash( const char* str )
{
    uint32 hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void CharacterDB::load_name_validation_set()
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  characterID, itemName AS characterName"
        " FROM character_"
        "    JOIN entity ON characterID = itemID"
        ))
    {
        codelog(SERVICE__ERROR, "Error in query for %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    while(res.GetRow(row) == true)
    {
        uint32 characterID = row.GetUInt(0);
        const char* name = row.GetText(1);

        //printf("initializing name validation: %s\n", name);
        uint32 hash = djb2_hash(name);

        mNameValidation.insert(hash);
        mIdNameContainer.insert(std::make_pair(characterID, name));
    }
}

bool CharacterDB::add_name_validation_set( const char* name, uint32 characterID )
{
    if (name == NULL || *name == '\0')
        return false;

    uint32 hash = djb2_hash(name);

    /* check if the name is already present ( this should not be possible but we all know how hackers are ) */
    if (mNameValidation.find(hash) != mNameValidation.end())
    {
        printf("CharacterDB::add_name_validation_set: unable to add: %s as its a dupe", name);
        return false;
    }

    mNameValidation.insert(hash);
    mIdNameContainer.insert(std::make_pair(characterID, name));
    return true;
}

bool CharacterDB::del_name_validation_set( uint32 characterID )
{
    CharIdNameMapItr helper_itr = mIdNameContainer.find(characterID);

    /* if we are unable to find the entry... return.
     * @note we do risk keeping the name in the name validation.
     * which I am willing to take.
     */
    if (helper_itr == mIdNameContainer.end())
        return false;

    const char* name = helper_itr->second.c_str();
    if (name == NULL || *name == '\0')
        return false;

    uint32 hash = djb2_hash(name);

    CharValidationSetItr name_itr = mNameValidation.find(hash);
    if (name_itr != mNameValidation.end())
    {
        // we found the name hash... deleting
        mNameValidation.erase(name_itr);
        mIdNameContainer.erase(helper_itr);
        return true;
    }
    else
    {
        /* normally this should never happen... */
        printf("CharacterDB::del_name_validation_set: unable to remove: %s as its not in the set", name);
        return false;
    }
}

double CharacterDB::GetCharRawStandingFromNPC(uint32 characterID, uint32 itemID)
{
    DBQueryResult res;

    if(!sDatabase.RunQuery(res, "SELECT standing FROM chrNPCStandings WHERE characterID=%u AND fromID=%u", characterID, itemID ) )
    {
        _log(SERVICE__ERROR, "Error in GetCharNPCStandings query: %s", res.error.c_str());
        return 0;
    }

    if(res.GetRowCount() == 0)
        return 0;
    DBResultRow row;
    res.GetRow(row);
    return row.GetDouble(0);
}

//  this needs work
void CharacterDB::SetCharNPCStandings(uint32 characterID) {
    /*
    DBerror err;

    sDatabase.RunQuery(err,
                       "INSERT INTO `chrStandings`(`characterID`, `toID`, `standing`) "
                       "VALUES (%u,[value-2],[value-3])", characterID );
    */
}

bool CharacterDB::LoadSkillQueue(uint32 characterID, SkillQueue &into) {
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT"
        "  typeID, level"
        " FROM chrSkillQueue"
        " WHERE characterID = %u"
        " ORDER BY orderIndex ASC",
        characterID ) )
    {
        _log(DATABASE__ERROR, "Failed to query skill queue of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while( res.GetRow( row ) )
    {
        QueuedSkill qs;
        qs.typeID = row.GetUInt( 0 );
        qs.level = row.GetUInt( 1 );

        into.push_back( qs );
    }

    return true;
}

bool CharacterDB::LoadPausedSkillQueue(uint32 characterID, SkillQueue &into) {
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT"
        "  typeID, level"
        " FROM chrPausedSkillQueue"
        " WHERE characterID = %u"
        " ORDER BY orderIndex ASC",
        characterID ) )
    {
        _log(DATABASE__ERROR, "Failed to query skill queue of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while( res.GetRow( row ) )
    {
        QueuedSkill qs;
        qs.typeID = row.GetUInt( 0 );
        qs.level = row.GetUInt( 1 );

        into.push_back( qs );
    }

    // now, delete paused queue because subsquent pressess of 'apply' button will add paused queue again, and again, etc...
    DBerror err;
    if( !sDatabase.RunQuery( err,
        "DELETE FROM chrPausedSkillQueue"
        " WHERE characterID = %u",
        characterID ) )
    {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool CharacterDB::SaveSkillQueue(uint32 characterID, SkillQueue &queue) {
    DBerror err;

    if( !sDatabase.RunQuery( err,
        "DELETE FROM chrSkillQueue"
        " WHERE characterID = %u",
        characterID ) )
    {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    if( queue.empty() )
        // nothing else to do
        return true;

    // now build insert query:
    std::string query;

    for(size_t i = 0; i < queue.size(); i++)
    {
        const QueuedSkill &qs = queue[ i ];

        char buf[ 64 ];
        snprintf( buf, 64, "(%u, %lu, %u, %u)", characterID, (unsigned long)i, qs.typeID, qs.level );

        if( i != 0 )
            query += ',';
        query += buf;
    }

    if( !sDatabase.RunQuery( err,
        "INSERT"
        " INTO chrSkillQueue (characterID, orderIndex, typeID, level)"
        " VALUES %s",
        query.c_str() ) )
    {
        _log(DATABASE__ERROR, "Failed to insert skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool CharacterDB::SavePausedSkillQueue(uint32 characterID, SkillQueue &queue) {
    DBerror err;

    if( !sDatabase.RunQuery( err,
        "DELETE FROM chrPausedSkillQueue"
        " WHERE characterID = %u",
        characterID ) )
    {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    if( queue.empty() )
        // nothing else to do
        return true;

    std::string query;

    for(size_t i = 0; i < queue.size(); i++)
    {
        const QueuedSkill &qs = queue[ i ];

        char buf[ 64 ];
        snprintf( buf, 64, "(%u, %lu, %u, %u)", characterID, (unsigned long)i, qs.typeID, qs.level );

        if( i != 0 )
            query += ',';
        query += buf;
    }

    if( !sDatabase.RunQuery( err,
        "INSERT"
        " INTO chrPausedSkillQueue (characterID, orderIndex, typeID, level)"
        " VALUES %s",
        query.c_str() ) )
    {
        _log(DATABASE__ERROR, "Failed to insert paused skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

void CharacterDB::SaveSkillHistory(int eventID, double logDate, uint32 characterID, uint32 skillTypeID, int skillLevel, double relativePoints, double absolutePoints) {
    DBerror err;
    if( !sDatabase.RunQuery( err,
        "INSERT INTO chrSkillHistory (eventTypeID, logDate, characterID, skillTypeID, skillLevel, relativePoints, absolutePoints)"
        " VALUES (%u, %f, %u, %u, %u, %f, %f)", eventID, logDate, characterID, skillTypeID, skillLevel, relativePoints, absolutePoints ))
            _log(DATABASE__ERROR, "Failed to set chrSkillHistory for character %u: %s", characterID, err.c_str());
}

PyObject* CharacterDB::GetSkillHistory(uint32 characterID) {
    // eventTypeIDs:
    // 34 - SkillClonePenalty
    // 36 - SkillTrainingStarted
    // 37 - SkillTrainingComplete
    // 38 - SkillTrainingCanceled
    // 39 - GMGiveSkill
    // 307 - SkillPointsApplied

    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT logDate, eventTypeID, skillTypeID, relativePoints AS absolutePoints"
        " FROM chrSkillHistory"
        " WHERE characterID = %d"
        " ORDER BY logDate DESC"
        " LIMIT 100",
        characterID )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToRowset(res));
}

void CharacterDB::SetLoginTime(uint32 characterID) {
	//    char just logged in...set time im db.
	//  TODO:  change to mem object to avoid DB hits....

    DBerror err;
    if( !sDatabase.RunQuery( err, "UPDATE character_ SET logonDateTime = %" PRIu64 " WHERE characterID = %u ", Win32TimeNow(), characterID )) {
        codelog(SERVICE__ERROR, "CharacterDB::SetLoginTime - Error in query: %s", err.c_str());
    }
}

void CharacterDB::UpdateLoginTime(uint32 characterID) {
        // updated logonDateTime to now....last check was then to now, so remove that time and reset
        DBerror err;
        if( !sDatabase.RunQuery( err, "UPDATE character_ SET logonDateTime = %" PRIu64 " WHERE characterID = %u ", Win32TimeNow(), characterID )) {
            codelog(SERVICE__ERROR, "CharacterDB::UpdateLoginTime - Error in query: %s", err.c_str());
        }
}

EvilNumber CharacterDB::GetLoginTime(uint32 characterID) {
    //  logged as Win32TimeNow();
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT logonDateTime FROM character_ WHERE characterID = %u", characterID );

    DBResultRow row;
    uint32 loginTime = 0;
    if(res.GetRow(row)) {
        uint64 logonDateTime = row.GetUInt64(0);
        if (logonDateTime > 0 ) loginTime = ( (Win32TimeNow() - logonDateTime ) / 1000000000);  // * 2;
    }
    return loginTime;
}

bool CharacterDB::isOffline(uint32 characterID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT Online FROM character_ WHERE characterID = %u", characterID );

    DBResultRow row;
    return(!res.GetRow(row));
}

void CharacterDB::addOwnerCache(uint32 ownerID, std::string ownerName, uint32 typeID) {
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO cacheOwners(ownerID, ownerName, typeID)"
        " VALUES (%u, %s, %u)",
        ownerID, ownerName.c_str(), typeID);
}

PyRep* CharacterDB::GetBounty(uint32 charID, uint32 ownerID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT characterID, bounty FROM character_ WHERE bounty > 0 ORDER BY bounty DESC");
    return(DBResultToRowset(res));
}

PyRep* CharacterDB::GetTopBounties() {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT characterID, bounty FROM character_ WHERE bounty > 0 ORDER BY bounty DESC LIMIT 15");
    return(DBResultToRowset(res));
}

void CharacterDB::AddBounty(uint32 charID, uint32 ownerID, uint32 amount) {
    DBerror err;

    sDatabase.RunQuery(err,
        "UPDATE character_ SET bounty = bounty + %u WHERE characterID = %u",
        amount, charID);

    sDatabase.RunQuery(err,
        "INSERT INTO webBounties(characterID, ownerID, bounty, timePlaced)"
        " VALUES (%u, %u, %u, UNIX_TIMESTAMP(CURRENT_TIMESTAMP) )",
        charID, ownerID, amount );
}

void CharacterDB::VisitSystem(uint32 solarSystemID, uint32 charID) {
    DBQueryResult res;
    sDatabase.RunQuery(res,
      "SELECT visits FROM chrVisitedSystems WHERE characterID = %u AND solarSystemID = %u",
      charID, solarSystemID
      );

    DBResultRow row;
    uint16 visits = 0;
    if(res.GetRow(row)) visits = row.GetUInt(0);
    visits ++;

    DBerror err;
    if (visits > 1) {
      sDatabase.RunQuery(err,
        "UPDATE chrVisitedSystems SET visits = %u, lastDateTime = %" PRIu64 " WHERE characterID = %u AND solarSystemID = %u",
        visits, Win32TimeNow(), charID, solarSystemID
        );
    }else{
      sDatabase.RunQuery(err,
        "INSERT INTO chrVisitedSystems (characterID, solarSystemID, visits, lastDateTime)"
        "VALUES (%u, %u, %u, %" PRIu64 ")", charID, solarSystemID, visits, Win32TimeNow()
        );
    }
}

void CharacterDB::chkDynamicSystemID(uint32 systemID) {
  /**  this ensures mapDynamicData.solarSystemID for `systemID` is in the DB for later calls. -allan 16Mar14 */
    DBQueryResult chk;
    sDatabase.RunQuery(chk, "SELECT solarSystemID FROM mapDynamicData WHERE solarSystemID = %u", systemID );

    DBResultRow row;
    if(chk.GetRow(row)) {
        sLog.Success("CharacterDB::chkDynamicSystemID"," System %u already in DB", systemID );
    } else {
        DBerror err;
        sDatabase.RunQuery(err, "INSERT INTO mapDynamicData (solarSystemID) VALUES (%u)", systemID );
        sLog.Warning("CharacterDB::chkDynamicSystemID"," System %u inserted in DB", systemID );
    }
}

/** the following functions rely on solarSystemID being in the mapDynamicData table.
  * the check is called before these are used, and solarSystemID is then verified for existance and added if needed.
  *   the function is as follows and is declared above...
  *         void SystemDB::chkDynamicSystemID(uint32 systemID)
  *
  *  NOTE: these will have to be reset each server start for true dynamic data tracking
  *        really should trunicate table on restart after everything is working.
  */

void CharacterDB::AddJumpToDynamicData(uint32 solarSystemID) {
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET jumpsHour = jumpsHour + 1 WHERE solarSystemID = %u", solarSystemID );
}

void CharacterDB::AddPilotToDynamicData(uint32 solarSystemID, bool isDocked, bool isLogin) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT pilotsDocked, pilotsInSpace FROM mapDynamicData WHERE solarSystemID = %u", solarSystemID );

    DBResultRow row;
    uint16 docked = 0, space = 0;
    if(res.GetRow(row)) {
        docked = row.GetUInt(0);
        space = row.GetUInt(1);
    }

    if(isLogin) {
        if(isDocked) docked ++; else space ++;
    } else {
        if(isDocked) {
            docked ++;
            space --;
        } else {
            docked --;
            space ++;
        }
    }

    if (docked < 0 || docked > 100) docked = 0;
    if (space < 0 || space > 100) space = 0;

    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET pilotsDocked = %u, pilotsInSpace = %u, pilotsDateTime = %" PRIu64 " WHERE solarSystemID = %u", docked, space, Win32TimeNow(), solarSystemID );
}

void CharacterDB::AddKillToDynamicData(uint32 solarSystemID) {  /**killsHour, kills24Hours */
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE mapDynamicData SET killsHour = killsHour + 1, kills24Hour = kills24Hour + 1, kills24DateTime = %" PRIu64 " WHERE solarSystemID = %u",
        Win32TimeNow(), solarSystemID );
}

void CharacterDB::AddPodKillToDynamicData(uint32 solarSystemID) {   /**podKillsHour, podKills24Hour */
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE mapDynamicData SET podKillsHour = podKillsHour + 1, podKills24Hour = podKills24Hour + 1, pod24DateTime = %" PRIu64 " WHERE solarSystemID = %u", Win32TimeNow(), solarSystemID );
}

void CharacterDB::AddFactionKillToDynamicData(uint32 solarSystemID) {     /**factionKills*/
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE mapDynamicData SET factionKills = factionKills + 1, factionKills24Hour = factionKills24Hour + 1, faction24DateTime = %" PRIu64 " WHERE solarSystemID = %u", Win32TimeNow(), solarSystemID );
}

uint16 CharacterDB::GetActivePilotsFromDynamicData(uint32 solarSystemID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT pilotsDocked, pilotsInSpace FROM mapDynamicData WHERE solarSystemID = %u", solarSystemID );

    DBResultRow row;
    uint16 pilotsDocked = 0, pilotsInSpace = 0;
    if(res.GetRow(row)) {
        pilotsDocked = row.GetUInt(0);
        pilotsInSpace = row.GetUInt(1);
    }
    return(pilotsDocked, pilotsInSpace);
}
