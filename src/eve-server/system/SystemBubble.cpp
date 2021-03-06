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

#include "ship/DestinyManager.h"
#include "system/BubbleManager.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "Client.h"

uint32 SystemBubble::m_bubbleIncrementer = 0;

//TODO change this to a multimap, using systemid as key.  this will provide faster searches later on   -allan
SystemBubble::SystemBubble(const GPoint &center, double radius) //, uint32 systemID)
: m_center(center),
  m_radius(radius),
  m_radius2(radius*radius),
  m_position_check_radius_sqrd((radius+BUBBLE_HYSTERESIS_METERS) * (radius+BUBBLE_HYSTERESIS_METERS))
  //m_system(systemID)
{
    m_bubbleID = m_bubbleIncrementer;
    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Constructor - Created new bubble %u (%p) at (%.2f,%.2f,%.2f) with radius %.2f", m_bubbleID, this, m_center.x, m_center.y, m_center.z, m_radius);
    m_bubbleIncrementer++;
}

//send a set of destiny events and updates to everybody in the bubble.
void SystemBubble::BubblecastDestiny(std::vector<PyTuple *> &updates, std::vector<PyTuple *> &events, const char *desc) const {
    BubblecastDestinyUpdate(updates, desc);
	BubblecastDestinyEvent(events, desc);
}

//send a set of destiny updates to everybody in the bubble.
void SystemBubble::BubblecastDestinyUpdate(std::vector<PyTuple *> &updates, const char *desc) const {
    std::vector<PyTuple *>::iterator cur, end;
	cur = updates.begin();
	end = updates.end();
	for(; cur != end; cur++) {
	    PyTuple *up = *cur;
		BubblecastDestinyUpdate(&up, desc); //update is consumed.
	}
    updates.clear();
}

//send a set of destiny events to everybody in the bubble.
void SystemBubble::BubblecastDestinyEvent(std::vector<PyTuple *> &events, const char *desc) const {
    std::vector<PyTuple *>::iterator cur, end;
	cur = events.begin();
	end = events.end();
	for(; cur != end; cur++) {
	    PyTuple *ev = *cur;
		BubblecastDestinyEvent(&ev, desc); //update is consumed.
	}
	events.clear();
}

//send a destiny update to everybody in the bubble.
//assume that static entities are also not interested in destiny updates.
void SystemBubble::BubblecastDestinyUpdate( PyTuple** payload, const char* desc ) const
{
    PyTuple* up = *payload;
    *payload = NULL;    //could optimize out one of the Clones in here...

    PyTuple* up_dup = NULL;

    std::set<SystemEntity*>::const_iterator cur, end, tmp;
    cur = m_dynamicEntities.begin();
    end = m_dynamicEntities.end();
    for (; cur != end; ++cur) {
        if ( (*cur)->IsClient() ) {
            if( NULL == up_dup )
                up_dup = new PyTuple( *up );

            _log( DESTINY__BUBBLE_TRACE, "Bubblecast %s update to %s (%u)", desc, (*cur)->GetName(), (*cur)->GetID() );
            (*cur)->QueueDestinyUpdate( &up_dup );
        }
    }

    PySafeDecRef( up_dup );
    PySafeDecRef( up );
}

//send a destiny update to everybody in the bubble EXCLUDING the given SystemEntity 'ent':
//assume that static entities are also not interested in destiny updates.
void SystemBubble::BubblecastDestinyUpdateExclusive( PyTuple** payload, const char* desc, SystemEntity *ent ) const
{
    PyTuple* up = *payload;
    *payload = NULL;    //could optimize out one of the Clones in here...

    PyTuple* up_dup = NULL;

    std::set<SystemEntity*>::const_iterator cur, end, tmp;
    cur = m_dynamicEntities.begin();
    end = m_dynamicEntities.end();
    for(; cur != end; ++cur) {
        // make sure current entity is NOT inanimate (clients only)
        if ( (*cur)->IsClient() ) {
            // Only queue a Destiny update for this bubble if the current SystemEntity is not 'ent':
            // (this is an update to all SystemEntity objects in the bubble EXCLUDING 'ent')
            if( (*cur)->GetID() != ent->GetID() ) {
                if( NULL == up_dup )
                    up_dup = new PyTuple( *up );

                (*cur)->QueueDestinyUpdate( &up_dup );
                _log( DESTINY__BUBBLE_TRACE, "Exclusive Bubblecast %s update to %s (%u)", desc, (*cur)->GetName(), (*cur)->GetID() );
            }
        }
    }


    PySafeDecRef( up_dup );
    //PySafeDecRef( up );
}

//send a destiny event to everybody in the bubble.
//assume that static entities are also not interested in destiny updates.
void SystemBubble::BubblecastDestinyEvent( PyTuple** payload, const char* desc ) const
{
    PyTuple* ev = *payload;
    *payload = NULL;    //could optimize out one of the Clones in here...

    PyTuple* ev_dup = NULL;

    std::set<SystemEntity *>::const_iterator cur, end, tmp;
    cur = m_dynamicEntities.begin();
    end = m_dynamicEntities.end();
    for(; cur != end; ++cur)
    {
        // make sure current entity is NOT inanimate (clients only)
        if ((*cur)->IsClient()) {
            if( NULL == ev_dup )
            ev_dup = new PyTuple( *ev );

            (*cur)->QueueDestinyEvent( &ev_dup );
            _log( DESTINY__BUBBLE_TRACE, "Bubblecast %s event to %s (%u)", desc, (*cur)->GetName(), (*cur)->GetID() );
        }
    }

    PySafeDecRef( ev_dup );
    PySafeDecRef( ev );
}

//called at some regular interval from the bubble manager.
//verifies that each entity is still in this bubble.
//if any entity is no longer in the bubble, they are removed
//from the bubble and stuck into the vector for re-classification.
bool SystemBubble::ProcessWander(std::vector<SystemEntity *> &wanderers) {
    //check to see if any of our dynamic entities are no longer in our bubble...
    std::set<SystemEntity *>::const_iterator cur, end;
    cur = m_dynamicEntities.begin();
    end = m_dynamicEntities.end();
    std::vector<SystemEntity *> found_wandering;    //the wanderers array may have other stuff in it, so use a local first.
    for(; cur != end; ++cur) {
        if(!InBubble((*cur)->GetPosition())) {
            //we cannot use Remove directly here because it will invalidate
            //our iterator, so store them away for now.
            found_wandering.push_back(*cur);
            wanderers.push_back(*cur);
        }
    }

    std::vector<SystemEntity *>::const_iterator curw, endw;
    curw = found_wandering.begin();
    endw = found_wandering.end();
    for(; curw != endw; ++curw) {
        _log( DESTINY__BUBBLE_TRACE, "SystemBubble::ProcessWander() - entity %s (%u) found wandering, but listed in bubble %u ... removing",
              (*curw)->GetName(), (*curw)->GetID(), this->GetBubbleID() );

        Remove(*curw);
    }
    return false;
}

void SystemBubble::Add(SystemEntity *ent, bool notify, bool isPostWarp) {
	//if they are already in this bubble, do not continue.
    if(m_entities.find(ent->GetID()) != m_entities.end()) {
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Tried to add entity %u to bubble %u, but it is already in here.",
             ent->GetID(), this->GetBubbleID());
        return;
    }

    GPoint startPoint( ent->GetPosition() );
    GVector direction(startPoint, NULL_ORIGIN);
    double rangeToStar = direction.length();
    rangeToStar /= ONE_AU_IN_METERS;

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Adding entity %u at %.2f,%.2f,%.2f to bubble %u at %.2f,%.2f,%.2f with radius %.2f.  \
                                 Distance to Star %.2f AU", \
                                 ent->GetID(), startPoint.x, startPoint.y, startPoint.z, \
                                 this->GetBubbleID(), m_center.x, m_center.y, m_center.z, \
                                 m_radius, rangeToStar);

    //if this entity is a Client and it is NOT cloaked, then notify everybody else in the bubble of the add.
    if( ent->IsClient() ) {
        _SendAddBalls(ent);
        Client *pClient = ent->CastToClient();
        if( (pClient->Destiny() != NULL) ) {
            if( !(pClient->Destiny()->IsCloaked()) ) {
                //if this entity is a Client and it is NOT cloaked, then notify everybody else in the bubble of the add.
                _BubblecastAddBall(ent);
                // Trigger SpawnManager for this bubble to generate NPC Spawn,
                ent->System()->DoSpawnForBubble(*this);
            }
		}
    } else
        _BubblecastAddBall(ent);

    //insert the entity into the list
    m_entities[ent->GetID()] = ent;
    ent->m_bubble = this;

    // if this is static entity then exit function, as following conditionals and functions are for clients only.
    if (ent->IsStaticEntity())
        return;

    m_dynamicEntities.insert(ent);

	// if entity is warping into this bubble, let everybody know their warp to point.
	if (isPostWarp) {
        Client *c = ent->CastToClient();
        if (c->Destiny() == NULL) return;

        GPoint target = c->Destiny()->GetTargetPoint();

        DoDestiny_CmdWarpTo wt;
        wt.entityID = ent->GetID();
        wt.dest_x = target.x;
        wt.dest_y = target.y;
        wt.dest_z = target.z;
        wt.distance = c->Destiny()->GetDistance();
        wt.warpSpeed = c->Destiny()->GetWarpSpeed();

        //exclusive bubblecast the update to all entities in bubble except warping entity, as they already know where they're going.
        PyTuple* t = wt.Encode();
        BubblecastDestinyUpdateExclusive( &t, "Exclusive WarpTo", ent );
        //PySafeDecRef( t );
	}
}

void SystemBubble::Remove(SystemEntity *ent, bool notify) {
    //assume that the entity is properly registered for its ID, and that
    //we do not need to search other values.
    if( ent->m_bubble == NULL )
        return;     // Get outta here in case this was called again

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Remove() - Removing entity %u at %.2f,%.2f,%.2f from bubble %u at %.2f,%.2f,%.2f", ent->GetID(), ent->GetPosition().x, ent->GetPosition().y, ent->GetPosition().z, this->GetBubbleID(), m_center.x, m_center.y, m_center.z, m_radius);

    ent->m_bubble = NULL;
    m_entities.erase(ent->GetID());
    m_dynamicEntities.erase(ent);

    //notify after removal so we do not remove ourself.
    if ((notify) && (ent->IsClient())) _SendRemoveBalls(ent);

    //regardless, notify everybody else in the bubble of the removal.
    _BubblecastRemoveBall(ent);
}

void SystemBubble::AddExclusive(SystemEntity *ent, bool notify) {
    //notify before addition so we do not include ourself.
    //if(notify) {
    //    _SendAddBalls(ent);
    //}
    ////if they are already in this bubble, do not continue.
    //if(m_entities.find(ent->GetID()) != m_entities.end()) {
    //    _log(DESTINY__BUBBLE_TRACE, "Tried to add entity %u to bubble %p, but it is already in here.", ent->GetID(), this);
    //    return;
    //}
    //regardless, notify everybody else in the bubble of the add.
    _BubblecastAddBallExclusive(ent);

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::AddExclusive() - Adding entity %u at %.2f,%.2f,%.2f to bubble %u at %.2f,%.2f,%.2f with radius %.2f", ent->GetID(), ent->GetPosition().x, ent->GetPosition().y, ent->GetPosition().z, this->GetBubbleID(), m_center.x, m_center.y, m_center.z, m_radius);
    //m_entities[ent->GetID()] = ent;
    //ent->m_bubble = this;
    //if(ent->IsStaticEntity() == false) {
    //    m_dynamicEntities.insert(ent);
    //}

    //// Trigger SpawnManager for this bubble to generate NPC Spawn, if any:
    //if( ent->IsClient() )
    //  ent->System()->DoSpawnForBubble(*this);
}

void SystemBubble::RemoveExclusive(SystemEntity *ent, bool notify) {
    //assume that the entity is properly registered for its ID, and that
    //we do not need to search other values.
    if( ent->m_bubble == NULL )
        return;     // Get outta here in case this was called again

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::RemoveExclusive() - Removing entity %u at %.2f,%.2f,%.2f from bubble %u at %.2f,%.2f,%.2f", ent->GetID(), ent->GetPosition().x, ent->GetPosition().y, ent->GetPosition().z, this->GetBubbleID(), m_center.x, m_center.y, m_center.z, m_radius);
    //ent->m_bubble = NULL;
    //m_entities.erase(ent->GetID());
    //m_dynamicEntities.erase(ent);
    //notify after removal so we do not remove ourself.
    //if(notify) {
    //    _SendRemoveBalls(ent);
    //}
    //regardless, notify everybody else in the bubble of the removal.
    _BubblecastRemoveBallExclusive(ent);
}

void SystemBubble::clear() {
    m_entities.clear();
    m_dynamicEntities.clear();
}

SystemEntity * const SystemBubble::GetEntity(uint32 entityID) const
{
    std::map<uint32, SystemEntity *>::const_iterator entity_iterator = m_entities.find(entityID);
    if( entity_iterator != m_entities.end() )
        return (entity_iterator->second);
    else
        return NULL;
}

void SystemBubble::GetEntities(std::set<SystemEntity *> &into) const {
    std::map<uint32, SystemEntity *>::const_iterator cur, end;
    cur = m_entities.begin();
    end = m_entities.end();
    for(; cur != end; cur++) {
        into.insert(cur->second);
    }
}

bool SystemBubble::InBubble(const GPoint &pt) const
{
    // Return true (we're still in this bubble) when System Entity is still within BUBBLE_RADIUS_METERS + BUBBLE_HYSTERESIS_METERS
    // from the center of the bubble
    return(GVector(m_center, pt).lengthSquared() < m_position_check_radius_sqrd);
}

//NOTE: not used right now. May never be used... see SystemManager::MakeSetState
//this is called as a part of the SetState routine for initial enter space.
//it appends information for all entities contained within the bubble.
/*
void SystemBubble::AppendBalls(DoDestiny_SetState &ss, std::vector<uint8> &setstate_buffer) const {
    if(m_entities.empty()) {
        return;
    }

    //go through all entities and gather the info we need...
    std::map<uint32, SystemEntity *>::const_iterator cur, end;
    cur = m_entities.begin();
    end = m_entities.end();
    for(; cur != end; cur++) {
        //ss.damageState
        ss.damageState[ cur->second->GetID() ] = cur->second->MakeDamageState();

        //ss.slims
        PyDict *slim_dict = cur->second->MakeSlimItem();
        ss.slims.add(new PyObject("foo.SlimItem", slim_dict));

        //append the destiny binary data...
        cur->second->EncodeDestiny(setstate_buffer);
    }
}
*/

void SystemBubble::_PrintEntityList() {
    std::map<uint32, SystemEntity*>::const_iterator cur, end;
    cur = m_entities.begin();
    end = m_entities.end();
    for (; cur != end; ++cur) {
        if (cur->second->IsVisibleSystemWide())
            sLog.Error( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Global.", cur->second->GetName(), cur->first );
        else if (cur->second->IsStaticEntity())
            sLog.Error( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Static.", cur->second->GetName(), cur->first );
        else if (cur->second->IsCelestial())
            sLog.Error( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Celestial.", cur->second->GetName(), cur->first );
        else if (cur->second->IsNPC())
            sLog.Error( "SystemBubble::_PrintEntityList()", "entity %s(%u) is NPC.", cur->second->GetName(), cur->first );
        else if (cur->second->IsClient())
            sLog.Error( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Client.", cur->second->GetName(), cur->first );
        else if (cur->second->IsPOS())
            sLog.Error( "SystemBubble::_PrintEntityList()", "entity %s(%u) is POS.", cur->second->GetName(), cur->first );
        else
            sLog.Error( "SystemBubble::_PrintEntityList()", "entity %s(%u) is None of the Above.", cur->second->GetName(), cur->first );
    }
}

void SystemBubble::_SendAddBalls( SystemEntity* to_who )
{
    if( m_entities.empty() )
    {
        _log( DESTINY__DEBUG, "SystemBubble::_SendAddBalls() - Nothing to send." );
        return;
    }
    _PrintEntityList();

    Buffer* destinyBuffer = new Buffer;

    Destiny::AddBall_header head;
    head.packet_type = 1;   // 0 = full state   1 = balls
    head.sequence = DestinyManager::GetStamp();

    destinyBuffer->Append( head );

    DoDestiny_AddBalls addballs;
    addballs.slims = new PyList;

    std::map<uint32, SystemEntity*>::const_iterator cur, end;
    cur = m_entities.begin();
    end = m_entities.end();
    for(; cur != end; ++cur)
    {
        //FIXME  this isnt right.   update xmlp and here to use PackagedAction and AddBalls2
        if( cur->second->IsVisibleSystemWide() )
            continue;    //it is already in their destiny state

        //damageState
        addballs.damages[ cur->second->GetID() ] = cur->second->MakeDamageState();
        //slim item -bullshit.   addballs DOES NOT have slimitems
        addballs.slims->AddItem( new PyObject( "foo.SlimItem", cur->second->MakeSlimItem() ) );
        //append the destiny binary data...
        cur->second->EncodeDestiny( *destinyBuffer );
    }

    if (addballs.slims->size() < 1) {
        _log( DESTINY__DEBUG, "SystemBubble::_SendAddBalls() - Nothing to send after check." );
        return;
    }

    addballs.destiny_binary = new PyBuffer( &destinyBuffer );
    SafeDelete( destinyBuffer );

    _log( DESTINY__DEBUG, "SystemBubble::_SendAddBalls():" );
    addballs.Dump( DESTINY__DEBUG, "    " );
    //_log( DESTINY__TRACE, "    Ball Binary:" );
    //_hex( DESTINY__TRACE, &( addballs.destiny_binary->content() )[0], addballs.destiny_binary->content().size() );
    _log( DESTINY__DEBUG, "    Ball Decoded:" );
    Destiny::DumpUpdate( DESTINY__TRACE, &( addballs.destiny_binary->content() )[0],
                                         addballs.destiny_binary->content().size() );

    PyTuple* t = addballs.Encode();
    to_who->QueueDestinyUpdate( &t );    //may consume, but may not.
    PySafeDecRef( t );
}

void SystemBubble::_SendRemoveBalls( SystemEntity* to_who )
{
    if( m_entities.empty() )
    {
        _log( DESTINY__DEBUG, "SystemBubble::_SendRemoveBalls() - Nothing to send." );
        return;
    }

    DoDestiny_RemoveBalls remove_balls;

    std::map<uint32, SystemEntity*>::const_iterator cur, end;
    cur = m_entities.begin();
    end = m_entities.end();
    for(; cur != end; ++cur)
    {
        if( cur->second->IsVisibleSystemWide() )
            continue;    //do not remove these from their state!

        remove_balls.balls.push_back( cur->second->GetID() );
    }

    if (remove_balls.balls.empty()) {
        return;
    }

    _log( DESTINY__DEBUG, "SystemBubble::_SendRemoveBalls():" );
    remove_balls.Dump( DESTINY__DEBUG, "    " );

    PyTuple* tmp = remove_balls.Encode();
    to_who->QueueDestinyUpdate( &tmp );    //may consume, but may not.
    PySafeDecRef( tmp );
}

void SystemBubble::_BubblecastAddBall( SystemEntity* about_who )
{
    if( m_dynamicEntities.empty() )
    {
        _log( DESTINY__DEBUG, "SystemBubble::_BubblecastAddBall() - Nobody to receive." );
        return;
    }

    Buffer* destinyBuffer = new Buffer;

    //create AddBalls header
    Destiny::AddBall_header head;
    head.packet_type = 1;   // 0 = full state   1 = balls
    head.sequence = DestinyManager::GetStamp();
    destinyBuffer->Append( head );

    DoDestiny_AddBalls addballs;
    addballs.slims = new PyList;

    //encode destiny binary
    about_who->EncodeDestiny( *destinyBuffer );
    addballs.destiny_binary = new PyBuffer( &destinyBuffer );
    SafeDelete( destinyBuffer );

    //encode damage state
    addballs.damages[ about_who->GetID() ] = about_who->MakeDamageState();
    //encode SlimItem
    addballs.slims->AddItem( new PyObject( "foo.SlimItem", about_who->MakeSlimItem() ) );

    //bubblecast the update
    PyTuple* t = addballs.Encode();
    BubblecastDestinyUpdate( &t, "AddBall" );
    PySafeDecRef( t );
}

void SystemBubble::_BubblecastAddBallExclusive( SystemEntity* about_who )
{
    if( m_dynamicEntities.empty() )
    {
        _log( DESTINY__DEBUG, "SystemBubble::_BubblecastAddBallExclusive() - Nobody to receive." );
        return;
    }

    Buffer* destinyBuffer = new Buffer;

    //create AddBalls header
    Destiny::AddBall_header head;
    head.packet_type = 1;   // 0 = full state   1 = balls
    head.sequence = DestinyManager::GetStamp();
    destinyBuffer->Append( head );

    DoDestiny_AddBalls addballs;
    addballs.slims = new PyList;

    //encode destiny binary
    about_who->EncodeDestiny( *destinyBuffer );
    addballs.destiny_binary = new PyBuffer( &destinyBuffer );
    SafeDelete( destinyBuffer );

    //encode damage state
    addballs.damages[ about_who->GetID() ] = about_who->MakeDamageState();
    //encode SlimItem
    addballs.slims->AddItem( new PyObject( "foo.SlimItem", about_who->MakeSlimItem() ) );

    //bubblecast the update
    PyTuple* t = addballs.Encode();
    BubblecastDestinyUpdateExclusive( &t, "AddBall", about_who );
    PySafeDecRef( t );
}

void SystemBubble::_BubblecastRemoveBall(SystemEntity *about_who) {
    if(m_dynamicEntities.empty()) {
        _log(DESTINY__DEBUG, "SystemBubble::_BubblecastRemoveBall() - Nobody to receive.");
        return;
    }

    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    DoDestiny_RemoveBalls removeball;
    removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__DEBUG, "SystemBubble::_BubblecastRemoveBall():");
    removeball.Dump(DESTINY__DEBUG, "    ");

    PyTuple *tmp = removeball.Encode();
    BubblecastDestinyUpdate(&tmp, "RemoveBall");    //consumed
}


void SystemBubble::_BubblecastRemoveBallExclusive(SystemEntity *about_who) {
    if(m_dynamicEntities.empty()) {
        _log(DESTINY__DEBUG, "SystemBubble::_BubblecastRemoveBallExclusive() - Nobody to receive.");
        return;
    }

    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    DoDestiny_RemoveBalls removeball;
    removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__DEBUG, "SystemBubble::_BubblecastRemoveBallExclusive():");
    removeball.Dump(DESTINY__DEBUG, "    ");

    PyTuple *tmp = removeball.Encode();
    BubblecastDestinyUpdateExclusive(&tmp, "RemoveBall", about_who);    //consumed
}
