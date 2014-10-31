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
const double TIC_DURATION_IN_SECONDS = 1.0;    //straight from client. Do not change.

static const double DESTINY_UPDATE_RANGE = 1.0e8;    //totally made up. a more complex spatial partitioning system is needed.
static const double FOLLOW_BAND_WIDTH = 100.0f;    //totally made up

static const double EULERS_NUMBER = 2.7182818284590452353;

uint32 DestinyManager::m_stamp(40000);    //completely arbitrary starting point.
Timer DestinyManager::m_stampTimer(static_cast<int32>(TIC_DURATION_IN_SECONDS * 1000), true);    //accurate timing is essential.

DestinyManager::DestinyManager(SystemEntity *self, SystemManager *system)
: m_self(self),
  m_system(system),
//  m_body(NULL),
//  m_shape(NULL),
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
  m_position(0, 0, 0),    //generally right in the middle of the star!
//  m_direction(0, 0, 0),
//  m_velocity(0),
  m_velocity(0, 0, 0),
  m_acceleration(0.0),
  m_maxVelocity(1.0f),
  m_accelerationFactor(0.0),
  m_velocityAdjuster(0.0),
  m_avgWarpAcceleration(1.0),
  State(DSTBALL_STOP),
  m_userSpeedFraction(0.0f),
  m_activeSpeedFraction(0.0f),
  m_targetPoint(0, 0, 0),
  m_targetDistance(0.0),
  m_radius(1.0),
  m_mass(1.0),
  m_maxShipVelocity(1.0),
  m_shipAgility(1.0),
  m_shipInertiaModifier(1.0),
  m_warpCapacitorNeed(1.0),
  m_warpAccelTime(1.0),
  m_warpState(NULL)
{
    //do not touch m_self here, it may not be fully constructed.
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;

	m_cloaked = false;

    m_warpDecelerateFactor = 0.75;
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
        _log( DESTINY__TRACE, "DestinyManager::SendDestinyUpdate - [%u] Broadcasting destiny update", GetStamp() );
		m_self->Bubble()->BubblecastDestinyUpdate( up, "destiny" );
	}
}

void DestinyManager::SendDestinyEvent(PyTuple **ev, bool self_only) const {
    if( self_only )
	    SendSelfDestinyEvent( ev );
	else {
        _log( DESTINY__TRACE, "DestinyManager::SendDestinyEvent - [%u] Broadcasting destiny event", GetStamp() );
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
	    _log( DESTINY__TRACE, "DestinyManager::SendDestinyUpdate - [%u] Broadcasting destiny update (%lu)", GetStamp(), updates.size() );
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
        _log( DESTINY__TRACE, "DestinyManager::SendDestinyEvent - [%u] Broadcasting destiny event (%lu)", GetStamp(), events.size() );
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

void DestinyManager::_UpdateDerrived() {
    m_maxVelocity = m_maxShipVelocity * m_activeSpeedFraction;
    m_velocityAdjuster = exp(- (SPACE_FRICTION * TIC_DURATION_IN_SECONDS) / (m_mass * m_shipAgility));
    m_accelerationFactor = (SPACE_FRICTION * m_maxVelocity) / (m_mass * m_shipAgility);

    _log(PHYSICS__TRACE, "DestinyManager::_UpdateDerrived - Entity %s(%u) has derrived: maxVelocity=%f, velocityAdjuster=%f, accelerationFactor=%f",
        m_self->GetName(), m_self->GetID(), m_maxVelocity, m_velocityAdjuster, m_accelerationFactor);
}

void DestinyManager::SetSpeedFraction(double fraction, bool update) {
    m_userSpeedFraction = fraction;
    m_activeSpeedFraction = fraction;
    _UpdateDerrived();

    if(update) {
        DoDestiny_CmdSetSpeedFraction du;
        du.entityID = m_self->GetID();
        du.fraction = fraction;

        PyTuple *tmp = du.Encode();
        SendDestinyUpdate(&tmp);    //consumed
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
        if(m_velocity.isNotZero()) {
            //no freakin clue what this is about, maybe to dampen vertical movement.
            //you can see result of this just by going directly up or down in space and
            //then stopping ... deaccelerate is faster than normal.
            m_velocity.y *= 0.93/1.07;

            _Move();
        }
        break;

    case DSTBALL_WARP: {
        //warp is a 3 stage thing... first you align and get up to speed,
        //then you cruise, and then you stop.

        if(m_warpState != NULL) {
            //warp is already started!
            _Warp();
            break;
        }

        //first check the angle between our warp vector and our
        //velocity vector.
        GVector warp_vector(m_position, m_targetPoint);
        warp_vector.normalize();
		double m_distance = warp_vector.length();
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
        double needed_velocity2 = m_maxVelocity*0.75;
        needed_velocity2 *= needed_velocity2;
        if(m_velocity.lengthSquared() < needed_velocity2) {
            //execute normal GOTO code.
            _Move();
            break;
        }

        //ok, we are starting the warp!
        _InitWarp();
        _Warp();

        } break;

    case DSTBALL_ORBIT:
        _Orbit();
        break;

    case DSTBALL_MISSILE:
    case DSTBALL_MUSHROOM:
    case DSTBALL_BOID:
    case DSTBALL_TROLL:
    case DSTBALL_MINIBALL:		// used for missiles
    case DSTBALL_FIELD:
    case DSTBALL_FORMATION:
    case DSTBALL_RIGID:
        break;
    //no default on purpose
    }
}

//Global Actions:
void DestinyManager::Stop(bool update) {
    if( m_self->IsClient() ) {
	    Client *c = m_self->CastToClient();
	    c->m_autoPilot = false;

    //Clear any pending docking operation since the user stopped ship movement:
		c->SetPendingDockOperation( false );

    // THIS IS A HACK AS WE DONT KNOW WHY THE CLIENT CALLS STOP AT UNDOCK
		if( c->GetJustUndocking() )
		{
			// Client just undocked from a station so DO NOT STOP:
			c->SetJustUndocking( false );
			GPoint dest;
			c->GetUndockAlignToPoint( dest );
			dest.normalize();
			GotoDirection( dest, true );
			SetSpeedFraction( 1.0, true );
			_UpdateDerrived();
			return;
		}
	}

	if(State == DSTBALL_STOP)
		return;

	if(State == DSTBALL_WARP && m_warpState != NULL) {
		//warp aborted!
		delete m_warpState;
		m_warpState = NULL;
		//put ourself back into a bubble.
		m_system->bubbles.UpdateBubble(m_self);
	}

	m_targetEntity.first = 0;
	m_targetEntity.second = NULL;
	//redude velocity to 0, applying reverse thrust until we get there.
//    m_targetPoint = m_position - (m_velocity * 1.0e6);    //opposite direction
    m_userSpeedFraction = 0.0f;
	m_activeSpeedFraction = 0.0f;
	_UpdateDerrived();

	State = DSTBALL_STOP;

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
    m_velocity = GVector(0, 0, 0);
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    _UpdateDerrived();

    //Clear any pending docking operation since the user halted ship movement:
	//  remove autopilot checks also
	if( m_self->IsClient() ) {
		m_self->CastToClient()->SetPendingDockOperation( false );
	    //autopilot shit
	    m_self->CastToClient()->m_autoPilot = false;
	}

    State = DSTBALL_STOP;

    //ensure that our bubble is correct.
    m_system->bubbles.UpdateBubble(m_self);

    if(update) {
        std::vector<PyTuple *> updates;

        {
        DoDestiny_CmdSetSpeedFraction du;
        du.entityID = m_self->GetID();
        du.fraction = 0.0f;

        updates.push_back(du.Encode());
        }
        {
        DoDestiny_SetBallVelocity du;
        du.entityID = m_self->GetID();
        du.x = 0.0;
        du.y = 0.0;
        du.z = 0.0;

        updates.push_back(du.Encode());
        }

        SendDestinyUpdate(updates, false);
    }
}

void DestinyManager::_Follow() {

    /*
    this was my idea of how it should be done, which doesn't agree with
    the client.

    GVector targetDirection = GVector(m_position, goal);
    double distance = targetDirection.normalize();
    _log(PHYSICS__TRACEPOS, "Entity %u: Moving towards (%.1f, %.1f, %.1f) which is %f away.",
        m_self->GetID(),
        goal.x, goal.y, goal.z,
        distance);
    distance -= FOLLOW_BAND_WIDTH;

    if(distance <= m_targetDistance) {
        //there has to be some band of distance that causes us not to move.
        if((m_targetDistance-distance) < (FOLLOW_BAND_WIDTH+FOLLOW_BAND_WIDTH)) {
            //close enough.
            _log(PHYSICS__TRACEPOS, " Entity %u is %f/%f away from followed entity. Close enough. Not moving.", m_self->GetID());
            m_activeSpeedFraction = 0.0f;
            _UpdateDerrived();
            return;
        }
        //swap around the direction, see what that does...
        targetDirection *= -1.0f;

        _log(PHYSICS__TRACEPOS, " Entity %u is %f/%f away from followed entity. Should probably back off...", m_self->GetID());
        return;
    }

    targetDirection /= distance;    //normalize the direction vector.

    //set our goal point to be m_targetDistance away from targetPos
    //directly along the straight-line vector between us.
    m_targetPoint = goal - (targetDirection * m_targetDistance);

    if(m_activeSpeedFraction != m_userSpeedFraction) {
        //we stopped temporarily, go again.
        m_activeSpeedFraction = m_userSpeedFraction;
        _UpdateDerrived();
    }*/

    // First check to see if our target has somehow been removed from space
    // _OR_ the player has left the target ship..
    // if so, then we need to call DestinyManager::Stop() to stop the ship from
    // following a non-existent space object or a pilot-less ship,
    // otherwise, proceed to continue following:
    Client * targetClient = NULL;
    if( m_system->get( m_targetEntity.first ) == NULL )
    {
        // Our target was removed, so STOP
        SetSpeedFraction( 0.0f, true );
        Stop( true );
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
                SetSpeedFraction( 0.0f, true );
                Stop( true );
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

    _Move();
}

void DestinyManager::_Move() {

    //CalcAcceleration:
    GVector vector_to_goal(m_position, m_targetPoint); //m
    Ga::GaFloat distance_to_goal2 = vector_to_goal.lengthSquared(); //m^2
//    Ga::GaFloat delta = ((tic_duration_in_seconds * speed_fraction * maxVelocity) + radius); //m
//    Ga::GaFloat accel = (space_friction*speed_fraction*maxVelocity) / (agility * mass); //fric*m/(s*agi*kg)
/*  if(delta*delta >= distance_to_goal2) {
        // we are going to reach our goal
        Ga::GaFloat tmp = distance_to_goal2/(delta*delta);
        accel *= tmp*tmp;
    } else {*/
        vector_to_goal /= sqrt(distance_to_goal2); //normalize  yeilds unitless
    //}
    //_log(PHYSICS__TRACEPOS, "DestinyManager::_Move - Entity %u: Accel Magnitude = %.13f",m_self->GetID(), m_accelerationFactor);
    GVector calc_acceleration = vector_to_goal * m_accelerationFactor;    //fric*m/(s*agi*kg) = m/s^2

    // Check to see if we have a pending docking operation and attempt to dock if so:
    if( m_self->IsClient() && m_self->CastToClient()->GetPendingDockOperation() )
        AttemptDockOperation();

    _MoveAccel(calc_acceleration);
}

void DestinyManager::_MoveAccel(const GVector &calc_acceleration) {
  /*
    _log(PHYSICS__TRACEPOS, "DestinyManager::_MoveAccel - Entity %u: Goal Point (%.1f, %.1f, %.1f) with accel (%f, %f, %f)",
        m_self->GetID(),
        m_targetPoint.x, m_targetPoint.y, m_targetPoint.z,
        calc_acceleration.x, calc_acceleration.y, calc_acceleration.z);
		*/

    double mass_agility_friction = m_mass * m_shipAgility / SPACE_FRICTION;

    GVector max_velocity = calc_acceleration * mass_agility_friction;


    // position:

    m_position +=
        max_velocity * TIC_DURATION_IN_SECONDS
        - (max_velocity - m_velocity) * (1 - m_velocityAdjuster) * mass_agility_friction;


    //now on to velocity:



    //velocity_to_m_us:
    m_velocity =
        max_velocity - (max_velocity - m_velocity) * m_velocityAdjuster;



#if 0
    Ga::GaVec3 start_acceleration = CalcAcceleration();
    global Ga::GaFloat space_friction = 1.0e6;
    Ga::GaFloat mass_times_agility = mass * agility;
    Ga::GaFloat tic_duration_in_seconds = 1.0;
    Ga::GaFloat velocity_adjuster = exp(- (space_friction * tic_duration_in_seconds) / mass_times_agility);

    Ga::GaVec3 start_position = m_body->getPosition();
    Ga::GaVec3 start_velocity = m_body->getLinearVelocity();

    Ga::GaFloat friction_squared = space_friction*space_friction;
    Ga::GaVec3 pos_times_f2 = start_position * friction_squared;

    Ga::GaVec3 velocity_reduced = start_velocity * velocity_adjuster;

    Ga::GaVec3 acceleration = start_acceleration * tic_duration_in_seconds;
    acceleration += start_velocity;
    velocity_reduced -= acceleration;

    //next_crap:
    acceleration = velocity_reduced * space_friction;

    //next_crap2:
    Ga::GaVec3 acceleration_ma = start_acceleration * mass_times_agility;
    acceleration_remainder = acceleration_ma * (velocity_adjuster - 1);

    //apply_acceleration:
    Ga::GaVec3 accel_sum = acceleration_remainder + acceleration;
    accel_sum *= mass_times_agility;

    //apply_velocity:
    Ga::GaVec3 position_adjusted = accel_sum + pos_times_f2;
    //reduce_big_to_si:
    position_adjusted /= friction_squared;
    //-- this is the resulting position --

    //velocity_to_m_us:
    Ga::GaVec3 velocity_friction = start_velocity * space_friction;

    //acceleration_times_arg_c re-calculates acceleration_ma

    //next_crap4:
    Ga::GaVec3 remaining_accel = acceleration_ma - velocity_friction;

    //next_crap5:
    Ga::GaVec3 remaining_accel_adjusted = remaining_accel * velocity_adjuster;

    //next_crap6:
    Ga::GaVec3 velocity_adjusted = acceleration_ma - remaining_accel_adjusted;
    //divide_by_bp60:
    velocity_adjusted /= space_friction;
    //-- this is the resulting acceleration --

#endif
}

void DestinyManager::_InitWarp() {

    //init warp:
    //destiny_WARP_Math....work in progress.   allan 24Oct14


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

  /*
  m_targetPoint
  m_position
  m_targetDistance

  m_maxShipVelocity
  m_shipInertiaModifier
  m_shipWarpSpeed
  m_shipAgility
  m_avgWarpAcceleration
  m_warpAccelTime
  m_warpCapacitorNeed

  K = max speed (6au/s)
  k = inital speed (0.75 * base speed)
  z = final speed (base speed / 2 : drop out of warp when current speed gets below 50% sub-warp top speed or 100km/s, whatever is less)
  T = total time in warp
  t = time at max speed (K)
  accel...
  d = (K + k / 2) / ((T - t) / 4)     `this is 25% of non-max speed warp
  cruise...
  d = K * t
  decel...
  d = z - K / ((T - t) * 0.75)   `this is 75% of non-max speed warp

  d = total warp distance (in meters)

*/

	GVector warp_vector(m_position, m_targetPoint);
	warp_vector.normalize();

	if(m_self->IsClient()) {
		Client *p_Client = m_self->CastToClient();
		InventoryItemRef ship = p_Client->GetShip();
		// adjust warp distance based on cap left.   NPC's are not restricted by this. (or wont be in EvEmu)
		double currentShipCap = ship->GetAttribute(AttrCapacitorCharge).get_float();

		/*  capacitor for warp forumlas from https://oldforums.eveonline.com/?a=topic&threadID=332116
		*	 d = distance travelled
		*	 c = capacitor used [ignoring warp drive operation skill, if you want your actual cap value then multiply c by 1 - (0.1 * warp drive operation level)]
		*	 m = ship mass
		*	 d = c / (m * warpCapacitorNeed)
		*
		*  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
		*/
		double adjWarpCapNeed = m_warpCapacitorNeed * (1 - (p_Client->GetChar()->GetSkillLevel(skillWarpDriveOperation, true) * 0.1));
		double adjDistance = m_targetDistance / ONE_AU_IN_METERS;	// change distance in meters to AU.
		double capNeeded = adjDistance * m_mass * adjWarpCapNeed;

		//  check if ship has enough capacitor to warp full distance
		if(capNeeded > currentShipCap){
			//  nope...reset distance based on avalible capacitor
			m_targetDistance = currentShipCap / (m_mass * adjWarpCapNeed);
			capNeeded = currentShipCap;
		} else
			capNeeded -= currentShipCap;

		//drain cap
		ship->SetAttribute(AttrCapacitorCharge, capNeeded, true);
	}

	// set time in warp, from enter to exit
	double accelDistance = m_avgWarpAcceleration * m_warpAccelTime;  //this is in meters (accel=m/s^2, time=s)
	double cruiseDistance = m_targetDistance - (accelDistance * 2);  //decel = accel...times are same.
	double cruiseTime = cruiseDistance / m_shipWarpSpeed;

	double warpTime = (m_warpAccelTime * 2) + cruiseTime;

	//  acceleration formulas
    /*
     *  For the acceleration phase, k is equal to the ship's maximum warp speed (in AU/s).
     *  x = e^(k.t)
     *  v = k.e^(k.t)
     *  distance = EULERS_NUMBER *(k*s)
     *  speed = k*EULERS_NUMBER * (k*s)
	 *
	 * min time = 7.5s
	 * max time = 15s
	 */

	//  this is to check my math so far.....
	double acceldistance_check = EULERS_NUMBER * (m_shipWarpSpeed * m_warpAccelTime);
	sLog.Log("DestinyManager::_InitWarp()","Calculate Time Check.  AccelDistance = %f, distance = %f", accelDistance, acceldistance_check);

    _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Entity %u: Calculate has determined we will exit warp at %f, %f, %f at a distance of %f.",
		 m_self->GetName(), m_self->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance/ONE_AU_IN_METERS);
/*

    if(warp_distance < warp_speed) {
        warp_speed = warp_distance;
        _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Adjusting warp speed to %f for short warp.", warp_speed);
    } else
        _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Warp speed in system %u is %f", m_system->GetID(), warp_speed);



    _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Warp will accelerate for %fs, then slow down at %fs", warp_acceleration_time, warp_slow_time);
*/
    delete m_warpState;
    m_warpState = new WarpState(
        GetStamp(),
		m_targetDistance,
		m_shipWarpSpeed,
		m_warpAccelTime,
		m_warpAccelTime,  //decel time = accel time
		warp_vector
        );

	std::vector<PyTuple *> updates;
	{
		DoDestiny_CmdWarpTo du;
		du.entityID = m_self->GetID();
		du.dest_x = m_targetPoint.x;
		du.dest_y = m_targetPoint.y;
		du.dest_z = m_targetPoint.z;
		du.distance = m_targetDistance; //static_cast<int32>(distance);
		du.warpSpeed = m_shipWarpSpeed * 10;  // client expects speed x 10.  dumb ccp shit again

		updates.push_back(du.Encode());
	}

	{
		//send a warping special effects update...
		DoDestiny_OnSpecialFX10 du;
		du.effect_type = "effects.Warping";
		du.entityID = m_self->GetID();
		du.isOffensive = 0;
		du.start = 1;
		du.active = 0;

		updates.push_back(du.Encode());
	}

	{
        //the client clears massive during warp,  (massive means object is solid)
        //  set massive to false!" )      ---works....13July14
		DoDestiny_SetBallMassive sbmassive;
		sbmassive.entityID = m_self->GetID();
		sbmassive.is_massive = 0;
		updates.push_back(sbmassive.Encode());
	}

	SendDestinyUpdate(updates, true);
}

void DestinyManager::_Warp() {
    if(m_warpState == NULL) {
        codelog(DESTINY__ERROR, "Error: _Warp() called with no warp state!");
        return;
    }
    const double seconds_into_warp = (GetStamp() - m_warpState->start_stamp) * TIC_DURATION_IN_SECONDS;
    //_log(PHYSICS__TRACE, "DestinyManager::_Warp() - Entity %u: Seconds into warp %.1f", m_self->GetID(), seconds_into_warp);

    /*
     * There are three stages of warp, which are related to our speed:
     *
     * stage 1) Accelerating. This is where we are accelerating, and
     *    lasts until we have reached the warp speed for this system.
     * stage 2) Cruising. This is where we have reached warp speed
     *    and are just moving along at warp speed until we need to
     *    start slowing down.
     * stage 3) Slowing. We are approaching our destination, and start
     *    slowing down out of warp speed.
     *
     */
    //variables set by the current stage
    double dist_remaining;
    double velocity_magnitude;
    bool stop = false;

    if(seconds_into_warp < m_warpState->acceleration_time) {
        double warp_progress = exp( 3.0 * seconds_into_warp );
        dist_remaining = m_warpState->total_distance - warp_progress;

        velocity_magnitude = warp_progress * 3.0;

        // Remove ship from bubble only when distance traveled takes the ship beyond the bubble's radius
        m_system->bubbles.UpdateBubble(m_self,true,true);   // use optional 2nd param to indicate ship is warping so as to not add to or create new bubble while accelerating into warp.

        //_log(PHYSICS__TRACE, "DestinyManager::_Warp():Accelerate - Entity %u: Warp Accelerating: velocity %f m/s with %f m left to go.",
        //    m_self->GetID(), velocity_magnitude, dist_remaining);

    } else if(seconds_into_warp < m_warpState->slow_time) {
        //dont simplify this (or use floats), the roundoff error caused by all the extra
        // multiply/divide crap then is major enough as to cause problems
        double delta_t = (m_warpState->acceleration_time * 3.0) - ((seconds_into_warp * 3.0)+1.0);
        double delta_s = (m_warpState->speed * delta_t)/(-3.0);
        dist_remaining = m_warpState->total_distance - delta_s;

        velocity_magnitude = m_warpState->speed;

    //    if( (m_warpState->total_distance - dist_remaining) > 1.0e6f )
    //    {
    //        // warp is long enough to be in cruise and we're out of the warp-out bubble,
    //        // so remove ourselves from our previous bubble:
    //        m_self->Bubble()->Remove(m_self);
    //    }

        _log(PHYSICS__TRACE, "DestinyManager::_Warp():Cruise - Entity %s(%u): Warp Cruising: velocity %f m/s with %f m left to go.",
            m_self->GetName(), m_self->GetID(), velocity_magnitude, dist_remaining);
    } else {
        //warp_completely_done

        // TODO: Adjust these equations because warp-in final position depends on them
        // and is still wrong.
        //double v58 = (((m_warpState->acceleration_time + (m_warpState->total_distance/m_warpState->speed) - seconds_into_warp) )) - (m_warpNumerator/m_warpDenomenator);
        //velocity_magnitude = exp(m_warpExpFactor * v58) * m_warpState->speed / m_warpVelocityMagnitudeFactorDivisor;
		velocity_magnitude = (-1.0 * exp(-1.0 * seconds_into_warp)) * m_warpState->speed;

        if(velocity_magnitude < 0)
            velocity_magnitude = -velocity_magnitude;

        //dist_remaining = velocity_magnitude;// / 1.65;
        //double delta_t = (m_warpState->acceleration_time * 3.0f) - ((seconds_into_warp * 3.0f)+1.0f);
        //double delta_s = (m_warpState->speed * delta_t)/(-3.0f);
        //dist_remaining = m_warpState->total_distance - (velocity_magnitude * exp(-1.0 * seconds_into_warp) + (m_warpState->slow_time * m_warpState->speed));
        dist_remaining = velocity_magnitude * m_warpDecelerateFactor;

        //_log(PHYSICS__TRACE, "DestinyManager::_Warp():Slow - Entity %u: Warp Slowing: velocity %f m/s with %f m left to go.",
        //    m_self->GetID(), velocity_magnitude, dist_remaining);

        // Put ourself back into a bubble once we reach the outer edge of the bubble's radius:
        if( dist_remaining <=  BUBBLE_RADIUS_METERS )
        {
            // This MUST be called BEFORE SetPosition() since SetPosition does not
            // currently support passing in the isPostWarp boolean nor the isWarping boolean
            m_system->bubbles.UpdateBubble(m_self, true, false, true);
			//  need to find a way to update existing clients in bubble to this entity's true posistion
        }

        /*
		 * Ships will exit warp mode when their warping speed drops below
		 * 50% of sub-warp max speed, or 100m/s, whichever is the lower.
		 */

        if(velocity_magnitude < m_maxShipVelocity) {
        //note, this should actually be checked AFTER we change new_velocity.
        //but hey, it doesn't get copied into ball.velocity until later anyhow.
            stop = true;
            //SetPosition( GetPosition(), true );
        }
    }

	//  update position and velocity for all stages.
    GVector vector_to_us = m_warpState->normvec_them_to_us * dist_remaining;
    m_position = m_targetPoint + vector_to_us;
	m_velocity = m_warpState->normvec_them_to_us * (-velocity_magnitude);
	//SetPosition( m_position );

    if(stop) {
        _log(PHYSICS__TRACE, "DestinyManager::_Warp():Complete - Entity %s(%u): Warp completed. Exit velocity %f m/s with %f m left to go.",
            m_self->GetName(), m_self->GetID(), velocity_magnitude, dist_remaining);
        delete m_warpState;
        m_warpState = NULL;
        SetSpeedFraction( 0.0f, true );
        Stop(true);
    }
}

void DestinyManager::TractorBeamFollow(SystemEntity *who, double mass, double maxVelocity, double distance, bool update) {
	if (State == DSTBALL_FOLLOW && m_targetEntity.second == who && m_targetDistance == distance)
		return;

	State = DSTBALL_FOLLOW;
	m_targetEntity.first = who->GetID();
	m_targetEntity.second = who;
	m_targetDistance = distance;

	if (m_userSpeedFraction == 0.0f)
		m_userSpeedFraction = 1.0f;
	if (m_activeSpeedFraction != m_userSpeedFraction) {
		m_activeSpeedFraction = m_userSpeedFraction;
		_UpdateDerrived();
	}

	//Clear any pending docking operation since the user set a new course:
	if (m_self->IsClient())
		m_self->CastToClient()->SetPendingDockOperation(false);

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
		followball.unknown = uint32(distance);
		updates.push_back(followball.Encode());

		SendDestinyUpdate(updates, false); //consumed
	}

	sLog.Debug("DestinyManager::TractorBeamFollow()", "SystemEntity '%s' following SystemEntity '%s' at velocity %f",
		m_self->GetName(), who->GetName(), m_maxVelocity);

	// Forcibly set Speed since it doesn't get updated when Following upon Undock from stations:
	SetSpeedFraction(m_activeSpeedFraction, true);
}

void DestinyManager::TractorBeamHalt(bool update)
{
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;
    m_velocity = GVector(0, 0, 0);
    m_activeSpeedFraction = 0.0f;
    _UpdateDerrived();

    //Clear any pending docking operation since the user halted ship movement:
    if (m_self->IsClient())
        m_self->CastToClient()->SetPendingDockOperation(false);

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
        SetSpeedFraction( 0.0f, true );
        Stop( true );
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
                SetSpeedFraction( 0.0f, true );
                Stop( true );
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
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - m_accelerationFactor = %.15e", m_accelerationFactor);
    accel_vector *=  m_accelerationFactor;
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
    if(State == DSTBALL_FOLLOW && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    State = DSTBALL_FOLLOW;
    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;
    if(m_userSpeedFraction == 0.0)
        m_userSpeedFraction = 1.0f;
    if(m_activeSpeedFraction != m_userSpeedFraction) {
        m_activeSpeedFraction = 1.0f;
        _UpdateDerrived();
    }

    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    if(update) {
        DoDestiny_CmdFollowBall du;
        du.entityID = m_self->GetID();
        du.ballID = who->GetID();
        du.unknown = uint32(distance);

        PyTuple *tmp = du.Encode();
        SendDestinyUpdate(&tmp);    //consumed
    }

    sLog.Debug( "DestinyManager::Follow()", "SystemEntity '%s' following SystemEntity '%s' at velocity %f",
                m_self->GetName(), who->GetName(), m_maxVelocity );

    // Forcibly set Speed since it doesn't get updated when Following upon Undock from stations:
    SetSpeedFraction( m_activeSpeedFraction, true );
}

void DestinyManager::Orbit(SystemEntity *who, double distance, bool update) {
    if(State == DSTBALL_ORBIT && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    State = DSTBALL_ORBIT;
    m_stateStamp = GetStamp()+1;

    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;
    if(m_userSpeedFraction == 0.0)
        m_userSpeedFraction = 1.0f;    //doesn't seem to do this.
    if(m_activeSpeedFraction != m_userSpeedFraction) {
        m_activeSpeedFraction = 1.0f;
        _UpdateDerrived();
    }

    if(update) {
        DoDestiny_CmdOrbit du;
        du.entityID = m_self->GetID();
        du.orbitEntityID = who->GetID();
        du.distance = uint32(distance);

        PyTuple *tmp = du.Encode();
        SendDestinyUpdate(&tmp);    //consumed
    }
}

void DestinyManager::OrbitingCruise(SystemEntity *who, double distance, bool update, double cruiseSpeed) {
    if(State == DSTBALL_ORBIT && m_targetEntity.second == who && m_targetDistance == distance)
        return;

	if( cruiseSpeed > 0 )
	{
		m_maxShipVelocity = cruiseSpeed;
		_UpdateDerrived();
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
        _UpdateDerrived();
    }

    if(update) {
        DoDestiny_CmdOrbit du;
        du.entityID = m_self->GetID();
        du.orbitEntityID = who->GetID();
        du.distance = uint32(distance);

        PyTuple *tmp = du.Encode();
        SendDestinyUpdate(&tmp);    //consumed
    }
}

void DestinyManager::SetShipVariables(InventoryItemRef ship)
{
	m_radius = ship->GetAttribute(AttrRadius).get_float();

	/* AttrMass = 4,
	 * AttrMassLimit = 622,
	 * AttrMassAddition = 796,
	 * AttrMassMultiplier = 1471,
	 */
	double shipMass = ship->GetAttribute(AttrMass).get_float();

	//  check for rigs that modify mass here
	m_mass = shipMass;

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

	float capCapacity = ship->GetAttribute(AttrCapacitorCapacity).get_float();	// initial value from loaded data
	float capChargeRate = ship->GetAttribute(AttrRechargeRate).get_float();	// initial value from loaded data, in ms.

	if( m_self->IsClient() ) {
		Character *p_Char = m_self->CastToClient()->GetChar().get();
		capCapacity += capCapacity * (( 5 * (p_Char->GetSkillLevel(skillEnergyManagement, true)))/100);
		capChargeRate -= capChargeRate * (( 5 * (p_Char->GetSkillLevel(skillEnergySystemsOperation, true)))/100);
	}

	// add checks for modules, rigs and implants that affect cap capacity here

	// add checks for modules, rigs and implants that affect cap charge rate here

	// set capacitor to full, and apply recharge rate.
	ship->SetAttribute(AttrCapacitorCharge, capCapacity, true);
	ship->SetAttribute(AttrRechargeRate, capChargeRate, true);
}

//  called from Client::BoardShip, Client::UndockFromStation, NPC::NPC
void DestinyManager::SetShipCapabilities(InventoryItemRef ship)
{
	/* this now sets variables needed for correct warp math.
	 * noted modifiers to look into later, after everything is working
	 * skill bonuses to agility and velocity are now implemented.
	 *
	 * also check warp capacitor need here, setting modifiers as needed
	 * noted supercap variables for later
	 */

	// AttrWarpCapacitorNeed(153)
	double warpCapNeed = ship->GetAttribute(AttrWarpCapacitorNeed).get_float();

	float massMKg = m_mass / 1000000;  //changes mass from Kg to MillionKg (10^-6)

	double adjInertiaModifier = ship->GetAttribute(AttrAgility).get_float();
	float adjShipMaxVelocity = ship->GetAttribute(AttrMaxVelocity).get_float();
	float warpSpeedMultiplier = 1.0f;
	float shipBaseWarpSpeed = 3.0f;

	// skill bonuses to agility and velocity and warpCapacitorNeed
	if( m_self->IsClient() ) {
		Character *p_Char = m_self->CastToClient()->GetChar().get();
		bool IsCapShip = false;
		// add check here for capital ships
		adjInertiaModifier -= ( adjInertiaModifier * p_Char->GetAgilitySkills(IsCapShip) );
		adjShipMaxVelocity += ( adjShipMaxVelocity * ( 5 * (p_Char->GetSkillLevel(skillNavigation, true)))/100);
		shipBaseWarpSpeed = ship->GetAttribute(AttrBaseWarpSpeed).get_float();
		warpSpeedMultiplier = ship->GetAttribute(AttrWarpSpeedMultiplier).get_float();
		warpCapNeed -= ( warpCapNeed * ( 10 * (p_Char->GetSkillLevel(skillWarpDriveOperation, true))));
		// TODO check for implants  AttrWarpCapacitorNeedBonus(319)
		//warpCapNeed -= ( warpCapNeed * p_Char->GetAttribute(AttrBaseWarpSpeed).get_float());
	} else {
		shipBaseWarpSpeed = m_self->Item()->GetAttribute(AttrBaseWarpSpeed).get_float();
		warpSpeedMultiplier = m_self->Item()->GetAttribute(AttrWarpSpeedMultiplier).get_float();
	}

	// TODO add module and rig bounses to inertia, agility, velocity here

	m_maxShipVelocity = adjShipMaxVelocity;
	m_shipInertiaModifier = adjInertiaModifier;
	m_warpCapacitorNeed = warpCapNeed;

	/* The product of Mass and the Inertia Modifier gives the ship's agility
	 * Agility = Mass x Inertia Modifier
	 */
	m_shipAgility = massMKg * m_shipInertiaModifier;

	/*   look into these, too...
	 * AttrWarpSBonus(624) [rigs]
	 * AttrWarpFactor(21)
	 * AttrWarpInhibitor(29)
	 */

	//TODO  add module and rig modifiers to warp speed here
	m_shipWarpSpeed = ( warpSpeedMultiplier * shipBaseWarpSpeed );

	/*  my research into warp formulas, i have used these sites, with a few exerpts from each...
	 *     https://wiki.eveonline.com/en/wiki/Acceleration
	 * 	   http://oldforums.eveonline.com/?a=topic&threadID=1251486
	 *     http://gaming.stackexchange.com/questions/115271/how-does-one-calculate-a-ships-agility
	 *     http://eve-search.com/thread/1514884-0
	 *     http://www.eve-search.com/thread/478431-0/page/1
	 *
	 * main formula for time taken to accelerate from v to V, from https://wiki.eveonline.com/en/wiki/Acceleration
	 * t=IM(10^-6) * -e(1-(v/V))
	 *
	 * t Time in seconds
	 * v Velocity after time t in m/s
	 * V Ship's maximum velocity in m/s
	 * I Ship's inertia modifier
	 * M Ship's mass in kg
	 * e Base of natural logarithms (EULERS_NUMBER)
	 *
	 */
	uint64 warpSpeedInMeters = m_shipWarpSpeed * ONE_AU_IN_METERS;
	double ln_speed = fabs (EULERS_NUMBER * (1 - ((m_maxShipVelocity * 0.75) / warpSpeedInMeters)));
	m_warpAccelTime = (m_shipInertiaModifier * massMKg * ln_speed);

	/* find average accel based on time to accel from entering warp (75% of max_velocity) to m_shipWarpSpeed.
	 * accel = final speed - initial speed / time
	 *   this will be used in _InitWarp
	 */

	m_avgWarpAcceleration = (((m_maxShipVelocity * 0.75) / warpSpeedInMeters) / m_warpAccelTime);

	_log(PHYSICS__TRACE, "DestinyManager::SetShipCapabilities - Entity %s(%u) has set ship attributes:"
	     "Radius=%f, Mass=%f, maxVelocity=%f, agility=%f, inertia=%f, warpSpeed=%u, accel=%f",
		 m_self->GetName(), m_self->GetID(), m_radius, m_mass, m_maxShipVelocity, m_shipAgility,
		 m_shipInertiaModifier, m_shipWarpSpeed, m_acceleration);

    _UpdateDerrived();
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

        PyTuple *tmp = du.Encode();
        SendDestinyUpdate(&tmp);    //consumed
    }
    m_system->bubbles.UpdateBubble(m_self, update, isWarping, isPostWarp);
}

void DestinyManager::AlignTo(SystemEntity *ent, bool update) {
  /**  this is for turning ship for alignment to object -allan 7Aug14
   *		continue this later....   allan 11Aug14
   */
    const GPoint position = ent->GetPosition();

	State = DSTBALL_GOTO;
    m_targetPoint = position;  //m_position + (position * 1.0e16);

    m_userSpeedFraction = 0.1f;
    m_activeSpeedFraction = 0.1f;
    _UpdateDerrived();

    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    /**  need a while loop in here for checking alignment
	 *     when alignment is good (~8º), call Stop()
	 *
        GVector align_vector(m_position, m_targetPoint);
        align_vector.normalize();
        GVector vel_normalized(m_velocity);
        vel_normalized.normalize();
        double dot = warp_vector.dotProduct(vel_normalized);    //cosine between vectors.
        while(dot < (1.0 - 0.01)) {    //this is about 8 degrees
        }
	 */
	GotoDirection(position, true);

    if(update) {
        DoDestiny_CmdAlignTo au;
        au.entityID = ent->GetID();

        PyTuple *tmp = au.Encode();
        SendDestinyUpdate(&tmp);    //consumed
    }

    _log(PHYSICS__TRACEPOS, "DestinyManager::AlignTo() - '%s' aligning to SystemEntity '%s'(%u) @ (%f,%f,%f)",
                m_self->GetName(), ent->GetName(), ent->GetID(), position.x, position.y, position.z );
}

void DestinyManager::GotoDirection(const GPoint &direction, bool update) {
    //Clear any pending docking operation since the user set a new course:
	if( m_self->IsClient() )
		m_self->CastToClient()->SetPendingDockOperation( false );

    State = DSTBALL_GOTO;
    m_targetPoint = m_position + (direction * 1.0e16);
    bool process = false;
    if(m_userSpeedFraction == 0.0f) {
        m_userSpeedFraction = 1.0f;
        process = true;
    }

    if(m_activeSpeedFraction != m_userSpeedFraction)
	    SetSpeedFraction( 1.0f, update );

    if(update) {
        DoDestiny_CmdGotoDirection du;
        du.entityID = m_self->GetID();
        du.x = direction.x;
        du.y = direction.y;
        du.z = direction.z;

        PyTuple *tmp = du.Encode();
        SendDestinyUpdate(&tmp);    //consumed
    }

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

	GPoint stationOrigin = static_cast< StationEntity* >( station )->GetPosition();
    GPoint position = who->GetPosition();

    GVector direction(position, stationOrigin);
    double rangeToStationPerimiter = direction.length() - station->GetRadius();

    // WARNING: DO NOT uncomment the following line as it for some reason causes HEAP corruption to occur on auto-docking
    //if( !(who->GetPendingDockOperation()) )
        //GotoDirection( direction, true );   // Turn ship and move toward docking point

    // Verify range to station is within docking perimeter of 1500 meters:
    //if( (rangeToStation - station->GetRadius()) > 1500 )
	sLog.Warning("DestinyManager::AttemptDockOperation()", "rangeToStationPerimiter is %u", rangeToStationPerimiter);
	if( rangeToStationPerimiter > 1500 )
    {
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
    who->SaveAllToDatabase();

    // Docking was accepted, so send the OnDockingAccepted packet:
    // Packet::Notification
    /* -- In the marshalstream:
    Tuple: 2 elements
      [ 0] Integer field: 0
      [ 1] Tuple: 2 elements
      [ 1]   [ 0] Integer field: 1
      [ 1]   [ 1] Tuple: 3 elements
      [ 1]   [ 1]   [ 0] List: 3 elements                           // Station Dockpoint
      [ 1]   [ 1]   [ 0]   [ 0] Real field: 273625620310.000000     // x
      [ 1]   [ 1]   [ 0]   [ 1] Real field: -12937663343.000000     // y
      [ 1]   [ 1]   [ 0]   [ 2] Real field: 50607993006.845703      // z
      [ 1]   [ 1]   [ 1] List: 3 elements                           // Ship Position
      [ 1]   [ 1]   [ 1]   [ 0] Real field: 273625620310.000000     // x
      [ 1]   [ 1]   [ 1]   [ 1] Real field: -12937663343.000000     // y
      [ 1]   [ 1]   [ 1]   [ 2] Real field: 50607993007.845703      // z
      [ 1]   [ 1]   [ 2] Integer field: 60004450                    // Station ID
    */
	/*  begin docking accepted packet building for return
    PyTuple *da = new PyTuple(2);
	PyTuple *da2 = new PyTuple(2);
	PyTuple *da3 = new PyTuple(3);
	PyList *da_list = new PyList(3);
	PyList *da_list2 = new PyList(3);
	da_list->SetItems
*/
	return NULL;
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
    SetSpeedFraction(1.0f, true);

    if(m_warpState != NULL) {
        delete m_warpState;
        m_warpState = NULL;
    }

	if( m_self->IsClient() ) {
		//Clear any pending docking operation since the user initiated warp:
		m_self->CastToClient()->SetPendingDockOperation( false );
	}

    State = DSTBALL_WARP;
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;
    m_targetPoint = where;
	m_targetDistance = distance;
}

bool DestinyManager::_Turn() {
#ifdef OLD_STUFF
    double direction_cross = m_direction.dotProduct(m_targetDirection);    //cos(angle between)
    if(direction_cross != 1) {
        //we are not heading in the exact direction we want to go...
        if(direction_cross > 0.98) {
            //close enough, dont mess with the thrust crap, just put them on course
            m_body->setAngularVelocity(Ga::GaVec3(0, 0, 0));
            Ga::GaVec3 vel = Ga::GaVec3(m_direction);
            vel *= m_velocity;
            m_body->setLinearVelocity(vel);
            return false;
        }

        //else, we need some turning action... apply our "turning" forces
#   pragma message( "TODO: apply turning force (torque)" )
        m_body->applyTorque(Ga::GaVec3(0, 0, 0));

        if(direction_cross < 0.5) {
            //we have a lot of turning to do, let the caller know in case they want to delay acceleration
            return true;
        }
    }
#endif
    return false;
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
    std::vector<PyTuple *> updates;

    //Clear any pending docking operation since the user's ship exploded:
	if( m_self->IsClient() )
        m_self->CastToClient()->SetPendingDockOperation( false );

    {
		SystemBubble *b = m_self->Bubble();
        //send an explosion special effects update...
        DoDestiny_TerminalExplosion du;
        du.shipID =  m_self->CastToClient()->GetShipID();	//m_self->GetID();
        du.bubbleID = b->GetBubbleID();  					//  add check for bubbleID here.
        du.ballIsGlobal = false; 							//  is this ball Global?  boolean
        updates.push_back(du.Encode());
    }

    SendDestinyUpdate(updates, true);
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

    PyTuple *tmp = du.Encode();
    SendDestinyUpdate(&tmp);    //consumed
}

void DestinyManager::SendSetState(const SystemBubble *b) const {
    DoDestiny_SetState ss;

    ss.stamp = GetStamp();
    ss.ego = m_self->GetID();
    m_system->MakeSetState(b, ss);

    PyTuple *tmp = ss.Encode();
    SendDestinyUpdate(&tmp, true);    //consumed
}

void DestinyManager::SendBallInfoOnUndock(bool update) const {
    std::vector<PyTuple *> updates;

    DoDestiny_SetBallMassive sbmassive;
    sbmassive.entityID = m_self->GetID();
    sbmassive.is_massive = 0;
    updates.push_back(sbmassive.Encode());

    DoDestiny_SetBallMass sbmass;
    sbmass.entityID = m_self->GetID();
    sbmass.mass = m_self->Item()->GetAttribute(AttrMass).get_float();
    updates.push_back(sbmass.Encode());

    DoDestiny_SetBallVelocity sbvelocity;
    sbvelocity.entityID = m_self->GetID();
    sbvelocity.x = m_velocity.x;
    sbvelocity.y = m_velocity.y;
    sbvelocity.z = m_velocity.z;
    updates.push_back(sbvelocity.Encode());

    SendDestinyUpdate(updates, true);    //consumed
}

void DestinyManager::SendBoardShip(const ShipRef boardShipRef) const {

    std::vector<PyTuple *> updates;

    DoDestiny_SetBallInteractive du_setBallInteractiveCapsule;
    du_setBallInteractiveCapsule.entityID = boardShipRef->itemID();
    du_setBallInteractiveCapsule.interactive = 1;
    updates.push_back(du_setBallInteractiveCapsule.Encode());

    SendDestinyUpdate(updates, false);
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

