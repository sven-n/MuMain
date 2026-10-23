#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEdits.h"

#include "ItemHeldByCharacter.h"
#include "ItemSelection.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/EffectData/GlowColorList.h"
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

std::vector<Data::Effects::GlowColor> ReadGlowColors()
{
    const std::span<const Data::Effects::GlowColor> colors = g_GlowColors.GetColors();
    return {colors.begin(), colors.end()};
}

void RestoreGlowColors(std::span<const Data::Effects::GlowColor> colors)
{
    g_ItemDataHandler.RestoreGlowColors(colors);
}

Data::Items::ItemStatFile ReadStatFile()
{
    return g_ItemDataHandler.GetStatFile();
}

void RestoreStatFile(const Data::Items::ItemStatFile& file)
{
    g_ItemDataHandler.SetStatFile(file);
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
                 { return g_ItemDataHandler.MoveItem(from, to); }, &ReadSharedModel, &RestoreSharedModel,
                 &IsHeldByCharacter, &ReadGlowColors, &RestoreGlowColors, &ReadStatFile, &RestoreStatFile})
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
    Record(std::move(entry));
}

bool ItemEdits::SetItem(std::string label, const ItemDefinition& item)
{
    return SetItems(std::move(label), std::span<const ItemDefinition>(&item, 1));
}

bool ItemEdits::SetItems(std::string label, std::span<const ItemDefinition> items,
                         Data::Items::StatProfileStore profiles)
{
    std::vector<int> itemTypes;
    for (const ItemDefinition& item : items)
    {
        if (!Data::Items::IsValidItemId(item.group, item.number))
        {
            continue;
        }
        const int itemType = Data::Items::MakeItemType(item.group, item.number);
        // A form disables these fields for a held item, but a field typed in
        // before the item was taken is still stored when it is left or drawn
        // again. The whole change is refused, as an undo refuses a whole entry.
        if (!KeepsShape(g_ItemDatabase.GetAllSlots()[itemType], item) && IsHeldByCharacter(itemType))
        {
            return false;
        }
        if (std::ranges::find(itemTypes, itemType) == itemTypes.end())
        {
            itemTypes.push_back(itemType);
        }
    }
    RecordSlots(std::move(label), itemTypes, [&] { g_ItemDataHandler.SetItems(items, profiles); });
    return true;
}

bool ItemEdits::SetItemName(std::string label, int itemType, std::string_view locale, std::string text)
{
    bool stored = false;
    const int itemTypes[] = {itemType};
    RecordSlots(std::move(label), itemTypes,
                [&] { stored = g_ItemDataHandler.SetItemName(itemType, locale, std::move(text)); });
    return stored;
}

bool ItemEdits::AddItem(std::string label, const ItemDefinition& item, const ItemModelDefinition* model,
                        const ItemDefinition* copiedFrom)
{
    bool added = false;
    const int itemTypes[] = {Data::Items::MakeItemType(item.group, item.number)};
    RecordSlots(std::move(label), itemTypes, [&] { added = g_ItemDataHandler.AddItem(item, model, copiedFrom); });
    return added;
}

bool ItemEdits::RemoveItem(std::string label, int itemType)
{
    if (IsHeldByCharacter(itemType))
    {
        return false;
    }
    bool removed = false;
    const int itemTypes[] = {itemType};
    RecordSlots(std::move(label), itemTypes, [&] { removed = g_ItemDataHandler.RemoveItem(itemType); });
    return removed;
}

bool ItemEdits::MoveItem(std::string label, int fromItemType, int toItemType)
{
    if (IsHeldByCharacter(fromItemType))
    {
        return false;
    }
    ItemSlotState before = ReadSlot(fromItemType);
    if (!g_ItemDataHandler.MoveItem(fromItemType, toItemType))
    {
        return false;
    }
    Record({std::move(label),
            {{ItemEditStep::Kind::Move, fromItemType, toItemType, std::move(before), ReadSlot(toItemType), std::nullopt,
              std::nullopt}}});
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
    Record(std::move(entry));
}

bool ItemEdits::RemoveSharedModel(std::string label, std::string_view name)
{
    const std::optional<SharedItemModel> before = ReadSharedModel(std::string(name));
    if (!before || !g_ItemDataHandler.RemoveSharedModel(name))
    {
        return false;
    }
    Record({std::move(label), {{ItemEditStep::Kind::SharedModel, 0, 0, {}, {}, before, std::nullopt}}});
    return true;
}

void ItemEdits::SetGlowColor(std::string label, const Data::Effects::GlowColor& color)
{
    std::vector<Data::Effects::GlowColor> before = ReadGlowColors();
    g_ItemDataHandler.SetGlowColor(color);
    std::vector<Data::Effects::GlowColor> after = ReadGlowColors();
    if (after == before)
    {
        return;
    }
    ItemEditStep step{ItemEditStep::Kind::GlowColors};
    step.colorsBefore = std::move(before);
    step.colorsAfter = std::move(after);
    Record({std::move(label), {std::move(step)}});
}

bool ItemEdits::RemoveGlowColors(std::string label, std::span<const std::string> names)
{
    std::vector<Data::Effects::GlowColor> before = ReadGlowColors();
    for (const std::string& name : names)
    {
        g_ItemDataHandler.RemoveGlowColor(name);
    }
    std::vector<Data::Effects::GlowColor> after = ReadGlowColors();
    if (after == before)
    {
        return false;
    }
    ItemEditStep step{ItemEditStep::Kind::GlowColors};
    step.colorsBefore = std::move(before);
    step.colorsAfter = std::move(after);
    Record({std::move(label), {std::move(step)}});
    return true;
}

void ItemEdits::SetStatFile(std::string label, const Data::Items::ItemStatFile& file,
                            std::span<const ItemDefinition> repointed)
{
    auto before = std::make_shared<const Data::Items::ItemStatFile>(g_ItemDataHandler.GetStatFile());
    std::vector<ItemSlotState> slotsBefore;
    for (const ItemDefinition& item : repointed)
    {
        slotsBefore.push_back(ReadSlot(Data::Items::MakeItemType(item.group, item.number)));
    }

    // The file first, so the items' new profile names resolve.
    g_ItemDataHandler.SetStatFile(file);
    g_ItemDataHandler.RestoreItems(repointed);

    ItemEditEntry entry{std::move(label), {}};
    auto after = std::make_shared<const Data::Items::ItemStatFile>(g_ItemDataHandler.GetStatFile());
    if (*after != *before)
    {
        ItemEditStep step{ItemEditStep::Kind::StatFile};
        step.statFileBefore = std::move(before);
        step.statFileAfter = std::move(after);
        entry.steps.push_back(std::move(step));
    }
    for (size_t index = 0; index < repointed.size(); ++index)
    {
        const int itemType = Data::Items::MakeItemType(repointed[index].group, repointed[index].number);
        ItemSlotState slotAfter = ReadSlot(itemType);
        if (slotAfter != slotsBefore[index])
        {
            entry.steps.push_back({ItemEditStep::Kind::Slot, itemType, 0, std::move(slotsBefore[index]),
                                   std::move(slotAfter), std::nullopt, std::nullopt});
        }
    }
    Record(std::move(entry));
}

void ItemEdits::BeginGroup(std::string label)
{
    if (!m_group)
    {
        m_group = ItemEditEntry{std::move(label), {}};
    }
}

void ItemEdits::EndGroup()
{
    if (m_group)
    {
        ItemEditEntry entry = std::move(*m_group);
        m_group.reset();
        m_history.Record(std::move(entry));
    }
}

void ItemEdits::Record(ItemEditEntry entry)
{
    if (!m_group)
    {
        m_history.Record(std::move(entry));
        return;
    }
    // One step per item type, one for the glow colors and one for the item
    // stat file: what changed twice keeps its first "before" and its last
    // "after".
    for (ItemEditStep& step : entry.steps)
    {
        const auto same = std::ranges::find_if(
            m_group->steps,
            [&](const ItemEditStep& grouped)
            {
                return grouped.kind == step.kind &&
                       ((step.kind == ItemEditStep::Kind::Slot && grouped.itemType == step.itemType) ||
                        step.kind == ItemEditStep::Kind::GlowColors || step.kind == ItemEditStep::Kind::StatFile);
            });
        if (same == m_group->steps.end())
        {
            m_group->steps.push_back(std::move(step));
        }
        else if (step.kind == ItemEditStep::Kind::GlowColors)
        {
            same->colorsAfter = std::move(step.colorsAfter);
        }
        else if (step.kind == ItemEditStep::Kind::StatFile)
        {
            same->statFileAfter = std::move(step.statFileAfter);
        }
        else
        {
            same->after = std::move(step.after);
        }
    }
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
