#include "stdafx.h"

#ifdef _EDITOR

#include "ItemHeldByCharacter.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Models/ZzzBMD.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/ItemMng.h"
#include "UI/Social/FriendWindow.h"
#include "UI/Social/LetterReadWindow.h"
#include "UI/Social/LetterWriteWindow.h"

#include <algorithm>

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
    // The photo characters of the letter windows, also of hidden ones.
    const mu::ui::window::CFriendWindow* friends = g_pWindowMgr;
    const CUIWindowMgr* windows = friends != nullptr ? friends->GetWindowMgr() : nullptr;
    if (windows == nullptr)
    {
        return;
    }
    for (const auto& entry : windows->GetWindows())
    {
        CUIBaseWindow* window = entry.second;
        if (auto* read = dynamic_cast<CUILetterReadWindow*>(window))
        {
            DeleteCloth(read->m_Photo.GetPhotoChar());
        }
        else if (auto* write = dynamic_cast<CUILetterWriteWindow*>(window))
        {
            DeleteCloth(write->m_Photo.GetPhotoChar());
        }
    }
}

void PrepareClothMeshes(std::span<const int> itemTypes)
{
    if (Models == nullptr || std::ranges::find(itemTypes, ITEM_GRAND_SOUL_PANTS) == itemTypes.end())
    {
        return;
    }
    // Not for a slot left without a model (the pants moved away or removed).
    if (Models[MODEL_GRAND_SOUL_PANTS].NumMeshs > 0)
    {
        PrepareGrandSoulPantsClothMesh();
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
