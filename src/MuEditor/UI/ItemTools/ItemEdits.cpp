#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEdits.h"

#include "ItemHeldByCharacter.h"
#include "ItemSelection.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Render/Items/LookDrawing.h"

#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <algorithm>
#include <memory>
#include <set>
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

std::vector<Data::Items::PartModel> ReadPartModels()
{
    const std::span<const Data::Items::PartModel> parts = g_ItemModelDatabase.GetPartModels();
    return {parts.begin(), parts.end()};
}

void RestorePartModels(const std::vector<Data::Items::PartModel>& parts)
{
    g_ItemDataHandler.SetPartModels(parts);
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

Data::Items::ItemNameCases ReadNameCases()
{
    return g_ItemDatabase.GetNameCases();
}

void RestoreNameCases(const Data::Items::ItemNameCases& cases)
{
    g_ItemDataHandler.SetNameCases(cases);
}

std::vector<Data::Items::GroundLabelStyle> ReadGroundLabelStyles()
{
    return g_ItemDatabase.GetGroundLabels().styles;
}

void RestoreGroundLabelStyles(std::span<const Data::Items::GroundLabelStyle> styles)
{
    g_ItemDataHandler.RestoreGroundLabelStyles(styles);
}

std::vector<Data::Colors::ColorFolder> ReadColorFolders(std::string_view root)
{
    if (root == ItemEdits::GlowFolders)
    {
        return g_GlowColors.GetFolders();
    }
    return g_ItemDatabase.GetGroundLabels().folders;
}

void RestoreColorFolders(std::string_view root, std::span<const Data::Colors::ColorFolder> folders)
{
    std::vector<Data::Colors::ColorFolder> copy(folders.begin(), folders.end());
    if (root == ItemEdits::GlowFolders)
    {
        g_ItemDataHandler.SetGlowColorFolders(std::move(copy));
    }
    else
    {
        g_ItemDataHandler.SetGroundLabelFolders(std::move(copy));
    }
}

Data::Looks::LookFile ReadLookFile(Data::Looks::LookKind kind)
{
    return g_ItemDataHandler.GetLookFile(kind);
}

// The game draws the looks at once, with the textures they name.
void RestoreLookFile(Data::Looks::LookKind kind, const Data::Looks::LookFile& file)
{
    g_ItemDataHandler.SetLookFile(kind, file);
    for (const Data::Looks::LookDefinition& look : g_ItemDataHandler.GetLookFile(kind).looks)
    {
        Render::Items::Looks::PrepareTextures(look);
    }
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
    : m_history({&ItemEdits::ReadSlot,
                 &RestoreSlots,
                 [](int from, int to) { return g_ItemDataHandler.MoveItem(from, to); },
                 &ReadSharedModel,
                 &RestoreSharedModel,
                 &IsHeldByCharacter,
                 &ReadGlowColors,
                 &RestoreGlowColors,
                 &ReadStatFile,
                 &RestoreStatFile,
                 &ReadGroundLabelStyles,
                 &RestoreGroundLabelStyles,
                 &ReadColorFolders,
                 &RestoreColorFolders,
                 &ReadLookFile,
                 &RestoreLookFile,
                 [](int number) { return g_MonsterDataHandler.ReadMonster(number); },
                 [](std::span<const Data::Monsters::MonsterSnapshot> snapshots)
                 { g_MonsterDataHandler.RestoreMonsters(snapshots); },
                 [](int id) { return g_MonsterDataHandler.ReadGroup(id); },
                 [](std::span<const Data::Monsters::MonsterGroupSnapshot> snapshots)
                 { g_MonsterDataHandler.RestoreGroups(snapshots); },
                 [this](const ItemEditStep& step, bool undoing) { NoteMonsterItems(step, undoing); },
                 &ReadNameCases,
                 &RestoreNameCases,
                 &ReadPartModels,
                 &RestorePartModels})
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

bool ItemEdits::MoveItem(std::string label, int fromItemType, int toItemType, bool updateMonsters)
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
    ItemEditEntry entry{std::move(label),
                        {{ItemEditStep::Kind::Move, fromItemType, toItemType, std::move(before), ReadSlot(toItemType),
                          std::nullopt, std::nullopt}}};
    if (updateMonsters)
    {
        if (std::optional<ItemEditStep> monsters = UpdateMonsterItems(fromItemType, toItemType))
        {
            entry.steps.push_back(std::move(*monsters));
        }
    }
    Record(std::move(entry));
    return true;
}

std::optional<ItemEditStep> ItemEdits::UpdateMonsterItems(int fromItemType, int toItemType)
{
    ItemEditStep step{ItemEditStep::Kind::MonsterItems};
    std::set<int> groups;
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster == nullptr)
        {
            continue;
        }
        Data::Monsters::MonsterDefinition changed = *monster;
        if (Data::Monsters::ReplaceItemReferences(changed, fromItemType, toItemType) == 0)
        {
            continue;
        }
        step.monstersBefore.push_back({number, *monster});
        step.monstersAfter.push_back({number, std::move(changed)});
        if (const std::optional<int> group = g_MonsterDataHandler.FindGroupOf(number))
        {
            groups.insert(*group);
        }
    }
    if (step.monstersBefore.empty())
    {
        return std::nullopt;
    }
    for (const int id : groups)
    {
        std::optional<Data::Monsters::MonsterGroupLayout> group = g_MonsterDataHandler.ReadGroup(id);
        if (!group)
        {
            continue;
        }
        Data::Monsters::MonsterGroupLayout changed = *group;
        if (Data::Monsters::ReplaceItemReferences(changed.shared, fromItemType, toItemType) == 0)
        {
            continue;
        }
        step.monsterGroupsBefore.push_back({id, std::move(group)});
        step.monsterGroupsAfter.push_back({id, std::move(changed)});
    }
    g_MonsterDataHandler.RestoreMonsters(step.monstersAfter);
    g_MonsterDataHandler.RestoreGroups(step.monsterGroupsAfter);
    NoteMonsterItems(step, false);
    return step;
}

void ItemEdits::NoteMonsterItems(const ItemEditStep& step, bool undoing)
{
    using Data::Monsters::MonsterItemChange;
    using Data::Monsters::MonsterItemPlace;
    // The places in their order; a step changes only the items' numbers, so
    // both sides have the same places.
    const auto itemsOf = [](const std::optional<Data::Monsters::MonsterDefinition>& monster)
    {
        std::vector<std::pair<MonsterItemPlace, int>> items;
        if (monster)
        {
            Data::Monsters::ForEachMonsterItem(
                *monster, [&](const Data::Monsters::MonsterItem& item, const MonsterItemPlace& place)
                { items.emplace_back(place, Data::Items::MakeItemType(item.group, item.number)); });
        }
        return items;
    };
    for (size_t i = 0; i < step.monstersBefore.size() && i < step.monstersAfter.size(); ++i)
    {
        const auto before = itemsOf(step.monstersBefore[i].monster);
        const auto after = itemsOf(step.monstersAfter[i].monster);
        const int number = step.monstersBefore[i].number;
        for (size_t place = 0; place < before.size() && place < after.size(); ++place)
        {
            if (before[place].first != after[place].first || before[place].second == after[place].second)
            {
                continue;
            }
            const int from = undoing ? after[place].second : before[place].second;
            const int to = undoing ? before[place].second : after[place].second;
            const auto pending =
                std::ranges::find_if(m_monsterItems, [&](const MonsterItemChange& change)
                                     { return change.number == number && change.place == before[place].first; });
            if (pending == m_monsterItems.end())
            {
                m_monsterItems.push_back({number, before[place].first, from, to});
            }
            else if (pending->saved == to)
            {
                m_monsterItems.erase(pending);
            }
            else
            {
                pending->current = to;
            }
        }
    }
}

Data::Items::ItemDataSaveResult ItemEdits::Save(std::vector<Data::Items::ItemDataIssue>& issues)
{
    // The places the table still names as the moves left them.
    std::vector<Data::Monsters::MonsterItemChange> changes;
    for (const Data::Monsters::MonsterItemChange& change : m_monsterItems)
    {
        const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(change.number);
        if (monster != nullptr && Data::Monsters::FindItemAt(*monster, change.place) == change.current)
        {
            changes.push_back(change);
        }
    }
    std::optional<std::string> monsters;
    if (!changes.empty())
    {
        bool changed = false;
        const size_t first = issues.size();
        monsters = g_MonsterDataHandler.MakeMonstersFileWith(
            [&](std::vector<Data::Monsters::MonsterGroup>& groups)
            {
                changed = Data::Monsters::ApplyMonsterItemChanges(groups, changes);
                return changed;
            },
            issues);
        // The file cannot be read, or would not read back: no file is written,
        // so the items and the monsters stay alike.
        const bool failed = std::any_of(issues.begin() + static_cast<std::ptrdiff_t>(first), issues.end(),
                                        [](const Data::Items::ItemDataIssue& issue)
                                        { return issue.severity == Data::Items::ItemDataIssueSeverity::Fatal; });
        if (failed || (changed && !monsters))
        {
            return Data::Items::ItemDataSaveResult::InvalidData;
        }
    }
    std::vector<Data::TextFileToWrite> alsoWrite;
    if (monsters)
    {
        alsoWrite.push_back({Data::Monsters::GetMonstersFile(), *monsters});
    }
    const Data::Items::ItemDataSaveResult result = g_ItemDataHandler.Save(issues, alsoWrite);
    if (result == Data::Items::ItemDataSaveResult::Saved)
    {
        m_history.MarkSaved();
        m_monsterItems.clear();
    }
    return result;
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

void ItemEdits::AddGlowColor(std::string label, const Data::Effects::GlowColor& color,
                             std::span<const std::size_t> folder)
{
    std::vector<Data::Effects::GlowColor> before = ReadGlowColors();
    std::vector<Data::Colors::ColorFolder> foldersBefore = ReadColorFolders(GlowFolders);
    g_ItemDataHandler.SetGlowColor(color);
    std::vector<Data::Effects::GlowColor> after = ReadGlowColors();
    if (after == before)
    {
        return;
    }
    std::vector<Data::Colors::ColorFolder> folders = foldersBefore;
    if (Data::Colors::MoveColorInto(folders, color.name, folder))
    {
        RestoreColorFolders(GlowFolders, folders);
    }
    ItemEditStep step{ItemEditStep::Kind::GlowColors};
    step.colorsBefore = std::move(before);
    step.colorsAfter = std::move(after);
    ItemEditEntry entry{std::move(label), {std::move(step)}};
    AddFolderStep(entry, GlowFolders, std::move(foldersBefore));
    Record(std::move(entry));
}

bool ItemEdits::RemoveGlowColors(std::string label, std::span<const std::string> names)
{
    std::vector<Data::Effects::GlowColor> before = ReadGlowColors();
    std::vector<Data::Colors::ColorFolder> foldersBefore = ReadColorFolders(GlowFolders);
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
    ItemEditEntry entry{std::move(label), {std::move(step)}};
    AddFolderStep(entry, GlowFolders, std::move(foldersBefore));
    Record(std::move(entry));
    return true;
}

void ItemEdits::SetColorFolders(std::string label, std::string_view root,
                                std::vector<Data::Colors::ColorFolder> folders)
{
    std::vector<Data::Colors::ColorFolder> before = ReadColorFolders(root);
    // The same folders change nothing: the file isn't marked changed, and there is no step.
    if (folders == before)
    {
        return;
    }
    RestoreColorFolders(root, folders);
    ItemEditEntry entry{std::move(label), {}};
    AddFolderStep(entry, root, std::move(before));
    Record(std::move(entry));
}

void ItemEdits::SetRenderStyles(std::string label, Data::Looks::RenderStylesFile file)
{
    SetLookFile(std::move(label), Data::Looks::LookKind::RenderStyle, std::move(file));
}

void ItemEdits::SetLookFile(std::string label, Data::Looks::LookKind kind, Data::Looks::LookFile file)
{
    // Compared as the item data keeps the file, its looks sorted by name: a file as it is changes nothing, so it
    // neither marks the file for Save nor counts as a change of the looks.
    std::ranges::sort(file.looks, {}, &Data::Looks::LookDefinition::name);
    auto before = std::make_shared<const Data::Looks::LookFile>(ReadLookFile(kind));
    if (*before == file)
    {
        return;
    }
    RestoreLookFile(kind, file);
    auto after = std::make_shared<const Data::Looks::LookFile>(ReadLookFile(kind));
    ItemEditStep step{ItemEditStep::Kind::RenderStyles};
    step.lookKind = kind;
    step.renderStylesBefore = std::move(before);
    step.renderStylesAfter = std::move(after);
    Record({std::move(label), {std::move(step)}});
}

void ItemEdits::SetPartModels(std::string label, std::vector<Data::Items::PartModel> parts)
{
    std::vector<Data::Items::PartModel> before = ReadPartModels();
    if (parts == before)
    {
        return;
    }
    RestorePartModels(parts);
    ItemEditStep step{ItemEditStep::Kind::PartModels};
    step.partModelsBefore = std::move(before);
    step.partModelsAfter = ReadPartModels();
    Record({std::move(label), {std::move(step)}});
}

void ItemEdits::SetGlowColorOrder(std::string label, std::span<const Data::Effects::GlowColor> colors)
{
    std::vector<Data::Effects::GlowColor> before = ReadGlowColors();
    g_ItemDataHandler.RestoreGlowColors(colors);
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

void ItemEdits::SetGroundLabelStyleOrder(std::string label, std::span<const Data::Items::GroundLabelStyle> styles)
{
    std::vector<Data::Items::GroundLabelStyle> before = ReadGroundLabelStyles();
    std::vector<Data::Colors::ColorFolder> foldersBefore = ReadColorFolders(GroundLabelFolders);
    g_ItemDataHandler.RestoreGroundLabelStyles(styles);
    RecordGroundLabelStyles(std::move(label), std::move(before), std::move(foldersBefore));
}

void ItemEdits::AddFolderStep(ItemEditEntry& entry, std::string_view root,
                              std::vector<Data::Colors::ColorFolder> before)
{
    std::vector<Data::Colors::ColorFolder> after = ReadColorFolders(root);
    if (after == before)
    {
        return;
    }
    ItemEditStep step{ItemEditStep::Kind::ColorFolders};
    step.folderRoot = std::string(root);
    step.foldersBefore = std::move(before);
    step.foldersAfter = std::move(after);
    entry.steps.push_back(std::move(step));
}

void ItemEdits::SetNameCases(std::string label, const Data::Items::ItemNameCases& cases)
{
    Data::Items::ItemNameCases before = ReadNameCases();
    if (before == cases)
    {
        return;
    }
    g_ItemDataHandler.SetNameCases(cases);
    ItemEditStep step{ItemEditStep::Kind::NameCases};
    step.casesBefore = std::move(before);
    step.casesAfter = cases;
    Record({std::move(label), {std::move(step)}});
}

void ItemEdits::SetGroundLabelStyle(std::string label, const Data::Items::GroundLabelStyle& style)
{
    std::vector<Data::Items::GroundLabelStyle> before = ReadGroundLabelStyles();
    std::vector<Data::Colors::ColorFolder> foldersBefore = ReadColorFolders(GroundLabelFolders);
    g_ItemDataHandler.SetGroundLabelStyle(style);
    RecordGroundLabelStyles(std::move(label), std::move(before), std::move(foldersBefore));
}

void ItemEdits::AddGroundLabelStyle(std::string label, const Data::Items::GroundLabelStyle& style,
                                    std::span<const std::size_t> folder)
{
    std::vector<Data::Items::GroundLabelStyle> before = ReadGroundLabelStyles();
    std::vector<Data::Colors::ColorFolder> foldersBefore = ReadColorFolders(GroundLabelFolders);
    g_ItemDataHandler.SetGroundLabelStyle(style);
    if (ReadGroundLabelStyles() == before)
    {
        return;
    }
    std::vector<Data::Colors::ColorFolder> folders = foldersBefore;
    if (Data::Colors::MoveColorInto(folders, style.name, folder))
    {
        RestoreColorFolders(GroundLabelFolders, folders);
    }
    RecordGroundLabelStyles(std::move(label), std::move(before), std::move(foldersBefore));
}

bool ItemEdits::RemoveGroundLabelStyles(std::string label, std::span<const std::string> names)
{
    std::vector<Data::Items::GroundLabelStyle> before = ReadGroundLabelStyles();
    std::vector<Data::Colors::ColorFolder> foldersBefore = ReadColorFolders(GroundLabelFolders);
    for (const std::string& name : names)
    {
        g_ItemDataHandler.RemoveGroundLabelStyle(name);
    }
    return RecordGroundLabelStyles(std::move(label), std::move(before), std::move(foldersBefore));
}

bool ItemEdits::RecordGroundLabelStyles(std::string label, std::vector<Data::Items::GroundLabelStyle> before,
                                        std::vector<Data::Colors::ColorFolder> foldersBefore)
{
    std::vector<Data::Items::GroundLabelStyle> after = ReadGroundLabelStyles();
    if (after == before)
    {
        return false;
    }
    ItemEditStep step{ItemEditStep::Kind::GroundLabelStyles};
    step.stylesBefore = std::move(before);
    step.stylesAfter = std::move(after);
    ItemEditEntry entry{std::move(label), {std::move(step)}};
    AddFolderStep(entry, GroundLabelFolders, std::move(foldersBefore));
    Record(std::move(entry));
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
    if (m_groupDepth++ == 0)
    {
        m_group = ItemEditEntry{std::move(label), {}};
    }
}

void ItemEdits::EndGroup()
{
    if (m_groupDepth == 0 || --m_groupDepth > 0)
    {
        return;
    }
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
    // One step per item type, one for the glow colors, one for the item stat
    // file and one for the ground label styles: what changed twice keeps its
    // first "before" and its last "after".
    for (ItemEditStep& step : entry.steps)
    {
        const auto same = std::ranges::find_if(
            m_group->steps,
            [&](const ItemEditStep& grouped)
            {
                return grouped.kind == step.kind &&
                       ((step.kind == ItemEditStep::Kind::Slot && grouped.itemType == step.itemType) ||
                        step.kind == ItemEditStep::Kind::GlowColors || step.kind == ItemEditStep::Kind::StatFile ||
                        step.kind == ItemEditStep::Kind::GroundLabelStyles ||
                        (step.kind == ItemEditStep::Kind::RenderStyles && grouped.lookKind == step.lookKind) ||
                        step.kind == ItemEditStep::Kind::PartModels ||
                        (step.kind == ItemEditStep::Kind::ColorFolders && grouped.folderRoot == step.folderRoot));
            });
        if (same == m_group->steps.end())
        {
            m_group->steps.push_back(std::move(step));
        }
        else if (step.kind == ItemEditStep::Kind::GlowColors)
        {
            same->colorsAfter = std::move(step.colorsAfter);
        }
        else if (step.kind == ItemEditStep::Kind::GroundLabelStyles)
        {
            same->stylesAfter = std::move(step.stylesAfter);
        }
        else if (step.kind == ItemEditStep::Kind::ColorFolders)
        {
            same->foldersAfter = std::move(step.foldersAfter);
        }
        else if (step.kind == ItemEditStep::Kind::StatFile)
        {
            same->statFileAfter = std::move(step.statFileAfter);
        }
        else if (step.kind == ItemEditStep::Kind::RenderStyles)
        {
            same->renderStylesAfter = std::move(step.renderStylesAfter);
        }
        else if (step.kind == ItemEditStep::Kind::PartModels)
        {
            same->partModelsAfter = std::move(step.partModelsAfter);
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
