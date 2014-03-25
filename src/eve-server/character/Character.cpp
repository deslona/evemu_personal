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
    Author:     Zhur, Bloody.Rabbit, Allan
*/

#include "eve-server.h"
#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"
#include "character/Character.h"
#include "inventory/AttributeEnum.h"

/*
 * CharacterTypeData
 */
CharacterTypeData::CharacterTypeData(
    const char *_bloodlineName,
    EVERace _race,
    const char *_desc,
    const char *_maleDesc,
    const char *_femaleDesc,
    uint32 _shipTypeID,
    uint32 _corporationID,
    uint8 _perception,
    uint8 _willpower,
    uint8 _charisma,
    uint8 _memory,
    uint8 _intelligence,
    const char *_shortDesc,
    const char *_shortMaleDesc,
    const char *_shortFemaleDesc)
: bloodlineName(_bloodlineName),
  race(_race),
  description(_desc),
  maleDescription(_maleDesc),
  femaleDescription(_femaleDesc),
  shipTypeID(_shipTypeID),
  corporationID(_corporationID),
  perception(_perception),
  willpower(_willpower),
  charisma(_charisma),
  memory(_memory),
  intelligence(_intelligence),
  shortDescription(_shortDesc),
  shortMaleDescription(_shortMaleDesc),
  shortFemaleDescription(_shortFemaleDesc)
{
}

/*
 * CharacterType
 */
CharacterType::CharacterType(
    uint32 _id,
    uint8 _bloodlineID,
    // ItemType stuff:
    const ItemGroup &_group,
    const TypeData &_data,
    // CharacterType stuff:
    const ItemType &_shipType,
    const CharacterTypeData &_charData)
: ItemType(_id, _group, _data),
  m_bloodlineID(_bloodlineID),
  m_bloodlineName(_charData.bloodlineName),
  m_description(_charData.description),
  m_maleDescription(_charData.maleDescription),
  m_femaleDescription(_charData.femaleDescription),
  m_shipType(_shipType),
  m_corporationID(_charData.corporationID),
  m_perception(_charData.perception),
  m_willpower(_charData.willpower),
  m_charisma(_charData.charisma),
  m_memory(_charData.memory),
  m_intelligence(_charData.intelligence),
  m_shortDescription(_charData.shortDescription),
  m_shortMaleDescription(_charData.shortMaleDescription),
  m_shortFemaleDescription(_charData.shortFemaleDescription)
{
    // check for consistency
    assert(_data.race == _charData.race);
    assert(_charData.shipTypeID == _shipType.id());
}

CharacterType *CharacterType::Load(ItemFactory &factory, uint32 characterTypeID)
{
    return ItemType::Load<CharacterType>( factory, characterTypeID );
}

template<class _Ty>
_Ty *CharacterType::_LoadCharacterType(ItemFactory &factory, uint32 typeID, uint8 bloodlineID,
    // ItemType stuff:
    const ItemGroup &group, const TypeData &data,
    // CharacterType stuff:
    const ItemType &shipType, const CharacterTypeData &charData)
{
    // enough data for construction
    return new CharacterType( typeID, bloodlineID, group, data, shipType, charData );
}

/*
 * CharacterData
 */
CharacterData::CharacterData(
    uint32 _accountID,
    const char *_title,
    const char *_desc,
    bool _gender,
    double _bounty,
    double _balance,
    double _aurBalance,
    double _securityRating,
    uint32 _logonMinutes,
    double _skillPoints,
    uint32 _corporationID,
    uint32 _allianceID,
    uint32 _warFactionID,
    uint32 _stationID,
    uint32 _solarSystemID,
    uint32 _constellationID,
    uint32 _regionID,
    uint32 _ancestryID,
    uint32 _careerID,
    uint32 _schoolID,
    uint32 _careerSpecialityID,
    uint64 _startDateTime,
    uint64 _createDateTime,
    uint64 _corporationDateTime,
    uint32 _shipID)
: accountID(_accountID),
  title(_title),
  description(_desc),
  gender(_gender),
  bounty(_bounty),
  balance(_balance),
  aurBalance(_aurBalance),
  securityRating(_securityRating),
  logonMinutes(_logonMinutes),
  skillPoints(_skillPoints),
  corporationID(_corporationID),
  allianceID(_allianceID),
  warFactionID(_warFactionID),
  stationID(_stationID),
  solarSystemID(_solarSystemID),
  constellationID(_constellationID),
  regionID(_regionID),
  ancestryID(_ancestryID),
  careerID(_careerID),
  schoolID(_schoolID),
  careerSpecialityID(_careerSpecialityID),
  startDateTime(_startDateTime),
  createDateTime(_createDateTime),
  corporationDateTime(_corporationDateTime),
  shipID(_shipID)
{
}

/*
 * CharacterAppearance
 */

void CharacterAppearance::Build(uint32 ownerID, PyDict* data)
{
	CharacterAppearance capp;
	PyList* colors = new PyList();
	PyList* modifiers = new PyList();
	PyObjectEx* appearance;
	PyList* sculpts = new PyList();

	colors = data->GetItemString("colors")->AsList();
	modifiers = data->GetItemString("modifiers")->AsList();
	appearance = data->GetItemString("appearance")->AsObjectEx();
	sculpts = data->GetItemString("sculpts")->AsList();

	PyList::const_iterator color_cur, color_end;
	color_cur = colors->begin();
	color_end = colors->end();

	for(; color_cur != color_end; color_cur++)
	{
		if((*color_cur)->IsObjectEx())
		{
			PyObjectEx_Type2* color_obj = (PyObjectEx_Type2*)(*color_cur)->AsObjectEx();
			PyTuple* color_tuple = color_obj->GetArgs()->AsTuple();

			//color tuple data structure
			//[0] PyToken
			//[1] colorID
			//[2] colorNameA
			//[3] colorNameBC
			//[4] weight
			//[5] gloss

			m_db.SetAvatarColors(ownerID,
								color_tuple->GetItem(1)->AsInt()->value(),
								color_tuple->GetItem(2)->AsInt()->value(),
								color_tuple->GetItem(3)->AsInt()->value(),
								color_tuple->GetItem(4)->AsFloat()->value(),
								color_tuple->GetItem(5)->AsFloat()->value());

		}
	}

	PyObjectEx_Type2* app_obj = (PyObjectEx_Type2*)appearance;
	PyTuple* app_tuple = app_obj->GetArgs()->AsTuple();

	m_db.SetAvatar(ownerID, app_tuple->GetItem(1));

	PyList::const_iterator modif_cur, modif_end;
	modif_cur = modifiers->begin();
	modif_end = modifiers->end();

	for(; modif_cur != modif_end; modif_cur++)
	{
		if((*modif_cur)->IsObjectEx())
		{
			PyObjectEx_Type2* modif_obj = (PyObjectEx_Type2*)(*modif_cur)->AsObjectEx();
			PyTuple* modif_tuple = modif_obj->GetArgs()->AsTuple();

			//color tuple data structure
			//[0] PyToken
			//[1] modifierLocationID
			//[2] paperdollResourceID
			//[3] paperdollResourceVariation
			m_db.SetAvatarModifiers(ownerID,
										modif_tuple->GetItem(1),
										modif_tuple->GetItem(2),
										modif_tuple->GetItem(3));
		}
	}

	PyList::const_iterator sculpt_cur, sculpt_end;
	sculpt_cur = sculpts->begin();
	sculpt_end = sculpts->end();

	for(; sculpt_cur != sculpt_end; sculpt_cur++)
	{
		if((*sculpt_cur)->IsObjectEx())
		{
			PyObjectEx_Type2* sculpt_obj = (PyObjectEx_Type2*)(*sculpt_cur)->AsObjectEx();
			PyTuple* sculpt_tuple = sculpt_obj->GetArgs()->AsTuple();

			//sculpts tuple data structure
			//[0] PyToken
			//[1] sculptLocationID
			//[2] weightUpDown
			//[3] weightLeftRight
			//[4] weightForwardBack

			m_db.SetAvatarSculpts(ownerID,
									sculpt_tuple->GetItem(1),
									sculpt_tuple->GetItem(2),
									sculpt_tuple->GetItem(3),
									sculpt_tuple->GetItem(4));

		}
	}
}


/*
 * CorpMemberInfo
 */
CorpMemberInfo::CorpMemberInfo(
    uint32 _corpHQ,
    uint64 _corpRole,
    uint64 _rolesAtAll,
    uint64 _rolesAtBase,
    uint64 _rolesAtHQ,
    uint64 _rolesAtOther)
: corpHQ(_corpHQ),
  corpRole(_corpRole),
  rolesAtAll(_rolesAtAll),
  rolesAtBase(_rolesAtBase),
  rolesAtHQ(_rolesAtHQ),
  rolesAtOther(_rolesAtOther)
{
}

/*
 * Character
 */
Character::Character(
    ItemFactory &_factory,
    uint32 _characterID,
    // InventoryItem stuff:
    const CharacterType &_charType,
    const ItemData &_data,
    // Character stuff:
    const CharacterData &_charData,
    const CorpMemberInfo &_corpData)
: Owner(_factory, _characterID, _charType, _data),
  m_accountID(_charData.accountID),
  m_title(_charData.title),
  m_description(_charData.description),
  m_gender(_charData.gender),
  m_bounty(_charData.bounty),
  m_balance(_charData.balance),
  m_aurBalance(_charData.aurBalance),
  m_securityRating(_charData.securityRating),
  m_logonMinutes(_charData.logonMinutes),
  m_totalSPtrained(((double)(_charData.skillPoints))),
  m_corporationID(_charData.corporationID),
  m_corpHQ(_corpData.corpHQ),
  m_allianceID(_charData.allianceID),
  m_warFactionID(_charData.warFactionID),
  m_corpRole(_corpData.corpRole),
  m_rolesAtAll(_corpData.rolesAtAll),
  m_rolesAtBase(_corpData.rolesAtBase),
  m_rolesAtHQ(_corpData.rolesAtHQ),
  m_rolesAtOther(_corpData.rolesAtOther),
  m_stationID(_charData.stationID),
  m_solarSystemID(_charData.solarSystemID),
  m_constellationID(_charData.constellationID),
  m_regionID(_charData.regionID),
  m_ancestryID(_charData.ancestryID),
  m_careerID(_charData.careerID),
  m_schoolID(_charData.schoolID),
  m_careerSpecialityID(_charData.careerSpecialityID),
  m_startDateTime(_charData.startDateTime),
  m_createDateTime(_charData.createDateTime),
  m_corporationDateTime(_charData.corporationDateTime),
  m_shipID(_charData.shipID)
{
    // allow characters to be only singletons
    //assert(singleton() && quantity() == -1);
    assert(singleton());

    // Activate Save Info Timer with somewhat randomized timer value:
    //SetSaveTimerExpiry( MakeRandomInt( (10 * 60), (15 * 60) ) );        // Randomize save timer expiry to between 10 and 15 minutes
    //EnableSaveTimer();
}

CharacterRef Character::Load(ItemFactory &factory, uint32 characterID)
{
    return InventoryItem::Load<Character>( factory, characterID );
}

template<class _Ty>
RefPtr<_Ty> Character::_LoadCharacter(ItemFactory &factory, uint32 characterID,
    // InventoryItem stuff:
    const CharacterType &charType, const ItemData &data,
    // Character stuff:
    const CharacterData &charData, const CorpMemberInfo &corpData)
{
    // construct the item
    return CharacterRef( new Character( factory, characterID, charType, data, charData, corpData ) );
}

CharacterRef Character::Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data,
    // Character stuff:
    CharacterData &charData, CorpMemberInfo &corpData)
{
    uint32 characterID = Character::_Spawn( factory, data, charData, corpData );
    if( characterID == 0 )
        return CharacterRef();

    CharacterRef charRef = Character::Load( factory, characterID );

    // Create default dynamic attributes in the AttributeMap:
    charRef.get()->SetAttribute(AttrIsOnline, 1);     // Is Online

    return charRef;
}

uint32 Character::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data,
    // Character stuff:
    CharacterData &charData, CorpMemberInfo &corpData)
{
    // make sure it's a character
    const CharacterType *ct = factory.GetCharacterType(data.typeID);
    if(ct == NULL)
        return 0;

    // make sure it's a singleton with qty 1
    if(!data.singleton || data.quantity != 1) {
        _log(ITEM__ERROR, "Tried to create non-singleton character %s.", data.name.c_str());
        return 0;
    }

    // first the item
    uint32 characterID = Owner::_Spawn(factory, data);
    if(characterID == 0)
        return 0;

    // then character
    if(!factory.db().NewCharacter(characterID, charData, corpData)) {
        // delete the item
        factory.db().DeleteItem(characterID);

        return 0;
    }

    return characterID;
}

bool Character::_Load()
{
    if( !Owner::_Load() )
        return false;

	bool bLoadSuccessful = false;

    if( !LoadContents( m_factory ) )
        return false;

    if( !m_factory.db().LoadSkillQueue( itemID(), m_skillQueue ) )
        return false;

    bLoadSuccessful = Owner::_Load();

	// Update Skill Queue and Total Skill Points Trained:
	if( bLoadSuccessful )
		UpdateSkillQueue();

    if( !m_factory.db().LoadCertificates( itemID(), m_certificates ) )
        return false;

	return bLoadSuccessful;
}

void Character::Delete() {
    // delete contents
    DeleteContents( m_factory );

    // delete character record
    m_factory.db().DeleteCharacter(itemID());

    // let the parent care about the rest
    Owner::Delete();
}

bool Character::AlterBalance(double balanceChange) {
    if(balanceChange == 0)
        return true;

    double result = balance() + balanceChange;

    //remember, this can take a negative amount...
    if(result < 0)
        return false;

    m_balance = result;

    //TODO: save some info to journal.
    SaveCharacter();

    return true;
}

void Character::SetLocation(uint32 stationID, uint32 solarSystemID, uint32 constellationID, uint32 regionID) {
    m_stationID = stationID;
    m_solarSystemID = solarSystemID;
    m_constellationID = constellationID;
    m_regionID = regionID;

    SaveCharacter();
}

void Character::JoinCorporation(uint32 corporationID, const CorpMemberInfo &roles) {
	m_corporationID = corporationID;

	m_corpRole = roles.corpRole;
    m_rolesAtAll = roles.rolesAtAll;
    m_rolesAtBase = roles.rolesAtBase;
    m_rolesAtHQ = roles.rolesAtHQ;
	m_rolesAtOther = roles.rolesAtOther;

	SaveCharacter();
}

void Character::SetDescription(const char *newDescription) {
    m_description = newDescription;

    SaveCharacter();
}

bool Character::HasSkill(uint32 skillTypeID) const
{
    return GetSkill(skillTypeID);
}

bool Character::HasSkillTrainedToLevel(uint32 skillTypeID, uint32 skillLevel) const
{
    SkillRef requiredSkill;

    // First, check for existence of skill trained or in training:
    requiredSkill = GetSkill( skillTypeID );
    if( !requiredSkill )
        return false;

    // Second, check for required minimum level of skill, note it must already be trained to this level:
    if( requiredSkill->GetAttribute(AttrSkillLevel) < skillLevel )
        return false;

    return true;
}

bool Character::HasCertificate( uint32 certificateID ) const
{
    uint32 i = 0;
    for( i = 0; i < m_certificates.size(); i++ )
    {
        if( m_certificates.at( i ).certificateID == certificateID )
            return true;
    }

    return false;
}

SkillRef Character::GetSkill(uint32 skillTypeID) const
{
    InventoryItemRef skill = GetByTypeFlag( skillTypeID, flagSkill );
    if( !skill )
        skill = GetByTypeFlag( skillTypeID, flagSkillInTraining );

    return SkillRef::StaticCast( skill );
}

uint Character::GetSkillLevel(uint32 skillTypeID, bool zeroForNotInjected) const
{
    SkillRef requiredSkill;

    // First, check for existence of skill trained or in training:
    requiredSkill = GetSkill( skillTypeID );

    if( !requiredSkill )
        return zeroForNotInjected ? 0 : -1;
    return requiredSkill->GetAttribute(AttrSkillLevel).get_int() ;
}

SkillRef Character::GetSkillInTraining() const
{
    InventoryItemRef item;
    if (!FindSingleByFlag(flagSkillInTraining, item)){
        sLog.Debug("Character","  Unable to find skill in training");
    }

    return SkillRef::StaticCast( item );
}

void Character::GetSkillsList(std::vector<InventoryItemRef> &skills) const
{
    //find all the skills contained within ourself.
    FindByFlag( flagSkill, skills );
    FindByFlag( flagSkillInTraining, skills );
}

EvilNumber Character::GetSPPerMin( SkillRef skill )
{
    EvilNumber primarySkillTrainingAttr = skill->GetAttribute(AttrPrimaryAttribute);
    EvilNumber secondarySkillTrainingAttr = skill->GetAttribute(AttrSecondaryAttribute);

    EvilNumber primarySPperMin = GetAttribute( (uint32)(primarySkillTrainingAttr.get_int()) );
    EvilNumber secondarySPperMin = GetAttribute( (uint32)(secondarySkillTrainingAttr.get_int()) );

    primarySPperMin = primarySPperMin + secondarySPperMin / 2.0f;

///      *****   SP/Min is also calculated by client....if changed here, client needs many updates.  *****
    if( sConfig.rates.skillRate > 1 )    // is skillRate defined in config?
    {
      primarySPperMin = primarySPperMin * sConfig.rates.skillRate;
      sLog.Debug( "Character::GetSPPerMin()", " new SPperMin by rates.skillRate %u", primarySPperMin );
    }
/**
    // 100% Training bonus has been removed in Incursion...i like it, so re-instated   -allan 01/11/14
    EvilNumber chkSPMax =  0;
    if( sConfig.rates.MaxSPforBonus )    // is MaxSPforBonus defined in config?
      chkSPMax =  sConfig.rates.MaxSPforBonus;
    else
      chkSPMax =  MAX_SP_FOR_100PCT_TRAINING_BONUS;  // else set to 1m6 SP

    EvilNumber totalTrained = m_totalSPtrained;
      sLog.Debug( "Character::GetSPPerMin()", " totalTrained %f", totalTrained.get_float() ) ;

    if( totalTrained < chkSPMax )
      primarySPperMin += primarySPperMin;     // double sp/min

      sLog.Log( "Character::GetSPPerMin()", " final of %u SPperMin", primarySPperMin );
*/
    return primarySPperMin;
}

EvilNumber Character::GetEndOfTraining() const
{
    SkillRef skill = GetSkillInTraining();
    if( !skill )
        return 0;

    return skill->GetAttribute(AttrExpiryTime);
}

bool Character::InjectSkillIntoBrain(SkillRef skill)
{
    Client *c = m_factory.entity_list.FindCharacter( itemID() );

    SkillRef oldSkill = GetSkill( skill->typeID() );
    if( oldSkill )
    {
        //TODO: build and send proper UserError for CharacterAlreadyKnowsSkill.
        if( c != NULL )
            c->SendNotifyMsg( "You already know this skill." );
        return false;
    }

    // TODO: based on config options later, check to see if another character, owned by this characters account,
    // is training a skill.  If so, return. (flagID=61).
    if( !skill->SkillPrereqsComplete( *this ) )
    {
        // TODO: need to send back a response to the client.  need packet specs.
        _log( ITEM__TRACE, "%s (%u): Requested to train skill %u item %u but prereq not complete.", itemName().c_str(), itemID(), skill->typeID(), skill->itemID() );

        if( c != NULL )
            c->SendNotifyMsg( "Injection failed!  Skill prerequisites incomplete." );
        return false;
    }

    // are we injecting from a stack of skills?
    if( skill->quantity() > 1 )
    {
        // split the stack to obtain single item
        InventoryItemRef single_skill = skill->Split( 1 );
        if( !single_skill )
        {
            _log( ITEM__ERROR, "%s (%u): Unable to split stack of %s (%u).", itemName().c_str(), itemID(), skill->itemName().c_str(), skill->itemID() );
            return false;
        }

        // use single_skill ...
        single_skill->MoveInto( *this, flagSkill );
        single_skill->SetAttribute(AttrSkillPoints, 1);
    }
    else
        // use original skill
        skill->MoveInto( *this, flagSkill );
        skill->SetAttribute(AttrSkillPoints, 1);

    if( c != NULL )
        c->SendNotifyMsg( "Injection of skill complete." );
    return true;
}

bool Character::InjectSkillIntoBrain(SkillRef skill, uint8 level)
{
    Client *c = m_factory.entity_list.FindCharacter( itemID() );

    SkillRef oldSkill = GetSkill( skill->typeID() );
    if( oldSkill )
    {
        oldSkill->SetAttribute(AttrSkillLevel, level);
        EvilNumber eTmp = EVIL_SKILL_BASE_POINTS * skill->GetAttribute(AttrSkillTimeConstant) * EvilNumber::pow(2, (2.5 * (level - 1)));
        oldSkill->SetAttribute(AttrSkillPoints, eTmp);
        oldSkill->SetFlag(flagSkill);
        return true;
    }

    // are we injecting from a stack of skills?
    if( skill->quantity() > 1 )
    {
        // split the stack to obtain single item
        InventoryItemRef single_skill = skill->Split( 1 );
        if( !single_skill )
        {
            _log( ITEM__ERROR, "%s (%u): Unable to split stack of %s (%u).", itemName().c_str(), itemID(), skill->itemName().c_str(), skill->itemID() );
            return false;
        }

        // use single_skill ...
        single_skill->MoveInto( *this, flagSkill );
    }
    else
        skill->MoveInto( *this, flagSkill );

    skill->SetAttribute(AttrSkillLevel, level);
    skill->SetFlag(flagSkill);
    EvilNumber tmp = EVIL_SKILL_BASE_POINTS * skill->GetAttribute(AttrSkillTimeConstant) * EvilNumber::pow(2,( 2.5 * (level - 1)));
    skill->SetAttribute(AttrSkillPoints, tmp);

    return true;
}

void Character::AddToSkillQueue(uint32 typeID, uint8 level)
{
    QueuedSkill qs;
    qs.typeID = typeID;
    qs.level = level;
    m_skillQueue.push_back( qs );
}

bool Character::GrantCertificate( uint32 certificateID )
{
    cCertificates i;
    i.certificateID = certificateID;
    i.grantDate = Win32TimeNow();
    i.visibilityFlags = true;
    m_certificates.push_back( i );

    return true;
}

void Character::UpdateCertificate( uint32 certificateID, bool pub )
{
    uint32 i;
    for( i = 0; i < m_certificates.size(); i ++ )
    {
        if( m_certificates.at( i ).certificateID == certificateID )
        {
            m_certificates.at( i ).visibilityFlags = pub ;
        }
    }
}

void Character::GetCertificates( Certificates &crt )
{
    crt = m_certificates;
}

void Character::ClearSkillQueue()
{
    m_skillQueue.clear();
}

void Character::PauseSkillQueue()
{
    m_factory.db().SavePausedSkillQueue(
        itemID(),
        m_skillQueue
    );
}

void Character::LoadPausedSkillQueue()
{
    m_factory.db().LoadPausedSkillQueue( itemID(), m_skillQueue );
}

void Character::UpdateSkillQueue()
{
    Client *c = m_factory.entity_list.FindCharacter( itemID() );

    SkillRef currentTraining = GetSkillInTraining();
    if( currentTraining )
    {
        if( m_skillQueue.empty()
            || currentTraining->typeID() != m_skillQueue.front().typeID )
        {
            // either queue is empty or skill with different typeID is in training ...
            // stop training:

            EvilNumber timeEndTrain = currentTraining->GetAttribute(AttrExpiryTime);
            if (timeEndTrain != 0) {
                EvilNumber nextLevelSP = currentTraining->GetSPForLevel(currentTraining->GetAttribute(AttrSkillLevel) + 1);
                EvilNumber SPPerMinute = GetSPPerMin(currentTraining);

                EvilNumber skillPointsTrained = nextLevelSP - (((timeEndTrain - EvilTimeNow()) / EvilTime_Minute) * SPPerMinute);

                currentTraining->SetAttribute(AttrSkillPoints, skillPointsTrained);

                  sLog.Debug( "Character::UpdateSkillQueue()", "  Skill %s trained %f skill points before termination from training queue", currentTraining->itemName().c_str(), skillPointsTrained.get_float() );
            }

            currentTraining->SetAttribute(AttrExpiryTime, 0);
            currentTraining->MoveInto( *this, flagSkill, true );

            if( c != NULL )
            {
                OnSkillTrainingStopped osst;
                osst.itemID = currentTraining->itemID();
                osst.endOfTraining = 0;

                PyTuple* tmp = osst.Encode();
                c->QueueDestinyEvent( &tmp );
                PySafeDecRef( tmp );

                c->UpdateSkillTraining();
            }

            // Save changes to this skill before removing it from training:
            currentTraining->SaveItem();
            // nothing currently in training
            currentTraining = SkillRef();
            // Re-Calculate total SP trained and store in internal variable:
            _CalculateTotalSPTrained();
            // Save character skill data:
            SaveSkillQueue();
        }
    }

    while( !m_skillQueue.empty() )
    {
        if( !currentTraining )
        {
            // something should be trained, get first skill in queue
            uint32 skillTypeID = m_skillQueue.front().typeID;

            currentTraining = GetSkill( skillTypeID );
            if( !currentTraining )
            {
                _log( ITEM__ERROR, "%s (%u): Skill %u to train was not found.", itemName().c_str(), itemID(), skillTypeID );
                break;
            }
              sLog.Debug( "Character::UpdateSkillQueue()", "  %s (%u): Starting training of skill %s",  m_itemName.c_str(), m_itemID, currentTraining->itemName().c_str() );

            EvilNumber SPPerMinute = GetSPPerMin(currentTraining);
            EvilNumber SPToNextLevel = currentTraining->GetSPForLevel(currentTraining->GetAttribute(AttrSkillLevel) + 1);
            EvilNumber CurrentSP = currentTraining->GetAttribute(AttrSkillPoints);

            SPToNextLevel = SPToNextLevel.get_float() - CurrentSP.get_float();
              sLog.Debug( "Character::UpdateSkillQueue()", "  CurrentSP = %f : %f SPToNextLevel", CurrentSP.get_float(),  SPToNextLevel.get_float() );

            EvilNumber timeTraining = EvilTimeNow() + EvilTime_Minute * SPToNextLevel / SPPerMinute;

            currentTraining->MoveInto( *this, flagSkillInTraining );
            double dbl_timeTraining = timeTraining.get_float() + (double)(Win32Time_Second * 5);
            currentTraining->SetAttribute(AttrExpiryTime, dbl_timeTraining);

              sLog.Debug( "Character::UpdateSkillQueue()", "  Calculated time training will complete = %s", Win32TimeToString(dbl_timeTraining).c_str() );

            if( c != NULL )
            {
                OnSkillStartTraining osst;
                osst.itemID = currentTraining->itemID();
                osst.endOfTraining = timeTraining.get_float();

                PyTuple* tmp = osst.Encode();
                c->QueueDestinyEvent( &tmp );
                PySafeDecRef( tmp );
                c->UpdateSkillTraining();
            }
            // Save changes to this skill now that it is put into training:
            currentTraining->SaveItem();
        }

        if( currentTraining->GetAttribute(AttrExpiryTime) <= EvilTimeNow() )
        {
            // training has been finished
            /**  NOTE:  This needs a periodic (persistant) check, not just for chars ingame.  API will need CURRENT skilltraining  */
            currentTraining->SetAttribute(AttrSkillLevel, currentTraining->GetAttribute(AttrSkillLevel) + 1 );
            currentTraining->SetAttribute(AttrSkillPoints, currentTraining->GetSPForLevel( currentTraining->GetAttribute(AttrSkillLevel) ), true);

            //  save finished skill in history  -allan
            uint32 skillID = m_skillQueue.front().typeID;
            DBerror err;
            if( !sDatabase.RunQuery( err,
              "INSERT INTO chrSkillHistory (logDateTime, characterID, skillTypeID, skillLevel, absolutePoints)"
              " VALUES (%f, %u, %u, %f, %f)", currentTraining->GetAttribute(AttrExpiryTime).get_float(), itemID(), skillID, currentTraining->GetAttribute(AttrSkillLevel).get_float(), GetTotalSP().get_float() ))
                  _log(DATABASE__ERROR, "Failed to set chrSkillHistory for character %u: %s", itemID(), err.c_str());
            sLog.Log("Character::SaveFinishedSkill","%s:%u saved skill %u as finished to level %f on %f DateTime", itemName().c_str(), itemID(), skillID, currentTraining->GetAttribute(AttrSkillLevel).get_float(), currentTraining->GetAttribute(AttrExpiryTime).get_float() );

            currentTraining->SetAttribute(AttrExpiryTime, 0);
            currentTraining->MoveInto( *this, flagSkill, true );
			currentTraining->SaveItem();

              sLog.Debug( "Character::UpdateSkillQueue()", "%s: Finishing training of skill %s.", itemName().c_str(), currentTraining->itemName().c_str());

            if( c != NULL )
            {
                OnSkillTrained ost;
                ost.itemID = currentTraining->itemID();

                PyTuple* tmp = ost.Encode();
                c->QueueDestinyEvent( &tmp );
                PySafeDecRef( tmp );
                c->UpdateSkillTraining();
            }

            // erase first element in skill queue
            m_skillQueue.erase( m_skillQueue.begin() );

            // nothing currently in training
            currentTraining = SkillRef();
            // Re-Calculate total SP trained and store in internal variable:
            _CalculateTotalSPTrained();
            // Save character skill data:
            SaveSkillQueue();
        }
        // else the skill is in training ...
        else
            break;
    }
/**  moving this to {skill finished} and {stop training} loops for faster load times....
    // Re-Calculate total SP trained and store in internal variable:
    _CalculateTotalSPTrained();

    // Save character and skill data:
	//  too many DB hits...commented out
    //SaveCharacter();
    SaveSkillQueue();
    */
}

//  this still needs work...in progress...see commented code for using flatSkillQueue
void Character::UpdateSkillQueueEndTime(const SkillQueue &queue)
{
    EvilNumber chrMinRemaining = 0;    // explicit init to 0
    /**   this code is start for looping skillqueue for multiple levels of same skill.
    std::map<uint32, QueuedSkill> flatSkillQueue;
    const QueuedSkill &qs = queue;
    //if (flatSkillQueue.find(qs.typeID) != flatSkillQueue.end()){
    // flatSkillQueue.insert(std::make_pair(qs.typeID,qs));}
    //  else{ flatSkillQueue.find(qs.typeID)->second.level = qs.level;}

    //flatSkillQueue.insert(std::make_pair(qs.typeID,qs));
    //flatSkillQueue.insert( std::pair<uint32, QueuedSkill>(qs.typeID, qs) );
    */

    for(size_t i = 0; i < queue.size(); i++)    // loop thru skills currently in queue
    {
        const QueuedSkill &qs = queue[ i ];     // get skill id from queue
        SkillRef skill = Character::GetSkill( qs.typeID );   //make ref for current skill

        chrMinRemaining += (skill->GetSPForLevel(qs.level) - skill->GetAttribute( AttrSkillPoints )) / GetSPPerMin(skill);
    }
    chrMinRemaining = chrMinRemaining * EvilTime_Minute + EvilTimeNow();

    DBerror err;
    if( !sDatabase.RunQuery( err, "UPDATE character_ SET skillQueueEndTime = %f WHERE characterID = %u ", chrMinRemaining.get_float(), itemID() ) )
    {
        _log(DATABASE__ERROR, "Failed to set skillQueueEndTime for character %u: %s", itemID(), err.c_str());
        return;
    }
    sLog.Debug("Character::UpdateSkillQueueEndTime()", " %s (%u): Saved Queue Data to DB", itemName().c_str(), itemID());
}

void Character::UpdateSkillQueueEndTime()
{
    UpdateSkillQueueEndTime(m_skillQueue);
}

void Character::RemoveSkillFromQueue(uint32 charID, uint16 skillID)
{       /**  unused?  */
    DBerror err;
    if( !sDatabase.RunQuery( err, "DELETE FROM chrSkillQueue WHERE characterID = %u AND typeID = %u ", charID, skillID ) )
    {
        _log(DATABASE__ERROR, "Failed to delete skill %u for character %u: %s", skillID, charID, err.c_str());
        return;
    }
}

double Character::GetEffectiveStandingFromNPC(uint32 itemID)
{
    Client *const c = m_factory.entity_list.FindCharacter( itemID );

    double res = m_db.GetCharRawStandingFromNPC(this->itemID(), itemID);
//TODO: also use skillCriminalConnections.
// check itemID() for faction/corp standing -> char faction/corp

    if(res > 0) res += (10-res) * 0.04 * c->GetChar()->GetSkillLevel(skillConnections);
    else
    if(res < 0) res += (10-res) * 0.04 * c->GetChar()->GetSkillLevel(skillDiplomacy);

    return res;
}

PyDict *Character::CharGetInfo() {
    //TODO: verify that we are a char?

    if( !LoadContents( m_factory ) ) {
        codelog(ITEM__ERROR, "%s (%u): Failed to load contents for CharGetInfo", m_itemName.c_str(), m_itemID);
        return NULL;
    }

    PyDict *result = new PyDict;
    Rsp_CommonGetInfo_Entry entry;

    if(!Populate(entry))
        return NULL;
    result->SetItem(new PyInt(m_itemID), new PyObject("util.KeyVal", entry.Encode()));

    //now encode skills...
    std::vector<InventoryItemRef> skills;
    //find all the skills contained within ourself.
    FindByFlag( flagSkill, skills );
    FindByFlag( flagSkillInTraining, skills );

    //encode an entry for each one.
    std::vector<InventoryItemRef>::iterator cur, end;
    cur = skills.begin();
    end = skills.end();
    for(; cur != end; cur++) {
        if(!(*cur)->Populate(entry)) {
            codelog(ITEM__ERROR, "%s (%u): Failed to load skill item %u for CharGetInfo", m_itemName.c_str(), itemID(), (*cur)->itemID());
        } else {
            result->SetItem(new PyInt((*cur)->itemID()), new PyObject("util.KeyVal", entry.Encode()));
        }
    }

    return result;
}

PyObject *Character::GetDescription() const
{
    util_Row row;

    row.header.push_back("description");

    row.line = new PyList;
    row.line->AddItemString( description().c_str() );

    return row.Encode();
}

PyTuple *Character::GetSkillQueue() {
    // return skills from skill queue
    PyList *list = new PyList;

    SkillQueue::iterator cur, end;
    cur = m_skillQueue.begin();
    end = m_skillQueue.end();
    for(; cur != end; cur++)
    {
        SkillQueue_Element el;

        el.typeID = cur->typeID;
        el.level = cur->level;

        list->AddItem( el.Encode() );
    }

    // now encapsulate it in a tuple with the free points
    PyTuple *tuple = new PyTuple(2);
    tuple->SetItem(0, list);
    // sending 0, as done on retail, doesn't fuck up calculation for some reason
    // so we can take the same shortcut here
    tuple->SetItem(1, new PyInt(0));

    return tuple;
}

void Character::AddItem(InventoryItemRef item)
{
    Inventory::AddItem( item );

    if( item->flag() == flagSkill
        || item->flag() == flagSkillInTraining )
    {
        // Skill has been added ...
        if( item->categoryID() != EVEDB::invCategories::Skill ) {
            _log( ITEM__WARNING, "%s (%u): %s has been added with flag %d.", itemName().c_str(), itemID(), item->category().name().c_str(), (int)item->flag() );
        } else
        {
            SkillRef skill = SkillRef::StaticCast( item );

            if( !skill->singleton() )
            {
                _log( ITEM__TRACE, "%s (%u): Injecting %s.", itemName().c_str(), itemID(), item->itemName().c_str() );

                // Make it singleton and set initial skill values.
                skill->ChangeSingleton( true );

                skill->SetAttribute(AttrSkillLevel, 0);
                skill->SetAttribute(AttrSkillPoints, 0);

                if( skill->flag() != flagSkillInTraining )
                    skill->SetAttribute(AttrExpiryTime, 0);
            }
        }
    }
}

void Character::SaveCharacter()
{
    _log( ITEM__TRACE, "Saving character %u.", itemID() );

    // Calculate total Skill Points trained at this time to save to DB:
    _CalculateTotalSPTrained();

    sLog.Debug( "Character::SaveCharacter()", "Saving character info to DB for character %s...", itemName().c_str() );
    // character data
    m_factory.db().SaveCharacter(
        itemID(),
        CharacterData(
            accountID(),
            title().c_str(),
            description().c_str(),
            gender(),
            bounty(),
            balance(),
            aurBalance(),
            securityRating(),
            logonMinutes(),
            m_totalSPtrained.get_float(),
            corporationID(),
            allianceID(),
            warFactionID(),
            stationID(),
            solarSystemID(),
            constellationID(),
            regionID(),
            ancestryID(),
            careerID(),
            schoolID(),
            careerSpecialityID(),
            startDateTime(),
            createDateTime(),
            corporationDateTime(),
            shipID()
        )
    );

    // corporation data
    m_factory.db().SaveCorpMemberInfo(
        itemID(),
        CorpMemberInfo(
            corporationHQ(),
            corpRole(),
            rolesAtAll(),
            rolesAtBase(),
            rolesAtHQ(),
            rolesAtOther()
        )
    );
}

void Character::SaveFullCharacter()
{
    _log( ITEM__TRACE, "Saving character %u.", itemID() );

    sLog.Debug( "Character::SaveFullCharacter()", "Saving FULL set of character info, skills, items, etc to DB for character %s...", itemName().c_str() );

	// First save basic character info and attributes:
	SaveCharacter();

    // Save this character's own attributes:
    SaveAttributes();

    // BAD BAD BAD: This is taking minutes and minutes for high SP chars when all we need to do is save the current skill in training:
    // Loop through all skills and save each one:
    /*
    std::vector<InventoryItemRef> skills;
    GetSkillsList( skills );
    std::vector<InventoryItemRef>::iterator cur, end;
    cur = skills.begin();
    end = skills.end();
    for(; cur != end; cur++)
		cur->get()->SaveItem();
    */

    SkillRef currentTraining = GetSkillInTraining();
    if( currentTraining != NULL )
        currentTraining->SaveItem();

    // Loop through all items owned by this Character and save each one:
	// TODO

	// Loop through all contracts or other non-item things owned by this Character and save each one:
	// TODO

    SaveSkillQueue();
	SaveCertificates();
}

void Character::SaveSkillQueue() const {
    _log( ITEM__TRACE, "Saving skill queue of character %u.", itemID() );

    // skill queue
    m_factory.db().SaveSkillQueue(
        itemID(),
        m_skillQueue
    );
}

void Character::SaveCertificates() const
{
    _log( ITEM__TRACE, "Saving Implants of character %u", itemID() );

    m_factory.db().SaveCertificates(
        itemID(),
        m_certificates
        );
}

void Character::SetActiveShip(uint32 shipID)
{
    m_shipID = shipID;
}

void Character::_CalculateTotalSPTrained()
{
    // Loop through all skills trained and calculate total SP this character has trained so far
    EvilNumber totalSP = 0.0f;
    std::vector<InventoryItemRef> skills;
    GetSkillsList( skills );
    std::vector<InventoryItemRef>::iterator cur, end;
    cur = skills.begin();
    end = skills.end();
    for(; cur != end; cur++)
    {
        totalSP += cur->get()->GetAttribute( AttrSkillPoints );    // much cleaner and more accurate    -allan
    }

    m_totalSPtrained = totalSP;
}

EvilNumber Character::GetTotalSP()
{
    // Loop through all skills trained and calculate total SP this character has trained so far
    EvilNumber totalSP = 0.0f;
    std::vector<InventoryItemRef> skills;
    GetSkillsList( skills );
    std::vector<InventoryItemRef>::iterator cur, end;
    cur = skills.begin();
    end = skills.end();
    for(; cur != end; cur++)
    {
        totalSP += cur->get()->GetAttribute( AttrSkillPoints );
    }

    return(totalSP);
}

void Character::VisitSystem(uint32 solarSystemID)
{
    uint16 visits = GetSystemVisits(solarSystemID);
    DBerror err;
    if (visits)
    {
      visits ++;
      if(!sDatabase.RunQuery(err,
        "UPDATE chrVisitedSystems SET visits = %u, lastDateTime = %" PRIu64 " WHERE characterID = %u AND solarSystemID = %u",
        visits, Win32TimeNow(), itemID(), solarSystemID
        ))
        {
            sLog.Error("Character::VisitSystem","%s: Query Failed: %s", itemName().c_str(), err.c_str() );
            return;
        }
    }else{
      if(!sDatabase.RunQuery(err,
        "INSERT INTO chrVisitedSystems (characterID, solarSystemID, visits, lastDateTime)"
        "VALUES (%u, %u, 1, %" PRIu64 ")", itemID(), solarSystemID, Win32TimeNow()
        ))
        {
            sLog.Error("Character::VisitSystem","%s: Query Failed: %s", itemName().c_str(), err.c_str() );
            return;
        }
    }
    sLog.Log("Character::VisitSystem","%s: Query saved as charID=%u, solSys=%u, visits=%u(0=1), lastTime=%" PRIu64 ")", itemName().c_str(), itemID(), solarSystemID, visits, Win32TimeNow() );
}

uint16 Character::GetSystemVisits(uint32 solarSystemID)
{
    DBQueryResult res;
    sDatabase.RunQuery(res,
      "SELECT visits FROM chrVisitedSystems WHERE characterID = %u AND solarSystemID = %u",
      itemID(), solarSystemID
      );

    DBResultRow row;
    if(res.GetRow(row))
      return row.GetUInt(0);
    else
      return false;
}

PyObject* Character::GetSkillHistory() {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT logDateTime, eventID, skillTypeID, skillLevel, relativePoints, absolutePoints"
        " FROM chrSkillHistory"
        " WHERE characterID = %d", itemID() ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToRowset(res));
}


void Character::chkDynamicSystemID(uint32 systemID) {
  /**  this ensures mapDynamicData.solarSystemID for `systemID` is in the DB for later calls. -allan 16Mar14 */
    DBQueryResult chk;
    sDatabase.RunQuery(chk, "SELECT solarSystemID FROM mapDynamicData WHERE solarSystemID = %u", systemID );
    DBResultRow row;
    if(chk.GetRow(row)) {
        sLog.Success("Character::chkDynamicSystemID"," System %u already in DB", systemID );
    } else {
        DBerror err;
        if(!sDatabase.RunQuery(err, "INSERT INTO mapDynamicData (solarSystemID) VALUES (%u)", systemID )) {
            sLog.Error("Character::chkDynamicSystemID","%u: Query Failed: %s", systemID, err.c_str() );
        } else {
          sLog.Warning("Character::chkDynamicSystemID"," System %u inserted in DB", systemID );
        }
    }
}

/** the following functions rely on solarSystemID being in the mapDynamicData table.
  * the check is called before these are used, and solarSystemID is then verified for existance and added if needed.
  *   the function is as follows and is declared above...
  *         void SystemDB::chkDynamicSystemID(uint32 systemID)
  *
  *  NOTE: these will have to be reset each server start.  should prolly trunicate table on restart after everything is  working as i am using the count() in this table for active systems on website.
  */

void Character::AddJumpToDynamicData(uint32 solarSystemID) {
    uint16 jumps = GetJumpsFromDynamicData(solarSystemID);
    DBerror err;

    jumps ++;
    if(!sDatabase.RunQuery(err,
        "UPDATE mapDynamicData SET jumpsHour = %u, jumpsDateTime = %" PRIu64 " WHERE solarSystemID = %u",
        jumps, Win32TimeNow(), solarSystemID )) {
            sLog.Error("Character::AddJumpToDynamicData","%u: Query Failed: %s", itemID(), err.c_str() );
            return;
        }
    sLog.Log("Character::AddJumpToDynamicData","%s (%u): Query saved as solSys=%u, jumpsHour=%u, jumpsDateTime=%" PRIu64 ")", itemName().c_str(), itemID(), solarSystemID, jumps, Win32TimeNow() );
}

uint16 Character::GetJumpsFromDynamicData(uint32 solarSystemID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT jumpsHour FROM mapDynamicData WHERE solarSystemID = %u", solarSystemID );

    DBResultRow row;
    if(res.GetRow(row))
      return row.GetUInt(0);
    else
      return false;
}

//  this should prolly be changed to a dynamic call from memory, instead of from db....
//   however, showing pilots in system on the webpage will need the db.  *shrugs*
void Character::AddPilotToDynamicData(uint32 solarSystemID, bool add) {
    uint16 pilots = GetPilotsFromDynamicData(solarSystemID);
    DBerror err;
    if (add) pilots ++; else pilots --;
    if (pilots < 0) pilots = 0;
    if(!sDatabase.RunQuery(err, "UPDATE mapDynamicData SET numpilots = %u WHERE solarSystemID = %u", pilots, solarSystemID )){
        sLog.Error("Character::AddPilotToDynamicData","%s: Query Failed: %s", itemID(), err.c_str() );
        return;
    }
    sLog.Log("Character::AddPilotToDynamicData","%s (%u): Query saved as solSys=%u, pilots=%u",itemName().c_str(), itemID(), solarSystemID, pilots );
}

uint16 Character::GetPilotsFromDynamicData(uint32 solarSystemID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT numpilots FROM mapDynamicData WHERE solarSystemID = %u", solarSystemID );

    DBResultRow row;
    if(res.GetRow(row))
      return row.GetUInt(0);
    else
      return false;
}

bool Character::isOffline(uint32 charID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT Online FROM character_ WHERE characterID = %u", charID );

    DBResultRow row;
    if(res.GetRow(row))
      return false; //row.GetUInt(0);
    else
      return true;
}
