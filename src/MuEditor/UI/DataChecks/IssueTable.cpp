#include "stdafx.h"

#ifdef _EDITOR

#include "IssueTable.h"

#include "DataCheckArea.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/IssueLevels.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string>
#include <utility>

namespace MuEditor::DataChecks
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;

// Pixel widths before the editor UI scale.
constexpr float LevelColumnWidth = 70.0f;
constexpr float AreaColumnWidth = 110.0f;
constexpr float WhatColumnWidth = 220.0f;
constexpr float FieldColumnWidth = 160.0f;
constexpr float TooltipWrapWidth = 420.0f;
constexpr int ColumnCount = 5;

// The box of a level: the format of its label ("Fatal: 2"), read when drawn
// so it follows the editor language, and its id. The level's name and color
// are those of IssueLevels.
struct LevelBox
{
    ItemDataIssueSeverity severity;
    const char* const* countFormat;
    const char* boxId;
};

// Worst first: the order of the boxes and of the rows.
const std::array<LevelBox, IssueTable::LevelCount> Levels = {{
    {ItemDataIssueSeverity::Fatal, &I18N::Editor::Fatal0, "###ShowFatal"},
    {ItemDataIssueSeverity::Error, &I18N::Editor::Errors0, "###ShowErrors"},
    {ItemDataIssueSeverity::Warning, &I18N::Editor::Warnings0, "###ShowWarnings"},
}};

// The level's place in Levels.
size_t LevelIndex(ItemDataIssueSeverity severity)
{
    for (size_t index = 0; index < Levels.size(); ++index)
    {
        if (Levels[index].severity == severity)
        {
            return index;
        }
    }
    return Levels.size() - 1;
}

// The whole message, which the column may cut, and the file it is about.
void RenderIssueTooltip(const ItemDataIssue& issue)
{
    if (!ImGui::BeginItemTooltip())
    {
        return;
    }
    ImGui::PushTextWrapPos(TooltipWrapWidth * g_MuEditorCore.GetUIScale());
    if (!issue.source.empty())
    {
        ImGui::TextDisabled("%s", issue.source.c_str());
    }
    ImGui::TextUnformatted(issue.message.c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

// The box that shows or hides a level, with its count. The id stays when the
// count changes.
bool RenderKindBox(const char* countFormat, int count, const char* id, bool& shown)
{
    const std::string label = I18N::Format(countFormat, {std::to_string(count)}) + id;
    return ImGui::Checkbox(label.c_str(), &shown);
}

void SetUpColumns()
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn(I18N::Editor::Level, ImGuiTableColumnFlags_None, LevelColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Area, ImGuiTableColumnFlags_None, AreaColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::What, ImGuiTableColumnFlags_None, WhatColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Field, ImGuiTableColumnFlags_None, FieldColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Message, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
}
} // namespace

void IssueTable::SetIssues(std::vector<AreaIssue> issues)
{
    m_issues = std::move(issues);
    std::stable_sort(m_issues.begin(), m_issues.end(), [](const AreaIssue& left, const AreaIssue& right)
                     { return LevelIndex(left.issue.severity) < LevelIndex(right.issue.severity); });
    m_clickedIssue = NoIssue;
    FilterIssues();
}

void IssueTable::SetAreaShown(int area, bool shown)
{
    if (area < 0)
    {
        return;
    }
    if (static_cast<size_t>(area) >= m_shownAreas.size())
    {
        m_shownAreas.resize(static_cast<size_t>(area) + 1, true);
    }
    m_shownAreas[static_cast<size_t>(area)] = shown;
    FilterIssues();
}

bool IssueTable::IsAreaShown(int area) const
{
    return area < 0 || static_cast<size_t>(area) >= m_shownAreas.size() || m_shownAreas[static_cast<size_t>(area)];
}

int IssueTable::GetCount(ItemDataIssueSeverity severity) const
{
    return m_counts[LevelIndex(severity)];
}

void IssueTable::FilterIssues()
{
    m_shownIssues.clear();
    m_counts.fill(0);
    for (int index = 0; index < static_cast<int>(m_issues.size()); ++index)
    {
        const AreaIssue& issue = m_issues[index];
        if (!IsAreaShown(issue.area))
        {
            continue;
        }
        const size_t level = LevelIndex(issue.issue.severity);
        ++m_counts[level];
        if (m_shownLevels[level])
        {
            m_shownIssues.push_back(index);
        }
    }
}

void IssueTable::RenderKindBoxes()
{
    for (size_t level = 0; level < Levels.size(); ++level)
    {
        if (level > 0)
        {
            ImGui::SameLine();
        }
        bool shown = m_shownLevels[level];
        if (RenderKindBox(*Levels[level].countFormat, m_counts[level], Levels[level].boxId, shown))
        {
            m_shownLevels[level] = shown;
            FilterIssues();
        }
    }
}

void IssueTable::RenderChangedNote(const char* text)
{
    // In the warnings' color.
    ImGui::TextColored(IssueLevels::GetColor(ItemDataIssueSeverity::Warning), "%s", text);
}

void IssueTable::RenderTable(std::span<DataCheckArea* const> areas)
{
    // Only when the check found nothing: with problems in hidden areas or of
    // hidden levels the table is empty.
    if (m_issues.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoFatalProblemsErrorsOrWarnings);
        return;
    }
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Issues", ColumnCount, flags))
    {
        return;
    }
    SetUpColumns();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_shownIssues.size()));
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            RenderRow(areas, m_shownIssues[row]);
        }
    }
    ImGui::EndTable();
}

void IssueTable::RenderRow(std::span<DataCheckArea* const> areas, int issueIndex)
{
    const AreaIssue& row = m_issues[issueIndex];
    const ItemDataIssue& issue = row.issue;
    DataCheckArea* area = row.area >= 0 && static_cast<size_t>(row.area) < areas.size() ? areas[row.area] : nullptr;
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushID(issueIndex);
    ImGui::PushStyleColor(ImGuiCol_Text, IssueLevels::GetColor(issue.severity));
    const bool clicked = ImGui::Selectable(IssueLevels::GetName(issue.severity), issueIndex == m_clickedIssue,
                                           ImGuiSelectableFlags_SpanAllColumns | KeyboardSelection::RowFlags);
    const bool movedOnto = KeyboardSelection::MovedOnto();
    ImGui::PopStyleColor();
    ImGui::PopID();
    RenderIssueTooltip(issue);
    if (clicked)
    {
        m_clickedIssue = issueIndex;
        if (area != nullptr)
        {
            area->Open(issue, !movedOnto);
        }
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(area != nullptr ? area->GetName() : "");
    ImGui::TableSetColumnIndex(2);
    if (area != nullptr)
    {
        area->RenderSubject(issue);
    }
    ImGui::TableSetColumnIndex(3);
    const std::string_view field = area != nullptr ? area->GetField(issue) : std::string_view(issue.field);
    ImGui::TextUnformatted(field.data(), field.data() + field.size());
    ImGui::TableSetColumnIndex(4);
    ImGui::TextUnformatted(issue.message.c_str());
}
} // namespace MuEditor::DataChecks

#endif // _EDITOR
