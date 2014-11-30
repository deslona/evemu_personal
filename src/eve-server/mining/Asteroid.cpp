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

#include "mining/Asteroid.h"
#include "ship/DestinyManager.h"

using namespace Destiny;

AsteroidEntity::AsteroidEntity(
    InventoryItemRef asteroid,
    SystemManager *system,
    PyServiceMgr &services,
    const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), asteroid),
  m_system(system),
  m_services(services)
{
    _asteroidRef = asteroid;
    m_destiny->SetPosition(position, false);
}

void AsteroidEntity::Process() {
    SystemEntity::Process();
}

void AsteroidEntity::ForcedSetPosition(const GPoint &pt) {
    m_destiny->SetPosition(pt, false);
}

void AsteroidEntity::Grow() {

}

void AsteroidEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();
    const std::string itemName( GetName() );
    BallHeader head;
    head.entityID = GetID();
    head.mode = Destiny::DSTBALL_RIGID;
    head.radius = GetRadius();
    head.x = position.x;
    head.y = position.y;
    head.z = position.z;
    head.sub_type = /*HasMiniBalls | */IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
    main.formationID = 0xFF;
    into.Append( main );
}

void AsteroidEntity::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = 1.0;
    into.tau = 100000;    //no freakin clue.
    into.timestamp = Win32TimeNow();
    into.armor = 1.0;
    into.structure = 1.0;
}


// OLD Asteroid class implementation:
/*
Asteroid::Asteroid(
    SystemManager *system,
    InventoryItemRef self
)
: ItemSystemEntity(self),
  m_system(system)
{
}

Asteroid::~Asteroid() {
}

void Asteroid::Grow() {
}

bool Asteroid::ApplyDamage(Damage &d)
{
    //ignore it.
    _log( MINING__DEBUG, "Asteroid %u: Ignoring %f damage from %s", m_self->itemID(), d.GetTotal(), d.source->GetName() );

    return false;
}

void Asteroid::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();
    const std::string itemName( GetName() );

    BallHeader head;
    head.entityID = GetID();
    head.mode = Destiny::DSTBALL_RIGID;
    head.radius = GetRadius();
    head.x = position.x;
    head.y = position.y;
    head.z = position.z;
    head.sub_type = AddBallSubType_cargoContainer_asteroid;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
    main.formationID = 0xFF;
    into.Append( main );

    const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
    into.Append( nameLen );

    const Buffer::iterator<uint16> name = into.end<uint16>();
    into.ResizeAt( name, nameLen );
    utf8::utf8to16( itemName.begin(), itemName.end(), name );
}

/*PyDict *Asteroid::MakeSlimItem() const {
    PyDict *slim = new PyDict();
    slim->add("typeID", new PyInt(m_typeID));
    slim->add("ownerID", new PyInt(500021));    //no idea.
    slim->add("itemID", new PyInt(m_itemID));
    return(slim);
}*/
