#include "stdafx.h"

#ifdef _EDITOR

#include "IssueTable.h"

#include "IssueSubjects.h"
#include "KeyboardSelection.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string>
#include <utility>

namespace MuEditor
{
namespace
{
using Data::Items::ItemDataIssue;

// Pixel widths before the editor UI scale.
constexpr float KindColumnWidth = 70.0f;
constexpr float FieldColumnWidth = 160.0f;
constexpr float TooltipWrapWidth = 420.0f;
constexpr int ColumnCount = 4;

constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};
// Also the note that the problems are from before the last change.
constexpr ImVec4 WarningColor{1.0f, 0.8f, 0.3f, 1.0f};

bool IsError(const ItemDataIssue& issue)
{
    return issue.severity == Data::Items::ItemDataIssueSeverity::Error;
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
    std::stable_partition(m_issues.begin(), m_issues.end(), IsError);
    m_errorCount = static_cast<int>(std::count_if(m_issues.begin(), m_issues.end(), IsError));
    m_warningCount = static_cast<int>(m_issues.size()) - m_errorCount;
    m_clickedIssue = NoIssue;
    FilterIssues();
}

void IssueTable::FilterIssues()
{
    m_shownIssues.clear();
    for (int index = 0; index < static_cast<int>(m_issues.size()); ++index)
    {
        if (IsError(m_issues[index]) ? m_showErrors : m_showWarnings)
        {
            m_shownIssues.push_back(index);
        }
    }
}

void IssueTable::RenderKindBoxes()
{
    if (RenderKindBox(I18N::Editor::Errors0, m_errorCount, "###ShowErrors", m_showErrors))
    {
        FilterIssues();
    }
    ImGui::SameLine();
    if (RenderKindBox(I18N::Editor::Warnings0, m_warningCount, "###ShowWarnings", m_showWarnings))
    {
        FilterIssues();
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
    const bool error = IsError(issue);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushID(issueIndex);
    ImGui::PushStyleColor(ImGuiCol_Text, error ? ErrorColor : WarningColor);
    const bool clicked =
        ImGui::Selectable(error ? I18N::Editor::Error : I18N::Editor::Warning, issueIndex == m_clickedIssue,
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
