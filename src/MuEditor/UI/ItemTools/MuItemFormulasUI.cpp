#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemFormulasUI.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemSelection.h"
#include "ItemStatTermFields.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
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

// The stats of the default and the profiles that name the table.
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
    case Entry::Kind::None:
        break;
    }
    return file;
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

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
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
        if (CountTableUsers(stored, table.name) == 0)
        {
            tables.push_back(table.name);
        }
    }
    if (!profiles.empty())
    {
        RenderNote(I18N::Format(I18N::Editor::NoItemOrRuleUsesTheseProfiles0, {JoinNames(profiles)}).c_str());
        if (ImGui::SmallButton((std::string(I18N::Editor::RemoveThem) + "##Profiles").c_str()))
        {
            ItemStatFile file = stored;
            std::erase_if(file.profiles, [&](const WrittenStatProfile& profile)
                          { return std::ranges::find(profiles, profile.name) != profiles.end(); });
            Apply(std::move(file), I18N::Editor::RemoveUnusedStatProfiles);
        }
    }
    if (!tables.empty())
    {
        RenderNote(I18N::Format(I18N::Editor::NoStatUsesTheseTables0, {JoinNames(tables)}).c_str());
        if (ImGui::SmallButton((std::string(I18N::Editor::RemoveThem) + "##Tables").c_str()))
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
    if (ImGui::SmallButton(I18N::Editor::NewStatProfile))
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
    if (ImGui::SmallButton(I18N::Editor::NewLevelBonusTable))
    {
        OpenNameDialog(NamePurpose::NewTable, {});
    }
    for (const LevelBonusTable& table : stored.tables)
    {
        const int users = CountTableUsers(stored, table.name);
        ImGui::PushID(table.name.c_str());
        row(table.name + " (" + std::to_string(users) + ")", {Entry::Kind::Table, table.name}, users == 0);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Format(I18N::Editor::NamedBy0Stats, {std::to_string(users)}).c_str());
        }
        ImGui::PopID();
    }
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
    case Entry::Kind::None:
        break;
    }
}

void CMuItemFormulasUI::RenderApplyButtons()
{
    const ItemStatFile& stored = Stored();
    const bool changed = Compose(stored, m_edit, m_picked) != stored;
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
        m_errors.clear();
    }
    ImGui::EndDisabled();
    if (changed)
    {
        ImGui::SameLine();
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::NotApplied);
    }
    for (const Data::Items::ItemDataIssue& issue : m_errors)
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(ErrorColor, "%s", issue.ToString().c_str());
        ImGui::PopTextWrapPos();
    }
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
    if (ImGui::SmallButton(I18N::Editor::Rename))
    {
        OpenNameDialog(NamePurpose::RenameProfile, profile->name);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::Copy))
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
    const int users = CountTableUsers(Stored(), table->name);
    ImGui::TextUnformatted(table->name.c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::Rename))
    {
        OpenNameDialog(NamePurpose::RenameTable, table->name);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::Copy))
    {
        OpenNameDialog(NamePurpose::CopyTable, table->name);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(users > 0);
    const bool remove = ImGui::SmallButton(I18N::Editor::Remove);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && users > 0)
    {
        ImGui::SetTooltip("%s", I18N::Editor::RemovedOnlyWhileNoStatNamesIt);
    }
    if (remove)
    {
        ItemStatFile removed = Stored();
        std::erase_if(removed.tables, [&](const LevelBonusTable& entry) { return entry.name == m_picked.name; });
        Apply(std::move(removed), I18N::Editor::RemoveALevelBonusTable);
        return;
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::NamedBy0Stats, {std::to_string(users)}).c_str());

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

void CMuItemFormulasUI::Apply(ItemStatFile file, std::string label, std::string renamedFrom, std::string renamedTo)
{
    m_errors.clear();
    Data::Items::ValidateItemStatsAsWritten(file, m_errors);
    if (!m_errors.empty())
    {
        return;
    }
    const Data::Items::ItemStatProfiles& current = g_ItemDatabase.GetStatProfiles();
    const Data::Items::ItemStatProfiles after(file);
    const std::vector<StatProfileChange> changes =
        FindStatProfileChanges(g_ItemDatabase.GetAllSlots(), current, after, renamedFrom, renamedTo);
    const std::vector<std::string> changedProfiles = FindProfilesWithChangedTerms(current, after);
    m_pending = {std::move(file),
                 std::move(label),
                 std::move(renamedFrom),
                 std::move(renamedTo),
                 FindItemsReached(g_ItemDatabase.GetAllSlots(), changes, changedProfiles),
                 m_pending.pickAfter};
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
    const std::vector<StatProfileChange> changes =
        FindStatProfileChanges(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(), after,
                               m_pending.renamedFrom, m_pending.renamedTo);
    g_ItemEdits.SetStatFile(m_pending.label, m_pending.file,
                            ApplyStatProfileChanges(g_ItemDatabase.GetAllSlots(), changes));
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
        Apply(std::move(file), I18N::Editor::RenameALevelBonusTable);
        break;
    }
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
        m_errors.clear();
    }
    m_editVersion = version;
}

#endif // _EDITOR
