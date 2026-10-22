#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemSyncUI.h"

#include "ItemHeldByCharacter.h"
#include "ItemSelection.h"
#include "ItemSyncApply.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <SDL3/SDL_dialog.h>

#include <algorithm>
#include <set>

using namespace MuEditor::Items;
using Data::Items::ItemDataIssue;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float ChangeColumnWidth = 80.0f;
constexpr float FieldColumnWidth = 220.0f;
constexpr float KnownColumnWidth = 60.0f;
constexpr float ItemListHeight = 260.0f;
constexpr float FieldListHeight = 220.0f;
constexpr const char* NoValue = "-";
constexpr const char* ExchangeFileName = "ItemExchange.json";

constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};
constexpr ImVec4 WarningColor{1.0f, 0.8f, 0.3f, 1.0f};

// The paths are UTF-8 text in the window.
std::filesystem::path ToPath(const std::string& text)
{
    return std::filesystem::path(std::u8string(text.begin(), text.end()));
}

std::string ToText(const std::filesystem::path& path)
{
    const std::u8string text = path.generic_u8string();
    return std::string(text.begin(), text.end());
}

float Scaled(float width)
{
    return width * g_MuEditorCore.GetUIScale();
}

const char* ChangeName(SyncChange change)
{
    switch (change)
    {
    case SyncChange::New:
        return I18N::Editor::New;
    case SyncChange::Removed:
        return I18N::Editor::Removed;
    default:
        return I18N::Editor::Changed;
    }
}

std::string ItemLabel(const SyncItem& row)
{
    return row.name + " (" + std::to_string(row.group) + "," + std::to_string(row.number) + ")";
}

// The fields a click on the row's box changes: New and Removed rows are one box.
bool IsWhole(const SyncItem& row)
{
    return row.change != SyncChange::Changed;
}

bool IsRowChecked(const SyncItem& row)
{
    if (IsWhole(row))
    {
        return row.checked;
    }
    return std::ranges::any_of(row.fields, [](const SyncField& field) { return field.checked; });
}

void SetRowChecked(SyncItem& row, bool checked)
{
    if (IsWhole(row))
    {
        row.checked = checked && row.locked.empty();
        return;
    }
    for (SyncField& field : row.fields)
    {
        field.checked = checked && field.locked.empty();
    }
}

void RenderValue(const std::optional<std::string>& value)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(value ? value->c_str() : NoValue);
    ImGui::PopTextWrapPos();
}

void SDLCALL OnFileChosen(void*, const char* const* files, int)
{
    // Nothing on an error, an empty list when the dialog was cancelled.
    if (files != nullptr && files[0] != nullptr)
    {
        g_MuItemSyncUI.ChooseFile(files[0]);
    }
}
} // namespace

CMuItemSyncUI& CMuItemSyncUI::GetInstance()
{
    static CMuItemSyncUI instance;
    return instance;
}

void CMuItemSyncUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemSync, MuEditor::WindowIds::ItemSync, open))
    {
        if (!m_knownLoaded)
        {
            std::vector<ItemDataIssue> issues;
            m_known = LoadKnownDifferences(GetKnownDifferencesFile(), issues);
            for (const ItemDataIssue& issue : issues)
            {
                g_MuEditorConsoleUI.LogEditor(issue.ToString());
            }
            m_knownLoaded = true;
        }
        TakeChosenFile();
        RenderButtons();
        if (m_diff)
        {
            ImGui::Separator();
            RenderSummary();
            RenderFieldToggle();
            RenderItems();
            RenderFormulas();
            const char* apply = m_direction == SyncDirection::Import ? I18N::Editor::ApplyTheCheckedChanges
                                                                     : I18N::Editor::WriteTheCheckedChangesIntoTheFile;
            if (ImGui::Button(apply))
            {
                Apply();
            }
        }
        RenderMessages();
    }
    ToolWindow::End();
}

void CMuItemSyncUI::ChooseFile(std::string path)
{
    const std::scoped_lock lock(m_chosenMutex);
    m_chosenPath = std::move(path);
}

void CMuItemSyncUI::TakeChosenFile()
{
    std::optional<std::string> path;
    {
        const std::scoped_lock lock(m_chosenMutex);
        path.swap(m_chosenPath);
    }
    if (path)
    {
        Compare(m_chosenDirection, ToPath(*path));
    }
}

void CMuItemSyncUI::OpenFileDialog(SyncDirection direction)
{
    static const SDL_DialogFileFilter Filters[] = {{I18N::Editor::ItemExchangeFiles, "json"}};
    {
        const std::scoped_lock lock(m_chosenMutex);
        m_chosenDirection = direction;
    }
    if (direction == SyncDirection::Import)
    {
        SDL_ShowOpenFileDialog(&OnFileChosen, nullptr, nullptr, Filters, 1, nullptr, false);
    }
    else
    {
        SDL_ShowSaveFileDialog(&OnFileChosen, nullptr, nullptr, Filters, 1, ExchangeFileName);
    }
}

void CMuItemSyncUI::RenderButtons()
{
    if (ImGui::Button(I18N::Editor::Import))
    {
        OpenFileDialog(SyncDirection::Import);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Export))
    {
        OpenFileDialog(SyncDirection::Export);
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::WhatImportAndExportDo);
    ImGui::PopTextWrapPos();

    // The path can be typed too, e.g. where no file dialog is at hand.
    ImGui::SetNextItemWidth(-Scaled(FieldColumnWidth));
    InputString(I18N::Editor::ExchangeFile, m_path);
    ImGui::SameLine();
    ImGui::BeginDisabled(m_path.empty());
    if (ImGui::Button(I18N::Editor::CompareAgain))
    {
        Compare(m_direction, ToPath(m_path));
    }
    ImGui::EndDisabled();
}

void CMuItemSyncUI::Compare(SyncDirection direction, const std::filesystem::path& path)
{
    m_direction = direction;
    m_path = ToText(path);
    m_messages.clear();
    m_messagesAreErrors = false;
    m_diff.reset();
    m_shownItem = -1;
    m_file = {};

    std::vector<ItemDataIssue> issues;
    if (const std::optional<std::string> text = Data::Items::Json::ReadTextFile(path))
    {
        ReadItemExchangeJson(*text, m_path, m_file, issues);
    }
    else if (direction == SyncDirection::Import)
    {
        issues.push_back({Data::Items::ItemDataIssueSeverity::Error, m_path, ItemDataIssue::NoItem,
                          ItemDataIssue::NoItem, "", "could not be read"});
    }
    for (const ItemDataIssue& issue : issues)
    {
        m_messages.push_back(issue.ToString());
    }
    if (Data::Items::HasErrors(issues))
    {
        m_messagesAreErrors = true;
        return;
    }

    std::vector<std::string> locales;
    for (const char* locale : I18N::GetAvailableLocales())
    {
        locales.emplace_back(locale);
    }
    const SyncOptions options{direction, &m_known, locales, [](int itemType) { return IsHeldByCharacter(itemType); }};
    m_diff = BuildItemSyncDiff(BuildClientExchange(), m_file, options);
    m_diffVersion = ToolWindow::GetDataVersion();
}

bool CMuItemSyncUI::Apply()
{
    if (!m_diff)
    {
        return false;
    }
    std::vector<ItemDataIssue> issues;
    const bool applied = m_direction == SyncDirection::Import
                             ? ApplyItemSyncImport(*m_diff, m_file, issues)
                             : ApplyItemSyncExport(*m_diff, m_file, ToPath(m_path), issues);
    if (!applied)
    {
        m_messages.clear();
        m_messages.push_back(I18N::Editor::TheChecksRefuseTheChange);
        for (const ItemDataIssue& issue : issues)
        {
            m_messages.push_back(issue.ToString());
            // The rows of the items with errors are unchecked.
            for (SyncItem& row : m_diff->items)
            {
                if (row.group == issue.group && row.number == issue.number)
                {
                    SetRowChecked(row, false);
                }
            }
        }
        m_messagesAreErrors = true;
        return false;
    }
    const std::string done = m_direction == SyncDirection::Import
                                 ? std::string(I18N::Editor::AppliedAsOneStepOfTheUndoHistory)
                                 : I18N::Format(I18N::Editor::WrittenTo0, {m_path});
    g_MuEditorConsoleUI.LogEditor(done);
    // The differences that are left: what was not checked.
    Compare(m_direction, ToPath(m_path));
    m_messages.insert(m_messages.begin(), done);
    return true;
}

void CMuItemSyncUI::SetAllChecked(bool checked)
{
    for (SyncItem& row : m_diff->items)
    {
        SetRowChecked(row, checked);
    }
    for (SyncFormula& row : m_diff->formulas)
    {
        row.checked = checked;
    }
}

void CMuItemSyncUI::RenderSummary()
{
    const char* format = m_direction == SyncDirection::Import ? I18N::Editor::ImportFrom0 : I18N::Editor::ExportInto0;
    ImGui::TextUnformatted(I18N::Format(format, {m_path}).c_str());
    if (m_file.source == Data::Items::ItemExchangeSourceOpenMU)
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::FromOpenMUServer0,
                                               {m_file.server.empty() ? std::string(NoValue) : m_file.server})
                                      .c_str());
    }
    int added = 0;
    int changed = 0;
    int removed = 0;
    for (const SyncItem& row : m_diff->items)
    {
        added += row.change == SyncChange::New ? 1 : 0;
        changed += row.change == SyncChange::Changed ? 1 : 0;
        removed += row.change == SyncChange::Removed ? 1 : 0;
    }
    if (m_diff->IsEmpty())
    {
        ImGui::TextUnformatted(I18N::Editor::NoDifferences);
    }
    else
    {
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::TheDiffCounts,
                                            {std::to_string(added), std::to_string(changed), std::to_string(removed),
                                             std::to_string(m_diff->formulas.size())})
                                   .c_str());
    }
    if (ToolWindow::GetDataVersion() != m_diffVersion)
    {
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::TheClientDataChangedSinceTheComparison);
    }
    if (ImGui::SmallButton(I18N::Editor::CheckAll))
    {
        SetAllChecked(true);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::CheckNone))
    {
        SetAllChecked(false);
    }
}

void CMuItemSyncUI::RenderFieldToggle()
{
    std::set<std::string> names;
    for (const SyncItem& row : m_diff->items)
    {
        if (row.change == SyncChange::Changed)
        {
            for (const SyncField& field : row.fields)
            {
                names.insert(field.field);
            }
        }
    }
    if (names.empty())
    {
        return;
    }
    if (!names.contains(m_toggleField))
    {
        m_toggleField = *names.begin();
    }
    ImGui::SetNextItemWidth(Scaled(FieldColumnWidth));
    if (ImGui::BeginCombo(I18N::Editor::AFieldInAllItems, m_toggleField.c_str()))
    {
        for (const std::string& name : names)
        {
            if (ImGui::Selectable(name.c_str(), name == m_toggleField))
            {
                m_toggleField = name;
            }
        }
        ImGui::EndCombo();
    }
    for (const bool checked : {true, false})
    {
        ImGui::SameLine();
        if (ImGui::SmallButton(checked ? I18N::Editor::CheckThem : I18N::Editor::UncheckThem))
        {
            for (SyncItem& row : m_diff->items)
            {
                for (SyncField& field : row.fields)
                {
                    if (row.change == SyncChange::Changed && field.field == m_toggleField)
                    {
                        field.checked = checked && field.locked.empty();
                    }
                }
            }
        }
    }
}

void CMuItemSyncUI::RenderItems()
{
    const std::string header =
        I18N::Format(I18N::Editor::Items0, {std::to_string(m_diff->items.size())}) + "###SyncItems";
    if (m_diff->items.empty() || !ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("##SyncItems", 4, Flags, ImVec2(0.0f, Scaled(ItemListHeight))))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("##Checked", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
        ImGui::TableSetupColumn(I18N::Editor::Change, ImGuiTableColumnFlags_WidthFixed, Scaled(ChangeColumnWidth));
        ImGui::TableSetupColumn(I18N::Editor::Item);
        ImGui::TableSetupColumn(I18N::Editor::Fields, ImGuiTableColumnFlags_WidthFixed, Scaled(ChangeColumnWidth));
        ImGui::TableHeadersRow();
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(m_diff->items.size()));
        while (clipper.Step())
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
            {
                SyncItem& row = m_diff->items[static_cast<size_t>(index)];
                ImGui::PushID(index);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                bool checked = IsRowChecked(row);
                ImGui::BeginDisabled(!row.locked.empty());
                if (ImGui::Checkbox("##Checked", &checked))
                {
                    SetRowChecked(row, checked);
                }
                ImGui::EndDisabled();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ChangeName(row.change));
                ImGui::TableNextColumn();
                if (row.known)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                }
                if (ImGui::Selectable(ItemLabel(row).c_str(), m_shownItem == index,
                                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
                {
                    m_shownItem = index;
                    const int itemType = Data::Items::MakeItemType(row.group, row.number);
                    if (g_ItemDatabase.Find(itemType) != nullptr)
                    {
                        g_ItemSelection.Select(itemType);
                    }
                }
                if (row.known)
                {
                    ImGui::PopStyleColor();
                }
                ImGui::TableNextColumn();
                ImGui::Text("%d", static_cast<int>(row.fields.size()));
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
    if (m_shownItem >= 0 && m_shownItem < static_cast<int>(m_diff->items.size()))
    {
        RenderItemFields(m_diff->items[static_cast<size_t>(m_shownItem)]);
    }
}

void CMuItemSyncUI::RenderItemFields(SyncItem& row)
{
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::FieldsOf0, {ItemLabel(row)}).c_str());
    if (!row.locked.empty())
    {
        ImGui::TextColored(WarningColor, "%s", row.locked.c_str());
    }
    if (row.change == SyncChange::New && m_direction == SyncDirection::Import)
    {
        const auto incoming = std::ranges::find_if(m_file.items, [&](const Data::Items::ItemDefinition& item)
                                                   { return item.group == row.group && item.number == row.number; });
        if (incoming != m_file.items.end())
        {
            for (const std::string& note : DescribeNewItem(*incoming))
            {
                ImGui::TextColored(WarningColor, "%s", note.c_str());
            }
        }
    }
    if (IsWhole(row))
    {
        bool known = row.known;
        if (ImGui::Checkbox(I18N::Editor::Known, &known))
        {
            SetKnown(row.group, row.number, "", known);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::WhatAKnownDifferenceIs);
        }
    }
    if (row.fields.empty())
    {
        return;
    }
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    const bool changed = row.change == SyncChange::Changed;
    if (!ImGui::BeginTable("##SyncFields", changed ? 6 : 3, Flags, ImVec2(0.0f, Scaled(FieldListHeight))))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##Checked", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
    ImGui::TableSetupColumn(I18N::Editor::Field, ImGuiTableColumnFlags_WidthFixed, Scaled(FieldColumnWidth));
    if (changed)
    {
        ImGui::TableSetupColumn(I18N::Editor::Now);
    }
    ImGui::TableSetupColumn(m_direction == SyncDirection::Import ? I18N::Editor::FromTheFile
                                                                 : I18N::Editor::FromTheClient);
    if (changed)
    {
        ImGui::TableSetupColumn(I18N::Editor::Note);
        ImGui::TableSetupColumn(I18N::Editor::Known, ImGuiTableColumnFlags_WidthFixed, Scaled(KnownColumnWidth));
    }
    ImGui::TableHeadersRow();
    for (size_t index = 0; index < row.fields.size(); ++index)
    {
        SyncField& field = row.fields[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (changed)
        {
            ImGui::BeginDisabled(!field.locked.empty());
            ImGui::Checkbox("##Checked", &field.checked);
            ImGui::EndDisabled();
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(field.field.c_str());
        if (changed)
        {
            ImGui::TableNextColumn();
            RenderValue(field.current);
        }
        ImGui::TableNextColumn();
        RenderValue(field.incoming);
        if (changed)
        {
            ImGui::TableNextColumn();
            const std::string& note = !field.locked.empty() ? field.locked : field.inexact;
            if (!note.empty())
            {
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextColored(WarningColor, "%s", note.c_str());
                ImGui::PopTextWrapPos();
            }
            ImGui::TableNextColumn();
            bool known = field.known;
            if (ImGui::Checkbox("##Known", &known))
            {
                SetKnown(row.group, row.number, field.field, known);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", I18N::Editor::WhatAKnownDifferenceIs);
            }
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void CMuItemSyncUI::RenderFormulas()
{
    const std::string header =
        I18N::Format(I18N::Editor::Formulas0, {std::to_string(m_diff->formulas.size())}) + "###SyncFormulas";
    if (m_diff->formulas.empty() || !ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    if (!ImGui::BeginTable("##SyncFormulas", 5, Flags, ImVec2(0.0f, Scaled(FieldListHeight))))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##Checked", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
    ImGui::TableSetupColumn(I18N::Editor::Entry, ImGuiTableColumnFlags_WidthFixed, Scaled(FieldColumnWidth));
    ImGui::TableSetupColumn(I18N::Editor::Now);
    ImGui::TableSetupColumn(m_direction == SyncDirection::Import ? I18N::Editor::FromTheFile
                                                                 : I18N::Editor::FromTheClient);
    ImGui::TableSetupColumn(I18N::Editor::Known, ImGuiTableColumnFlags_WidthFixed, Scaled(KnownColumnWidth));
    ImGui::TableHeadersRow();
    for (size_t index = 0; index < m_diff->formulas.size(); ++index)
    {
        SyncFormula& row = m_diff->formulas[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Checkbox("##Checked", &row.checked);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.entry == AllFormulasEntry ? I18N::Editor::AllFormulas : row.entry.c_str());
        ImGui::TableNextColumn();
        RenderValue(row.current);
        ImGui::TableNextColumn();
        RenderValue(row.incoming);
        ImGui::TableNextColumn();
        bool known = row.known;
        if (row.entry != AllFormulasEntry && ImGui::Checkbox("##Known", &known))
        {
            SetKnownFormula(row.entry, known);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::WhatAKnownDifferenceIs);
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void CMuItemSyncUI::RenderMessages()
{
    for (const std::string& message : m_messages)
    {
        ImGui::PushTextWrapPos(0.0f);
        if (m_messagesAreErrors)
        {
            ImGui::TextColored(ErrorColor, "%s", message.c_str());
        }
        else
        {
            ImGui::TextUnformatted(message.c_str());
        }
        ImGui::PopTextWrapPos();
    }
}

void CMuItemSyncUI::SetKnown(int group, int number, const std::string& field, bool known)
{
    if (known)
    {
        m_known.fields.insert({group, number, field});
    }
    else
    {
        m_known.fields.erase({group, number, field});
    }
    for (SyncItem& row : m_diff->items)
    {
        if (row.group != group || row.number != number)
        {
            continue;
        }
        if (field.empty())
        {
            row.known = known;
            row.checked = !known && row.locked.empty() && row.change == SyncChange::New;
        }
        for (SyncField& entry : row.fields)
        {
            if (entry.field == field)
            {
                entry.known = known;
                entry.checked = !known && entry.locked.empty() && entry.inexact.empty();
            }
        }
    }
    SaveKnownDifferences();
}

void CMuItemSyncUI::SetKnownFormula(const std::string& entry, bool known)
{
    if (known)
    {
        m_known.formulas.insert(entry);
    }
    else
    {
        m_known.formulas.erase(entry);
    }
    for (SyncFormula& row : m_diff->formulas)
    {
        if (row.entry == entry)
        {
            row.known = known;
            row.checked = !known;
        }
    }
    SaveKnownDifferences();
}

void CMuItemSyncUI::SaveKnownDifferences()
{
    std::vector<ItemDataIssue> issues;
    Data::Items::WriteItemTextFile(GetKnownDifferencesFile(), WriteKnownDifferencesJson(m_known), issues);
    for (const ItemDataIssue& issue : issues)
    {
        g_MuEditorConsoleUI.LogEditor(issue.ToString());
    }
}

#endif // _EDITOR
