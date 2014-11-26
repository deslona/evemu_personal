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
    Author:        Zhur, Aknor Jaden
*/

#ifndef __DESTINYMANAGER_H_INCL__
#define __DESTINYMANAGER_H_INCL__

#include "PyCallable.h"
#include "destiny/DestinyStructs.h"
#include "inventory/ItemRef.h"
#include "system/SystemEntity.h"
#include <eve-compat.h>

class SystemEntity;
class SystemManager;
class InventoryItem;
class PyRep;
class PyList;
class PyTuple;
class SystemBubble;

extern const double SPACE_FRICTION;
extern const double SPACE_FRICTION_SQUARED;
extern const float TIC_DURATION_IN_SECONDS;

static const GPoint NULL_ORIGIN = (0,0,0);

//this object manages an entity's position in the system.
class DestinyManager {
public:

	typedef enum {
		destinyStopped = 1010,
		destinyMoving,
		destiny
	} DestinyMotion;

	typedef enum {
		destinyOrbiting = 2020,
		destinyFollowing,
		destinyWarping,
		destinyJumping
	} DestinyAction;


    static uint32 GetStamp() { return(m_stamp); }
    static bool IsTicActive() { return(m_stampTimer.Check(false)); }
    static void TicCompleted() { if(m_stampTimer.Check(true)) m_stamp++; }

    Destiny::BallMode GetState() { return State; }

    DestinyManager(SystemEntity *self, SystemManager *system);
    ~DestinyManager();

    void Process();

	void SendSelfDestinyUpdate(PyTuple **up) const;
	void SendSelfDestinyEvent(PyTuple **up) const;
	void SendDestinyEvent(PyTuple **up, bool self_only = false) const;
    void SendDestinyEvent(std::vector<PyTuple *> &events, bool self_only = false) const;
    void SendDestinyUpdate(PyTuple **up, bool self_only = false) const;
    void SendDestinyUpdate(std::vector<PyTuple *> &updates, bool self_only = false) const;
    void SendDestinyUpdate(std::vector<PyTuple *> &updates, std::vector<PyTuple *> &events, bool self_only = false) const;

	// Information query functions:
    const GPoint &GetPosition() 	const { return(m_position); }
    const GVector &GetVelocity() 	const { return(m_velocity); }
    double GetSpeedFraction() 		{ return(m_activeSpeedFraction); }
	SystemManager *const GetSystemManager() { return m_system; }
	SystemBubble *const GetCurrentBubble() { return m_self->Bubble(); }

    //called whenever an entity is going away and can no longer be used as a target
    void EntityRemoved(SystemEntity *who);

    //Configuration:
    void SetShipCapabilities(InventoryItemRef ship, bool undock =  false);
	void SetShipVariables(InventoryItemRef ship);
    void SetMaxVelocity(double maxVelocity) { m_maxShipVelocity = maxVelocity; }
    void SetPosition(const GPoint &pt, bool update=true, bool isWarping=false, bool isPostWarp=false);
    void SetBubble(bool set = false) { m_inBubble = set; }

    //Global Actions:
	void Stop();
	void Halt();    // puts entity at 0 velocity
    void TractorBeamHalt(bool update = true);

    //Local Movement:
    void Follow(SystemEntity *who, double distance, bool update=true);
	void Orbit(SystemEntity *who, double distance, bool update=true);
    void OrbitingCruise(SystemEntity *who, double distance, bool update=false, double cruiseSpeed=-1.0);
	void SetSpeedFraction(float fraction, bool update=true);
	void AlignTo(SystemEntity *ent, bool update=true);
    void GotoDirection(const GPoint &direction);
	void TractorBeamFollow(SystemEntity *who, double mass, double maxVelocity, double distance, bool update=true);
    PyResult AttemptDockOperation();

	void Cloak();
	void UnCloak();

	void Undock(GPoint dockPosition, GPoint direction);

    //bigger movement:
    void WarpTo(const GPoint where, int32 distance);

	//Ship State Query functions:
	bool IsMoving() { return (((State == Destiny::DSTBALL_GOTO) || (State == Destiny::DSTBALL_FOLLOW) || (State == Destiny::DSTBALL_ORBIT)) ? true : false); }
	bool IsStopped() { return ((State == Destiny::DSTBALL_STOP) ? true : false); }
	bool IsAligned() { return ((State == Destiny::DSTBALL_GOTO) ? true : false); }
	bool IsOrbiting() { return ((State == Destiny::DSTBALL_ORBIT) ? true : false); }
	bool IsFollowing() { return ((State == Destiny::DSTBALL_FOLLOW) ? true : false); }
	bool IsJumping() { return ((State == Destiny::DSTBALL_STOP) ? true : false); }
	bool IsWarping() { return ((State == Destiny::DSTBALL_WARP) ? true : false); }
	bool IsCloaked() { return m_cloaked; }

    //Destiny Update stuff:
    void SendSetState(const SystemBubble *b) const;
	void SendBallInfoOnUndock(GPoint direction) const;
    void SendJumpIn() const;
    void SendJumpOut(uint32 stargateID) const;
	void SendJumpInEffect(std::string JumpEffect) const;
	void SendJumpOutEffect(std::string JumpEffect, uint32 locationID) const;
	void SendGateActivity(uint32) const;
    void SendTerminalExplosion() const;
    void SendBoardShip(const ShipRef boardShipRef) const;
    void SendEjectShip(const ShipRef capsuleRef, const ShipRef oldShipRef) const;
    void SendJettisonCargo(const InventoryItemRef itemRef) const;
    void SendAnchorDrop(const InventoryItemRef itemRef) const;
    void SendAnchorLift(const InventoryItemRef itemRef) const;
    void SendCloakShip(const bool IsWarpSafe) const;
    void SendUncloakShip() const;
    void SendSpecialEffect(const ShipRef shipRef, uint32 moduleID, uint32 moduleTypeID, uint32 targetID, \
		uint32 chargeTypeID, std::string effectString, bool isOffensive, bool start, bool isActive, \
		double duration, uint32 repeat) const;
	void SendSpecialEffect10(uint32 entityID, const ShipRef shipRef, uint32 targetID, std::string effectString, \
		bool isOffensive, bool start, bool isActive) const;

protected:
    void ProcessState();

    SystemEntity *const m_self;			//we do not own this.
    SystemManager *const m_system;		//we do not own this.
	//Ga::Body *m_body;					//we own a reference to this
	//Ga::Shape *m_shape;				//we own a reference to this

	//Timer m_destinyTimer;
    static uint32 m_stamp;
    static Timer m_stampTimer;
	//uint32 m_lastDestinyTime;			//from Timer::GetTimeSeconds()

    //the results of our labors:
    GPoint m_position;					//in m
    GVector m_velocity;					//in m/s
	//GVector m_direction;				//normalized, `m_velocity` stores our magnitude
	//double m_velocity;				//in m/s, the magnitude of direction

	//things dictated by our entity's configuration/equipment:
	double m_radius;					//in m
	float m_mass;						//in kg
	float m_massMKg;					//in Millions of kg
	double m_maxShipVelocity;			//in m/s
	double m_shipAgility;				//in s/kg
	float m_shipWarpSpeed;				//in au/s
	float m_speedToLeaveWarp;			//in m/s
	float m_warpAccelTime;				//in s
	float m_warpDecelTime;				//in s
	double m_warpCapacitorNeed;			//in GJ
	double m_shipInertiaModifier;
	uint8 m_warpStrength;				//interger
	double m_capNeeded;

    //derrived from other params:
    void _UpdateVelocity(bool update=false);
    double m_maxVelocity;				//in m/s
    double m_shipMaxAccelTime;				//in s  this is used to determine accel rate, and total accel time

    //User controlled information used by a state to determine what to do.
    Destiny::BallMode State;
    float m_userSpeedFraction;			//unitless
    float m_activeSpeedFraction;		//unitless
    GPoint m_targetPoint;
	double m_targetDistance;			//in m
	int32 m_stopDistance;				//from destination, in m
    uint32 m_stateStamp;				//used for ?something?
    double m_moveTimer;
    bool m_inBubble;                    //used to tell if client is in bubble or not.
	bool m_cloaked;
    std::pair<uint32, SystemEntity *> m_targetEntity;   //we do not own the SystemEntity *
    //SystemEntity *m_targetEntity;		//we do not own this.

    void _Move();						//apply our velocity and direction to our position for 1 unit of time (a second)
    void _Follow();
    void _Warp();						//carry on our current warp.
    void _Orbit();

private:

    //internal state variables used during warp.
    class WarpState {
    public:
        WarpState(
		  double start_time_,
		  double total_distance_,
		  double warp_speed_,
		  float accel_time_,
		  double accel_dist_,
		  float cruise_time_,
		  double cruise_dist_,
		  float decel_time_,
		  double decel_dist_,
		  float warp_time_,
		  const GVector &warp_vector_)
            : start_time(start_time_),
            total_distance(total_distance_),
            warpSpeed(warp_speed_),
            accelTime(accel_time_),
            accelDist(accel_dist_),
            cruiseTime(cruise_time_),
            cruiseDist(cruise_dist_),
            decelTime(decel_time_),
            decelDist(decel_dist_),
            warpTime(warp_time_),
            warp_vector(warp_vector_) { }
        double start_time;			//timestamp of when the warp started.  1/100000 resolution
        double total_distance;		//in m
        double warpSpeed;			//in m/s
        float accelTime;			//in s
		double accelDist;			//in m
		float cruiseTime;			//in s
		double cruiseDist;			//in m
		float decelTime;			//in s
		double decelDist;			//in m
		float warpTime;				//in s
        GVector warp_vector;
    };
    const WarpState *m_warpState;		//we own this. Allocated so we can have consts.
    void _InitWarp();		// various checks added.
};

#endif
