#include "stdafx.h"

#ifdef _EDITOR

#include "ItemHeldByCharacter.h"

#include "Engine/Object/ZzzInfomation.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/MyInventory.h"

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
    const mu::ui::window::CMyInventory* inventory = g_pMyInventory;
    return inventory != nullptr && inventory->FindItemIndex(static_cast<short>(itemType)) >= 0;
}
} // namespace MuEditor::Items

#endif // _EDITOR
