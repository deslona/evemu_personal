/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2014 The EVEmu Team
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
    Author:        Aknor Jaden, Allan
*/

#ifndef WRECKS_AND_LOOT_H
#define WRECKS_AND_LOOT_H

#include "system/SystemDB.h"
#include <unordered_map>
#include <vector>


// //////////////// Permanent Memory Object Classes //////////////////////

// This class is a singleton object, containing all Effects loaded from dgmEffects table
//     as memory objects of type MEffect:
class DGM_Types_to_Wrecks_Table
: public Singleton< DGM_Types_to_Wrecks_Table >
{
public:
    DGM_Types_to_Wrecks_Table();
    ~DGM_Types_to_Wrecks_Table();

    // Initializes the Table:
    int Initialize();

    // Returns typeID of wreck that is appropriate for the given typeID,
	//  returns 0 if no match
    uint32 GetWreckID(uint32 typeID);

protected:
    void _Populate();

    std::map<uint32, uint32> m_WrecksToTypesMap;
};

#define sDGM_Types_to_Wrecks_Table \
    ( DGM_Types_to_Wrecks_Table::get() )
// -----------------------------------------------------------------------

//  CLASS DEFINITION FOR LOOT SYSTEM
//  struct objects for holding loot data (POD).

struct DBLootGroup {
    //uint32 groupID;
    uint32 lootGroupID;
    double dropChance;
};

struct DBLootGroupType {
    //uint32 lootGroupID;
    uint32 typeID;
    double chance;
    uint32 minQuantity;
    uint32 maxQuantity;
};

struct DBSalvageGroup {
    //uint32 wreckTypeID;
    uint32 salvageItemID;
    uint8 groupID;
    double dropChance;
    uint8 minDrop;
    uint8 maxDrop;
};

struct LootList {
    uint32 itemID;
    uint8 minDrop;
    uint8 maxDrop;
};

// This class is a singleton object, containing all loot items/defs loaded from npcLoot* table
//  Allan 27Nov14
class DGM_Loot_Groups_Table
: public Singleton< DGM_Loot_Groups_Table >
{
  public:
      DGM_Loot_Groups_Table();
      ~DGM_Loot_Groups_Table();

      typedef std::vector<LootList> LootListDef;
      typedef std::unordered_multimap<uint32, DBLootGroup> LootGroupDef;    /* groupID is key */
      typedef std::unordered_multimap<uint32, DBLootGroupType> LootGroupTypeDef;    /* lootGroupID is key */

      // Initializes the Table:
      int Initialize();

      // Returns vector lootGroupIDs
      //  0 if no match
      void GetLoot(uint32 groupID, LootListDef &lootList);

  protected:
      void _Populate();

      LootGroupDef m_LootGroupMap;
      LootGroupTypeDef m_LootGroupTypeMap;
};

#define sDGM_Loot_Groups_Table \
( DGM_Loot_Groups_Table::get() )
//////////////////////////////////////////////////////////////////////////

// This class is a singleton object, containing all salvage items/defs loaded from wrecksToSalvage table
//  Allan 30Nov14
class DGM_Salvage_Table
: public Singleton< DGM_Salvage_Table >
{
public:
    DGM_Salvage_Table();
    ~DGM_Salvage_Table();

    typedef std::vector<LootList> LootListDef;
    typedef std::unordered_multimap<uint32, DBSalvageGroup> SalvageDef;     /* wreckTypeID is key */

    // Initializes the Table:
    int Initialize();

    // Returns vector of SalvageIDs
    //  0 if no match
    void GetSalvage(uint32 wreckTypeID, LootListDef &salvageList);

protected:
    void _Populate();

    SalvageDef m_SalvageMap;
};

#define sDGM_Salvage_Table \
( DGM_Salvage_Table::get() )
//////////////////////////////////////////////////////////////////////////


#endif
