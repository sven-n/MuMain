#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemFormulasUI.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemGroupNames.h"
#include "ItemSelection.h"
#include "ItemStatFileChange.h"
#include "ItemStatTermFields.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using Data::Items::ItemStat;
using Data::Items::ItemStatFile;
using Data::Items::LevelBonusTable;
using Data::Items::StatProfileRule;
using Data::Items::WrittenStatProfile;
using Data::Items::WrittenStatSection;
using Entry = CMuItemFormulasUI::Entry;

namespace
{
constexpr char ConfirmPopupId[] = "###StatFileChange";
constexpr char NamePopupId[] = "###StatFileName";
// Pixel widths before the editor UI scale.
constexpr float ListWidth = 240.0f;
constexpr float LevelValueWidth = 52.0f;
constexpr float UserListLines = 8.0f;
// The item levels shown in one row of a table's values.
constexpr int LevelsPerRow = 8;
constexpr int MaxDropLevelBonus = 1000;
constexpr int MaxValue = 65535;
const ImVec4 WarningColor(1.0f, 0.8f, 0.3f, 1.0f);
const ImVec4 ErrorColor(1.0f, 0.4f, 0.4f, 1.0f);

const ItemStatFile& Stored()
{
    return g_ItemDataHandler.GetStatFile();
}

template <typename TEntry> TEntry* FindNamed(std::vector<TEntry>& entries, std::string_view name)
{
    const auto found = std::ranges::find_if(entries, [name](const TEntry& entry) { return entry.name == name; });
    return found != entries.end() ? &*found : nullptr;
}

template <typename TEntry> const TEntry* FindNamed(const std::vector<TEntry>& entries, std::string_view name)
{
    const auto found = std::ranges::find_if(entries, [name](const TEntry& entry) { return entry.name == name; });
    return found != entries.end() ? &*found : nullptr;
}

// The stats of the default and the profiles that name the table (the item
// descriptions that name it are counted apart).
int CountTableUsers(const ItemStatFile& file, std::string_view name)
{
    int users = 0;
    const auto count = [&](const std::array<WrittenStatSection, Data::Items::ItemStatCount>& sections)
    {
        for (const WrittenStatSection& section : sections)
        {
            users += section.table && *section.table == name ? 1 : 0;
        }
    };
    count(file.defaults);
    for (const WrittenStatProfile& profile : file.profiles)
    {
        count(profile.stats);
    }
    return users;
}

void RenameTable(ItemStatFile& file, std::string_view from, const std::string& to)
{
    if (LevelBonusTable* table = FindNamed(file.tables, from))
    {
        table->name = to;
    }
    const auto rename = [&](std::array<WrittenStatSection, Data::Items::ItemStatCount>& sections)
    {
        for (WrittenStatSection& section : sections)
        {
            if (section.table && *section.table == from)
            {
                section.table = to;
            }
        }
    };
    rename(file.defaults);
    for (WrittenStatProfile& profile : file.profiles)
    {
        rename(profile.stats);
    }
}

std::string JoinNames(const std::vector<std::string>& names)
{
    std::string text;
    for (const std::string& name : names)
    {
        text += (text.empty() ? "" : ", ") + name;
    }
    return text;
}

// The stored file with the picked entry as edited.
ItemStatFile Compose(const ItemStatFile& stored, const ItemStatFile& edit, const Entry& entry)
{
    ItemStatFile file = stored;
    switch (entry.kind)
    {
    case Entry::Kind::DropLevelBonus:
        file.excellentDropLevelBonus = edit.excellentDropLevelBonus;
        file.ancientDropLevelBonus = edit.ancientDropLevelBonus;
        break;
    case Entry::Kind::Default:
        file.defaults = edit.defaults;
        file.defaultRequirements = edit.defaultRequirements;
        break;
    case Entry::Kind::Profile:
        if (WrittenStatProfile* profile = FindNamed(file.profiles, entry.name))
        {
            if (const WrittenStatProfile* edited = FindNamed(edit.profiles, entry.name))
            {
                *profile = *edited;
            }
        }
        break;
    case Entry::Kind::Table:
        if (LevelBonusTable* table = FindNamed(file.tables, entry.name))
        {
            if (const LevelBonusTable* edited = FindNamed(edit.tables, entry.name))
            {
                *table = *edited;
            }
        }
        break;
    case Entry::Kind::Rules:
        file.profileRules = edit.profileRules;
        break;
    case Entry::Kind::None:
        break;
    }
    return file;
}

bool HasValue(const Data::Items::ItemTagSet& values, Data::Items::ItemTag tag)
{
    return values.Has(tag);
}

void SetValue(Data::Items::ItemTagSet& values, Data::Items::ItemTag tag, bool on)
{
    values.Set(tag, on);
}

template <size_t Size, typename TEnum> bool HasValue(const std::bitset<Size>& values, TEnum value)
{
    const auto index = static_cast<size_t>(value);
    return index < Size && values.test(index);
}

template <size_t Size, typename TEnum> void SetValue(std::bitset<Size>& values, TEnum value, bool on)
{
    values.set(static_cast<size_t>(value), on);
}

// The names of the values a condition has, in the order of the enum's names.
template <typename TEnum, typename TSet> std::string DescribeValues(const TSet& values)
{
    std::vector<std::string> names;
    for (const Data::Items::EnumName<TEnum>& entry : Data::Items::GetEnumNames(TEnum{}))
    {
        if (HasValue(values, entry.value))
        {
            names.push_back(entry.name);
        }
    }
    return JoinNames(names);
}

std::string DescribeGroups(const std::bitset<MAX_ITEM_TYPE>& groups)
{
    std::vector<std::string> names;
    for (size_t group = 0; group < groups.size(); ++group)
    {
        if (groups.test(group))
        {
            names.push_back(std::to_string(group));
        }
    }
    return JoinNames(names);
}

// "slot: helm, boots; group: 15", or that it matches every item.
std::string DescribeConditions(const StatProfileRule& rule)
{
    std::vector<std::string> parts;
    const auto add = [&parts](const char* label, const std::string& values)
    {
        if (!values.empty())
        {
            parts.push_back(std::string(label) + ": " + values);
        }
    };
    add(I18N::Editor::Group, DescribeGroups(rule.groups));
    add(I18N::Metadata::Slot, DescribeValues<Data::Items::ItemSlot>(rule.slots));
    add(I18N::Editor::WingTier, DescribeValues<Data::Items::WingTier>(rule.wingTiers));
    add(I18N::Editor::WeaponKind, DescribeValues<Data::Items::WeaponKind>(rule.weaponKinds));
    add(I18N::Editor::Tags, DescribeValues<Data::Items::ItemTag>(rule.tags));
    add(I18N::Editor::BaseRequirementAbove0, DescribeValues<Data::Items::ItemRequirement>(rule.requirements));
    if (parts.empty())
    {
        return I18N::Editor::EveryItem;
    }
    std::string text;
    for (const std::string& part : parts)
    {
        text += (text.empty() ? "" : "; ") + part;
    }
    return text;
}

// A condition as a list to tick its values in; true when it changed.
template <typename TEnum, typename TSet> bool ConditionCombo(const char* label, TSet& values)
{
    bool changed = false;
    const std::string preview = DescribeValues<TEnum>(values);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
    if (ImGui::BeginCombo(label, preview.empty() ? I18N::Editor::Any : preview.c_str()))
    {
        for (const Data::Items::EnumName<TEnum>& entry : Data::Items::GetEnumNames(TEnum{}))
        {
            const bool on = HasValue(values, entry.value);
            if (ImGui::Selectable(entry.name, on, ImGuiSelectableFlags_NoAutoClosePopups))
            {
                SetValue(values, entry.value, !on);
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

// The item types of the stored items that match `rule` first in `rules`.
std::vector<int> FindRuleItems(std::span<const StatProfileRule> rules, int rule)
{
    std::vector<int> items;
    for (const Data::Items::ItemDefinition& item : g_ItemDatabase.GetAllSlots())
    {
        if (item.Exists() && Data::Items::FindMatchingRule(rules, item) == rule)
        {
            items.push_back(Data::Items::MakeItemType(item.group, item.number));
        }
    }
    return items;
}

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

// A small button that is off while the picked entry has unapplied changes,
// which its change of the stored file would drop.
bool SmallButtonUnlessUnapplied(const char* label, bool unapplied)
{
    ImGui::BeginDisabled(unapplied);
    const bool clicked = ImGui::SmallButton(label);
    ImGui::EndDisabled();
    if (unapplied && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::ApplyOrRevertTheChangesFirst);
    }
    return clicked;
}

// The items as a list; a click selects one.
void RenderUsers(const char* id, const std::vector<int>& users)
{
    if (!ImGui::BeginListBox(id, ImVec2(-FLT_MIN, UserListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        return;
    }
    for (const int user : users)
    {
        ImGui::PushID(user);
        if (ImGui::Selectable(g_ItemDatabase.GetLogName(user).c_str(), user == g_ItemSelection.GetItemType()))
        {
            g_ItemSelection.Select(user);
        }
        ImGui::PopID();
    }
    ImGui::EndListBox();
}

const char* StatLabel(ItemStat stat)
{
    switch (stat)
    {
    case ItemStat::Damage:
        return I18N::Editor::Damage;
    case ItemStat::MagicPower:
        return I18N::Metadata::MagicPower;
    case ItemStat::BlockRate:
        return I18N::Metadata::BlockRate;
    case ItemStat::Defense:
        return I18N::Metadata::Defense;
    case ItemStat::MagicDefense:
    default:
        return I18N::Metadata::MagicDefense;
    }
}

// The five stats and the requirements of the default (`defaults` nullptr) or of a profile.
bool RenderSections(std::array<WrittenStatSection, Data::Items::ItemStatCount>& sections,
                    Data::Items::WrittenRequirementSection& requirements, const ItemStatFile& stored, bool profile)
{
    bool changed = false;
    for (size_t stat = 0; stat < Data::Items::ItemStatCount; ++stat)
    {
        const auto kind = static_cast<ItemStat>(stat);
        ImGui::PushID(static_cast<int>(stat));
        if (ImGui::CollapsingHeader(StatLabel(kind), ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= FormulaFields::StatSection(kind, sections[stat], profile ? &stored.defaults[stat] : nullptr,
                                                  stored.tables);
        }
        ImGui::PopID();
    }
    if (ImGui::CollapsingHeader(I18N::Editor::Requirements, ImGuiTreeNodeFlags_DefaultOpen))
    {
        changed |= FormulaFields::RequirementSection(requirements, profile ? &stored.defaultRequirements : nullptr);
    }
    return changed;
}
} // namespace

CMuItemFormulasUI& CMuItemFormulasUI::GetInstance()
{
    static CMuItemFormulasUI instance;
    return instance;
}

void CMuItemFormulasUI::Show(const Entry& entry)
{
    m_picked = entry;
}

void CMuItemFormulasUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemFormulas, MuEditor::WindowIds::ItemFormulas, open))
    {
        RenderNotes();
        RenderUnused();
        ImGui::Separator();

        const ImVec2 listSize(ListWidth * g_MuEditorCore.GetUIScale(), 0.0f);
        if (ImGui::BeginChild("##Entries", listSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            RenderList();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("##Picked"))
        {
            RenderPicked();
        }
        ImGui::EndChild();
        RenderConfirm();
        RenderNameDialog();
    }
    ToolWindow::End();
}

void CMuItemFormulasUI::RenderNotes()
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(WarningColor, "%s", I18N::Editor::TheServerKeepsItsOwnFormulas);
    ImGui::PopTextWrapPos();
    RenderNote(I18N::Editor::ItemsKeepTheirValuesUntilMadeAgain);
}

// D26: a profile or table goes only while nothing uses it.
void CMuItemFormulasUI::RenderUnused()
{
    const ItemStatFile& stored = Stored();
    std::vector<std::string> profiles;
    for (const WrittenStatProfile& profile : stored.profiles)
    {
        if (m_counts.Items(profile.name) == 0 && m_counts.Rules(profile.name) == 0)
        {
            profiles.push_back(profile.name);
        }
    }
    std::vector<std::string> tables;
    for (const LevelBonusTable& table : stored.tables)
    {
        if (CountTableUsers(stored, table.name) == 0 && m_counts.DescriptionUsers(table.name).empty())
        {
            tables.push_back(table.name);
        }
    }
    const bool unapplied = HasUnappliedChanges();
    if (!profiles.empty())
    {
        RenderNote(I18N::Format(I18N::Editor::NoItemOrRuleUsesTheseProfiles0, {JoinNames(profiles)}).c_str());
        if (SmallButtonUnlessUnapplied((std::string(I18N::Editor::RemoveThem) + "##Profiles").c_str(), unapplied))
        {
            ItemStatFile file = stored;
            std::erase_if(file.profiles, [&](const WrittenStatProfile& profile)
                          { return std::ranges::find(profiles, profile.name) != profiles.end(); });
            Apply(std::move(file), I18N::Editor::RemoveUnusedStatProfiles);
        }
    }
    if (!tables.empty())
    {
        RenderNote(I18N::Format(I18N::Editor::NoStatOrItemDescriptionUsesTheseTables0, {JoinNames(tables)}).c_str());
        if (SmallButtonUnlessUnapplied((std::string(I18N::Editor::RemoveThem) + "##Tables").c_str(), unapplied))
        {
            ItemStatFile file = stored;
            std::erase_if(file.tables, [&](const LevelBonusTable& table)
                          { return std::ranges::find(tables, table.name) != tables.end(); });
            Apply(std::move(file), I18N::Editor::RemoveUnusedLevelBonusTables);
        }
    }
}

void CMuItemFormulasUI::RenderList()
{
    const ItemStatFile& stored = Stored();
    const bool unapplied = HasUnappliedChanges();
    const auto row = [this](const std::string& label, const Entry& entry, bool unused)
    {
        if (unused)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }
        if (ImGui::Selectable(label.c_str(), m_picked == entry, MuEditor::KeyboardSelection::RowFlags))
        {
            m_picked = entry;
        }
        if (unused)
        {
            ImGui::PopStyleColor();
        }
    };
    row(I18N::Format(I18N::Editor::DropLevelBonus01,
                     {std::to_string(stored.excellentDropLevelBonus), std::to_string(stored.ancientDropLevelBonus)}) +
            "###DropLevelBonus",
        {Entry::Kind::DropLevelBonus, {}}, false);
    row(I18N::Format(I18N::Editor::Default0, {std::to_string(m_counts.Items(""))}) + "###Default",
        {Entry::Kind::Default, {}}, false);

    ImGui::SeparatorText(I18N::Editor::Profiles);
    if (SmallButtonUnlessUnapplied(I18N::Editor::NewStatProfile, unapplied))
    {
        OpenNameDialog(NamePurpose::NewProfile, {});
    }
    for (const WrittenStatProfile& profile : stored.profiles)
    {
        const int items = m_counts.Items(profile.name);
        const int rules = m_counts.Rules(profile.name);
        ImGui::PushID(profile.name.c_str());
        row(profile.name + " (" + std::to_string(items) + ")", {Entry::Kind::Profile, profile.name},
            items == 0 && rules == 0);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(
                "%s", I18N::Format(I18N::Editor::UsedBy0ItemsAnd1Rules, {std::to_string(items), std::to_string(rules)})
                          .c_str());
        }
        ImGui::PopID();
    }

    ImGui::SeparatorText(I18N::Editor::Tables);
    if (SmallButtonUnlessUnapplied(I18N::Editor::NewLevelBonusTable, unapplied))
    {
        OpenNameDialog(NamePurpose::NewTable, {});
    }
    for (const LevelBonusTable& table : stored.tables)
    {
        const int stats = CountTableUsers(stored, table.name);
        const int descriptions = static_cast<int>(m_counts.DescriptionUsers(table.name).size());
        ImGui::PushID(table.name.c_str());
        row(table.name + " (" + std::to_string(stats + descriptions) + ")", {Entry::Kind::Table, table.name},
            stats + descriptions == 0);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Format(I18N::Editor::NamedBy0StatsAnd1ItemDescriptions,
                                                 {std::to_string(stats), std::to_string(descriptions)})
                                        .c_str());
        }
        ImGui::PopID();
    }

    ImGui::SeparatorText(I18N::Editor::ProfileRules);
    row(I18N::Format(I18N::Editor::ProfileRules0, {std::to_string(stored.profileRules.size())}) + "###Rules",
        {Entry::Kind::Rules, {}}, false);
}

void CMuItemFormulasUI::RenderPicked()
{
    UpdateEdit();
    if (m_picked.kind == Entry::Kind::None)
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAnEntryInTheList);
        return;
    }
    RenderApplyButtons();
    ImGui::Separator();
    switch (m_picked.kind)
    {
    case Entry::Kind::DropLevelBonus:
        RenderDropLevelBonus(m_edit);
        break;
    case Entry::Kind::Default:
        RenderDefault(m_edit);
        break;
    case Entry::Kind::Profile:
        RenderProfile(m_edit);
        break;
    case Entry::Kind::Table:
        RenderTable(m_edit);
        break;
    case Entry::Kind::Rules:
        RenderRules(m_edit);
        break;
    case Entry::Kind::None:
        break;
    }
}

void CMuItemFormulasUI::RenderApplyButtons()
{
    const ItemStatFile& stored = Stored();
    const bool changed = HasUnappliedChanges();
    ImGui::BeginDisabled(!changed);
    if (ImGui::Button(I18N::Editor::ApplyToTheFormulas))
    {
        Apply(Compose(stored, m_edit, m_picked), I18N::Editor::ChangeTheItemFormulas);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        m_edit = stored;
        m_loaded = stored;
        m_fatalIssues.clear();
    }
    ImGui::EndDisabled();
    if (changed)
    {
        ImGui::SameLine();
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::NotApplied);
    }
    for (const Data::Items::ItemDataIssue& issue : m_fatalIssues)
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(ErrorColor, "%s", issue.ToString().c_str());
        ImGui::PopTextWrapPos();
    }
}

bool CMuItemFormulasUI::HasUnappliedChanges() const
{
    // Before UpdateEdit in a frame the edit can still be of the entry picked
    // before.
    if (m_picked.kind == Entry::Kind::None || m_editOf != m_picked)
    {
        return false;
    }
    const ItemStatFile& stored = Stored();
    return Compose(stored, m_edit, m_picked) != stored;
}

void CMuItemFormulasUI::RenderDropLevelBonus(ItemStatFile& file)
{
    ImGui::TextUnformatted(I18N::Editor::DropLevelBonus);
    RenderNote(I18N::Editor::WhatTheDropLevelBonusIs);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth));
    if (ImGui::InputInt(I18N::Editor::Excellent, &file.excellentDropLevelBonus, 0, 0))
    {
        file.excellentDropLevelBonus = std::clamp(file.excellentDropLevelBonus, 0, MaxDropLevelBonus);
    }
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth));
    if (ImGui::InputInt(I18N::Editor::Ancient, &file.ancientDropLevelBonus, 0, 0))
    {
        file.ancientDropLevelBonus = std::clamp(file.ancientDropLevelBonus, 0, MaxDropLevelBonus);
    }
}

void CMuItemFormulasUI::RenderDefault(ItemStatFile& file)
{
    ImGui::TextUnformatted(Data::Items::StatProfileNames::Default);
    RenderNote(I18N::Editor::TheDefaultWritesEveryTerm);
    const std::vector<int> users = FindStatProfileUsers("");
    if (ImGui::TreeNode("##Users", "%s",
                        I18N::Format(I18N::Editor::UsedBy0Items, {std::to_string(users.size())}).c_str()))
    {
        RenderUsers("##UserList", users);
        ImGui::TreePop();
    }
    RenderSections(file.defaults, file.defaultRequirements, Stored(), false);
}

void CMuItemFormulasUI::RenderProfile(ItemStatFile& file)
{
    WrittenStatProfile* profile = FindNamed(file.profiles, m_picked.name);
    if (profile == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAnEntryInTheList);
        return;
    }
    const int items = m_counts.Items(profile->name);
    const int rules = m_counts.Rules(profile->name);
    ImGui::TextUnformatted(profile->name.c_str());
    ImGui::SameLine();
    // Remove takes the changes along with the profile.
    const bool unapplied = HasUnappliedChanges();
    if (SmallButtonUnlessUnapplied(I18N::Editor::Rename, unapplied))
    {
        OpenNameDialog(NamePurpose::RenameProfile, profile->name);
    }
    ImGui::SameLine();
    if (SmallButtonUnlessUnapplied(I18N::Editor::Copy, unapplied))
    {
        OpenNameDialog(NamePurpose::CopyProfile, profile->name);
    }
    ImGui::SameLine();
    // D26: only a profile nothing uses goes.
    ImGui::BeginDisabled(items > 0 || rules > 0);
    const bool remove = ImGui::SmallButton(I18N::Editor::Remove);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && (items > 0 || rules > 0))
    {
        ImGui::SetTooltip("%s", I18N::Editor::RemovedOnlyWhileNoItemAndNoRuleUsesIt);
    }
    if (remove)
    {
        ItemStatFile removed = Stored();
        std::erase_if(removed.profiles, [&](const WrittenStatProfile& entry) { return entry.name == m_picked.name; });
        Apply(std::move(removed), I18N::Editor::RemoveAStatProfile);
        return;
    }

    const std::vector<int> users = FindStatProfileUsers(profile->name);
    const std::string usedBy =
        I18N::Format(I18N::Editor::UsedBy0ItemsAnd1Rules, {std::to_string(items), std::to_string(rules)});
    if (ImGui::TreeNode("##Users", "%s", usedBy.c_str()))
    {
        RenderUsers("##UserList", users);
        ImGui::TreePop();
    }
    RenderNote(I18N::Editor::AProfileWritesOnlyWhatDiffersFromTheDefault);
    RenderSections(profile->stats, profile->requirements, Stored(), true);
}

void CMuItemFormulasUI::RenderTable(ItemStatFile& file)
{
    LevelBonusTable* table = FindNamed(file.tables, m_picked.name);
    if (table == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAnEntryInTheList);
        return;
    }
    const int stats = CountTableUsers(Stored(), table->name);
    // A copy: the counts change with the item data.
    const std::vector<int> descriptions = m_counts.DescriptionUsers(table->name);
    const bool used = stats > 0 || !descriptions.empty();
    ImGui::TextUnformatted(table->name.c_str());
    ImGui::SameLine();
    const bool unapplied = HasUnappliedChanges();
    if (SmallButtonUnlessUnapplied(I18N::Editor::Rename, unapplied))
    {
        OpenNameDialog(NamePurpose::RenameTable, table->name);
    }
    ImGui::SameLine();
    if (SmallButtonUnlessUnapplied(I18N::Editor::Copy, unapplied))
    {
        OpenNameDialog(NamePurpose::CopyTable, table->name);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(used);
    const bool remove = ImGui::SmallButton(I18N::Editor::Remove);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && used)
    {
        ImGui::SetTooltip("%s", I18N::Editor::RemovedOnlyWhileNoStatAndNoItemDescriptionNamesIt);
    }
    if (remove)
    {
        ItemStatFile removed = Stored();
        std::erase_if(removed.tables, [&](const LevelBonusTable& entry) { return entry.name == m_picked.name; });
        Apply(std::move(removed), I18N::Editor::RemoveALevelBonusTable);
        return;
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::NamedBy0StatsAnd1ItemDescriptions,
                                        {std::to_string(stats), std::to_string(descriptions.size())})
                               .c_str());
    if (!descriptions.empty())
    {
        // D26: where a shared definition is used.
        ImGui::TextDisabled("%s", I18N::Editor::ItemDescriptionsThatNameIt);
        RenderUsers("##DescriptionUsers", descriptions);
    }

    std::string caption = table->caption.GetNeutral();
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
    if (InputString(I18N::Editor::Caption, caption))
    {
        table->caption.Set(Data::LocalizedString::NeutralLocale, caption);
    }

    ImGui::SeparatorText(I18N::Editor::BonusAtEachItemLevel);
    for (int level = 0; level < Data::Items::ItemLevelCount; ++level)
    {
        if (level % LevelsPerRow != 0)
        {
            ImGui::SameLine();
        }
        char label[16];
        std::snprintf(label, sizeof(label), "+%d", level);
        int& value = table->values[static_cast<size_t>(level)];
        ImGui::SetNextItemWidth(Fields::ScaledWidth(LevelValueWidth));
        if (ImGui::InputInt(label, &value, 0, 0))
        {
            value = std::clamp(value, 0, MaxValue);
        }
    }
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth));
    if (ImGui::InputInt(I18N::Editor::AddedPerLevel, &table->perLevel, 0, 0))
    {
        table->perLevel = std::clamp(table->perLevel, 0, MaxValue);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::AddedForEveryLevelAlsoAbove15);
    }
}

void CMuItemFormulasUI::RenderRules(ItemStatFile& file)
{
    std::vector<StatProfileRule>& rules = file.profileRules;
    RenderNote(I18N::Editor::HowTheProfileRulesWork);
    const int selectedRule = [&]
    {
        const Data::Items::ItemDefinition* selected = g_ItemDatabase.Find(g_ItemSelection.GetItemType());
        return selected != nullptr ? Data::Items::FindMatchingRule(rules, *selected) : Data::Items::NoStatProfileRule;
    }();
    if (ImGui::BeginTable("##Rules", 4,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("#");
        ImGui::TableSetupColumn(I18N::Editor::StatProfile);
        ImGui::TableSetupColumn(I18N::Editor::Conditions, ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(I18N::Editor::Items);
        ImGui::TableHeadersRow();
        std::vector<std::string> profiles = {Data::Items::StatProfileNames::Default};
        for (const WrittenStatProfile& profile : file.profiles)
        {
            profiles.push_back(profile.name);
        }
        for (size_t index = 0; index < rules.size(); ++index)
        {
            StatProfileRule& rule = rules[index];
            ImGui::PushID(static_cast<int>(index));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // The selected item's rule is marked.
            const std::string number =
                std::to_string(index + 1) + (static_cast<int>(index) == selectedRule ? " <" : "");
            if (ImGui::Selectable(number.c_str(), m_pickedRule == static_cast<int>(index),
                                  MuEditor::KeyboardSelection::RowFlags))
            {
                m_pickedRule = static_cast<int>(index);
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 1.4f));
            if (ImGui::BeginCombo("##Profile", rule.profile.c_str()))
            {
                for (const std::string& profile : profiles)
                {
                    if (ImGui::Selectable(profile.c_str(), profile == rule.profile))
                    {
                        rule.profile = profile;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(DescribeConditions(rule).c_str());
            ImGui::PopTextWrapPos();
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", static_cast<int>(FindRuleItems(rules, static_cast<int>(index)).size()));
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    const bool picked = m_pickedRule >= 0 && static_cast<size_t>(m_pickedRule) < rules.size();
    if (ImGui::Button(I18N::Editor::NewRule))
    {
        // A copy of the picked rule below it, else a rule for the default at the end.
        const size_t at = picked ? static_cast<size_t>(m_pickedRule) + 1 : rules.size();
        rules.insert(rules.begin() + static_cast<std::ptrdiff_t>(at),
                     picked ? rules[static_cast<size_t>(m_pickedRule)]
                            : StatProfileRule{.profile = Data::Items::StatProfileNames::Default});
        m_pickedRule = static_cast<int>(at);
        return;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!picked || m_pickedRule == 0);
    if (ImGui::Button(I18N::Editor::MoveUp))
    {
        std::swap(rules[static_cast<size_t>(m_pickedRule)], rules[static_cast<size_t>(m_pickedRule) - 1]);
        --m_pickedRule;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!picked || static_cast<size_t>(m_pickedRule) + 1 >= rules.size());
    if (ImGui::Button(I18N::Editor::MoveDown))
    {
        std::swap(rules[static_cast<size_t>(m_pickedRule)], rules[static_cast<size_t>(m_pickedRule) + 1]);
        ++m_pickedRule;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!picked);
    const bool remove = ImGui::Button(I18N::Editor::RemoveTheRule);
    ImGui::EndDisabled();
    if (remove)
    {
        rules.erase(rules.begin() + m_pickedRule);
        m_pickedRule = -1;
        return;
    }
    if (picked)
    {
        ImGui::SeparatorText(I18N::Format(I18N::Editor::Rule0, {std::to_string(m_pickedRule + 1)}).c_str());
        RenderRuleConditions(rules[static_cast<size_t>(m_pickedRule)]);
        const std::vector<int> items = FindRuleItems(rules, m_pickedRule);
        if (ImGui::TreeNode("##RuleItems", "%s",
                            I18N::Format(I18N::Editor::Matches0Items, {std::to_string(items.size())}).c_str()))
        {
            RenderUsers("##RuleItemList", items);
            ImGui::TreePop();
        }
    }
}

void CMuItemFormulasUI::RenderRuleConditions(StatProfileRule& rule)
{
    std::string groups = DescribeGroups(rule.groups);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
    if (ImGui::BeginCombo(I18N::Editor::Group, groups.empty() ? I18N::Editor::Any : groups.c_str()))
    {
        for (int group = 0; group < MAX_ITEM_TYPE; ++group)
        {
            const bool on = rule.groups.test(static_cast<size_t>(group));
            if (ImGui::Selectable(MuEditor::Items::GetItemGroupLabel(group).c_str(), on,
                                  ImGuiSelectableFlags_NoAutoClosePopups))
            {
                rule.groups.set(static_cast<size_t>(group), !on);
            }
        }
        ImGui::EndCombo();
    }
    ConditionCombo<Data::Items::ItemSlot>(I18N::Metadata::Slot, rule.slots);
    ConditionCombo<Data::Items::WingTier>(I18N::Editor::WingTier, rule.wingTiers);
    ConditionCombo<Data::Items::WeaponKind>(I18N::Editor::WeaponKind, rule.weaponKinds);
    ConditionCombo<Data::Items::ItemTag>(I18N::Editor::Tags, rule.tags);
    ConditionCombo<Data::Items::ItemRequirement>(I18N::Editor::BaseRequirementAbove0, rule.requirements);
}

void CMuItemFormulasUI::Apply(ItemStatFile file, std::string label, std::string renamedFrom, std::string renamedTo,
                              bool renamedTable)
{
    m_fatalIssues.clear();
    Data::Items::ValidateItemStatsAsWritten(file, m_fatalIssues);
    if (!m_fatalIssues.empty())
    {
        return;
    }
    const Data::Items::ItemStatProfiles& current = g_ItemDatabase.GetStatProfiles();
    const Data::Items::ItemStatProfiles after(file);
    const std::vector<StatProfileChange> changes = FindStatProfileChanges(
        g_ItemDatabase.GetAllSlots(), current, after, renamedTable ? "" : renamedFrom, renamedTable ? "" : renamedTo);
    const std::vector<std::string> changedProfiles = FindProfilesWithChangedTerms(current, after);
    std::vector<int> reached = FindItemsReached(g_ItemDatabase.GetAllSlots(), changes, changedProfiles);
    if (renamedTable)
    {
        const std::vector<int> descriptions = FindTableDescriptionUsers(g_ItemDatabase.GetAllSlots(), renamedFrom);
        reached.insert(reached.end(), descriptions.begin(), descriptions.end());
        std::ranges::sort(reached);
        reached.erase(std::unique(reached.begin(), reached.end()), reached.end());
    }
    m_pending = {std::move(file), std::move(label),   std::move(renamedFrom), std::move(renamedTo),
                 renamedTable,    std::move(reached), m_pending.pickAfter};
    if (m_pending.reached.empty())
    {
        Commit();
    }
    else
    {
        // D26: a change of the formulas reaches all the items it changes; they
        // are listed before it is applied.
        m_openConfirm = true;
    }
}

void CMuItemFormulasUI::Commit()
{
    const Data::Items::ItemStatProfiles after(m_pending.file);
    const std::vector<StatProfileChange> changes = FindStatProfileChanges(
        g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(), after,
        m_pending.renamedTable ? "" : m_pending.renamedFrom, m_pending.renamedTable ? "" : m_pending.renamedTo);
    std::vector<Data::Items::ItemDefinition> changed = ApplyStatProfileChanges(g_ItemDatabase.GetAllSlots(), changes);
    if (m_pending.renamedTable)
    {
        // D26: renaming a table renames it in the descriptions that name it too.
        RenameTableInDescriptions(g_ItemDatabase.GetAllSlots(), changed, m_pending.renamedFrom, m_pending.renamedTo);
    }
    g_ItemEdits.SetStatFile(m_pending.label, m_pending.file, changed);
    if (m_pending.pickAfter.kind != Entry::Kind::None)
    {
        m_picked = m_pending.pickAfter;
        m_pending.pickAfter = {};
    }
    // The edit takes the stored file again.
    m_editOf = {};
}

void CMuItemFormulasUI::RenderConfirm()
{
    if (m_openConfirm)
    {
        ImGui::OpenPopup(ConfirmPopupId);
        m_openConfirm = false;
    }
    const std::string title = std::string(I18N::Editor::ChangeTheItemFormulas) + ConfirmPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::TheChangeReachesThese0Items, {std::to_string(m_pending.reached.size())}).c_str());
    RenderUsers("##Reached", m_pending.reached);
    if (ImGui::Button(I18N::Editor::ApplyToTheFormulas))
    {
        Commit();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        m_pending.pickAfter = {};
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuItemFormulasUI::OpenNameDialog(NamePurpose purpose, std::string from)
{
    m_namePurpose = purpose;
    m_nameFrom = std::move(from);
    const bool rename = purpose == NamePurpose::RenameProfile || purpose == NamePurpose::RenameTable;
    m_name = rename ? m_nameFrom : std::string();
    m_openName = true;
}

bool CMuItemFormulasUI::IsFreeName(NamePurpose purpose, const std::string& name) const
{
    if (!Data::Items::Json::IsName(name) || name == Data::Items::StatProfileNames::Default)
    {
        return false;
    }
    const ItemStatFile& stored = Stored();
    const bool table =
        purpose == NamePurpose::NewTable || purpose == NamePurpose::CopyTable || purpose == NamePurpose::RenameTable;
    return table ? FindNamed(stored.tables, name) == nullptr : FindNamed(stored.profiles, name) == nullptr;
}

void CMuItemFormulasUI::RenderNameDialog()
{
    if (m_openName)
    {
        ImGui::OpenPopup(NamePopupId);
        m_openName = false;
    }
    const char* titles[] = {I18N::Editor::NewStatProfile,       I18N::Editor::CopyAStatProfile,
                            I18N::Editor::RenameAStatProfile,   I18N::Editor::NewLevelBonusTable,
                            I18N::Editor::CopyALevelBonusTable, I18N::Editor::RenameALevelBonusTable};
    const std::string title = std::string(titles[static_cast<size_t>(m_namePurpose)]) + NamePopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 2.0f));
    InputString(I18N::Editor::Name, m_name);
    const bool free = IsFreeName(m_namePurpose, m_name);
    if (!free)
    {
        RenderNote(I18N::Editor::AFreeNameOfLettersAndDigits);
    }
    const bool rename = m_namePurpose == NamePurpose::RenameProfile || m_namePurpose == NamePurpose::RenameTable;
    ImGui::BeginDisabled(!free);
    if (ImGui::Button(rename ? I18N::Editor::Rename : I18N::Editor::Add))
    {
        ApplyName(m_namePurpose, m_name);
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuItemFormulasUI::ApplyName(NamePurpose purpose, const std::string& name)
{
    ItemStatFile file = Stored();
    switch (purpose)
    {
    case NamePurpose::NewProfile:
    {
        WrittenStatProfile profile;
        profile.name = name;
        file.profiles.push_back(profile);
        m_pending.pickAfter = {Entry::Kind::Profile, name};
        Apply(std::move(file), I18N::Editor::NewStatProfile);
        break;
    }
    case NamePurpose::CopyProfile:
        if (const WrittenStatProfile* from = FindNamed(file.profiles, m_nameFrom))
        {
            WrittenStatProfile copy = *from;
            copy.name = name;
            file.profiles.push_back(copy);
            m_pending.pickAfter = {Entry::Kind::Profile, name};
            Apply(std::move(file), I18N::Editor::CopyAStatProfile);
        }
        break;
    case NamePurpose::RenameProfile:
        RenameStatProfile(file, m_nameFrom, name);
        m_pending.pickAfter = {Entry::Kind::Profile, name};
        Apply(std::move(file), I18N::Editor::RenameAStatProfile, m_nameFrom, name);
        break;
    case NamePurpose::NewTable:
    {
        LevelBonusTable table;
        table.name = name;
        file.tables.push_back(table);
        m_pending.pickAfter = {Entry::Kind::Table, name};
        Apply(std::move(file), I18N::Editor::NewLevelBonusTable);
        break;
    }
    case NamePurpose::CopyTable:
        if (const LevelBonusTable* from = FindNamed(file.tables, m_nameFrom))
        {
            LevelBonusTable copy = *from;
            copy.name = name;
            file.tables.push_back(copy);
            m_pending.pickAfter = {Entry::Kind::Table, name};
            Apply(std::move(file), I18N::Editor::CopyALevelBonusTable);
        }
        break;
    case NamePurpose::RenameTable:
        RenameTable(file, m_nameFrom, name);
        m_pending.pickAfter = {Entry::Kind::Table, name};
        Apply(std::move(file), I18N::Editor::RenameALevelBonusTable, m_nameFrom, name, true);
        break;
    }
}

void MuEditor::Items::OpenItemFormulas(const Entry& entry)
{
    g_MuEditorCore.GetToolWindows().itemFormulas = true;
    g_MuItemFormulasUI.Show(entry);
    ImGui::SetWindowFocus(MuEditor::WindowIds::ItemFormulas);
}

void CMuItemFormulasUI::UpdateEdit()
{
    const long long version = g_ItemDatabase.GetVersion();
    const bool unchanged = m_edit == m_loaded;
    if (m_editOf != m_picked || (m_editVersion != version && unchanged))
    {
        m_edit = Stored();
        m_loaded = m_edit;
        m_editOf = m_picked;
        m_fatalIssues.clear();
    }
    m_editVersion = version;
}

#endif // _EDITOR
