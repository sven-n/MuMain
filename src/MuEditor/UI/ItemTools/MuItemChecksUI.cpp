#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemChecksUI.h"

#include "ItemCheckTargets.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"
#include "MuItemFormulasUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"

#include <algorithm>
#include <string>

using namespace MuEditor::Items;
using Data::Items::ItemDataIssue;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float KindColumnWidth = 70.0f;
constexpr float ItemColumnWidth = 220.0f;
constexpr float FieldColumnWidth = 160.0f;
constexpr float TooltipWrapWidth = 420.0f;
constexpr int ColumnCount = 4;

constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};
// Also the hint that the problems are from before the last change.
constexpr ImVec4 WarningColor{1.0f, 0.8f, 0.3f, 1.0f};

bool IsError(const ItemDataIssue& issue)
{
    return issue.severity == Data::Items::ItemDataIssueSeverity::Error;
}

bool IsAboutAnItem(const ItemDataIssue& issue)
{
    return Data::Items::IsValidItemId(issue.group, issue.number);
}

// "Kris (0,0)", "(0,5)" for a number without an item (a model without one),
// or the file for a problem that is not about one item.
void RenderItemCell(const ItemDataIssue& issue)
{
    if (!IsAboutAnItem(issue))
    {
        ImGui::TextDisabled("%s", issue.source.c_str());
        return;
    }
    const int itemType = Data::Items::MakeItemType(issue.group, issue.number);
    if (g_ItemDatabase.Find(itemType) == nullptr)
    {
        ImGui::Text("(%d,%d)", issue.group, issue.number);
        return;
    }
    ImGui::TextUnformatted(g_ItemDatabase.GetLogName(itemType).c_str());
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

// Selects the item of the problem; a click also opens the tool of its field
// in front (an arrow key keeps the keyboard in the list and opens no window).
// A problem that is not about one item has nothing to open.
void OpenIssue(const ItemDataIssue& issue, bool clicked)
{
    if (!IsAboutAnItem(issue))
    {
        // A problem of the item stat file opens Item Formulas at its entry.
        if (clicked && issue.source == Data::Items::GetItemStatsFile().generic_string())
        {
            OpenItemFormulas(FindFormulaEntryForField(issue.field));
        }
        return;
    }
    g_ItemSelection.Select(Data::Items::MakeItemType(issue.group, issue.number));
    if (!clicked)
    {
        return;
    }
    const ItemToolWindowRef tool = GetToolWindow(GetToolForIssue(issue));
    g_MuEditorCore.GetToolWindows().*tool.open = true;
    ImGui::SetWindowFocus(tool.windowId);
}

// The box that shows or hides a kind of problem, with their count. The id
// stays when the count changes.
bool RenderKindBox(const char* countFormat, int count, const char* id, bool& shown)
{
    const std::string label = I18N::Format(countFormat, {std::to_string(count)}) + id;
    return ImGui::Checkbox(label.c_str(), &shown);
}
} // namespace

CMuItemChecksUI& CMuItemChecksUI::GetInstance()
{
    static CMuItemChecksUI instance;
    return instance;
}

void CMuItemChecksUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemChecks, MuEditor::WindowIds::ItemChecks, open))
    {
        if (!m_checked)
        {
            RunChecks();
        }
        RenderCheckLine();
        RenderNotes();
        ImGui::Separator();
        RenderTable();
    }
    ToolWindow::End();
}

void CMuItemChecksUI::RunChecks()
{
    m_issues.clear();
    g_ItemDataHandler.Validate(m_issues);
    // Errors first, each kind in the order the checks found them.
    std::stable_partition(m_issues.begin(), m_issues.end(), IsError);
    m_errorCount = static_cast<int>(std::count_if(m_issues.begin(), m_issues.end(), IsError));
    m_warningCount = static_cast<int>(m_issues.size()) - m_errorCount;
    m_checked = true;
    m_checkedDataVersion = ToolWindow::GetDataVersion();
    m_clickedIssue = NoIssue;
    FilterIssues();
}

void CMuItemChecksUI::FilterIssues()
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

void CMuItemChecksUI::RenderCheckLine()
{
    if (ImGui::Button(I18N::Editor::Check))
    {
        RunChecks();
    }
    ImGui::SameLine();
    if (RenderKindBox(I18N::Editor::Errors0, m_errorCount, "###ShowErrors", m_showErrors))
    {
        FilterIssues();
    }
    ImGui::SameLine();
    if (RenderKindBox(I18N::Editor::Warnings0, m_warningCount, "###ShowWarnings", m_showWarnings))
    {
        FilterIssues();
    }
    // Checking again on every change would cost tens of milliseconds each time.
    if (ToolWindow::GetDataVersion() != m_checkedDataVersion)
    {
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::TheItemsChangedSinceTheLastCheckCheckAgain);
    }
}

void CMuItemChecksUI::RenderNotes()
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", I18N::Editor::SaveItemsRefusesDataWithErrorsAndSavesDataWithWarnings);
    ImGui::TextWrapped("%s", I18N::Editor::ClickAProblemOfAnItemToSelectTheItemAndOpenTheToolOfItsField);
    ImGui::PopStyleColor();
}

void CMuItemChecksUI::RenderTable()
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
    ImGui::TableSetupColumn(I18N::Editor::Item, ImGuiTableColumnFlags_None, ItemColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Field, ImGuiTableColumnFlags_None, FieldColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Message, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_shownIssues.size()));
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            RenderRow(m_shownIssues[row]);
        }
    }
    ImGui::EndTable();
}

void CMuItemChecksUI::RenderRow(int issueIndex)
{
    const ItemDataIssue& issue = m_issues[issueIndex];
    const bool error = IsError(issue);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushID(issueIndex);
    ImGui::PushStyleColor(ImGuiCol_Text, error ? ErrorColor : WarningColor);
    const bool clicked =
        ImGui::Selectable(error ? I18N::Editor::Error : I18N::Editor::Warning, issueIndex == m_clickedIssue,
                          ImGuiSelectableFlags_SpanAllColumns | MuEditor::KeyboardSelection::RowFlags);
    const bool movedOnto = MuEditor::KeyboardSelection::MovedOnto();
    ImGui::PopStyleColor();
    ImGui::PopID();
    RenderIssueTooltip(issue);
    if (clicked)
    {
        m_clickedIssue = issueIndex;
        OpenIssue(issue, !movedOnto);
    }
    ImGui::TableSetColumnIndex(1);
    RenderItemCell(issue);
    ImGui::TableSetColumnIndex(2);
    ImGui::TextUnformatted(issue.field.c_str());
    ImGui::TableSetColumnIndex(3);
    ImGui::TextUnformatted(issue.message.c_str());
}

#endif // _EDITOR
