#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemChecksUI.h"

#include "ItemCheckTargets.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"
#include "MuItemFormulasUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/IssueSubjects.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <string_view>
#include <utility>
#include <vector>

using namespace MuEditor::Items;
using Data::Items::ItemDataIssue;

namespace
{
// Pixel width before the editor UI scale.
constexpr float ItemColumnWidth = 220.0f;

bool IsAboutAnItem(const ItemDataIssue& issue)
{
    return Data::Items::IsValidItemId(issue.group, issue.number);
}

// The items of the problems in the issue table.
class ItemIssueSubjects final : public MuEditor::IssueSubjects
{
public:
    const char* GetHeader() const override
    {
        return I18N::Editor::Item;
    }

    float GetColumnWidth() const override
    {
        return ItemColumnWidth;
    }

    // "Kris (0,0)", "(0,5)" for a number without an item (a model without
    // one), or the file for a problem that is not about one item.
    void RenderCell(const ItemDataIssue& issue) const override
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

    std::string_view GetField(const ItemDataIssue& issue) const override
    {
        return issue.field;
    }

    // A problem that is not about one item has nothing to open.
    void Open(const ItemDataIssue& issue, bool clicked) override
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
};
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
        MuEditor::IssueTable::RenderNotes(I18N::Editor::SaveItemsRefusesDataWithErrorsAndSavesDataWithWarnings,
                                          I18N::Editor::ClickAProblemOfAnItemToSelectTheItemAndOpenTheToolOfItsField);
        ImGui::Separator();
        ItemIssueSubjects subjects;
        m_issueTable.RenderTable(subjects);
    }
    ToolWindow::End();
}

void CMuItemChecksUI::RunChecks()
{
    std::vector<ItemDataIssue> issues;
    g_ItemDataHandler.Validate(issues);
    m_issueTable.SetIssues(std::move(issues));
    m_checked = true;
    m_checkedDataVersion = ToolWindow::GetDataVersion();
}

void CMuItemChecksUI::RenderCheckLine()
{
    if (ImGui::Button(I18N::Editor::Check))
    {
        RunChecks();
    }
    ImGui::SameLine();
    m_issueTable.RenderKindBoxes();
    // Checking again on every change would cost tens of milliseconds each time.
    if (ToolWindow::GetDataVersion() != m_checkedDataVersion)
    {
        MuEditor::IssueTable::RenderChangedNote(I18N::Editor::TheItemsChangedSinceTheLastCheckCheckAgain);
    }
}

#endif // _EDITOR
