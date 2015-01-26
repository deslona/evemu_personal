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
    Author:     Aknor Jaden
*/

#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "pos/Structure.h"
#include "ship/DestinyManager.h"

/*
 * Structure
 */
Structure::Structure(
    ItemFactory &_factory,
    uint32 _structureID,
    // InventoryItem stuff:
    const ItemType &_itemType,
    const ItemData &_data)
: InventoryItem(_factory, _structureID, _itemType, _data) {}

StructureRef Structure::Load(ItemFactory &factory, uint32 structureID)
{
    return InventoryItem::Load<Structure>( factory, structureID );
}

template<class _Ty>
RefPtr<_Ty> Structure::_LoadStructure(ItemFactory &factory, uint32 structureID,
    // InventoryItem stuff:
    const ItemType &itemType, const ItemData &data)
{
    // we don't need any additional stuff
    return StructureRef( new Structure( factory, structureID, itemType, data ) );
}

StructureRef Structure::Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    uint32 structureID = Structure::_Spawn( factory, data );
    if( structureID == 0 )
        return StructureRef();
    return Structure::Load( factory, structureID );
}

uint32 Structure::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // make sure it's a Structure
    const ItemType *st = factory.GetType(data.typeID);
    if(st == NULL)
        return 0;

    // store item data
    uint32 structureID = InventoryItem::_Spawn(factory, data);
    if(structureID == 0)
        return 0;

    // nothing additional

    return structureID;
}

bool Structure::_Load()
{
    // load contents
    if( !LoadContents( m_factory ) )
        return false;

    return InventoryItem::_Load();
}

void Structure::Delete()
{
    // delete contents first
    DeleteContents( m_factory );

    InventoryItem::Delete();
}

double Structure::GetCapacity(EVEItemFlags flag) const
{
    switch( flag ) {
        // the .get_float() part is a evil hack.... as this function should return a EvilNumber.
        case flagAutoFit:
        case flagCargoHold:           return GetAttribute(AttrCapacity).get_float();
        case flagSecondaryStorage:    return GetAttribute(AttrCapacitySecondary).get_float();
        case flagSpecializedAmmoHold: return GetAttribute(AttrAmmoCapacity).get_float();
        case flagSpecializedFuelBay:  return GetAttribute(AttrSpecialFuelBayCapacity).get_float();
        default:                      return 0.0;
    }
}

void Structure::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item, Client *c)
{
    CharacterRef character = c->GetChar();
    if( flag == flagCargoHold )
    {
        //get all items in cargohold
        EvilNumber capacityUsed(0);
        std::vector<InventoryItemRef> items;
        c->GetShip()->FindByFlag(flag, items);
        for(uint32 i = 0; i < items.size(); i++){
            capacityUsed += items[i]->GetAttribute(AttrVolume);
        }
        if( capacityUsed + item->GetAttribute(AttrVolume) > c->GetShip()->GetAttribute(AttrCapacity) )
        //if( capacityUsed + item->GetAttribute(AttrVolume) > itemID()->GetAttribute(AttrCapacity) )
            throw PyException( MakeCustomError( "Not enough cargo space!") );
    }
    else if( flag == flagSecondaryStorage )
    {
        //get all items in SecondaryStorage
        EvilNumber capacityUsed(0);
        std::vector<InventoryItemRef> items;
        c->GetShip()->FindByFlag(flag, items);
        for(uint32 i = 0; i < items.size(); i++){
            capacityUsed += items[i]->GetAttribute(AttrVolume);
        }
        if( capacityUsed + item->GetAttribute(AttrVolume) > c->GetShip()->GetAttribute(AttrCapacitySecondary) )
            throw PyException( MakeCustomError( "Not enough Secondary Storage space!") );
    }
    else if( flag == flagSpecializedAmmoHold )
    {
        //get all items in ammo hold
        EvilNumber capacityUsed(0);
        std::vector<InventoryItemRef> items;
        c->GetShip()->FindByFlag(flag, items);
        for(uint32 i = 0; i < items.size(); i++){
            capacityUsed += items[i]->GetAttribute(AttrVolume);
        }
        if( capacityUsed + item->GetAttribute(AttrVolume) > c->GetShip()->GetAttribute(AttrAmmoCapacity) )
            throw PyException( MakeCustomError( "Not enough Ammo Storage space!") );
    }
    else if( flag == flagSpecializedFuelBay )
    {
        //get all items in fuel bay
        EvilNumber capacityUsed(0);
        std::vector<InventoryItemRef> items;
        c->GetShip()->FindByFlag(flag, items);
        for(uint32 i = 0; i < items.size(); i++){
            capacityUsed += items[i]->GetAttribute(AttrVolume);
        }
        if( capacityUsed + item->GetAttribute(AttrVolume) > c->GetShip()->GetAttribute(AttrSpecialFuelBayCapacity) )
            throw PyException( MakeCustomError( "Not enough Fuel Storage space!") );
    }
}

PyObject *Structure::StructureGetInfo()
{
    if( !LoadContents( m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for StructureGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the Structure.
    if( !Populate( entry ) )
        return NULL;    //print already done.

    result.items[ itemID() ] = entry.Encode();

    return result.Encode();
}

void Structure::AddItem(InventoryItemRef item)
{
    InventoryEx::AddItem( item );
}


using namespace Destiny;

StructureEntity::StructureEntity(
    StructureRef structure,
    SystemManager *system,
    PyServiceMgr &services,
    const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), structure),
  m_system(system),
  m_services(services)
{
    _structureRef = structure;
    m_destiny->SetPosition(position, false);
    m_harmonic = 0;     // for shield passage....FIXME for later  (dedicated POS class maybe?)
    m_timestamp = Win32TimeNow() - Win32Time_Day;
}

void StructureEntity::Process() {
    SystemEntity::Process();
}

void StructureEntity::ForcedSetPosition(const GPoint &pt) {
    m_destiny->SetPosition(pt, false);
}

uint32 StructureEntity::GetCorporationID() const {
    //TODO will have to fix this later
    uint32 ownerID = GetOwnerID();
    if (IsCorp(ownerID)) return ownerID; else return 0;
}

uint32 StructureEntity::GetAllianceID() const {
    //TODO fix this once alliances are implemented.
    return 0;
}

bool StructureEntity::IsTCU() const {
    // check for a TCU item.
    //  this is ONLY used for SetState.effectStates (type 32226)
    return false;
}

void StructureEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();
    const std::string itemName( GetName() );
    //const uint16 miniballsCount = GetMiniBalls();

    BallHeader head;
    head.entityID = GetID();
    head.mode = Destiny::DSTBALL_RIGID;
    head.radius = GetRadius();
    head.x = position.x;
    head.y = position.y;
    head.z = position.z;
    head.sub_type = /*HasMiniBalls | */IsGlobal;        //TODO check for miniballs and add here if found.
    into.Append( head );

    DSTBALL_RIGID_Struct main;
    main.formationID = 0xFF;
    into.Append( main );

    MassSector mass;
    mass.cloak = 0;
    mass.corpID = GetCorporationID();
    mass.allianceID = GetAllianceID();
    mass.Harmonic = m_harmonic;
    mass.mass = Item()->type().mass();
    into.Append( mass );

    /*
     * TODO  query and configure miniballs for POS entity
    into.Append( miniballsCount );

    MiniBall miniball;
    for (int16 i; i<miniballsCount; i++) {
        miniball.x = -7701.181;
        miniball.y = 8060.06;
        miniball.z = 27878.900;
        miniball.radius = 1639.241;
        into.Append( miniball );
        miniball.clear();
    }


                                    [MiniBall]
                                      [Radius: 963.8593]
                                      [Offset: (0, -2302, 1)]
                                    [MiniBall]
                                      [Radius: 1166.27]
                                      [Offset: (0, 1298, 1)]
                                    [MiniBall]
                                      [Radius: 876.2357]
                                      [Offset: (0, -502, 1)]
                                    [MiniBall]
                                      [Radius: 796.5781]
                                      [Offset: (0, 2598, 1)]

    */
}

void StructureEntity::MakeDamageState(DoDestinyDamageState &into) const
{
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHp).get_float());
}

PyDict *StructureEntity::MakeSlimItem() const {
    PyDict *slim = new PyDict();
    slim->SetItemString("itemID", new PyInt(Item()->itemID()));
    slim->SetItemString("typeID", new PyInt(Item()->typeID()));
    slim->SetItemString("ownerID", new PyInt(Item()->ownerID()));
    slim->SetItemString("itemName", new PyString(Item()->itemName()));
    slim->SetItemString("corpID", new PyInt(GetCorporationID()));
    slim->SetItemString("incapacitated", new PyInt(0)); //TODO fix this later....check for offline/vulnerable states
    slim->SetItemString("posTimestamp", new PyLong(m_timestamp));   //TODO this will need to be set to time deployed.
    slim->SetItemString("posState", new PyInt(GetPOSState()));
    slim->SetItemString("warFactionID", new PyInt(0));
    slim->SetItemString("allianceID", new PyInt(0));
    /*  for tower modules
    slim->SetItemString("controlTowerID", new PyInt(0));
    slim->SetItemString("nameID", new PyNone);
    */
    return(slim);
}

uint8 StructureEntity::GetPOSState() const {
    //TODO fix this later
    /*
     STRUCTURE_UNANCHORED = 0,
     STRUCTURE_ANCHORED = 1,
     STRUCTURE_ONLINING = 2,
     STRUCTURE_REINFORCED = 3,
     STRUCTURE_ONLINE = 4,
     STRUCTURE_OPERATING = 5,
     STRUCTURE_VULNERABLE = 6,
     STRUCTURE_SHIELD_REINFORCE = 7,
     STRUCTURE_ARMOR_REINFORCE = 8,
     STRUCTURE_INVULNERABLE = 9
    */
    return STRUCTURE_ONLINE;   // hack for pos online
}

PyTuple *StructureEntity::GetEffectState() const {
    //FIXME  this will have to be finished.  hacked for now.
    std::vector<PyTuple *> updates;
    std::vector<int32, std::allocator<int32> > area;

    DoDestiny_OnSpecialFX13 effect;
    effect.entityID = Item()->itemID();
    effect.moduleID = Item()->itemID();
    effect.moduleTypeID = Item()->typeID();
    effect.targetID = Item()->itemID();
    effect.otherTypeID = 0;
    effect.area = area;
    effect.effect_type = "effects.StructureOnline";
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 1;
    effect.duration_ms = -1;
    effect.repeat = 0;
    effect.startTime = Win32TimeNow();
    PyTuple *update = effect.Encode();
    /*
                      [PyString "OnSpecialFX"]
                      [PyTuple 14 items]
                        [PyIntegerVar 1002332856217]
                        [PyIntegerVar 1002332856217]
                        [PyInt 12235]
                        [PyIntegerVar 1002332856217]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.StructureOnline"]
                        [PyBool False]
                        [PyInt 1]
                        [PyInt 1]
                        [PyInt -1]
                        [PyInt 0]
                        [PyIntegerVar 129516995311514600]
                        [PyNone]
                        */
    return update;
}