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
 *  pupose of this class is to have a common location for methods used to define random points in solar systems.
 *   there are 3 sets of methods used to work with random points in a solarsystem.
 *   one set will query solar systems for planets, pick a random planet, and define a coordnate within that planet's bubble.
 *   the second set is used to pick a random coordnate off a plants moon, for complexes, signatures, and anomolies.
 *   the third set will pick a random point between either 2 or 3 planets, for warp-out/warp-in and other things as we see fit.
 *
 *   this can (and should) be used for positioning mission space, cosmic signatures,  anomolies, complexes, and other things needing a random
 *       position in a given system.
 *
 *		class DBGPointEntity has index, itemID, and position, and is found in SystemDB.   see class decelaration below
 *
 *  NOTE i remember reading *somewhere* that ALL SPAWNS are within *some distance* from planets.   cant find that info now.  -allan 31Jul14
 */


/// int64 MakeRandomInt( int64 low, int64 high )
/// double MakeRandomFloat( double low, double high )
/*
namespace SystemDB:
class DBGPointEntity {
  public:
	uint8 idx;
    uint32 itemID;
    GPoint position;
};
*/

void SystemGPoint::GetRandPointOnPlanet(uint32 systemID) {
	uint8 total;
    std::vector<DBGPointEntity> planetIDs;

    m_db.GetPlanets(systemID, &planetIDs, &total);
	//  add code to manipulate coords here....
}

void SystemGPoint::GetRandPointOnMoon(uint32 systemID) {
	uint8 total;
    std::vector<DBGPointEntity> moonIDs;

    m_db.GetMoons(systemID, &moonIDs, &total);
	//  add code to manipulate coords here....

}

void SystemGPoint::GetRandPlanet(uint32 systemID) {

}

void SystemGPoint::Get2RandPlanets(uint32 systemID) {

}

void SystemGPoint::Get3RandPlanets(uint32 systemID) {

}

void SystemGPoint::GetRandMoon(uint32 systemID) {

}

void SystemGPoint::GetRandPointInSystem(uint32 systemID, uint64 distance) {
    // get system max diameter, verify distance is within system.

}
