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
    Author:        Allan
*/


#ifndef __SEARCHDB_H_INCL__
#define __SEARCHDB_H_INCL__

#include "ServiceDB.h"
#include "system/Search.h"
// I'm anticipating a large method for searching entitys and items, so this is where I'm putting it.

class SearchDB : public ServiceDB
{
  public:
    SearchDB();

    PyObject *Query(std::string string, int32 int1, uint32 charID);
    PyObject *QueryAll(std::string string, uint32 charID);
    PyObject *QuickQuery(std::string string, int32 int1, int32 int2, int32 int3, uint32 charID);
    //PyObject *QuickQuery(std::string string, int32 int1, int32 int2, int32 int3, int32 hideNPC, int32 onlyAltName, uint32 charID);

  private:

};


#endif