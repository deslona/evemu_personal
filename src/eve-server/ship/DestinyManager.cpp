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
    Author:        Zhur
    Updated:		Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "PyServiceMgr.h"
#include "npc/NPC.h"
#include "ship/DestinyManager.h"
#include "station/Station.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

using namespace Destiny;

const double SPACE_FRICTION = 1.0e+6;        //straight from client. Do not change.
const double SPACE_FRICTION_SQUARED = SPACE_FRICTION*SPACE_FRICTION;
const float TIC_DURATION_IN_SECONDS = 1.0;    //straight from client. Do not change.
static const double DESTINY_UPDATE_RANGE = 1.0e8;    //totally made up. a more complex spatial partitioning system is needed.
static const double FOLLOW_BAND_WIDTH = 100.0f;    //totally made up
static const uint32 minWarpDistance = 150000;   //straight from client. Do not change.

static const double EULERS_NUMBER = 2.71828182845904;  //  set to the 15-digit limit of a double

static const GPoint NULL_ORIGIN = (0,0,0);

uint32 DestinyManager::m_stamp(40000);    //completely arbitrary starting point.
Timer DestinyManager::m_stampTimer(static_cast<int32>(TIC_DURATION_IN_SECONDS * 1000), true);    //accurate timing is essential...too bad it's not accurate.

DestinyManager::DestinyManager(SystemEntity *self, SystemManager *system)
: m_self(self),
  m_system(system),
//  m_body(NULL),
//  m_shape(NULL),
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
  m_position( NULL_ORIGIN ),    //generally right in the middle of the star!
  m_maxVelocity(1.0),
  m_shipMaxAccelTime(0.0),
  State(DSTBALL_STOP),
  m_moveTimer(0.0),
  m_userSpeedFraction(0.0f),
  m_activeSpeedFraction(0.0f),
  m_targetDistance(0.0),
  m_stopDistance(0),
  m_radius(1.0),
  m_mass(1.0f),
  m_massMKg(1.0f),
  m_shipWarpSpeed(0.0f),
  m_maxShipVelocity(1.0),
  m_shipAgility(1.0),
  m_shipInertiaModifier(1.0),
  m_warpStrength(1),
  m_warpCapacitorNeed(1.0),
  m_warpAccelTime(1.0f),
  m_warpDecelTime(1.0f),
  m_warpState(NULL)
{
    //do not touch m_self here, it may not be fully constructed.
    m_targetEntity.first = 0;
	m_targetEntity.second = NULL;

	m_velocity = GVector( NULL_ORIGIN );
	m_targetPoint = GPoint( NULL_ORIGIN );

	m_cloaked = false;
}

DestinyManager::~DestinyManager() {
    delete m_warpState;
}

void DestinyManager::Process() {
    //check for and process Destiny State changes.
    ProcessState();
}

void DestinyManager::SendSelfDestinyUpdate(PyTuple **up) const{
    m_self->QueueDestinyUpdate( up );
    PySafeDecRef( *up ); //they are not required to consume it.
}

void DestinyManager::SendSelfDestinyEvent(PyTuple **up) const {
    m_self->QueueDestinyEvent( up );
    PySafeDecRef( *up ); //they are not required to consume it.
}

void DestinyManager::SendDestinyUpdate(PyTuple **up, bool self_only) const {
    if( self_only )
	    SendSelfDestinyUpdate( up );
	else {
		_log( DESTINY__TRACE, "DestinyManager::SendDestinyUpdate - [%u] Broadcasting destiny update to bubble %u", GetStamp(), m_self->Bubble()->GetBubbleID() );
		m_self->Bubble()->BubblecastDestinyUpdate( up, "destiny" );
	}
}

void DestinyManager::SendDestinyEvent(PyTuple **ev, bool self_only) const {
    if( self_only )
	    SendSelfDestinyEvent( ev );
	else {
		_log( DESTINY__TRACE, "DestinyManager::SendDestinyEvent - [%u] Broadcasting destiny event to bubble %u", GetStamp(), m_self->Bubble()->GetBubbleID()  );
		m_self->Bubble()->BubblecastDestinyEvent( ev, "destiny" );
	}
}

void DestinyManager::SendDestinyUpdate( std::vector<PyTuple*> &updates, bool self_only ) const{
    if( self_only ) {
        _log( DESTINY__TRACE, "DestinyManager::SendDestinyUpdate - [%u] Sending destiny update (%lu) to self (%u).", GetStamp(), updates.size(), m_self->GetID() );

        std::vector<PyTuple*>::iterator cur, end;
        cur = updates.begin();
        end = updates.end();
        for(; cur != end; cur++) {
            PyTuple* t = *cur;
			SendSelfDestinyUpdate( &t );
        }
        updates.clear();
	} else if( NULL != m_self->Bubble() ) {
		_log( DESTINY__TRACE, "DestinyManager::SendDestinyUpdate - [%u] Broadcasting destiny update (%lu) to bubble %u", GetStamp(), updates.size(), m_self->Bubble()->GetBubbleID()  );
		m_self->Bubble()->BubblecastDestinyUpdate( updates, "destiny" );
	} else {
	    _log( DESTINY__ERROR, "DestinyManager::SendDestinyUpdate - [%u] Cannot broadcast destiny update (%lu); entity (%u) is not in any bubble.", GetStamp(), updates.size(), m_self->GetID() );
	}
}

void DestinyManager::SendDestinyEvent( std::vector<PyTuple*>& events, bool self_only ) const {
    if( self_only ) {
	    _log( DESTINY__TRACE, "DestinyManager::SendDestinyEvent - [%u] Sending destiny event (%lu) to self (%u).", GetStamp(), events.size(), m_self->GetID() );
		std::vector<PyTuple*>::iterator cur, end;
        cur = events.begin();
        end = events.end();
        for(; cur != end; cur++) {
            PyTuple* t = *cur;
			SendSelfDestinyEvent( &t );
        }
        events.clear();
    } else if( m_self->Bubble() ) {
		_log( DESTINY__TRACE, "DestinyManager::SendDestinyEvent - [%u] Broadcasting destiny event (%lu)to bubble %u", GetStamp(), events.size(), m_self->Bubble()->GetBubbleID()  );
		m_self->Bubble()->BubblecastDestinyEvent( events, "destiny" );
	} else {
	    _log( DESTINY__ERROR, "DestinyManager::SendDestinyEvent - [%u] Cannot broadcast destiny update (%lu); entity (%u) is not in any bubble.", GetStamp(), events.size(), m_self->GetID() );
	}
}

void DestinyManager::SendDestinyUpdate( std::vector<PyTuple*>& updates, std::vector<PyTuple*>& events, bool self_only ) const {
    if( self_only ) {
	    SendDestinyUpdate(updates, self_only);
		SendDestinyEvent(events, self_only);
    } else {
        _log( DESTINY__ERROR, "DestinyManager::SendDestinyUpdate - [%u] Cannot broadcast destiny update (u:%lu, e:%lu); entity (%u) is not in any bubble.", GetStamp(), updates.size(), events.size(), m_self->GetID() );
    }
}

void DestinyManager::ProcessState() {

  switch(State) {

    case DSTBALL_GOTO:
		//NOTE: WARP code assumes this is simply a call to _Move()!
        _Move();
        break;

    case DSTBALL_FOLLOW:
		if(m_targetEntity.second != NULL) {
            _Follow();
        } else {
            //nobody to follow?
            _log(PHYSICS__TRACE, "DestinyManager::ProcessTic - Entity %u has nothing to follow. Stopping.",
                m_self->GetID() );
            Stop();
        }
        break;

    case DSTBALL_STOP:
		Stop();
        break;

	case DSTBALL_WARP: {
        if(m_warpState != NULL) {
            //warp is already started
            _Warp();
            break;
        }

        //first check the angle between our warp vector and our velocity vector.
        GVector warp_vector(m_position, m_targetPoint);
        warp_vector.normalize();
        GVector vel_normalized(m_velocity);
        vel_normalized.normalize();
        double dot = warp_vector.dotProduct(vel_normalized);    //cos(angle) between vectors.
        if(dot < (1.0 - 0.01)) {    //this is about 8 degrees
            //execute normal GOTO code.
            _Move();
            break;
        }

        //ok, so they are heading in the right direction now, see about
        //velocity. We need to be going 75% of max velocity.
        double needed_velocity2 = m_maxShipVelocity*0.75;	// this is a TOTAL, MODIFIED ship speed...and affected by web.
        needed_velocity2 *= needed_velocity2;
        if(m_velocity.lengthSquared() < needed_velocity2) {
            //execute normal GOTO code.
            _Move();
            break;
        }

        //ok, we are starting the warp!
		if(_InitWarp())
			_Warp();

        } break;

	case DSTBALL_ORBIT:
        _Orbit();
        break;

    case DSTBALL_MUSHROOM:
    case DSTBALL_BOID:
    case DSTBALL_TROLL:
    case DSTBALL_MINIBALL:		// used for missiles
    case DSTBALL_MISSILE:
    case DSTBALL_FIELD:
    case DSTBALL_FORMATION:
    case DSTBALL_RIGID:
        break;
    //no default on purpose
    }
}

//Movement setting methods
//NOTE:   use this ONLY to set speed/velocity, DO NOT CALL _UpdateVelocity() directly.
void DestinyManager::SetSpeedFraction(float fraction) {
	/* movement is set according to speed fraction.
	 * m_activeSpeedFraction is used for accel and coast.
	 * m_userSpeedFraction is used for decel and stop/halt.
	 */
	if(fraction) {
		m_activeSpeedFraction = fraction;
	}
	m_userSpeedFraction = fraction;
	_UpdateVelocity();

	DoDestiny_CmdSetSpeedFraction du;
	du.entityID = m_self->GetID();
	du.fraction = fraction;

	PyTuple *tmp = du.Encode();
	SendDestinyUpdate(&tmp);    //consumed
}

void DestinyManager::_UpdateVelocity() {
	//if(IsMoving())  check for timers, decel, etc.
	if(m_activeSpeedFraction == 0) {
		// ship stopped (for whatever reason) so reset all velocity variables
		m_velocity = GVector( NULL_ORIGIN );
		m_moveTimer = 0.0;
		m_maxVelocity = 0.0;
		m_stopDistance = 0;
		m_targetDistance = 0.0;
		m_targetPoint = GPoint( NULL_ORIGIN );
	} else if(m_userSpeedFraction == 0) {
		// user command to Stop()
		// set decel math here.
		double currentTime = GetTimeMSeconds();
		currentTime /= 1000;
		m_moveTimer = currentTime;

		m_maxVelocity = m_maxShipVelocity * m_activeSpeedFraction;
		// use this to set a maximum deceleration time, after which the server will set speed to 0.
		//  see notes in _Move() for information relating to decel equations
		m_shipMaxAccelTime = (m_shipInertiaModifier * m_massMKg * -log(0.01));

		m_velocity = (m_maxVelocity - (m_maxVelocity * (1 - exp(-1000000 / (m_shipInertiaModifier * m_mass)))));
	} else {
		// velocity was changed (m_userSpeedFraction != 0) so reset timer and velocitys.
		double currentTime = GetTimeMSeconds();
		currentTime /= 1000;
		m_moveTimer = currentTime;

		m_maxVelocity = m_maxShipVelocity * m_activeSpeedFraction;
		// use this to set a maximum acceleration time, after which the server will set speed to max.
		//  see notes in _Move() for information relating to accel equations
		m_shipMaxAccelTime = (m_shipInertiaModifier * m_massMKg * -log(0.01));

		GVector moveVector(m_position, m_targetPoint);	//which way do we want to go?
		moveVector.normalize();		//change vector to direction
		m_velocity = (moveVector * (m_maxVelocity * (1 - exp(-1000000 / (m_shipInertiaModifier * m_mass)))));
	}

	DoDestiny_SetBallVelocity bv;
	bv.entityID = m_self->GetID();
	bv.x = m_velocity.x;
	bv.y = m_velocity.y;
	bv.z = m_velocity.z;
	PyTuple *up = bv.Encode();
	SendDestinyUpdate(&up);

	DoDestiny_SetMaxSpeed ms;
	ms.entityID = m_self->GetID();
	ms.speedValue = m_maxVelocity;
	up = ms.Encode();
	SendDestinyUpdate(&up);
	PySafeDecRef(up);
}

//Global Actions:
void DestinyManager::Stop(bool update) {
	//Clear any pending docking operation since the user halted ship movement:
	//  remove autopilot checks also
	if( m_self->IsClient() ) {
		m_self->CastToClient()->SetPendingDockOperation( false );
		m_self->CastToClient()->m_autoPilot = false;
	}

	if(State == DSTBALL_STOP)
		return;

	if(State == DSTBALL_WARP && m_warpState != NULL) {
		//warp aborted!
		delete m_warpState;
		m_warpState = NULL;
	}

	m_targetEntity.first = 0;
	m_targetEntity.second = NULL;
	m_userSpeedFraction = 0;

	State = DSTBALL_STOP;

	//ensure that our bubble is correct.
	m_system->bubbles.UpdateBubble(m_self);

	if(update) {
		DoDestiny_CmdStop du;
		du.entityID = m_self->GetID();

		PyTuple *tmp = du.Encode();
		SendDestinyUpdate(&tmp);    //consumed
	}
}

void DestinyManager::Halt(bool update) {
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;
	SetSpeedFraction(0.0f);

    //Clear any pending docking operation since the user halted ship movement:
	//  remove autopilot checks also
	if( m_self->IsClient() ) {
		m_self->CastToClient()->SetPendingDockOperation( false );
	    m_self->CastToClient()->m_autoPilot = false;
	}

    State = DSTBALL_STOP;

    //ensure that our bubble is correct.
    m_system->bubbles.UpdateBubble(m_self);
}

void DestinyManager::_Follow() {
//  this is also called by client as 'approach'

    // First check to see if our target has somehow been removed from space
    // _OR_ the player has left the target ship..
    // if so, then we need to call DestinyManager::Stop() to stop the ship from
    // following a non-existent space object or a pilot-less ship,
    // otherwise, proceed to continue following:
    Client * targetClient = NULL;

    if( m_system->get( m_targetEntity.first ) == NULL )
    {
        // Our target was removed, so STOP
        Stop();
        return;
    }
    else
    {
        // Our target still exists, so do more checking:
        if( (targetClient = (m_targetEntity.second->CastToClient())) != NULL )
        {
            // We are orbiting a Client object, i.e. a player ship
            if( m_targetEntity.first != targetClient->GetShipID() )
            {
                // The client is no longer in the ship we were targeting, so STOP
                Stop();
                return;
            }
        }
    }

    const GPoint &target_point = m_targetEntity.second->GetPosition();

    GVector them_to_us(target_point, m_position);
    them_to_us.normalize();

    double desired_distance =
        m_radius +
        m_targetEntity.second->GetRadius() +
        m_targetDistance;

    m_targetPoint = target_point + (them_to_us * desired_distance);

	SetSpeedFraction(1.0f);

	DoDestiny_CmdFollowBall fb;
	fb.entityID = m_self->GetID();
	fb.ballID = m_targetEntity.second->GetID();
	fb.range = desired_distance;

	PyTuple* up = fb.Encode();
	SendDestinyUpdate(&up);

    _Move();
}

void DestinyManager::_Move() {
	//apply our velocity and direction to our position for 1 unit of time (a second)
	// NOTE Orbit CANNOT use this method for movement as it will track/adjust postion and velocity differently.

    // Check to see if we have a pending docking operation and attempt to dock if so:
    if( m_self->IsClient() && m_self->CastToClient()->GetPendingDockOperation() )
        AttemptDockOperation();

	/* acceleration and deceleration are both logarithmic, and the server needs to keep up with client position.
	 * this is another step towards getting ready for collision detection.
	 *
	 * formula for time taken to accelerate from v to V, from https://wiki.eveonline.com/en/wiki/Acceleration
	 *
	 *   t=IM(10^-6) * -ln(1-(v/V))
	 *
	 * as this uses the natural log, the higher the speed, the slower the acceleration, to the limits of ln(0)
	 * since lim ln(x) = -INFINITY where x->0+. and ln(0) is undefined, we will use
	 *
	 *   m_shipMaxAccelTime = (m_shipInertiaModifier * m_massMKg * -log(0.01));
	 *
	 * to define the time it will take a give ship to reach 99% of m_maxShipVelocity, at which point,
	 * the server will set m_velocity to m_maxShipVelocity (or 100%).
	 *
	 * to define speed at time, we will use the following equation.
	 *
	 *   Vt = Vm * (1 - e(-t * 10^6 / IM))
	 *
	 * where
	 * Vt = ships velocity at t
	 * Vm = ships maximum velocity
	 *  t = time
	 *  I = ships inertia in s/kg
	 *  M = ships mass in kg
	 *  e = base of natural logarithms
	 *
	 * this replaces the convoluted _MoveAccel() method, which is now obsolete
	 */

	// make check for IsMoving() in here?
	//  to what purpose?

	GVector moveVector(m_position, m_targetPoint);	//which way do we want to go?
	double curDistance = moveVector.length();		//and how far away?
	GVector curDirection(m_velocity);				//which way are we currently pointing?

	moveVector.normalize();		//change vector to direction
	curDirection.normalize();

	double currentMSec = GetTimeMSeconds();
	currentMSec /= 1000;
	double timeStamp = (currentMSec - m_moveTimer);

	//will have to add for decel, opposite of accel, with checks for speed fraction... 0 = decel, >0 = accel (or maintain)
	if(timeStamp > m_shipMaxAccelTime) {
		if((m_activeSpeedFraction) && (m_userSpeedFraction)){
			// reached full speed
			m_velocity = moveVector * m_maxVelocity;
		} else {
			// reached full stop
			m_velocity = GVector( NULL_ORIGIN );
		}
	} else if(curDirection != moveVector) {
		//m_activeSpeedFraction != 0 for direction change
		//just changed direction.  reset velocity
		m_velocity = (moveVector * (m_maxVelocity * (1 - exp(-1000000 / (m_shipInertiaModifier * m_mass)))));
	} else if(curDistance < m_stopDistance){
		//too close
		m_velocity /= 2;
	} else {
		if(m_userSpeedFraction){
			//just accelerating
			/* accel in eve is logarithmic, following a modified bell curve, but capped to a specific time,
			 * based on ship class and maxSubWarpSpeed.
			 * this function is defined above, and implemented here.
			 */
			m_velocity = (m_maxVelocity * (1 - exp(-timeStamp * 1000000 / (m_shipInertiaModifier * m_mass))));
		} else {
			//just decelerating
			/* decel is opposite of accel, which are both logarithmic fuctions of time.
			 * see notes above
			 */
			m_velocity = m_maxVelocity - (m_maxVelocity * (1 - exp(-timeStamp * 1000000 / (m_shipInertiaModifier * m_mass))));
		}
	}

	//set postion for this round of movement
	m_position += m_velocity;

	//update client with velocity and position data
	DoDestiny_SetBallVelocity bv;
	bv.entityID = m_self->GetID();
	bv.x = m_velocity.x;
	bv.y = m_velocity.y;
	bv.z = m_velocity.z;
	PyTuple *up = bv.Encode();
	SendDestinyUpdate(&up);

	DoDestiny_SetBallPosition bp;
	bp.entityID = m_self->GetID();
	bp.x = m_position.x;
	bp.y = m_position.y;
	bp.z = m_position.z;
	up = bp.Encode();
	SendDestinyUpdate(&up);
	PySafeDecRef(up);

	//update bubble with velocity and position data
	m_system->bubbles.UpdateBubble(m_self);
}

void DestinyManager::_MoveAccel(const GVector &calc_acceleration) {
}

bool DestinyManager::_InitWarp() {
	GVector warp_distance(GetPosition(), m_targetPoint);
	m_targetDistance = warp_distance.length();
	m_targetDistance -= m_stopDistance;

	if(m_targetDistance < minWarpDistance) {
		// warp distance too close.  cancel and return
		if(m_self->IsClient())
			m_self->CastToClient()->SendErrorMsg("That is too close for your Warp Drive.");

		State = DSTBALL_STOP;
		delete m_warpState;
		return false;
	}

    //init warp:

    /*supercap warp modifiers
     * these will go here, and modify distance, target, and range accordingly
     *
     * AttrWarpAccuracyMaxRange = 1021,
     * AttrWarpAccuracyFactor = 1022,
     * AttrWarpAccuracyFactorMultiplier = 1023,
     * AttrWarpAccuracyMaxRangeMultiplier = 1024,
     * AttrWarpAccuracyFactorPercentage = 1025,
     * AttrWarpAccuracyMaxRangePercentage = 1026,
     */

    //destiny_WARP_Math....allan 1Nov14

	if(m_self->IsClient()) {
		Client *p_Client = m_self->CastToClient();
		InventoryItemRef ship = p_Client->GetShip();
		// adjust warp distance based on cap left.   NPC's are not restricted by this. (or wont be in EvEmu)
		double currentShipCap = ship->GetAttribute(AttrCapacitorCharge).get_float();

		/*  capacitor for warp forumlas from https://oldforums.eveonline.com/?a=topic&threadID=332116
		 *  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
		 */
		double adjWarpCapNeed = m_warpCapacitorNeed * (1 - (p_Client->GetChar()->GetSkillLevel(skillWarpDriveOperation, true) * 0.1));
		double adjDistance = m_targetDistance / ONE_AU_IN_METERS;	// change distance in meters to AU.
		double capNeeded = m_mass * adjWarpCapNeed * adjDistance;

		//  check if ship has enough capacitor to warp full distance
		if(capNeeded > currentShipCap) {
			//  nope...enough cap to min warp?
			double cap_check = 150000 * m_mass * adjWarpCapNeed;
			if(cap_check > 0) {
				//reset distance based on avalible capacitor   <<<<< FIXME this is wrong...
				m_stopDistance = currentShipCap / (m_mass * adjWarpCapNeed);
				capNeeded = currentShipCap;
			} else {
				if(m_self->IsClient())
					m_self->CastToClient()->SendErrorMsg("You don't have enough capacitor charge to warp.");

				State = DSTBALL_STOP;
				delete m_warpState;
				return false;
			}
		} else
			capNeeded = currentShipCap - capNeeded;

		//drain cap
		ship->SetAttribute(AttrCapacitorCharge, capNeeded, true);
	}

	// warp time and distance math

	/*  my research into warp formulas, i have used these sites, with a few exerpts and ideas from each...
	 *     https://wiki.eveonline.com/en/wiki/Acceleration
	 * 	   http://oldforums.eveonline.com/?a=topic&threadID=1251486
	 *     http://gaming.stackexchange.com/questions/115271/how-does-one-calculate-a-ships-agility
	 *     http://eve-search.com/thread/1514884-0
	 *     http://www.eve-search.com/thread/478431-0/page/1
	 */

	/*  PUT CHECK HERE FOR WARP BUBBLES
	 *     and other things that affect warp-in point.....when we get to there.
	 * AttrWarpBubbleImmune = 1538,
	 * AttrWarpBubbleImmuneModifier = 1539,
	 *   NOTE:  warp bubble in path (or within 100km of m_targetPoint) will change m_targetDistance and m_targetPoint
	 */

	/* this is my version of how warp should be timed and followed by the server.
	 * checks here for distance < warp speed and distance < 2au, (with all distances in meters)
	 *  and adjusts accel/decel times accordingly
	 *
	 *   accel/decel are logrithmic. this seems to now agree with client.
	 *   decel = accel x 2.5, so if warp distance < warp speed, dist's will have to be adjusted.
	 *   decel 71.5% and accel is 28.5% of (total time - cruise)
	 *
	 * all warps are in same time groups for all ships, except freighters and caps.
	 * distance checks are seperated into 3 time groups, with subgroups for freighters and caps.
	 *
	 */
	bool cruise = false;
	double warpSpeedInMeters = m_shipWarpSpeed * ONE_AU_IN_METERS;
	if(m_targetDistance < 1000000001) {
		// < 1mkm, total time for all ships is 11s.
		m_warpAccelTime = 11 * 0.285;
		m_warpDecelTime = 11 - m_warpAccelTime;
	} else if(m_targetDistance < ONE_AU_IN_METERS) {	// this covers 1mkm to 1au.
		// at 1mkm, total time for all ships except freighters is 23s.
		// freighters base time is 29s, and all freighter warp distances are covered here.
		if(m_targetDistance > warpSpeedInMeters) {
			m_warpAccelTime = 29 * 0.285;
			m_warpDecelTime = 29 - m_warpAccelTime;
			cruise = true;
		} else {
			m_warpAccelTime = 23 * 0.285;
			m_warpDecelTime = 23 - m_warpAccelTime;
		}
	} else if(m_targetDistance < (ONE_AU_IN_METERS * 2)) {	// this covers between 1 and 2 au, and excludes freighters.
		// at 1mkm, total time for all other ships except caps is 29s.
		// capitals base time is 30s, and all capital warp distances are covered here
		if(m_targetDistance > warpSpeedInMeters) {
			m_warpAccelTime = 30 * 0.285;
			m_warpDecelTime = 30 - m_warpAccelTime;
			cruise = true;
		} else {
			m_warpAccelTime = 29 * 0.285;
			m_warpDecelTime = 29 - m_warpAccelTime;
		}
	} else {
		// for all distances that are far enough to cruise, 30s is base time.
		m_warpAccelTime = 30 * 0.285;
		m_warpDecelTime = 30 - m_warpAccelTime;
		cruise = true;
	}

	/*  this is from http://community.eveonline.com/news/dev-blogs/warp-drive-active/
	 * For the acceleration phase, k is 3.
	 * For deceleration, k is 1.
	 * x = e^(k.t)
	 * x = distance in meters
	 * t = time in seconds
	 *
	 * this gives distances as functions of time.
	 * distances are reversed because it makes more sense that since accel is faster, it takes shorter distances.
	 *
	 */
	double accelDistance = exp(m_warpDecelTime);  //4.599e10
	double decelDistance = exp(3 * m_warpAccelTime);  //5.32e11
	double cruiseDistance = 0;
	float cruiseTime = 0.0f, warpTime = 0.0f;

	//  set cruise times
	if(cruise) {
		cruiseDistance = m_targetDistance - accelDistance - decelDistance;
		cruiseTime = cruiseDistance / warpSpeedInMeters;
	} else {
		//  short warp....no cruise
		cruiseDistance = 0;
		cruiseTime = 0.0f;
		warpSpeedInMeters = m_targetDistance / 2;
	}

	//  set total warp time base on above math.
	warpTime =  m_warpAccelTime + m_warpDecelTime + cruiseTime;
	// set warp decel time for linear check later.
	m_warpDecelTime = m_warpAccelTime + cruiseTime;

	_log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): Warp will accelerate for %fs, then slow down at %fs, with total time of %fs",
		 m_self->GetName(), m_self->GetID(), m_warpAccelTime, m_warpDecelTime, warpTime);

	_log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): We will exit warp at %f, %f, %f at a distance of %f AU (%fm).",
		 m_self->GetName(), m_self->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance/ONE_AU_IN_METERS, m_targetDistance);

	GVector warp_vector(m_targetPoint, m_position);
	warp_vector.normalize();

    delete m_warpState;
	double currentTime = GetTimeMSeconds();
	currentTime /= 1000;
    m_warpState = new WarpState(
		currentTime, //this gives 000000000000.000
		m_targetDistance,
		warpSpeedInMeters,
		m_warpAccelTime,
		accelDistance,
		cruiseTime,
		cruiseDistance,
		m_warpDecelTime,
		decelDistance,
		warpTime,
		warp_vector );

	// acknowledge client's warpto request
	DoDestiny_CmdWarpTo wt;
	wt.entityID = m_self->GetID();
	wt.dest_x = m_targetPoint.x;
	wt.dest_y = m_targetPoint.y;
	wt.dest_z = m_targetPoint.z;
	wt.distance = m_stopDistance;
	// client expects speed x 10.  dumb ccp shit again
	//  this dont show up right in fitting window, but is right on speedo.
	wt.warpSpeed = static_cast<uint32>(m_shipWarpSpeed * 10);
	PyTuple* up = wt.Encode();
	SendDestinyUpdate(&up);

	//send a warping special effects update...
	DoDestiny_OnSpecialFX10 sfx;
	sfx.effect_type = "effects.Warping";
	sfx.entityID = m_self->GetID();
	sfx.isOffensive = false;
	sfx.start = true;
	sfx.active = false;
	up = sfx.Encode();
	SendDestinyUpdate(&up);

	//the client clears massive during warp,  (massive means object is solid)
	DoDestiny_SetBallMassive bm;
	bm.entityID = m_self->GetID();
	bm.is_massive = false;
	up = bm.Encode();
	SendDestinyUpdate(&up);
	PySafeDecRef(up);

	return true;
}

void DestinyManager::_Warp() {
    if(m_warpState == NULL) {
        codelog(DESTINY__ERROR, "Error: _Warp() called with no warp state!");
        return;
    }

    /*
     * There are three stages of warp, which are functions of time and speed:
     *
     *  1) Acceleration.
	 * 		this is a fixed attribute, which is roughly 9s to full warp speed.
     *  2) Cruising.
	 * 		traveling at maximum warp speed
     *  3) Deceleration.
	 * 		this also is a fixed attribute, which is roughly 22s from full warp speed.
	 *
	 *	Acceleration and Deceleration are logrithmic with finite caps, instead of infinity, at the ends.
	 * 		see also:  my notes in _InitWarp()
     */

	double currentMSec = GetTimeMSeconds();
	currentMSec /= 1000;
	float msec_into_warp = (currentMSec - m_warpState->start_stamp);

	//variables set by the current stage....higher resolution = more accuracy
	double currentShipSpeed, currentDistance, remainingDistance;
	bool stop = false;

	//  speed and distance formulas based on current warp time
	if(msec_into_warp < m_warpState->accelTime) {	// in acceleration
		/* For acceleration, k = 3.
		 * distance = e^(k*s)
		 * speed = k*e^(k*s)
		 */
		currentDistance = exp(3 * msec_into_warp);
		currentShipSpeed = (3 * exp(3 * msec_into_warp));
        remainingDistance = m_warpState->total_distance - currentDistance;

		// update existing, but dont create or add to new bubbles
		//if( currentDistance <  BUBBLE_RADIUS_METERS )
        m_system->bubbles.UpdateBubble(m_self,true,true);

		_log(PHYSICS__TRACE, "DestinyManager::_Warp():Accelerate - Entity %s(%u): Warp Accelerating(%fs): velocity %f m/s with %f m left to go.",
			 m_self->GetName(), m_self->GetID(), msec_into_warp, currentShipSpeed, remainingDistance);
	} else if(msec_into_warp < m_warpState->decelTime)  {
		/* in cruise....calculate values to update postion and velocity */
		float cruiseTime = msec_into_warp - m_warpState->accelTime;
		currentDistance = m_warpState->accelDist + (m_warpState->warpSpeed * cruiseTime);
		remainingDistance = m_warpState->total_distance - currentDistance;
		currentShipSpeed = m_warpState->warpSpeed;

		_log(PHYSICS__TRACE, "DestinyManager::_Warp():Cruise - Entity %s(%u): Warp Crusing(%fs): velocity %f m/s with %f m left to go.",
			 m_self->GetName(), m_self->GetID(), msec_into_warp, currentShipSpeed, remainingDistance);
	} else {		// in deceleration
		/* For deceleration, k = -1.
		 * distance = e^(k*s)
		 * speed = k*e^(k*s)
		 */
		float decelTime = msec_into_warp - m_warpState->decelTime;
		currentDistance = m_warpState->total_distance - (exp(-decelTime) * m_warpState->decelDist);
		remainingDistance = m_warpState->total_distance - currentDistance;
		currentShipSpeed = (m_warpState->warpSpeed * (-exp(-decelTime)));
		if(currentShipSpeed < 0 ) currentShipSpeed = -currentShipSpeed;

		_log(PHYSICS__TRACE, "DestinyManager::_Warp():Decelerate - Entity %s(%u): Warp Decelerating(%fs): velocity %fm/s with %fm left to go.",
			 m_self->GetName(), m_self->GetID(), msec_into_warp, currentShipSpeed, remainingDistance);

        // Put ourself back into a bubble once we reach the outer edge of the bubble's radius
		//  this *SHOULD* update all clients in bubble with the new addition
        if( remainingDistance <  (BUBBLE_RADIUS_METERS  * 2) ) {
            m_system->bubbles.UpdateBubble(m_self, true, true);
		}

		if(currentShipSpeed < m_speedToLeaveWarp) {
			// stop warp, and return to normal(stop) mode
			_log(PHYSICS__TRACE, "DestinyManager::_Warp():Complete - Entity %s(%u): Warp completed. Exit velocity %f m/s with %f m left to go.",
				m_self->GetName(), m_self->GetID(), currentShipSpeed, remainingDistance);
			stop = true;
		}
    }

    //  update position and velocity for all stages.
    //  this isnt right.....
    //GVector vector_to_target = m_warpState->warp_vector * remainingDistance;
	m_position = m_targetPoint - remainingDistance;
	m_velocity = m_warpState->warp_vector * currentShipSpeed;
	//SetPosition( m_position, true, true );

	if(stop) {
		//  this *SHOULD* update all clients in bubble with the new information.
		delete m_warpState;
		m_warpState = NULL;
		//put ourself back into a bubble.
		m_system->bubbles.UpdateBubble(m_self, true, false, true);
		SetPosition( m_position, true, false, true );
		Stop();

		State = DSTBALL_STOP;

		//the client clears massive during warp, so reset for normal operation
		DoDestiny_SetBallMassive bm;
		bm.entityID = m_self->GetID();
		bm.is_massive = true;
		PyTuple *up = bm.Encode();
		SendDestinyUpdate(&up);
		PySafeDecRef(up);

		/*	this makes an abrupt halt when set to 0 ....dont use here except for caught in bubble.
		 * if(bubbled)
			m _velocity = GVector(NULL_ORIGIN);
		*/
	}
}

//this is still under construction. Its not working well right now.
void DestinyManager::_Orbit() {
#define PVN(v) _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - Entity %u: " #v ": (%.15e, %.15e, %.15e)   len=%.15e", \
            m_self->GetID(), \
            v.x, v.y, v.z, v.length() )

    // First check to see if our target has somehow been removed from space
    // _OR_ the player has left the target ship..
    // if so, then we need to call DestinyManager::Stop() to stop the ship from
    // following a non-existent space object or a pilot-less ship,
    // otherwise, proceed to continue orbiting:
    Client * targetClient = NULL;
    if( m_system->get( m_targetEntity.first ) == NULL )
    {
        // Our target was removed, so STOP
        Stop();
        return;
    }
    else
    {
        // Our target still exists, so do more checking:
        if( (targetClient = (m_targetEntity.second->CastToClient())) != NULL )
        {
            // We are orbiting a Client object, i.e. a player ship
            if( m_targetEntity.first != targetClient->GetShipID() )
            {
                // The client is no longer in the ship we were targeting, so STOP
                Stop();
                return;
            }
        }
    }

    const GPoint &orbit_point = m_targetEntity.second->GetPosition();

    GVector delta(m_position, orbit_point);
    PVN(delta);
    double current_distance = delta.normalize();
    PVN(delta);

    double something = 0;
    double desired_distance =
        m_radius +
        m_targetEntity.second->GetRadius() +
        m_targetDistance;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - desired_distance = %.15e", desired_distance);
    if(desired_distance != 0) {
        something = (m_maxVelocity * TIC_DURATION_IN_SECONDS * .01) / desired_distance;
    }
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - something = %.15e", something);

//this seems to be correct, without rounding error.
	//  use m_moveTimer here.....
    double v488 = double(GetStamp()-m_stateStamp) * something;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - v488 = %.15e", v488);

//this is not quite right... some sort of rounding I think.
    double coef = (v488 * 0.7) + 130001409/*entityID*/;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - coef = %.15e", coef);

//all of these are wrong due to rounding
    double cos_coef = cos( coef );
    double sin_coef = sin( coef );
    double cos_v488 = cos( v488 );
    double sin_v488 = sin( v488 );
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - coef(sin=%.15e, cos=%.15e) v488(sin=%.15e, cos=%.15e)", sin_coef, cos_coef, sin_v488, cos_v488);
    double v438 = cos_coef * sin_v488;
    double v3C0 = cos_v488 * cos_coef;

    GPoint pt( v3C0, sin_coef, v438 );    //this is a unit vector naturally
    PVN(pt);

    GVector tan_vector = pt.crossProduct(delta);
    tan_vector.normalize();
    PVN(tan_vector);

    double delta_d2 = current_distance*current_distance - desired_distance*desired_distance;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - delta_d2 = %.15e", delta_d2);
    if(delta_d2 >= 0) {
        double mag = sqrt(delta_d2) * desired_distance / current_distance;
        _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - mag = %.15e", mag);
        GVector s = tan_vector * mag;
        PVN(s);
        GVector d = delta * (delta_d2/current_distance);
        PVN(d);
        delta = s + d;
        delta.normalize();
        PVN(delta);
    }

    double d = desired_distance - current_distance;
    d = exp(d*d / (-40000.0));
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - d = %.15e", d);

    GVector negative_delta = delta * -1;
    PVN(negative_delta);

    double tdn = tan_vector.dotProduct(negative_delta);
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - tdn = %.15e", tdn);
    double jjj = ((tdn*tdn - 1.0) * d * d) + 1;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - jjj = %.15e", jjj);
    double iii;
    if(jjj < 0) {    //not sure on this condition at all.
        iii = d * tdn;    //not positive
    } else {
        iii = sqrt(jjj) + (d * tdn);
    }

    if((current_distance - desired_distance) < 0) {
        iii *= -1;
    }

    GPoint bliii = delta * iii;
    PVN(bliii);
    GPoint vliii = tan_vector * d;
    PVN(vliii);

    GPoint accel_vector = bliii + vliii;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - m_accelerationFactor = %.15e", m_shipMaxAccelTime);
    accel_vector *=  m_shipMaxAccelTime;
    PVN(accel_vector);

    //copy delta_position into acceleration for input into movement.

    static const double ten_au = 1.495978707e12;
    GVector big_delta_position = accel_vector * ten_au;
    PVN(big_delta_position);

    m_targetPoint = orbit_point + big_delta_position;
    PVN(m_targetPoint);
#undef PVN

    _MoveAccel(accel_vector);
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *who) {
    if(m_targetEntity.second == who) {
        m_targetEntity.first = 0;
        m_targetEntity.second = NULL;

        switch(State) {
        case DSTBALL_GOTO:
        case DSTBALL_STOP:
        case DSTBALL_MISSILE:
        case DSTBALL_MUSHROOM:
        case DSTBALL_BOID:
        case DSTBALL_TROLL:
        case DSTBALL_MINIBALL:
        case DSTBALL_FIELD:
        case DSTBALL_FORMATION:
        case DSTBALL_RIGID:
        case DSTBALL_WARP:
            //do nothing, they dont use the follow entity.
            break;

        case DSTBALL_FOLLOW:
        case DSTBALL_ORBIT:
            _log(DESTINY__DEBUG, "Entity %s(%u): Our target entity has gone away. Stopping.", m_self->GetName(), m_self->GetID());
            Stop();
            break;

        //no default on purpose
        }
    }
}

//Local Movement:
void DestinyManager::Follow(SystemEntity *who, double distance, bool update) {
//called from client as 'approach'
    if(State == DSTBALL_FOLLOW && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    State = DSTBALL_FOLLOW;
	m_targetPoint = GPoint(who->GetPosition());
    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;

	SetSpeedFraction(1.0f);

    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    sLog.Debug( "DestinyManager::Follow()", "SystemEntity '%s' following SystemEntity '%s' at velocity %f",
                m_self->GetName(), who->GetName(), m_maxVelocity );

    if(update) {
        DoDestiny_CmdFollowBall du;
        du.entityID = m_self->GetID();
        du.ballID = who->GetID();
        du.range = int32(distance);

        PyTuple *up = du.Encode();
		SendDestinyUpdate(&up);    //consumed
		PySafeDecRef(up);
    }
}

void DestinyManager::Orbit(SystemEntity *who, double distance, bool update) {
    if(State == DSTBALL_ORBIT && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    State = DSTBALL_ORBIT;
    m_stateStamp = GetStamp()+1;

    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;
	SetSpeedFraction(1.0f);

    if(update) {
        DoDestiny_CmdOrbit du;
        du.entityID = m_self->GetID();
        du.orbitEntityID = who->GetID();
        du.distance = uint32(distance);

        PyTuple *up = du.Encode();
		SendDestinyUpdate(&up);    //consumed
		PySafeDecRef(up);
    }
}

void DestinyManager::OrbitingCruise(SystemEntity *who, double distance, bool update, double cruiseSpeed) {
    if(State == DSTBALL_ORBIT && m_targetEntity.second == who && m_targetDistance == distance)
        return;

	if( cruiseSpeed > 0 )
	{
		m_maxShipVelocity = cruiseSpeed;
		_UpdateVelocity();
	}

    State = DSTBALL_ORBIT;
    m_stateStamp = GetStamp()+1;

    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;
    if(m_userSpeedFraction == 0.0)
        m_userSpeedFraction = 1.0f;    //doesn't seem to do this.
    if(m_activeSpeedFraction != m_userSpeedFraction) {
        m_activeSpeedFraction = 1.0f;
        _UpdateVelocity();
    }

    if(update) {
        DoDestiny_CmdOrbit du;
        du.entityID = m_self->GetID();
        du.orbitEntityID = who->GetID();
        du.distance = uint32(distance);

        PyTuple *up = du.Encode();
		SendDestinyUpdate(&up);    //consumed
		PySafeDecRef(up);
    }
}

void DestinyManager::TractorBeamFollow(SystemEntity *who, double mass, double maxVelocity, double distance, bool update) {
	if (State == DSTBALL_FOLLOW && m_targetEntity.second == who && m_targetDistance == distance)
		return;

	State = DSTBALL_FOLLOW;
	m_targetEntity.first = who->GetID();
	m_targetEntity.second = who;
	m_targetDistance = distance;

	// FIXME  need to set a target point here
	m_targetPoint = GPoint(0,0,0);

	SetSpeedFraction(1.0f);

	//Clear any pending docking operation since the user set a new course:
	if (m_self->IsClient())
		m_self->CastToClient()->SetPendingDockOperation(false);

	sLog.Debug("DestinyManager::TractorBeamFollow()", "SystemEntity '%s' following SystemEntity '%s' at velocity %f",
			   m_self->GetName(), who->GetName(), m_maxVelocity);

	std::vector<PyTuple *> updates;

	if (update) {
		DoDestiny_SetMaxSpeed maxspeed1;
		maxspeed1.entityID = m_self->GetID();
		maxspeed1.speedValue = maxVelocity;
		updates.push_back(maxspeed1.Encode());

		DoDestiny_SetBallFree ballfree;
		ballfree.entityID = m_self->GetID();
		ballfree.is_free = 1;
		updates.push_back(ballfree.Encode());

		DoDestiny_SetBallMass ballmass;
		ballmass.entityID = m_self->GetID();
		ballmass.mass = mass;
		updates.push_back(ballmass.Encode());

		DoDestiny_SetMaxSpeed maxspeed2;
		maxspeed2.entityID = m_self->GetID();
		maxspeed2.speedValue = maxVelocity;
		updates.push_back(maxspeed2.Encode());

		DoDestiny_CmdSetSpeedFraction speedfrac;
		speedfrac.entityID = m_self->GetID();
		speedfrac.fraction = 1.0;
		updates.push_back(speedfrac.Encode());

		DoDestiny_CmdFollowBall followball;
		followball.entityID = m_self->GetID();
		followball.ballID = who->GetID();
		followball.range = int32(m_targetDistance);
		updates.push_back(followball.Encode());

		SendDestinyUpdate(updates, false); //consumed
	}
}

void DestinyManager::TractorBeamHalt(bool update)
{
	m_targetEntity.first = 0;
	m_targetEntity.second = NULL;
	m_velocity = GVector( NULL_ORIGIN );
	//SetSpeedFraction(0.0f);
	Stop();

	State = DSTBALL_STOP;

	//ensure that our bubble is correct.
	m_system->bubbles.UpdateBubble(m_self);

	std::vector<PyTuple *> updates;

	if (update) {
		DoDestiny_SetMaxSpeed maxspeed1;
		maxspeed1.entityID = m_self->GetID();
		maxspeed1.speedValue = 0;
		updates.push_back(maxspeed1.Encode());

		DoDestiny_SetBallFree ballfree;
		ballfree.entityID = m_self->GetID();
		ballfree.is_free = 0;
		updates.push_back(ballfree.Encode());

		DoDestiny_SetBallMass ballmass;
		ballmass.entityID = m_self->GetID();
		ballmass.mass = 10000000000;
		updates.push_back(ballmass.Encode());

		DoDestiny_SetMaxSpeed maxspeed2;
		maxspeed2.entityID = m_self->GetID();
		maxspeed2.speedValue = 1000000;
		updates.push_back(maxspeed2.Encode());

		SendDestinyUpdate(updates, false); //consumed
	}
}

// settings for ship attributes
void DestinyManager::SetShipVariables(InventoryItemRef ship)
{
	m_radius = ship->GetAttribute(AttrRadius).get_float();

	/* AttrMass = 4,	(largest mass = Leviathan(3764) @ 2,430,000,000kg)
	 * AttrMassLimit = 622,
	 * AttrMassAddition = 796,
	 * AttrMassMultiplier = 1471,
	 */
	double shipMass = ship->GetAttribute(AttrMass).get_float();

	//  check for rigs that modify mass here
	m_mass = shipMass;
	m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)

	//  check for warp strength modifiers
	m_warpStrength = 1;

	/*  capacitor shit
	 *    AttrCapacitorCharge = 18,
	 *     AttrRechargeRate = 55,		// time to charge from 0 to full, in milliSeconds
	 *    AttrCapacitorBonus = 67,
	 *    AttrCapacitorRechargeRateMultiplier = 144,
	 *    AttrCapacitorCapacityMultiplier = 147,
	 *    AttrCapRechargeBonus = 314,
	 *     AttrRechargeratebonus = 338,
	 *    AttrCapacitorCapacity = 482,
	 *    AttrCapacitorCapacityBonus = 1079,
	 *    AttrDisplayCapacitorCapacityBonus = 1094,
	 *    AttrCapacitorCapacityMultiplierSystem = 1499,
	 *   AttrRechargeRateMultiplier = 1500,
	 */

	double capCapacity = ship->GetDefaultAttribute(AttrCapacitorCapacity).get_float();	// default value from db
	double capChargeRate = ship->GetDefaultAttribute(AttrRechargeRate).get_float();	// default value from db

	if( m_self->IsClient() ) {
		Character *p_Char = m_self->CastToClient()->GetChar().get();
		capCapacity *=  1 + ( 0.05 * (p_Char->GetSkillLevel(skillEnergyManagement, true)));		// 5% increase
		capChargeRate *=  1 - ( 0.05 * (p_Char->GetSkillLevel(skillEnergySystemsOperation, true)));		//5% decrease
	}

	// add checks for modules, rigs and implants that affect cap capacity here

	// add checks for modules, rigs and implants that affect cap charge rate here

	// set capacitor to full, and apply recharge rate.
	ship->SetAttribute(AttrCapacitorCharge, capCapacity, true);
	ship->SetAttribute(AttrRechargeRate, capChargeRate, true);
}

//  called from Client::BoardShip(), Client::UpdateLocation(), Undock(), NPC::NPC()
void DestinyManager::SetShipCapabilities(InventoryItemRef ship)
{
	/* this now sets variables needed for correct warp math.
	 * noted modifiers to look into later, after everything is working
	 * skill bonuses to agility, velocity, and cap are now implemented.
	 */

	SetShipVariables(ship);

	double warpCapNeed = ship->GetDefaultAttribute(AttrWarpCapacitorNeed).get_float();

	double adjInertiaModifier = ship->GetDefaultAttribute(AttrAgility).get_float();
	float adjShipMaxVelocity = ship->GetDefaultAttribute(AttrMaxVelocity).get_float();
	float warpSpeedMultiplier = 1.0f;
	float shipBaseWarpSpeed = 1.0f;

	// skill bonuses to agility and velocity and warpCapacitorNeed
	/*
	 *    Advanced Spaceship Command: 5% agility bonus s per level on ships requiring this skill
	 *    Capital Ships: 5% agility bonus s per level on ships requiring this skill
	 *    Spaceship Command: 2% agility for all ships per level
	 *    Evasive Maneuvering: 5% agility bonus for all ships per level
	 *    Skirmish Warfare: 2% agility to fleet per skill level
	 *    Skirmish warfare Mindlink (implant): 15% agility to fleet, replaces Skirmish warfare skill
	 *    Warp Drive Operation (skill) (only listed because it affects how far you can warp) :–)
	 */
	if( m_self->IsClient() ) {
		Character *p_Char = m_self->CastToClient()->GetChar().get();
		bool IsCapShip = false;
		// add check here for capital ships
		adjInertiaModifier -= ( adjInertiaModifier * p_Char->GetAgilitySkills(IsCapShip) );
		adjShipMaxVelocity += ( adjShipMaxVelocity * ( 5 * (p_Char->GetSkillLevel(skillNavigation, true)))/100);
		shipBaseWarpSpeed = ship->GetAttribute(AttrBaseWarpSpeed).get_float();
		warpSpeedMultiplier = ship->GetAttribute(AttrWarpSpeedMultiplier).get_float();
		warpCapNeed *=  1 + ( 0.1 * (p_Char->GetSkillLevel(skillWarpDriveOperation, true)));
		// TODO check for implants  AttrWarpCapacitorNeedBonus(319)
	} else {
		shipBaseWarpSpeed = m_self->Item()->GetAttribute(AttrBaseWarpSpeed).get_float();
		warpSpeedMultiplier = m_self->Item()->GetAttribute(AttrWarpSpeedMultiplier).get_float();
	}

	// TODO add module and rig bounses to inertia, agility, velocity here

	m_maxShipVelocity = adjShipMaxVelocity;
	m_shipInertiaModifier = adjInertiaModifier;
	ship->SetAttribute(AttrAgility, adjInertiaModifier);
	ship->SetAttribute(AttrMaxVelocity, adjShipMaxVelocity);

	/*  per https://forums.eveonline.com/default.aspx?g=posts&m=3912843   post#103
	 *
	 * Ships will exit warp mode when their warping speed drops below
	 * 50% of sub-warp max speed, or 100m/s, whichever is the lower.
	 */
	m_speedToLeaveWarp = m_maxShipVelocity / 2;
	if(m_speedToLeaveWarp > 100) m_speedToLeaveWarp = 100.0f;

	// TODO add module and rig bonuses to warp cap here

	m_warpCapacitorNeed = warpCapNeed;
	ship->SetAttribute(AttrWarpCapacitorNeed, warpCapNeed);

	/* The product of Mass and the Inertia Modifier gives the ship's agility
	 * Agility = Mass x Inertia Modifier
	 *   NOTE agility is an internal-use variable, and is not sent to the client.
	 */
	m_shipAgility = m_massMKg * m_shipInertiaModifier;

	/*   look into these, too...
	 * AttrWarpSBonus(624) [rigs and implants]
	 * AttrWarpFactor(21) [all are 0]
	 * AttrWarpInhibitor(29) [default is null]
	 */

	//TODO  add module and rig modifiers to warp speed here
	m_shipWarpSpeed = ( warpSpeedMultiplier * shipBaseWarpSpeed );

	_log(PHYSICS__TRACE, "DestinyManager::SetShipCapabilities - Entity %s(%u) has set ship attributes:"
	     "Radius=%f, Mass=%f, maxVelocity=%f, agility=%f, inertia=%f, warpSpeed=%f",
		 m_self->GetName(), m_self->GetID(), m_radius, m_mass, m_maxShipVelocity, m_shipAgility,
		 m_shipInertiaModifier, m_shipWarpSpeed);

    SetSpeedFraction(0.0f);
}

void DestinyManager::SetPosition(const GPoint &pt, bool update, bool isWarping, bool isPostWarp) {
    //m_body->setPosition( pt );
    m_position = pt;
    _log(PHYSICS__TRACEPOS, "DestinyManager::SetPosition - Entity %s(%u) set its position to (%.6f, %.6f, %.6f)",
        m_self->GetName(), m_self->GetID(), m_position.x, m_position.y, m_position.z );

    if(update) {
        DoDestiny_SetBallPosition du;
        du.entityID = m_self->GetID();
        du.x = pt.x;
        du.y = pt.y;
        du.z = pt.z;

        PyTuple *up = du.Encode();
		SendDestinyUpdate(&up);    //consumed
		PySafeDecRef(up);
    }
    m_system->bubbles.UpdateBubble(m_self, update, isWarping, isPostWarp);
}

void DestinyManager::AlignTo(SystemEntity *ent, bool update) {
    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    const GPoint position = ent->GetPosition();

	State = DSTBALL_GOTO;
	m_targetPoint = position;

	SetSpeedFraction(1.0f);

    DoDestiny_CmdAlignTo au;
    au.entityID = ent->GetID();

	PyTuple *up = au.Encode();
	SendDestinyUpdate(&up);    //consumed
	PySafeDecRef(up);

    _log(PHYSICS__TRACEPOS, "DestinyManager::AlignTo() - '%s' aligning to SystemEntity '%s'(%u) @ (%f,%f,%f)",
                m_self->GetName(), ent->GetName(), ent->GetID(), position.x, position.y, position.z );
}

void DestinyManager::GotoDirection(const GPoint &direction, bool update) {
    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    State = DSTBALL_GOTO;
	m_targetPoint = m_position + (direction * 1.0e16);

    SetSpeedFraction(1.0f);

    DoDestiny_CmdGotoDirection du;
    du.entityID = m_self->GetID();
    du.x = direction.x;
    du.y = direction.y;
    du.z = direction.z;

    PyTuple *up = du.Encode();
	SendDestinyUpdate(&up);    //consumed
	PySafeDecRef(up);

    _log(PHYSICS__TRACEPOS, "DestinyManager::GotoDirection() - SystemEntity '%s' vectoring to (%f,%f,%f) at velocity %f",
                m_self->GetName(), direction.x, direction.y, direction.z, m_maxVelocity );
}

PyResult DestinyManager::AttemptDockOperation()
{
    Client *who = m_self->CastToClient();
    SystemManager *sm = m_self->System();
    uint32 stationID = who->GetDockStationID();
    SystemEntity *station = sm->get(stationID);

    if(station == NULL) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", who->GetName(), stationID);
		who->SendErrorMsg("Station Not Found, Docking Aborted.");
        return NULL;
	}

	GPoint stationOrigin = station->GetPosition();
    GPoint position = who->GetPosition();

    GVector direction(stationOrigin, position);
    double rangeToStationPerimiter = (direction.length() - station->GetRadius());

    // WARNING: DO NOT uncomment the following line as it for some reason causes HEAP corruption to occur on auto-docking
    //if( !(who->GetPendingDockOperation()) )
        //GotoDirection( direction, true );   // Turn ship and move toward docking point

    // Verify range to station is within docking perimeter of 1500 meters:
    //if( (rangeToStation - station->GetRadius()) > 1500 )
	sLog.Warning("DestinyManager::AttemptDockOperation()", "rangeToStationPerimiter is %f", rangeToStationPerimiter);
	if( rangeToStationPerimiter > 1500.0 ) {
		who->SendErrorMsg("Outside Docking Perimiter.  Please Move Closer.");
    // This packet has to be returned to the client when outside the docking perimeter
	//   TODO:  make a packet response for this, named:  OnDockingApproach
    // Packet::ErrorResponse
    /* -- In the marshalstream:
    ObjectEx:
    Header:
      Tuple: 3 elements
        [ 0] Token: 'ccp_exceptions.UserError'
        [ 1] Tuple: 1 elements
        [ 1]   [ 0] String: 'DockingApproach'
        [ 2] Dictionary: 2 entries
        [ 2]   [ 0] Key: String: 'msg'
        [ 2]   [ 0] Value: String: 'DockingApproach'
        [ 2]   [ 1] Key: String: 'dict'
        [ 2]   [ 1] Value: (None)
    */
    // TODO: On docking failure: Send DoDestinyUpdates for GotoPoint for Station Dockpoint, SetBallMassive, SetBallMass

        //GotoDirection( direction, true );   // Turn ship and move toward docking point
        // WARNING: DO NOT uncomment the following line as it for some reason causes HEAP corruption to occur on auto-docking
        //who->SetPendingDockOperation( true );   // Set client object into state that a Docking operation is pending
        //return NULL;                            // so that DestinyManager can track when it needs to auto-dock
    }
    // Stop Destiny from trying to move ship because it is about to dock and if Destiny keeps trying to move the ship after
    // some point in the dock procedure, the server crashes hard.
    Stop(false);

	//GPoint stationDockPoint = static_cast< StationEntity* >( station )->GetStationObject()->GetStationType()->dockEntry(); //station->GetPosition();
    // When docking, Set X,Y,Z to origin so that when changing ships in stations, they don't appear outside:
	who->MoveToLocation( stationID, m_position );		// if stationDockPoint does not help, try m_position

    who->SetPendingDockOperation( false );

    //clear all targets
    who->targets.ClearAllTargets();

	//tell ship it's docking.  this only offlines all modules right now.
	who->GetShip()->Dock();

    //Check if player is in pod, in which case they get a rookie ship for free  (with 2 civilian modules and 1 trit)
    if( who->GetShip()->typeID() == itemTypeCapsule )
	    who->SpawnNewRookieShip();

    // Save all Character, Ship, Module data to Database on dock:
	//  really hit the db everytime a ship docks??
    //who->SaveAllToDatabase();

    // Docking was accepted, so send the OnDockingAccepted packet:
	 DoDestiny_OnDockingAccepted oda;
	 oda.ship_x = who->GetPosition().x;
	 oda.ship_y = who->GetPosition().y;
	 oda.ship_z = who->GetPosition().z;
	 oda.station_x = stationOrigin.x;
	 oda.station_y = stationOrigin.y;
	 oda.station_z = stationOrigin.z;
	 oda.stationID = stationID;

	return NULL;
}

void DestinyManager::Undock(GPoint dockPosition, GPoint direction) {
	//ship should never be NULL.
	SetShipCapabilities( m_self->CastToClient()->GetShip() );
	//set position at undock point of station
	SetPosition(dockPosition);
	//set direction and ship movement
	GotoDirection(direction);
	//set speed (squeezed out from dock is faster than maxSubWarpSpeed)
	m_velocity = direction * (m_maxVelocity * 1.3);
	//tell client it's moving from station's dock ramp
	SendBallInfoOnUndock(direction);
	//_Move();
}

void DestinyManager::Cloak()
{
	sLog.Warning("DestinyManager::Cloak()", "TODO - check for warp-safe-ness, and turn on any cloaking device module fitted");
	m_cloaked = true;
	SendCloakShip(true);
	m_self->Bubble()->RemoveExclusive(m_self,true);
}

void DestinyManager::UnCloak()
{
	sLog.Warning("DestinyManager::UnCloak()", "TODO - check for warp-safe-ness, and turn off any cloaking device module fitted");
	m_cloaked = false;
	SendUncloakShip();
	m_self->Bubble()->AddExclusive(m_self,true);
}

void DestinyManager::WarpTo(const GPoint &where, double distance) {
	if(m_warpState != NULL) {
		delete m_warpState;
		m_warpState = NULL;
	}

	if(m_self->IsClient()) {
		//Clear any pending docking operation since the user initiated warp:
		m_self->CastToClient()->SetPendingDockOperation( false );
	}

	// check for scramble
	//if(m_warpStrength)

	State = DSTBALL_WARP;
	m_targetEntity.first = 0;
	m_targetEntity.second = NULL;
	m_targetPoint = where;
	m_stopDistance = static_cast<int32>(distance);

	SetSpeedFraction(1.0f);

	sLog.Warning("DestinyManager::WarpTo()", "m_targetPoint: %f,%f,%f  m_stopDistance: %u",
				 m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_stopDistance);
}

void DestinyManager::SendJumpOut(uint32 stargateID) const {
    std::vector<PyTuple *> updates;

    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    DoDestiny_CmdStop du;
    du.entityID = m_self->GetID();
    updates.push_back(du.Encode());

    //send a warping special effects update...
    DoDestiny_OnSpecialFX10 effect;
    effect.entityID = m_self->GetID();
    effect.targetID = stargateID;
    effect.effect_type = "effects.JumpOut";
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJumpIn() const {
    //hacked for simplicity... I dont like jumping in until we have
    //jumping in general much better quantified.

    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.JumpDriveIn";
    effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    DoDestiny_CmdSetSpeedFraction ssf;
    ssf.entityID = m_self->GetID();
    ssf.fraction = 0.0;
    updates.push_back(ssf.Encode());

    DoDestiny_SetBallVelocity sbv;
    sbv.entityID = m_self->GetID();
    sbv.x = 0.0;
    sbv.y = 0.0;
    sbv.z = 0.0;
    updates.push_back(sbv.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 locationID) const {
    std::vector<PyTuple *> updates;

    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    DoDestiny_CmdStop du;
    du.entityID = m_self->GetID();
    updates.push_back(du.Encode());

    //send a warping special effects update...
    DoDestiny_OnSpecialFX10 effect;
    effect.entityID = m_self->GetID();
    effect.targetID = locationID;
	effect.effect_type = "effects.JumpDriveOut";
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
    //hacked for simplicity... I dont like jumping in until we have
    //jumping in general much better quantified.

    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.JumpDriveIn";
    effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    DoDestiny_CmdSetSpeedFraction ssf;
    ssf.entityID = m_self->GetID();
    ssf.fraction = 0.0;
    updates.push_back(ssf.Encode());

    DoDestiny_SetBallVelocity sbv;
    sbv.entityID = m_self->GetID();
    sbv.x = 0.0;
    sbv.y = 0.0;
    sbv.z = 0.0;
    updates.push_back(sbv.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendTerminalExplosion() const {
    //Clear any pending docking operation since the user's ship exploded:
	if( m_self->IsClient() )
        m_self->CastToClient()->SetPendingDockOperation( false );

	SystemBubble *b = m_self->Bubble();
    //send an explosion special effects update...
    DoDestiny_TerminalExplosion du;
    du.shipID =  m_self->CastToClient()->GetShipID();	//m_self->GetID();
    du.bubbleID = b->GetBubbleID();  					//  add check for bubbleID here.
    du.ballIsGlobal = false; 							//  is this ball Global?  boolean
    PyTuple *up = du.Encode();

	SendDestinyUpdate(&up);
	PySafeDecRef(up);
}

//this should only be available on gates.
//   not working correctly for gateActivate   -allan 6Aug14
void DestinyManager::SendGateActivity(uint32 stargateID) const {
    DoDestiny_OnSpecialFX10 du;
    du.entityID = stargateID;
    du.effect_type = "effects.GateActivity";
    du.isOffensive = 0;
    du.start = 1;
    du.active = 0;

    PyTuple *up = du.Encode();
	SendDestinyUpdate(&up);    //consumed
	PySafeDecRef(up);
}

void DestinyManager::SendSetState(const SystemBubble *b) const {
    DoDestiny_SetState ss;

    ss.stamp = GetStamp();
    ss.ego = m_self->GetID();
    m_system->MakeSetState(b, ss);

    PyTuple *up = ss.Encode();
	SendDestinyUpdate(&up);    //consumed
	PySafeDecRef(up);
}

void DestinyManager::SendBallInfoOnUndock(GPoint direction) const {
	std::vector<PyTuple *> updates;

	DoDestiny_SetBallMassive sbmassive;
	sbmassive.entityID = m_self->GetID();
	sbmassive.is_massive = true;
	updates.push_back(sbmassive.Encode());

	DoDestiny_SetBallMass sbmass;
	sbmass.entityID = m_self->GetID();
	sbmass.mass = m_mass;
	updates.push_back(sbmass.Encode());

	DoDestiny_SetBallRadius sbradius;
	sbradius.entityID = m_self->GetID();
	sbradius.radius = m_self->GetRadius();
	updates.push_back(sbradius.Encode());

	DoDestiny_CmdGotoDirection gtd;
	gtd.entityID = m_self->GetID();
	gtd.x = direction.x;
	gtd.y = direction.y;
	gtd.z = direction.z;
	updates.push_back(gtd.Encode());

	DoDestiny_SetBallVelocity sbvelocity;
	sbvelocity.entityID = m_self->GetID();
	sbvelocity.x = m_velocity.x;
	sbvelocity.y = m_velocity.y;
	sbvelocity.z = m_velocity.z;
	updates.push_back(sbvelocity.Encode());

	SendDestinyUpdate(updates, false);

	m_system->bubbles.UpdateBubble(m_self);
}

void DestinyManager::SendBoardShip(const ShipRef boardShipRef) const {
	DoDestiny_SetBallInteractive sbic;
	sbic.entityID = boardShipRef->itemID();
	sbic.interactive = 1;
	PyTuple * up = sbic.Encode();
    SendDestinyUpdate(&up);
	PySafeDecRef(up);
}

void DestinyManager::SendEjectShip(const ShipRef capsuleRef, const ShipRef oldShipRef) const {
    //DoDestinyUpdate
        //addball for only the capsule.

    //DoDestinyUpdate
        //AddBalls:
            //old ship (STOP, w/ship and mass)
            //capsule (STOP, w/ship and mass)
            //and our newbie ship (STOP, w/ship and mass)
        //a bunch of OnModuleAttributeChange and OnGodmaShipEffect for old ship and capsule

    //DoDestinyUpdate
        //effects.Jettison
        //SetBallInteractive (old ship, false)
        //OnSlimItemChange (old ship, SlimItem)
        //SetBallInteractive (capsule, true)
        //OnSlimItemChange (capsule, SlimItem)
        //SetMaxSpeed (capsule)

    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 du_jettisonEffect;
    du_jettisonEffect.entityID = m_self->GetID();
    du_jettisonEffect.effect_type = "effects.Jettison";
    du_jettisonEffect.isOffensive = 0;
    du_jettisonEffect.start = 1;
    du_jettisonEffect.active = 0;
    updates.push_back(du_jettisonEffect.Encode());

    DoDestiny_SetBallInteractive du_setBallInteractiveCapsule;
    du_setBallInteractiveCapsule.entityID = capsuleRef->itemID();
    du_setBallInteractiveCapsule.interactive = 1;
    updates.push_back(du_setBallInteractiveCapsule.Encode());

    DoDestiny_SlimItem du_slimItemCapsule;
//    du_slimItemCapsule.allianceID = m_self->CastToClient()->GetAllianceID();
//    du_slimItemCapsule.bounty = 0.0;
//    du_slimItemCapsule.charID = 0;
//    du_slimItemCapsule.color = 0;
//    du_slimItemCapsule.corpID = 0;
//    du_slimItemCapsule.groupID = 0;
//    du_slimItemCapsule.itemID = 0;
//    du_slimItemCapsule.modules = ;
//    du_slimItemCapsule.ownerID = 0;
//    du_slimItemCapsule.securityStatus = 0.0;
//    du_slimItemCapsule.typeID = 0;
//    du_slimItemCapsule.warFactionID = 0;

    DoDestiny_SetBallInteractive du_setBallInteractiveOldShip;
    du_setBallInteractiveOldShip.entityID = oldShipRef->itemID();
    du_setBallInteractiveOldShip.interactive = 0;
    updates.push_back(du_setBallInteractiveOldShip.Encode());

    DoDestiny_SlimItem du_slimItemOldShip;
    //du_slimItemOldShip.

    // Set Capsule's max velocity:
    DoDestiny_CmdSetMaxSpeed du_setMaxSpeed;
    du_setMaxSpeed.entityID = capsuleRef->itemID();
    du_setMaxSpeed.speed = capsuleRef->GetAttribute(AttrMaxVelocity).get_float();
    updates.push_back(du_setMaxSpeed.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJettisonCargo(const InventoryItemRef itemRef) const {
/*    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.JumpDriveIn";
    effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    DoDestiny

    SendDestinyUpdate(updates, false);
*/}

void DestinyManager::SendAnchorDrop(const InventoryItemRef itemRef) const {
    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.AnchorDrop";
    effect.entityID = itemRef->itemID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendAnchorLift(const InventoryItemRef itemRef) const {
    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.AnchorLift";
    effect.entityID = itemRef->itemID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendCloakShip(const bool IsWarpSafe) const {
    std::vector<PyTuple *> updates;

      DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.Cloak";
	effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendUncloakShip() const {
    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.Uncloak";
	effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendSpecialEffect(const ShipRef shipRef, uint32 moduleID, uint32 moduleTypeID,
    uint32 targetID, uint32 chargeTypeID, std::string effectString, bool isOffensive, bool start, bool isActive, double duration, uint32 repeat) const
{
    std::vector<PyTuple *> updates;
	std::vector<int32, std::allocator<int32> > area;

    DoDestiny_OnSpecialFX13 effect;
    effect.entityID = shipRef->itemID();
    effect.moduleID = moduleID;
    effect.moduleTypeID = moduleTypeID;
    effect.targetID = targetID;
    effect.otherTypeID = chargeTypeID;
    effect.area = area;
    effect.effect_type = effectString;
    effect.isOffensive = isOffensive;
    effect.start = start;
    effect.active = isActive;
    effect.duration_ms = duration;
	effect.repeat = repeat;  //new PyInt(repeat);
	effect.startTime = Win32TimeNow();
    updates.push_back(effect.Encode());
    SendDestinyUpdate(updates, false);

	//PyTuple* up = effect.Encode();
    //SendDestinyUpdate( &up );    //consumed
    //PySafeDecRef( up );
}

void DestinyManager::SendSpecialEffect10(uint32 entityID, const ShipRef shipRef, uint32 targetID, std::string effectString, bool isOffensive, bool start, bool isActive) const
{
    std::vector<PyTuple *> updates;
	std::vector<int32, std::allocator<int32> > area;

    DoDestiny_OnSpecialFX10 effect;
    effect.entityID = entityID;
	effect.targetID = targetID;
    effect.effect_type = effectString;
    effect.area = area;
	effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

