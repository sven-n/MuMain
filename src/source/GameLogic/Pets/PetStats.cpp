#include "stdafx.h"

#include "GameLogic/Pets/PetStats.h"

#include "Data/GameData/ItemData/ItemDatabase.h"

namespace GameLogic::Pets
{
const Data::Items::ItemPetStats& GetItemPetStats(int itemType)
{
    static const Data::Items::ItemPetStats None;
    const Data::Items::ItemDefinition* definition = g_ItemDatabase.Find(itemType);
    return definition != nullptr ? definition->petStats : None;
}
} // namespace GameLogic::Pets
