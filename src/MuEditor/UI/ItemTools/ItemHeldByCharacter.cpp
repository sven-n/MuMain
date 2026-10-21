#include "stdafx.h"

#ifdef _EDITOR

#include "ItemHeldByCharacter.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/ItemMng.h"

namespace MuEditor::Items
{
bool IsHeldByCharacter(int itemType)
{
    if (CharacterMachine != nullptr)
    {
        for (const ITEM& equipped : CharacterMachine->Equipment)
        {
            if (equipped.Type == itemType)
            {
                return true;
            }
        }
    }
    const mu::ui::window::CItemMng* windowItems = g_pNewItemMng;
    return windowItems != nullptr && windowItems->HasItemOfType(itemType);
}

std::vector<int> FindHeldItemTypes()
{
    std::vector<int> held;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        if (IsHeldByCharacter(itemType))
        {
            held.push_back(itemType);
        }
    }
    return held;
}

void DropWornCloth()
{
    if (CharactersClient != nullptr)
    {
        // Also the spare last one, which CreateCharacter hands out when all
        // are taken.
        for (int index = 0; index <= MAX_CHARACTERS_CLIENT; ++index)
        {
            DeleteCloth(&CharactersClient[index]);
        }
    }
    // The character of the character creation.
    DeleteCloth(&CharacterView);
}
} // namespace MuEditor::Items

#endif // _EDITOR
