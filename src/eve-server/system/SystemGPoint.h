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

#ifndef __SYSTEM_G_POINT_H__
#define __SYSTEM_G_POINT_H__

#include "system/SystemDB.h"

//work in progress


class SystemGPoint {
  public:
    void GetPlanets(uint32);
    void GetMoons(uint32);
	void GetRandPointOnPlanet(uint32);
	void GetRandPointOnMoon(uint32);
	void GetRandPlanet(uint32);
	void Get2RandPlanets(uint32);
	void Get3RandPlanets(uint32);
	void GetRandMoon(uint32);
	void GetRandPointInSystem(uint32, uint64);

  protected:
    SystemDB m_db;


};

#endif







