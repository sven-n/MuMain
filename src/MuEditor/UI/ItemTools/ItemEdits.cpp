#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEdits.h"

#include "ItemSelection.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDefinition;
using Data::Items::ItemModelDefinition;
using Data::Items::SharedItemModel;

ItemModelDefinition NoModel(int itemType)
{
    ItemModelDefinition model;
    model.group = Data::Items::GetItemGroup(itemType);
    model.number = Data::Items::GetItemNumber(itemType);
    return model;
}

void RestoreSlots(std::span<const ItemSlotState> states)
{
    std::vector<ItemDefinition> items;
    items.reserve(states.size());
    for (const ItemSlotState& state : states)
    {
        items.push_back(state.item);
    }
    g_ItemDataHandler.RestoreItems(items);
    for (const ItemSlotState& state : states)
    {
        const int itemType = Data::Items::MakeItemType(state.item.group, state.item.number);
        g_ItemDataHandler.SetItemModel(state.model ? *state.model : NoModel(itemType));
    }
}

std::optional<SharedItemModel> ReadSharedModel(const std::string& name)
{
    const SharedItemModel* model = g_ItemModelDatabase.FindSharedModel(name);
    return model != nullptr ? std::optional<SharedItemModel>(*model) : std::nullopt;
}

void RestoreSharedModel(const std::string& name, const std::optional<SharedItemModel>& model)
{
    if (model)
    {
        g_ItemDataHandler.SetSharedModel(*model, name);
    }
    else
    {
        g_ItemDataHandler.RemoveSharedModel(name);
    }
}

std::vector<int> SharedModelUsers(std::string_view name)
{
    std::vector<int> users;
    for (const ItemModelDefinition& model : g_ItemModelDatabase.GetAllSlots())
    {
        if (model.Exists() && model.model == name)
        {
            users.push_back(Data::Items::MakeItemType(model.group, model.number));
        }
    }
    return users;
}
} // namespace

ItemEdits& ItemEdits::GetInstance()
{
    static ItemEdits instance;
    return instance;
}

ItemEdits::ItemEdits()
    : m_history({&ItemEdits::ReadSlot, &RestoreSlots, [](int from, int to)
                 { return g_ItemDataHandler.MoveItem(from, to); }, &ReadSharedModel, &RestoreSharedModel})
{
}

ItemSlotState ItemEdits::ReadSlot(int itemType)
{
    ItemSlotState state;
    if (!Data::Items::IsValidItemType(itemType))
    {
        return state;
    }
    state.item = g_ItemDatabase.GetAllSlots()[itemType];
    state.item.group = Data::Items::GetItemGroup(itemType);
    state.item.number = Data::Items::GetItemNumber(itemType);
    state.item.name.clear();
    if (const ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType))
    {
        state.model = *model;
    }
    return state;
}

template <typename TChange>
void ItemEdits::RecordSlots(std::string label, std::span<const int> itemTypes, TChange&& change)
{
    std::vector<ItemSlotState> before;
    before.reserve(itemTypes.size());
    for (const int itemType : itemTypes)
    {
        before.push_back(ReadSlot(itemType));
    }
    change();

    ItemEditEntry entry{std::move(label), {}};
    for (size_t index = 0; index < itemTypes.size(); ++index)
    {
        ItemSlotState after = ReadSlot(itemTypes[index]);
        if (after != before[index])
        {
            entry.steps.push_back({ItemEditStep::Kind::Slot, itemTypes[index], 0, std::move(before[index]),
                                   std::move(after), std::nullopt, std::nullopt});
        }
    }
    m_history.Record(std::move(entry));
}

void ItemEdits::SetItem(std::string label, const ItemDefinition& item)
{
    SetItems(std::move(label), std::span<const ItemDefinition>(&item, 1));
}

void ItemEdits::SetItems(std::string label, std::span<const ItemDefinition> items)
{
    std::vector<int> itemTypes;
    for (const ItemDefinition& item : items)
    {
        const int itemType = Data::Items::MakeItemType(item.group, item.number);
        if (Data::Items::IsValidItemId(item.group, item.number) &&
            std::ranges::find(itemTypes, itemType) == itemTypes.end())
        {
            itemTypes.push_back(itemType);
        }
    }
    RecordSlots(std::move(label), itemTypes, [&] { g_ItemDataHandler.SetItems(items); });
}

bool ItemEdits::SetItemName(std::string label, int itemType, std::string_view locale, std::string text)
{
    bool stored = false;
    const int itemTypes[] = {itemType};
    RecordSlots(std::move(label), itemTypes,
                [&] { stored = g_ItemDataHandler.SetItemName(itemType, locale, std::move(text)); });
    return stored;
}

bool ItemEdits::AddItem(std::string label, const ItemDefinition& item, const ItemModelDefinition* model)
{
    bool added = false;
    const int itemTypes[] = {Data::Items::MakeItemType(item.group, item.number)};
    RecordSlots(std::move(label), itemTypes, [&] { added = g_ItemDataHandler.AddItem(item, model); });
    return added;
}

bool ItemEdits::RemoveItem(std::string label, int itemType)
{
    bool removed = false;
    const int itemTypes[] = {itemType};
    RecordSlots(std::move(label), itemTypes, [&] { removed = g_ItemDataHandler.RemoveItem(itemType); });
    return removed;
}

bool ItemEdits::MoveItem(std::string label, int fromItemType, int toItemType)
{
    if (!g_ItemDataHandler.MoveItem(fromItemType, toItemType))
    {
        return false;
    }
    m_history.Record(
        {std::move(label), {{ItemEditStep::Kind::Move, fromItemType, toItemType, {}, {}, std::nullopt, std::nullopt}}});
    return true;
}

void ItemEdits::SetItemModel(std::string label, const ItemModelDefinition& model)
{
    const int itemTypes[] = {Data::Items::MakeItemType(model.group, model.number)};
    RecordSlots(std::move(label), itemTypes, [&] { g_ItemDataHandler.SetItemModel(model); });
}

void ItemEdits::SetSharedModel(std::string label, const SharedItemModel& sharedModel, std::string_view previousName)
{
    const std::string oldName = previousName.empty() ? sharedModel.name : std::string(previousName);
    const std::optional<SharedItemModel> before = ReadSharedModel(oldName);
    const std::vector<int> users = SharedModelUsers(oldName);
    std::vector<ItemSlotState> usersBefore;
    for (const int user : users)
    {
        usersBefore.push_back(ReadSlot(user));
    }

    g_ItemDataHandler.SetSharedModel(sharedModel, oldName);

    ItemEditEntry entry{std::move(label), {}};
    entry.steps.push_back({ItemEditStep::Kind::SharedModel, 0, 0, {}, {}, before, ReadSharedModel(sharedModel.name)});
    for (size_t index = 0; index < users.size(); ++index)
    {
        ItemSlotState after = ReadSlot(users[index]);
        if (after != usersBefore[index])
        {
            entry.steps.push_back({ItemEditStep::Kind::Slot, users[index], 0, std::move(usersBefore[index]),
                                   std::move(after), std::nullopt, std::nullopt});
        }
    }
    m_history.Record(std::move(entry));
}

bool ItemEdits::RemoveSharedModel(std::string label, std::string_view name)
{
    const std::optional<SharedItemModel> before = ReadSharedModel(std::string(name));
    if (!before || !g_ItemDataHandler.RemoveSharedModel(name))
    {
        return false;
    }
    m_history.Record({std::move(label), {{ItemEditStep::Kind::SharedModel, 0, 0, {}, {}, before, std::nullopt}}});
    return true;
}

bool ItemEdits::Undo()
{
    if (!m_history.Undo())
    {
        return false;
    }
    if (const std::optional<int> changed = m_history.GetLastChangedItem())
    {
        g_ItemSelection.Select(*changed);
    }
    return true;
}

bool ItemEdits::Redo()
{
    if (!m_history.Redo())
    {
        return false;
    }
    if (const std::optional<int> changed = m_history.GetLastChangedItem())
    {
        g_ItemSelection.Select(*changed);
    }
    return true;
}
} // namespace MuEditor::Items

#endif // _EDITOR
