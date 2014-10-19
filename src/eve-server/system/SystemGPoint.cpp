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
    Author:        Allan   31Jul14
*/

//work in progress

#include "eve-server.h"

#include "system/SystemGPoint.h"

/**
 *   the pupose of this class is to have a common location with methods used to define
 *      random points in solar systems.
 *   GetRandPointOnPlanet() will query solar systems for planets, pick a random planet,
 *      and define a coordnate within that planet's bubble.
 *   GetRandPointOnMoon() does same as above, but using moons.  NOTE: the given point
 *      will be in the bubble of the planet the moon belongs to.
 *   Get2RandPlanets() will pick a random point between 2 planets, for warp-out/warp-in
 *      and other things as we see fit.
 *   Get3RandPlanets() does same as above, but using 3 planets.
 *   GetRandPointInSystem() will define a random point within a given system.
 *
 *   this class of methods should be used for positioning mission space, cosmic signatures,
 *      anomolies, complexes, and other things needing a random position in a given system.
 *
 *   class DBGPointEntity has index, itemID, and position, and is found in SystemDB.
 *      see copy of class decelaration below
 *
 *  NOTE i remember reading *somewhere* that ALL SPAWNS are within *some distance* from planets.   cant find that info now.  -allan 31Jul14
 */


/// int64 MakeRandomInt( int64 low, int64 high )
/// double MakeRandomFloat( double low, double high )

/*   copied from system/SystemDB.cpp
 * namespace SystemDB:
 * class DBGPointEntity {
 *  public:
 *	   uint8 idx;
 *    uint32 itemID;
 *    GPoint position;
 *    double x;
 *    double y;
 *    double z;
 * };
 *
*/

void SystemGPoint::GetPlanets(uint32 systemID) {
  uint8 total;
  std::vector<DBGPointEntity> planetIDs;

  m_db.GetPlanets(systemID, &planetIDs, &total);
}

void SystemGPoint::GetMoons(uint32 systemID) {
  uint8 total;
  std::vector<DBGPointEntity> moonIDs;

  m_db.GetMoons(systemID, &moonIDs, &total);
}

void SystemGPoint::GetRandPointOnPlanet(uint32 systemID) {

}

void SystemGPoint::GetRandPointOnMoon(uint32 systemID) {

}

void SystemGPoint::GetRandPlanet(uint32 systemID) {
	uint8 total;
    std::vector<DBGPointEntity> planetIDs;

    m_db.GetPlanets(systemID, &planetIDs, &total);

}

void SystemGPoint::Get2RandPlanets(uint32 systemID) {
  uint8 total;
  std::vector<DBGPointEntity> planetIDs;

  m_db.GetPlanets(systemID, &planetIDs, &total);

}

void SystemGPoint::Get3RandPlanets(uint32 systemID) {
  uint8 total;
  std::vector<DBGPointEntity> planetIDs;

  m_db.GetPlanets(systemID, &planetIDs, &total);

}

void SystemGPoint::GetRandMoon(uint32 systemID) {
	uint8 total;
    std::vector<DBGPointEntity> moonIDs;

    m_db.GetMoons(systemID, &moonIDs, &total);

}

void SystemGPoint::GetRandPointInSystem(uint32 systemID, uint64 distance) {
    // get system max diameter, verify distance is within system.

}
