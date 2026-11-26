#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemOptionsUI.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemOptionFileChange.h"
#include "ItemRuleConditionWidgets.h"
#include "ItemSelection.h"
#include "ItemTextInput.h"
#include "ItemToolWidgets.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemOptionEffects.h"
#include "Data/GameData/ItemData/ItemOptionValidation.h"
#include "Data/GameData/ItemData/ItemOptionsJson.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cstddef>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using Data::Items::ItemOption;
using Data::Items::ItemOptionAdd;
using Data::Items::ItemOptionDefinition;
using Data::Items::ItemOptionFiles;
using Data::Items::ItemOptionLevelType;
using Data::Items::ItemOptionTerm;
using Data::Items::ItemOptionType;
using Data::Items::ItemOptionValue;
using ToolWidgets::ErrorColor;
using ToolWidgets::RenderItemList;
using ToolWidgets::RenderNote;
using ToolWidgets::SmallButtonUnlessUnapplied;
using ToolWidgets::WarningColor;

namespace
{
constexpr char ConfirmPopupId[] = "###OptionFileChange";
constexpr char NamePopupId[] = "###OptionDefinitionName";
constexpr char FillPopupId[] = "###OptionFillLevels";
constexpr char ValueFillPopupId[] = "###OptionValueFill";
// Pixel widths before the editor UI scale.
constexpr float ListWidth = 360.0f;
constexpr float LevelValueWidth = 52.0f;
// The levels shown in one row of a value list.
constexpr int LevelsPerRow = 8;
// The option byte's level has four bits: a list longer than 15 gives levels no server sends.
constexpr int MaxOptionLevel = 15;
constexpr int MaxValue = 1000000;
constexpr int MaxOptionsPerItem = 255;

// The lowest number of the type's range no option of the definition has, or none when every number is taken (the loader
// refuses a number two options share).
std::optional<int> FindFreeOptionNumber(const ItemOptionDefinition& definition)
{
    const Data::Items::ItemOptionNumberRange range = Data::Items::GetOptionNumberRange(definition.type);
    for (int number = range.minimum; number <= range.maximum; ++number)
    {
        if (std::ranges::none_of(definition.options,
                                 [number](const ItemOption& each) { return each.number == number; }))
        {
            return number;
        }
    }
    return std::nullopt;
}

const ItemOptionFiles& StoredFiles()
{
    static ItemOptionFiles files;
    static long long version = -1;
    if (version != g_ItemDatabase.GetVersion())
    {
        files = g_ItemDatabase.GetOptions().GetFiles();
        version = g_ItemDatabase.GetVersion();
    }
    return files;
}

// A definition of the type with one option to fill in: the first attribute the client computes for the type. It goes in
// the file of the type's definitions in `files`.
ItemOptionDefinition MakeNewDefinition(const ItemOptionFiles& files, const std::string& name, ItemOptionType type)
{
    ItemOptionDefinition definition{.name = name, .type = type, .file = FindFileOfType(files, type)};
    definition.displayName.Set("en", name);
    ItemOption option;
    option.number = Data::Items::GetOptionNumberRange(type).minimum;
    const std::vector<std::string_view> attributes = Data::Items::FindItemOptionAttributes(type);
    option.attribute = attributes.empty() ? std::string() : std::string(attributes.front());
    option.value.values = {0};
    definition.options.push_back(std::move(option));
    return definition;
}

const char* TypeName(ItemOptionType type)
{
    const char* name = Data::Items::FindEnumName(type);
    return name != nullptr ? name : "?";
}

bool InputNumber(const char* label, int& value, int minimum, int maximum, float width = Fields::NumberWidth)
{
    ImGui::SetNextItemWidth(Fields::ScaledWidth(width));
    int edited = value;
    if (!ImGui::InputInt(label, &edited, 0, 0))
    {
        return false;
    }
    value = std::clamp(edited, minimum, maximum);
    return true;
}

std::string HighestLevelText(const ItemOptionDefinition& definition)
{
    const int highest = HighestOptionLevel(definition);
    return highest > 0 ? std::to_string(highest) : std::string("-");
}

} // namespace

CMuItemOptionsUI& CMuItemOptionsUI::GetInstance()
{
    static CMuItemOptionsUI instance;
    return instance;
}

void CMuItemOptionsUI::Show(const std::string& definition)
{
    m_picked = definition;
}

void CMuItemOptionsUI::Render(bool* open)
{
    m_textGrid.BeginFrame();
    if (ToolWindow::Begin(I18N::Editor::ItemOptions, MuEditor::WindowIds::ItemOptions, open))
    {
        UpdateEdit();
        UpdateUserCounts();
        m_unapplied = HasUnappliedChanges();
        m_textRows.SetTexts(&m_edit.texts, &StoredFiles().texts);
        RenderNote(I18N::Editor::TheServerKeepsItsOwnOptionDefinitions);
        RenderApplyButtons();
        if (ImGui::BeginTabBar("##Tabs"))
        {
            if (ImGui::BeginTabItem(I18N::Editor::Definitions))
            {
                RenderDefinitionsTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(I18N::Editor::Rules))
            {
                RenderRulesTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(I18N::Editor::Texts))
            {
                RenderTextsTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        RenderConfirm();
        RenderNameDialog();
        RenderFillLevels();
        RenderValueFill();
    }
    ToolWindow::End();
    m_textGrid.CommitEditNotDrawn(m_textRows);
}

void CMuItemOptionsUI::RenderDefinitionsTab()
{
    const ImVec2 listSize(ListWidth * g_MuEditorCore.GetUIScale(), 0.0f);
    if (ImGui::BeginChild("##Definitions", listSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
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
}

void CMuItemOptionsUI::RenderRulesTab()
{
    RenderNote(I18N::Editor::HowTheOptionRulesWork);
    UpdateRuleMatches();
    RenderRuleList();
    // A rule added, moved or removed is drawn from the next frame.
    if (RenderRuleButtons())
    {
        return;
    }
    if (m_pickedRule >= 0 && static_cast<size_t>(m_pickedRule) < m_edit.rules.size())
    {
        RenderPickedRule(m_edit.rules[static_cast<size_t>(m_pickedRule)]);
    }
}

void CMuItemOptionsUI::RenderRuleList()
{
    // The slots are in item type order.
    const int selectedType = g_ItemSelection.GetItemType();
    const int selectedRule = selectedType >= 0 && static_cast<size_t>(selectedType) < m_ruleOfSlot.size()
                                 ? m_ruleOfSlot[static_cast<size_t>(selectedType)]
                                 : -1;
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Rules", 4, Flags))
    {
        return;
    }
    ImGui::TableSetupColumn("#");
    ImGui::TableSetupColumn(I18N::Editor::Options);
    ImGui::TableSetupColumn(I18N::Editor::Conditions, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(I18N::Editor::Items);
    ImGui::TableHeadersRow();
    for (size_t index = 0; index < m_edit.rules.size(); ++index)
    {
        RenderRuleRow(index, static_cast<int>(index) == selectedRule);
    }
    ImGui::EndTable();
}

void CMuItemOptionsUI::RenderRuleRow(size_t index, bool selectedItemsRule)
{
    const Data::Items::OptionRule& rule = m_edit.rules[index];
    ImGui::PushID(static_cast<int>(index));
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    // The selected item's rule is marked, and a rule with changes not applied.
    const std::vector<Data::Items::OptionRule>& stored = StoredFiles().rules;
    const bool changed = index >= stored.size() || stored[index] != rule;
    const std::string number = std::to_string(index + 1) + (changed ? " *" : "") + (selectedItemsRule ? " <" : "");
    if (ImGui::Selectable(number.c_str(), m_pickedRule == static_cast<int>(index),
                          MuEditor::KeyboardSelection::RowFlags | ImGuiSelectableFlags_SpanAllColumns))
    {
        m_pickedRule = static_cast<int>(index);
    }
    ImGui::TableSetColumnIndex(1);
    std::string options;
    for (const std::string& name : rule.options)
    {
        options += (options.empty() ? "" : ", ") + name;
    }
    ImGui::TextUnformatted(options.empty() ? "-" : options.c_str());
    ImGui::TableSetColumnIndex(2);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(DescribeRuleConditions(rule.conditions).c_str());
    ImGui::PopTextWrapPos();
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%d", index < m_ruleMatches.size() ? m_ruleMatches[index] : 0);
    ImGui::PopID();
}

bool CMuItemOptionsUI::RenderRuleButtons()
{
    std::vector<Data::Items::OptionRule>& rules = m_edit.rules;
    const bool picked = m_pickedRule >= 0 && static_cast<size_t>(m_pickedRule) < rules.size();
    const auto at = static_cast<size_t>(std::max(m_pickedRule, 0));
    if (ImGui::Button(I18N::Editor::NewRule))
    {
        // A copy of the picked rule below it, else a rule without options and conditions at the end.
        const size_t to = picked ? at + 1 : rules.size();
        rules.insert(rules.begin() + static_cast<std::ptrdiff_t>(to), picked ? rules[at] : Data::Items::OptionRule{});
        m_pickedRule = static_cast<int>(to);
        return true;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!picked || at == 0);
    const bool up = ImGui::Button(I18N::Editor::MoveUp);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!picked || at + 1 >= rules.size());
    const bool down = ImGui::Button(I18N::Editor::MoveDown);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!picked);
    const bool remove = ImGui::Button(I18N::Editor::RemoveTheRule);
    ImGui::EndDisabled();
    if (up || down)
    {
        const size_t other = up ? at - 1 : at + 1;
        std::swap(rules[at], rules[other]);
        m_pickedRule = static_cast<int>(other);
    }
    else if (remove)
    {
        rules.erase(rules.begin() + static_cast<std::ptrdiff_t>(at));
        m_pickedRule = -1;
    }
    return up || down || remove;
}

void CMuItemOptionsUI::RenderPickedRule(Data::Items::OptionRule& rule)
{
    ImGui::SeparatorText(I18N::Format(I18N::Editor::Rule0, {std::to_string(m_pickedRule + 1)}).c_str());
    std::optional<size_t> removed;
    for (size_t index = 0; index < rule.options.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::SmallButton("x"))
        {
            removed = index;
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(rule.options[index].c_str());
        ImGui::PopID();
    }
    if (removed)
    {
        rule.options.erase(rule.options.begin() + static_cast<std::ptrdiff_t>(*removed));
    }
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 2.0f));
    if (ImGui::BeginCombo(I18N::Editor::LinkAnOptionDefinition, I18N::Editor::Pick))
    {
        for (const ItemOptionDefinition& definition : m_edit.definitions)
        {
            if (std::ranges::find(rule.options, definition.name) == rule.options.end() &&
                ImGui::Selectable(definition.name.c_str()))
            {
                rule.options.push_back(definition.name);
                g_ItemDatabase.GetOptions().OrderLinks(rule.options);
            }
        }
        ImGui::EndCombo();
    }
    RenderRuleConditionFields(rule.conditions, true);
}

void CMuItemOptionsUI::RenderTextsTab()
{
    RenderNote(I18N::Editor::TheTextsOfTheOptionsLines);
    if (ImGui::SmallButton(I18N::Editor::NewText))
    {
        OpenNameDialog(NamePurpose::NewText, {});
    }
    m_textGrid.UpdateCountsAndLabels(m_textRows);
    m_textGrid.RenderToolbar(m_textRows);
    m_textGrid.RenderTable(m_textRows, "##OptionTexts");
}

void CMuItemOptionsUI::RenderList()
{
    const bool unapplied = m_unapplied;
    ImGui::SetNextItemWidth(-FLT_MIN);
    InputStringWithHint("##Search", I18N::Editor::Search, m_search);
    ImGui::SetNextItemWidth(-FLT_MIN);
    const char* shownType =
        m_typeFilter < 0 ? I18N::Editor::AllTypes : TypeName(static_cast<ItemOptionType>(m_typeFilter));
    if (ImGui::BeginCombo("##Type", shownType))
    {
        if (ImGui::Selectable(I18N::Editor::AllTypes, m_typeFilter < 0))
        {
            m_typeFilter = -1;
        }
        for (const auto& [type, name] : Data::Items::GetEnumNames(ItemOptionType{}))
        {
            if (ImGui::Selectable(name, m_typeFilter == static_cast<int>(type)))
            {
                m_typeFilter = static_cast<int>(type);
            }
        }
        ImGui::EndCombo();
    }

    if (SmallButtonUnlessUnapplied(I18N::Editor::New, unapplied))
    {
        OpenNameDialog(NamePurpose::New, {});
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_marked.empty());
    if (SmallButtonUnlessUnapplied(
            I18N::Format(I18N::Editor::FillLevelsOf0Marked, {std::to_string(m_marked.size())}).c_str(), unapplied))
    {
        m_openFill = true;
    }
    ImGui::EndDisabled();

    constexpr ImGuiTableFlags Flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY |
                                      ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##List", 5, Flags))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##Mark");
    ImGui::TableSetupColumn(I18N::Editor::Name, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(I18N::Editor::OptionType);
    ImGui::TableSetupColumn(I18N::Editor::Items);
    ImGui::TableSetupColumn(I18N::Editor::HighestLevel);
    ImGui::TableHeadersRow();
    for (const ItemOptionDefinition* definition : ListDefinitions(StoredFiles(), m_edit, m_typeFilter, m_search))
    {
        RenderListRow(*definition, unapplied);
    }
    ImGui::EndTable();
}

void CMuItemOptionsUI::RenderListRow(const ItemOptionDefinition& definition, bool unapplied)
{
    ImGui::PushID(definition.name.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    bool marked = m_marked.contains(definition.name);
    const bool hasList = HighestOptionLevel(definition) > 0;
    ImGui::BeginDisabled(!hasList);
    if (ImGui::Checkbox("##Mark", &marked))
    {
        if (marked)
        {
            m_marked.insert(definition.name);
        }
        else
        {
            m_marked.erase(definition.name);
        }
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", hasList ? I18N::Editor::MarkForFillLevels : I18N::Editor::ItsValuesAreConstants);
    }
    ImGui::TableNextColumn();
    const int users = CountUsers(definition.name);
    const int rules = CountOptionRuleUses(StoredFiles().rules, definition.name);
    const ItemOptionDefinition* edited = FindDefinitionNamed(m_edit, definition.name);
    const bool changed = edited == nullptr || *edited != definition || IsOnlyInCopy(StoredFiles(), definition.name);
    const bool unused = users == 0 && rules == 0;
    if (unused)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }
    const std::string label = definition.name + (unapplied && changed ? " *" : "");
    if (ImGui::Selectable(label.c_str(), m_picked == definition.name,
                          MuEditor::KeyboardSelection::RowFlags | ImGuiSelectableFlags_SpanAllColumns))
    {
        m_picked = definition.name;
    }
    if (unused)
    {
        ImGui::PopStyleColor();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(
            "%s",
            I18N::Format(I18N::Editor::UsedBy0ItemsAnd1Rules, {std::to_string(users), std::to_string(rules)}).c_str());
    }
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(TypeName(definition.type));
    ImGui::TableNextColumn();
    ImGui::Text("%d", users);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(HighestLevelText(definition).c_str());
    ImGui::PopID();
}

void CMuItemOptionsUI::RenderPicked()
{
    ItemOptionDefinition* definition = FindEdited(m_picked);
    if (definition == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAnEntryInTheList);
        return;
    }
    RenderDefinition(*definition);
}

void CMuItemOptionsUI::RenderApplyButtons()
{
    const bool changed = m_unapplied;
    // The copy would write back what an undo or redo changed in the stored files since it was taken.
    ImGui::BeginDisabled(!changed || m_stale);
    if (ImGui::Button(I18N::Editor::ApplyToTheOptions))
    {
        Apply(m_edit, I18N::Editor::ChangeTheItemOptions);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!changed);
    if (ImGui::Button(I18N::Editor::Revert))
    {
        m_edit = StoredFiles();
        m_loaded = m_edit;
        m_stale = false;
        m_fatalIssues.clear();
        DropMissingMarks(m_marked, StoredFiles(), m_edit);
    }
    ImGui::EndDisabled();
    if (m_stale)
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(ErrorColor, "%s", I18N::Editor::TheOptionFilesChangedUnderTheChanges);
        ImGui::PopTextWrapPos();
    }
    if (changed)
    {
        ImGui::SameLine();
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::NotApplied);
    }
    ImGui::PushTextWrapPos(0.0f);
    for (const Data::Items::ItemDataIssue& issue : m_fatalIssues)
    {
        ImGui::TextColored(ErrorColor, "%s", issue.ToString().c_str());
    }
    for (const Data::Items::ItemDataIssue& issue : m_warnings)
    {
        ImGui::TextColored(WarningColor, "%s", issue.ToString().c_str());
    }
    ImGui::PopTextWrapPos();
}

bool CMuItemOptionsUI::HasUnappliedChanges() const
{
    return m_edit != StoredFiles();
}

void CMuItemOptionsUI::RenderDefinition(ItemOptionDefinition& definition)
{
    const bool unapplied = m_unapplied;
    const bool stored = !IsOnlyInCopy(StoredFiles(), definition.name);
    ImGui::Text("%s: %s", I18N::Editor::Name, definition.name.c_str());
    ImGui::SameLine();
    ImGui::BeginDisabled(!stored);
    if (SmallButtonUnlessUnapplied(I18N::Editor::Copy, unapplied))
    {
        OpenNameDialog(NamePurpose::Copy, definition.name);
    }
    ImGui::SameLine();
    // The items' attackType picks the ancient bonus definitions by their names: a rename would lose them.
    const bool pickedByName = IsPickedByName(definition);
    ImGui::BeginDisabled(pickedByName);
    if (SmallButtonUnlessUnapplied(I18N::Editor::Rename, unapplied))
    {
        OpenNameDialog(NamePurpose::Rename, definition.name);
    }
    ImGui::EndDisabled();
    if (pickedByName && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::TheItemsPickItByThisName);
    }
    ImGui::SameLine();
    const int users = CountUsers(definition.name);
    const int rules = CountOptionRuleUses(StoredFiles().rules, definition.name);
    // D26: a definition goes only while no item and no rule links it.
    ImGui::BeginDisabled(users > 0 || rules > 0);
    if (SmallButtonUnlessUnapplied(I18N::Editor::Remove, unapplied))
    {
        ItemOptionFiles files = StoredFiles();
        std::erase_if(files.definitions,
                      [&](const ItemOptionDefinition& removed) { return removed.name == definition.name; });
        m_marked.erase(definition.name);
        Apply(std::move(files), I18N::Editor::RemoveAnOptionDefinition);
        // Apply may take the copy again, which `definition` is part of: the rest is drawn next frame.
        ImGui::EndDisabled();
        ImGui::EndDisabled();
        return;
    }
    ImGui::EndDisabled();
    if ((users > 0 || rules > 0) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip(
            "%s",
            I18N::Format(I18N::Editor::UsedBy0ItemsAnd1Rules, {std::to_string(users), std::to_string(rules)}).c_str());
    }
    ImGui::EndDisabled();

    ImGui::Text("%s: %s   %s: %s", I18N::Editor::OptionType, TypeName(definition.type), I18N::Editor::File,
                definition.file.c_str());
    if (!definition.openMu.IsEmpty())
    {
        ImGui::TextDisabled("OpenMU: %s  %s", definition.openMu.id.c_str(), definition.openMu.name.c_str());
    }
    RenderDisplayName(definition);
    RenderServer(definition);

    ImGui::SeparatorText(I18N::Editor::Options);
    for (size_t index = 0; index < definition.options.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        RenderOption(definition, index);
        ImGui::PopID();
    }
    // The types whose definitions have one option take no second one (the reader stops on it), the numbered types none
    // when every number of their range is taken (six bits of the excellent byte).
    const bool numbered = Data::Items::HasOptionNumbers(definition.type);
    const std::optional<int> freeNumber = numbered ? FindFreeOptionNumber(definition) : std::nullopt;
    ImGui::BeginDisabled(numbered ? !freeNumber.has_value() : !definition.options.empty());
    if (ImGui::SmallButton(I18N::Editor::AddAnOption))
    {
        ItemOption option = definition.options.empty() ? MakeNewDefinition(m_edit, "", definition.type).options.front()
                                                       : definition.options.back();
        if (!definition.options.empty() && freeNumber)
        {
            option.number = *freeNumber;
        }
        definition.options.push_back(std::move(option));
    }
    ImGui::EndDisabled();

    ImGui::SeparatorText(I18N::Editor::UsedBy);
    RenderUsers(definition.name);
}

void CMuItemOptionsUI::RenderDisplayName(ItemOptionDefinition& definition)
{
    if (!ImGui::TreeNode(I18N::Format(I18N::Editor::DisplayName0, {definition.displayName.Get("en")}).c_str()))
    {
        return;
    }
    for (const char* locale : I18N::GetAvailableLocales())
    {
        ImGui::PushID(locale);
        std::string text = definition.displayName.Get(locale);
        ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
        if (InputString(locale, text))
        {
            definition.displayName.Set(locale, std::move(text));
        }
        ImGui::PopID();
    }
    if (!definition.openMu.name.empty())
    {
        // OpenMU's code looks definitions up by its English name, which the labels keep (O27).
        RenderNote(I18N::Editor::OpenMUFindsTheDefinitionByItsLabels);
    }
    ImGui::TreePop();
}

void CMuItemOptionsUI::RenderServer(ItemOptionDefinition& definition)
{
    bool hasServer = definition.server.has_value();
    if (ImGui::Checkbox(I18N::Editor::ServerValues, &hasServer))
    {
        definition.server =
            hasServer ? std::optional<Data::Items::ItemOptionServerValues>(Data::Items::ItemOptionServerValues{})
                      : std::nullopt;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::OnlyTheServerReadsThem);
    }
    if (!definition.server)
    {
        return;
    }
    ImGui::Indent();
    ImGui::Checkbox(I18N::Editor::AddsRandomly, &definition.server->addsRandomly);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth));
    double chance = definition.server->addChance;
    if (ImGui::InputDouble(I18N::Editor::AddChance, &chance, 0.0, 0.0, "%.4f"))
    {
        definition.server->addChance = std::clamp(chance, 0.0, 1.0);
    }
    ImGui::SameLine();
    InputNumber(I18N::Editor::MaximumPerItem, definition.server->maximumOptionsPerItem, 0, MaxOptionsPerItem,
                Fields::NumberWidth * 0.6f);
    ImGui::Unindent();
}

void CMuItemOptionsUI::RenderOption(ItemOptionDefinition& definition, size_t index)
{
    ItemOption& option = definition.options[index];
    if (Data::Items::HasOptionNumbers(definition.type))
    {
        const Data::Items::ItemOptionNumberRange range = Data::Items::GetOptionNumberRange(definition.type);
        InputNumber(I18N::Editor::Number, option.number, range.minimum, range.maximum, Fields::NumberWidth * 0.6f);
        ImGui::SameLine();
    }
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 2.0f));
    if (ImGui::BeginCombo(I18N::Editor::Attribute, option.attribute.c_str()))
    {
        for (const std::string_view attribute : Data::Items::FindItemOptionAttributes(definition.type))
        {
            if (ImGui::Selectable(std::string(attribute).c_str(), option.attribute == attribute))
            {
                option.attribute = std::string(attribute);
            }
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 2.0f));
        InputStringWithHint("##Other", I18N::Editor::AnotherAttribute, option.attribute);
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    int add = static_cast<int>(option.add);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 0.8f));
    if (ImGui::Combo(I18N::Editor::Adds, &add, Data::Items::ItemOptionAddKeys.data(),
                     static_cast<int>(Data::Items::ItemOptionAddKeys.size())))
    {
        option.add = static_cast<ItemOptionAdd>(add);
    }
    ImGui::SameLine();
    const bool removable = definition.options.size() > 1;
    ImGui::BeginDisabled(!removable);
    if (ImGui::SmallButton(I18N::Editor::RemoveTheOption))
    {
        definition.options.erase(definition.options.begin() + static_cast<std::ptrdiff_t>(index));
        ImGui::EndDisabled();
        return;
    }
    ImGui::EndDisabled();
    if (!Data::Items::FindItemOptionEffect(definition.type, option.attribute, option.add))
    {
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::TheClientComputesNothingForThisAttribute);
    }
    ImGui::Indent();
    RenderValue(option.value, index);
    RenderTexts(option);
    ImGui::Unindent();
    ImGui::Spacing();
}

void CMuItemOptionsUI::RenderValue(ItemOptionValue& value, size_t option)
{
    bool isList = value.isList;
    if (ImGui::Checkbox(I18N::Editor::OneValuePerLevel, &isList))
    {
        value.isList = isList;
        if (value.values.empty())
        {
            value.values = {0};
        }
        if (!isList)
        {
            // A constant has no levels, and no level type either.
            value.values.resize(1);
            value.levelType = ItemOptionLevelType::OptionLevel;
        }
    }
    if (!value.isList)
    {
        ImGui::SameLine();
        int constant = value.values.empty() ? 0 : value.values.front();
        if (InputNumber(I18N::Editor::Value, constant, -MaxValue, MaxValue))
        {
            value.values = {constant};
        }
    }
    else
    {
        ImGui::SameLine();
        bool byItemLevel = value.levelType == ItemOptionLevelType::ItemLevel;
        if (ImGui::Checkbox(I18N::Editor::ByTheItemSLevel, &byItemLevel))
        {
            value.levelType = byItemLevel ? ItemOptionLevelType::ItemLevel : ItemOptionLevelType::OptionLevel;
            // The item levels go 0 to 15, the option levels 1 to 15: drop what no option level reaches.
            if (!byItemLevel && value.values.size() > static_cast<size_t>(MaxOptionLevel))
            {
                value.values.resize(MaxOptionLevel);
            }
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::TheListGoesByTheItemSLevel0To15NotTheOptionLevel);
        }
        const int firstLevel = value.FirstLevel();
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::HighestLevel0,
                                               {std::to_string(static_cast<int>(value.values.size()) + firstLevel - 1)})
                                      .c_str());
        for (size_t level = 0; level < value.values.size(); ++level)
        {
            if (level % LevelsPerRow != 0)
            {
                ImGui::SameLine();
            }
            ImGui::PushID(static_cast<int>(level));
            ImGui::SetNextItemWidth(Fields::ScaledWidth(LevelValueWidth));
            int entry = value.values[level];
            if (ImGui::InputInt(("##" + std::to_string(level + 1)).c_str(), &entry, 0, 0))
            {
                value.values[level] = std::clamp(entry, -MaxValue, MaxValue);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(
                    "%s",
                    I18N::Format(I18N::Editor::Level0, {std::to_string(static_cast<int>(level) + firstLevel)}).c_str());
            }
            ImGui::PopID();
        }
        // Option levels 1 to 15, or the item levels 0 to 15.
        ImGui::BeginDisabled(value.values.size() >= static_cast<size_t>(MaxOptionLevel + 1 - firstLevel));
        if (ImGui::SmallButton("+"))
        {
            const int step = value.values.size() >= 2 ? value.values.back() - value.values[value.values.size() - 2]
                                                      : value.values.back();
            value.values.push_back(value.values.back() + step);
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(value.values.size() <= 1);
        if (ImGui::SmallButton("-"))
        {
            value.values.pop_back();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::FillLevels))
        {
            m_valueFillOption = option;
            m_valueFillFrom = firstLevel;
            m_valueFillStart = value.values.front();
            m_valueFillStep = value.values.size() >= 2 ? value.values[1] - value.values[0] : value.values.front();
            m_valueFillTo = static_cast<int>(value.values.size()) + firstLevel - 1;
            m_openValueFill = true;
        }
    }
    RenderTerms(value);
}

void CMuItemOptionsUI::RenderTerms(ItemOptionValue& value)
{
    for (size_t index = 0; index < value.per.size(); ++index)
    {
        ItemOptionTerm& term = value.per[index];
        ImGui::PushID(static_cast<int>(index) + 1000);
        ImGui::TextUnformatted("+");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth));
        if (ImGui::BeginCombo("##Attribute", term.attribute.c_str()))
        {
            for (const auto& attribute : Data::Items::ItemOptionTermAttributes)
            {
                if (ImGui::Selectable(std::string(attribute.name).c_str(), term.attribute == attribute.name))
                {
                    term.attribute = std::string(attribute.name);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        int divides = term.divisor > 0 ? 1 : 0;
        const std::array<const char*, 2> kinds = {I18N::Editor::Times, I18N::Editor::DividedBy};
        ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 0.8f));
        if (ImGui::Combo("##Kind", &divides, kinds.data(), static_cast<int>(kinds.size())))
        {
            const int number = std::max(term.times, term.divisor);
            term.times = divides == 0 ? number : 0;
            term.divisor = divides == 1 ? number : 0;
        }
        ImGui::SameLine();
        int& number = term.divisor > 0 ? term.divisor : term.times;
        InputNumber("##Number", number, 1, MaxValue, Fields::NumberWidth * 0.6f);
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            value.per.erase(value.per.begin() + static_cast<std::ptrdiff_t>(index));
            ImGui::PopID();
            return;
        }
        ImGui::PopID();
    }
    if (ImGui::SmallButton(I18N::Editor::AddATerm))
    {
        value.per.push_back({.attribute = "level", .divisor = 20});
    }
}

void CMuItemOptionsUI::RenderTexts(ItemOption& option)
{
    std::string names;
    for (const std::string& text : option.texts)
    {
        names += (names.empty() ? "" : ", ") + text;
    }
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
    if (InputString(I18N::Editor::Texts, names))
    {
        option.texts.clear();
        size_t start = 0;
        while (start <= names.size())
        {
            const size_t end = std::min(names.find(',', start), names.size());
            std::string name = names.substr(start, end - start);
            std::erase(name, ' ');
            if (!name.empty())
            {
                option.texts.push_back(std::move(name));
            }
            start = end + 1;
        }
    }
    for (const std::string& text : option.texts)
    {
        const Data::LocalizedString* found = m_edit.texts.Find(text);
        if (found == nullptr)
        {
            ImGui::TextColored(ErrorColor, "%s", I18N::Format(I18N::Editor::NoText0, {text}).c_str());
        }
        else
        {
            ImGui::TextDisabled("%s", found->Get("en").c_str());
        }
    }
}

void CMuItemOptionsUI::RenderUsers(const std::string& name)
{
    const long long version = ToolWindow::GetDataVersion();
    if (m_usersOf != name || m_usersVersion != version)
    {
        // A definition only the copy has (New, Copy) is picked by the items as the copy would apply it.
        m_users = FindOptionUsers(g_ItemDatabase.GetAllSlots(),
                                  IsOnlyInCopy(StoredFiles(), name) ? m_edit : StoredFiles(), name);
        m_usersOf = name;
        m_usersVersion = version;
    }
    const std::vector<int>& users = m_users;
    const int rules = CountOptionRuleUses(StoredFiles().rules, name);
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::UsedBy0ItemsAnd1Rules, {std::to_string(users.size()), std::to_string(rules)})
            .c_str());
    if (!users.empty())
    {
        RenderItemList("##Users", users);
    }
}

void CMuItemOptionsUI::RenderFillLevels()
{
    if (m_openFill)
    {
        ImGui::OpenPopup(FillPopupId);
        m_openFill = false;
        m_fillFilesLevel = -1;
    }
    const std::string title = std::string(I18N::Editor::FillLevels) + FillPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    RenderNote(I18N::Editor::EachListGoesOnByItsOwnStep);
    InputNumber(I18N::Editor::HighestLevel, m_fillLevel, 1, MaxOptionLevel, Fields::NumberWidth * 0.6f);
    UpdateFillFiles();
    for (const std::string& line : m_fillLines)
    {
        ImGui::BulletText("%s", line.c_str());
    }
    if (ImGui::Button(I18N::Editor::ApplyToTheOptions))
    {
        Apply(std::move(m_fillFiles), I18N::Editor::FillTheLevelsOfOptionLists);
        m_fillFilesLevel = -1;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuItemOptionsUI::UpdateFillFiles()
{
    const long long version = g_ItemDatabase.GetVersion();
    if (m_fillFilesLevel == m_fillLevel && m_fillFilesVersion == version)
    {
        return;
    }
    m_fillFiles = StoredFiles();
    m_fillLines.clear();
    for (ItemOptionDefinition& definition : m_fillFiles.definitions)
    {
        if (!m_marked.contains(definition.name))
        {
            continue;
        }
        const std::string before = HighestLevelText(definition);
        FillOptionLevels(definition, m_fillLevel);
        m_fillLines.push_back(definition.name + ": " + before + " -> " + HighestLevelText(definition));
    }
    m_fillFilesLevel = m_fillLevel;
    m_fillFilesVersion = version;
}

void CMuItemOptionsUI::RenderValueFill()
{
    if (m_openValueFill)
    {
        ImGui::OpenPopup(ValueFillPopupId);
        m_openValueFill = false;
    }
    const std::string title = std::string(I18N::Editor::FillLevels) + ValueFillPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ItemOptionDefinition* definition = FindEdited(m_picked);
    const bool valid = definition != nullptr && m_valueFillOption < definition->options.size();
    // A list by the item's level counts from its level 0.
    const int firstLevel = valid ? definition->options[m_valueFillOption].value.FirstLevel() : 1;
    InputNumber(I18N::Editor::FromLevel, m_valueFillFrom, firstLevel, MaxOptionLevel, Fields::NumberWidth * 0.6f);
    InputNumber(I18N::Editor::ValueThere, m_valueFillStart, -MaxValue, MaxValue);
    InputNumber(I18N::Editor::Step, m_valueFillStep, -MaxValue, MaxValue);
    InputNumber(I18N::Editor::ToLevel, m_valueFillTo, m_valueFillFrom, MaxOptionLevel, Fields::NumberWidth * 0.6f);
    m_valueFillTo = std::max(m_valueFillTo, m_valueFillFrom);
    ImGui::BeginDisabled(!valid);
    if (ImGui::Button(I18N::Editor::Fill))
    {
        FillOptionValue(definition->options[m_valueFillOption].value, m_valueFillFrom, m_valueFillStart,
                        m_valueFillStep, m_valueFillTo);
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

void CMuItemOptionsUI::Apply(ItemOptionFiles files, std::string label, std::string renamedFrom, std::string renamedTo)
{
    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Items::ValidateItemOptionFilesAsWritten(files, issues);
    m_fatalIssues.clear();
    std::ranges::copy_if(issues, std::back_inserter(m_fatalIssues), [](const Data::Items::ItemDataIssue& issue)
                         { return issue.severity == Data::Items::ItemDataIssueSeverity::Fatal; });
    if (!m_fatalIssues.empty())
    {
        m_pending.pickAfter.clear();
        return;
    }
    m_warnings.clear();
    std::ranges::copy_if(issues, std::back_inserter(m_warnings), [](const Data::Items::ItemDataIssue& issue)
                         { return issue.severity != Data::Items::ItemDataIssueSeverity::Fatal; });
    const std::vector<std::string> changed = FindChangedDefinitions(StoredFiles(), files);
    std::vector<int> reached =
        FindItemsReached(g_ItemDatabase.GetAllSlots(), StoredFiles(), files, changed, renamedFrom);
    // The items whose list a changed rule changes are reached too.
    for (const Data::Items::ItemDefinition& item :
         FindItemListChanges(g_ItemDatabase.GetAllSlots(), StoredFiles(), files, renamedFrom, renamedTo))
    {
        reached.push_back(Data::Items::MakeItemType(item.group, item.number));
    }
    std::ranges::sort(reached);
    reached.erase(std::unique(reached.begin(), reached.end()), reached.end());
    m_pending = {std::move(files),     std::move(label),   std::move(renamedFrom),
                 std::move(renamedTo), std::move(reached), std::move(m_pending.pickAfter)};
    if (m_pending.reached.empty())
    {
        Commit();
    }
    else
    {
        // D26: a change of the option files reaches all the items that link what it changes; they are listed before
        // it is applied.
        m_openConfirm = true;
    }
}

void CMuItemOptionsUI::Commit()
{
    // D26: renaming a definition renames it in the items' links and the rules; an item that follows the rules takes
    // what the changed rules give.
    const std::vector<Data::Items::ItemDefinition> relinked = FindItemListChanges(
        g_ItemDatabase.GetAllSlots(), StoredFiles(), m_pending.files, m_pending.renamedFrom, m_pending.renamedTo);
    g_ItemEdits.SetOptions(m_pending.label, m_pending.files, relinked);
    if (!m_pending.pickAfter.empty())
    {
        m_picked = m_pending.pickAfter;
        m_pending.pickAfter.clear();
    }
    if (!m_pending.renamedFrom.empty() && m_marked.erase(m_pending.renamedFrom) > 0)
    {
        m_marked.insert(m_pending.renamedTo);
    }
    // The copy takes the stored files again.
    m_edit = StoredFiles();
    m_loaded = m_edit;
    m_stale = false;
}

void CMuItemOptionsUI::RenderConfirm()
{
    if (m_openConfirm)
    {
        ImGui::OpenPopup(ConfirmPopupId);
        m_openConfirm = false;
    }
    const std::string title = std::string(I18N::Editor::ChangeTheItemOptions) + ConfirmPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::TheChangeReachesThese0Items, {std::to_string(m_pending.reached.size())}).c_str());
    RenderItemList("##Reached", m_pending.reached);
    if (ImGui::Button(I18N::Editor::ApplyToTheOptions))
    {
        Commit();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        m_pending.pickAfter.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuItemOptionsUI::OpenNameDialog(NamePurpose purpose, std::string from)
{
    m_namePurpose = purpose;
    m_nameFrom = std::move(from);
    m_name = purpose == NamePurpose::Rename ? m_nameFrom : std::string();
    m_openName = true;
}

bool CMuItemOptionsUI::IsFreeName(const std::string& name) const
{
    if (m_namePurpose == NamePurpose::NewText)
    {
        return Data::Items::Json::IsName(name) && m_edit.texts.Find(name) == nullptr;
    }
    return Data::Items::Json::IsName(name) && FindDefinitionNamed(StoredFiles(), name) == nullptr &&
           FindDefinitionNamed(m_edit, name) == nullptr;
}

void CMuItemOptionsUI::RenderNameDialog()
{
    if (m_openName)
    {
        ImGui::OpenPopup(NamePopupId);
        m_openName = false;
    }
    const std::array<const char*, 4> titles = {I18N::Editor::NewOptionDefinition, I18N::Editor::CopyAnOptionDefinition,
                                               I18N::Editor::RenameAnOptionDefinition, I18N::Editor::NewText};
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
    if (m_namePurpose == NamePurpose::New)
    {
        ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 2.0f));
        if (ImGui::BeginCombo(I18N::Editor::OptionType, TypeName(m_newType)))
        {
            for (const auto& [type, name] : Data::Items::GetEnumNames(ItemOptionType{}))
            {
                if (ImGui::Selectable(name, m_newType == type))
                {
                    m_newType = type;
                }
            }
            ImGui::EndCombo();
        }
    }
    else if (m_namePurpose == NamePurpose::Rename)
    {
        RenderNote(I18N::Editor::TheItemsAndRulesThatLinkItAreRenamedToo);
    }
    const bool free = IsFreeName(m_name);
    if (!free)
    {
        RenderNote(I18N::Editor::AFreeNameOfLettersAndDigits);
    }
    ImGui::BeginDisabled(!free);
    if (ImGui::Button(m_namePurpose == NamePurpose::Rename ? I18N::Editor::Rename : I18N::Editor::Add))
    {
        ApplyName();
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

void CMuItemOptionsUI::ApplyName()
{
    switch (m_namePurpose)
    {
    case NamePurpose::New:
        // A new or copied definition is changed on the copy and applied with the other changes.
        m_edit.definitions.push_back(MakeNewDefinition(m_edit, m_name, m_newType));
        m_picked = m_name;
        break;
    case NamePurpose::Copy:
        if (const ItemOptionDefinition* from = FindDefinitionNamed(StoredFiles(), m_nameFrom))
        {
            ItemOptionDefinition copy = *from;
            copy.name = m_name;
            // The copy is another definition for OpenMU: it has no labels until Update from server matches it.
            copy.openMu = {};
            m_edit.definitions.push_back(std::move(copy));
            m_picked = m_name;
        }
        break;
    case NamePurpose::NewText:
        // A text is changed on the copy too; its English text is the value until it is written.
        m_edit.texts.texts[m_name].Set("en", "{value}");
        break;
    case NamePurpose::Rename:
    {
        ItemOptionFiles files = StoredFiles();
        RenameOptionDefinition(files, m_nameFrom, m_name);
        m_pending.pickAfter = m_name;
        Apply(std::move(files), I18N::Editor::RenameAnOptionDefinition, m_nameFrom, m_name);
        break;
    }
    }
}

void CMuItemOptionsUI::UpdateEdit()
{
    const long long version = g_ItemDatabase.GetVersion();
    if (m_editVersion == version)
    {
        return;
    }
    m_editVersion = version;
    const EditedCopyState state = UpdateEditedCopy(m_edit, m_loaded, StoredFiles());
    m_stale = state == EditedCopyState::Stale;
    if (state == EditedCopyState::Follows)
    {
        m_fatalIssues.clear();
    }
    // An undo or redo can rename, remove or add back a definition: only the names a definition has stay marked.
    DropMissingMarks(m_marked, StoredFiles(), m_edit);
}

void CMuItemOptionsUI::UpdateUserCounts()
{
    const long long version = ToolWindow::GetDataVersion();
    // With the ancient bonus definitions only the copy has (New, Copy): items use them once applied.
    const AncientBonusPicks picks = MergeCopyPicks(StoredFiles(), m_edit);
    if (version == m_userCountsVersion && picks == m_userCountsPicks)
    {
        return;
    }
    m_userCounts = CountOptionUsers(g_ItemDatabase.GetAllSlots(), picks);
    m_usersVersion = -1;
    m_userCountsVersion = version;
    m_userCountsPicks = picks;
}

void CMuItemOptionsUI::UpdateRuleMatches()
{
    const long long version = ToolWindow::GetDataVersion();
    const std::vector<Data::Items::OptionRule>& rules = m_edit.rules;
    if (version == m_ruleMatchesVersion && rules == m_ruleMatchesOf)
    {
        return;
    }
    const std::span<const Data::Items::ItemDefinition> slots = g_ItemDatabase.GetAllSlots();
    m_ruleMatches.assign(rules.size(), 0);
    m_ruleOfSlot.assign(slots.size(), -1);
    for (size_t slot = 0; slot < slots.size(); ++slot)
    {
        const Data::Items::ItemDefinition& item = slots[slot];
        if (!item.Exists())
        {
            continue;
        }
        const auto rule = std::ranges::find_if(rules, [&item](const Data::Items::OptionRule& each)
                                               { return Data::Items::Matches(each.conditions, item); });
        if (rule != rules.end())
        {
            const auto index = static_cast<size_t>(rule - rules.begin());
            ++m_ruleMatches[index];
            m_ruleOfSlot[slot] = static_cast<int>(index);
        }
    }
    m_ruleMatchesOf = rules;
    m_ruleMatchesVersion = version;
}

int CMuItemOptionsUI::CountUsers(const std::string& name) const
{
    const auto found = m_userCounts.find(name);
    return found != m_userCounts.end() ? found->second : 0;
}

ItemOptionDefinition* CMuItemOptionsUI::FindEdited(std::string_view name)
{
    const auto found = std::ranges::find(m_edit.definitions, name, &ItemOptionDefinition::name);
    return found != m_edit.definitions.end() ? &*found : nullptr;
}

void MuEditor::Items::OpenItemOptions(const std::string& definition)
{
    g_MuEditorCore.GetToolWindows().itemOptions = true;
    g_MuItemOptionsUI.Show(definition);
    ImGui::SetWindowFocus(MuEditor::WindowIds::ItemOptions);
}

#endif // _EDITOR
