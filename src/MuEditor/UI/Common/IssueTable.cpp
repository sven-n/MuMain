#include "stdafx.h"

#ifdef _EDITOR

#include "IssueTable.h"

#include "IssueSubjects.h"
#include "KeyboardSelection.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

namespace MuEditor
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;

// Pixel widths before the editor UI scale.
constexpr float KindColumnWidth = 70.0f;
constexpr float FieldColumnWidth = 160.0f;
constexpr float TooltipWrapWidth = 420.0f;
constexpr int ColumnCount = 4;

constexpr ImVec4 FatalColor{1.0f, 0.4f, 0.4f, 1.0f};
constexpr ImVec4 ErrorColor{1.0f, 0.6f, 0.35f, 1.0f};
// Also the note that the problems are from before the last change.
constexpr ImVec4 WarningColor{1.0f, 0.8f, 0.3f, 1.0f};

// How the table shows a level: the format of its box ("Fatal: 2"), its name
// in the kind column, its color and the box's id. The labels are read when
// drawn, so they follow the editor language.
struct LevelLook
{
    ItemDataIssueSeverity severity;
    const char* const* countFormat;
    const char* const* name;
    ImVec4 color;
    const char* boxId;
};

// Worst first: the order of the boxes and of the rows.
const std::array<LevelLook, IssueTable::LevelCount> Levels = {{
    {ItemDataIssueSeverity::Fatal, &I18N::Editor::Fatal0, &I18N::Editor::Fatal, FatalColor, "###ShowFatal"},
    {ItemDataIssueSeverity::Error, &I18N::Editor::Errors0, &I18N::Editor::Error, ErrorColor, "###ShowErrors"},
    {ItemDataIssueSeverity::Warning, &I18N::Editor::Warnings0, &I18N::Editor::Warning, WarningColor, "###ShowWarnings"},
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

// The box that shows or hides a kind of problem, with their count. The id
// stays when the count changes.
bool RenderKindBox(const char* countFormat, int count, const char* id, bool& shown)
{
    const std::string label = I18N::Format(countFormat, {std::to_string(count)}) + id;
    return ImGui::Checkbox(label.c_str(), &shown);
}
} // namespace

void IssueTable::SetIssues(std::vector<ItemDataIssue> issues)
{
    m_issues = std::move(issues);
    std::stable_sort(m_issues.begin(), m_issues.end(), [](const ItemDataIssue& left, const ItemDataIssue& right)
                     { return LevelIndex(left.severity) < LevelIndex(right.severity); });
    m_counts.fill(0);
    for (const ItemDataIssue& issue : m_issues)
    {
        ++m_counts[LevelIndex(issue.severity)];
    }
    m_clickedIssue = NoIssue;
    FilterIssues();
}

int IssueTable::GetCount(ItemDataIssueSeverity severity) const
{
    return m_counts[LevelIndex(severity)];
}

void IssueTable::FilterIssues()
{
    m_shownIssues.clear();
    for (int index = 0; index < static_cast<int>(m_issues.size()); ++index)
    {
        if (m_shown[LevelIndex(m_issues[index].severity)])
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
        bool shown = m_shown[level];
        if (RenderKindBox(*Levels[level].countFormat, m_counts[level], Levels[level].boxId, shown))
        {
            m_shown[level] = shown;
            FilterIssues();
        }
    }
}

void IssueTable::RenderChangedNote(const char* text)
{
    ImGui::TextColored(WarningColor, "%s", text);
}

void IssueTable::RenderNotes(const char* saveNote, const char* clickNote)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", saveNote);
    ImGui::TextWrapped("%s", clickNote);
    ImGui::PopStyleColor();
}

void IssueTable::RenderTable(IssueSubjects& subjects)
{
    if (m_issues.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoErrorsOrWarnings);
        return;
    }
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Issues", ColumnCount, flags))
    {
        return;
    }
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn(I18N::Editor::Kind, ImGuiTableColumnFlags_None, KindColumnWidth * uiScale);
    ImGui::TableSetupColumn(subjects.GetHeader(), ImGuiTableColumnFlags_None, subjects.GetColumnWidth() * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Field, ImGuiTableColumnFlags_None, FieldColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Message, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_shownIssues.size()));
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            RenderRow(subjects, m_shownIssues[row]);
        }
    }
    ImGui::EndTable();
}

void IssueTable::RenderRow(IssueSubjects& subjects, int issueIndex)
{
    const ItemDataIssue& issue = m_issues[issueIndex];
    const LevelLook& level = Levels[LevelIndex(issue.severity)];
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushID(issueIndex);
    ImGui::PushStyleColor(ImGuiCol_Text, level.color);
    const bool clicked = ImGui::Selectable(*level.name, issueIndex == m_clickedIssue,
                                           ImGuiSelectableFlags_SpanAllColumns | KeyboardSelection::RowFlags);
    const bool movedOnto = KeyboardSelection::MovedOnto();
    ImGui::PopStyleColor();
    ImGui::PopID();
    RenderIssueTooltip(issue);
    if (clicked)
    {
        m_clickedIssue = issueIndex;
        subjects.Open(issue, !movedOnto);
    }
    ImGui::TableSetColumnIndex(1);
    subjects.RenderCell(issue);
    ImGui::TableSetColumnIndex(2);
    const std::string_view field = subjects.GetField(issue);
    ImGui::TextUnformatted(field.data(), field.data() + field.size());
    ImGui::TableSetColumnIndex(3);
    ImGui::TextUnformatted(issue.message.c_str());
}
} // namespace MuEditor

#endif // _EDITOR
