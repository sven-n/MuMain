#include "stdafx.h"

#ifdef _EDITOR

#include "ItemUsage.h"

#include "ItemCodeReferenceIndex.h"
#include "ItemUsers.h"

#include "Core/Utilities/_GlobalFunctions.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "GameLogic/Buffs/w_BuffStateSystem.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"
#include "GameLogic/Pets/w_PetProcess.h"
#include "I18N/All.h"
#include "UI/Core/UIManager.h"

namespace MuEditor::Items
{
namespace
{
void FindTags(int itemType, ItemUsage& usage)
{
    const Data::Items::ItemDefinition* item = g_ItemDatabase.Find(itemType);
    if (item == nullptr)
    {
        return;
    }
    for (int tag = 0; tag < static_cast<int>(Data::Items::ItemTag::Count); ++tag)
    {
        if (item->tags.Has(static_cast<Data::Items::ItemTag>(tag)))
        {
            usage.tags.push_back(static_cast<Data::Items::ItemTag>(tag));
        }
    }
}

void FindModel(int itemType, ItemUsage& usage)
{
    const Data::Items::ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
    usage.hasModel = model != nullptr;
    if (model == nullptr || model->model.empty())
    {
        return;
    }
    usage.sharedModel = model->model;
    usage.sharedModelOtherUsers = FindSharedModelUsers(model->model).size() - 1;
}

// Each of these game objects exists once the game has started; before that
// (and in tests) its data is not known and does not count.
void FindGameData(int itemType, ItemUsage& usage)
{
    if (CSItemOption::IsInitialized())
    {
        // GetSetItemName reads set number 0 and 255 as no set; IsChangeSetItem
        // only 255, while the shipped set file writes 0.
        wchar_t setName[MAX_ITEM_SET_NAME + 2]{};
        usage.inAncientSet =
            g_csItemOption.GetSetItemName(setName, itemType, 1) || g_csItemOption.GetSetItemName(setName, itemType, 2);
    }
    if (g_pItemAddOptioninfo != nullptr)
    {
        const ITEM_ADD_OPTION& addOption = g_pItemAddOptioninfo->GetItemAddOtioninfo(itemType);
        usage.hasAddOption = addOption.m_byOption1 != 0 || addOption.m_byOption2 != 0;
    }
    if (g_BuffSystem)
    {
        usage.givesBuff = TheBuffStateSystem().GetBuffInfo().GetBuffIndex(itemType) >= 0;
    }
    if (g_petProcess)
    {
        usage.isPet = ThePetProcess().IsPet(itemType);
    }
}
} // namespace

ItemUsage FindItemUsage(int itemType)
{
    ItemUsage usage;
    if (!Data::Items::IsValidItemType(itemType))
    {
        return usage;
    }
    FindTags(itemType, usage);
    FindModel(itemType, usage);
    const ItemCodeReferences& references = GetItemCodeReferences();
    usage.codeKnown = references.IsLoaded();
    usage.code = references.Find(itemType);
    FindGameData(itemType, usage);
    return usage;
}

ItemNumberHolder FindItemNumberHolder(int itemType)
{
    ItemNumberHolder holder;
    if (!Data::Items::IsValidItemType(itemType))
    {
        return holder;
    }
    if (g_ItemDatabase.Find(itemType) != nullptr)
    {
        holder.item = g_ItemDatabase.GetLogName(itemType);
        return holder;
    }
    holder.entryWithoutName = g_ItemDatabase.FindEntry(itemType) != nullptr;
    const Data::Items::ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
    if (model == nullptr && !holder.entryWithoutName)
    {
        return holder;
    }
    if (model != nullptr)
    {
        holder.model = model->model.empty() ? model->file : model->model;
    }
    holder.codeName = GetItemCodeReferences().GetCodeName(itemType);
    return holder;
}

std::string GetEntryLabel(int itemType)
{
    if (g_ItemDatabase.Find(itemType) != nullptr || g_ItemDatabase.FindEntry(itemType) == nullptr)
    {
        return g_ItemDatabase.GetLogName(itemType);
    }
    return std::string(I18N::Editor::EntryWithoutAName) + " (" + std::to_string(Data::Items::GetItemGroup(itemType)) +
           "," + std::to_string(Data::Items::GetItemNumber(itemType)) + ")";
}

const char* GetNumberWithoutItemLabel(int itemType)
{
    return g_ItemDatabase.FindEntry(itemType) != nullptr ? I18N::Editor::EntryWithoutAName
                                                         : I18N::Editor::NoItemAtThisNumber;
}
} // namespace MuEditor::Items

#endif // _EDITOR
