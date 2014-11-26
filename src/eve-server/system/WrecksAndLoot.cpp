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
    Author:        Aknor Jaden, Allan
*/

#include "eve-server.h"

#include "system/WrecksAndLoot.h"


// ////////////////////// DGM_Types_to_Wrecks_Table Class ////////////////////////////
DGM_Types_to_Wrecks_Table::DGM_Types_to_Wrecks_Table()
{
    m_WrecksToTypesMap.clear();
}

DGM_Types_to_Wrecks_Table::~DGM_Types_to_Wrecks_Table()
{
}

int DGM_Types_to_Wrecks_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Types_to_Wrecks_Table::_Populate()
{
    uint32 wreckID, typeID;

    //first get list of all effects from dgmEffects table
    DBQueryResult *res = new DBQueryResult();
    SystemDB::GetWrecksToTypes(*res);

    //counter
	uint32 total_wreck_count = 0;
	uint32 error_count = 0;

	//go through and populate each effect
    DBResultRow row;
    while( res->GetRow(row) )
    {
        typeID = row.GetInt(0);
        wreckID = row.GetInt(1);
		m_WrecksToTypesMap.insert(std::pair<uint32, uint32>(typeID,wreckID));

		total_wreck_count++;
    }

	sLog.Log("     Wrecks_Table", "%u total wreck objects loaded", total_wreck_count);

    //cleanup
    delete res;
    res = NULL;
}

uint32 DGM_Types_to_Wrecks_Table::GetWreckID(uint32 typeID)
{
    std::map<uint32, uint32>::iterator mWrecksMapIterator;

    if( (mWrecksMapIterator = m_WrecksToTypesMap.find(typeID)) == m_WrecksToTypesMap.end() )
        return 0;
    else
    {
        return mWrecksMapIterator->second;
    }
}



// ////////////////////// DGM_Loot_Groups_Table Class ////////////////////////////
DGM_Loot_Groups_Table::DGM_Loot_Groups_Table()
{
    m_LootGroupMap.clear();
    m_LootGroupTypeMap.clear();
}

DGM_Loot_Groups_Table::~DGM_Loot_Groups_Table()
{
}

int DGM_Loot_Groups_Table::Initialize()
{
    _Populate();
    return 1;
}

void DGM_Loot_Groups_Table::_Populate()
{
    //counter
    uint32 total_loot_count = 0;

    DBQueryResult *res = new DBQueryResult();

    //first get list of all loot groups from LootGroup table
    SystemDB::GetLootGroups(*res);
    DBResultRow row;
    DBLootGroup LootGroup;
    while( res->GetRow(row) ) {
        LootGroup.groupID = row.GetInt(0);
        LootGroup.lootGroupID = row.GetInt(1);
        LootGroup.dropChance = row.GetFloat(2);
        m_LootGroupMap.push_back(LootGroup);
        total_loot_count++;
    }

    res->Reset();

    //second get list of all types from LootGroupTypes table
    SystemDB::GetLootGroupTypes(*res);
    DBLootGroupType GroupType;
    while( res->GetRow(row) ) {
        GroupType.lootGroupID = row.GetInt(0);
        GroupType.typeID =  row.GetInt(1);
        GroupType.chance = row.GetFloat(3);
        GroupType.minQuantity = row.GetInt(4);
        GroupType.maxQuantity = row.GetInt(5);
        m_LootGroupTypeMap.push_back(GroupType);
        total_loot_count++;
    }

    sLog.Log("       Loot_Table", "%u total loot definitions loaded", total_loot_count);
}

void DGM_Loot_Groups_Table::GetLoot(uint32 groupID, std::vector<DBLootGroupType> &lootList) {
    double start = GetTimeMSeconds();
    uint16 count1 = 0, count2 = 0;
    DBLootGroupType loot_list1;
    std::vector<DBLootGroupType> loot_list2;
    std::vector<DBLootGroup>::iterator cur = begin(m_LootGroupMap);
    std::vector<DBLootGroup>::iterator last = end(m_LootGroupMap);
    std::vector<DBLootGroupType>::iterator cur2 = begin(m_LootGroupTypeMap);
    std::vector<DBLootGroupType>::iterator last2 = end(m_LootGroupTypeMap);
    while (cur != last) {
        if (cur->groupID == groupID) {
            if (rand() < cur->dropChance) {
                while (cur2 != last2) {
                    if (cur2->lootGroupID = cur->lootGroupID) {
                        loot_list1.lootGroupID = cur2->lootGroupID;
                        loot_list1.typeID = cur2->typeID;
                        loot_list1.chance = cur2->chance;
                        loot_list1.minQuantity = cur2->minQuantity;
                        loot_list1.maxQuantity = cur2->maxQuantity;
                        loot_list2.push_back(loot_list1);
                    }
                    cur2++;
                }
            }
        }
        cur++;
        count1++;
    }

    if (!loot_list2.empty()) {
        uint16 random = rand() % 100;
        float lootChance = 0;
        std::vector<DBLootGroupType>::iterator cur3 = begin(loot_list2);
        std::vector<DBLootGroupType>::iterator last3 = end(loot_list2);
        while (cur3 != last3) {
            lootChance += cur3->chance;
            if (random < lootChance) {
                loot_list1.lootGroupID = cur3->lootGroupID;
                loot_list1.typeID = cur3->typeID;
                loot_list1.chance = cur3->chance;
                loot_list1.minQuantity = cur3->minQuantity;
                loot_list1.maxQuantity = cur3->maxQuantity;
                lootList.push_back(loot_list1);
                count2++;
            }
        }
        cur3++;
    }

    double timer = (GetTimeMSeconds() - start);
    sLog.Log("        GetLoot()", "After iterating thru %u loops, Loot time is %f s for a total of %u loot items returned", count1, timer, count2);
}
