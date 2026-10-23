#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterChecksUI.h"

#include "MonsterCheckTargets.h"
#include "MonsterChecks.h"
#include "MonsterSelection.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/IssueSubjects.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <string_view>
#include <utility>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
using Data::Items::ItemDataIssue;

// Pixel width before the editor UI scale.
constexpr float MonsterColumnWidth = 220.0f;

bool IsAboutAMonster(const ItemDataIssue& issue)
{
    return issue.number >= 0 && issue.number < Data::Monsters::MonsterNumberCount;
}

// The monsters of the problems in the issue table.
class MonsterIssueSubjects final : public IssueSubjects
{
public:
    const char* GetHeader() const override
    {
        return I18N::Editor::Monster;
    }

    float GetColumnWidth() const override
    {
        return MonsterColumnWidth;
    }

    // "2 Budge Dragon" in the UI language, "76" for a number without an entry
    // (removed since the check), or the file for a problem that is not about
    // one monster. The name is an argument, never the format: names may hold
    // a "%".
    void RenderCell(const ItemDataIssue& issue) const override
    {
        if (!IsAboutAMonster(issue))
        {
            ImGui::TextDisabled("%s", issue.source.c_str());
            return;
        }
        const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(issue.number);
        if (monster == nullptr)
        {
            ImGui::Text("%d", issue.number);
            return;
        }
        ImGui::Text("%d %s", issue.number, monster->names.Get(I18N::GetCurrentLocale()).c_str());
    }

    // The monster is in its own column: "name.de", not "monster 2 name.de".
    std::string_view GetField(const ItemDataIssue& issue) const override
    {
        return GetMonsterField(issue.field);
    }

    // A problem that is not about one monster has nothing to open.
    void Open(const ItemDataIssue& issue, bool clicked) override
    {
        if (!IsAboutAMonster(issue))
        {
            return;
        }
        g_MonsterSelection.Select(issue.number);
        if (!clicked)
        {
            return;
        }
        const ToolWindowEntry& tool = GetToolWindow(GetToolForField(issue.field));
        g_MuEditorCore.GetToolWindows().*tool.open = true;
        ImGui::SetWindowFocus(tool.windowId);
    }
};
} // namespace

MuMonsterChecksUI& MuMonsterChecksUI::GetInstance()
{
    static MuMonsterChecksUI instance;
    return instance;
}

void MuMonsterChecksUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterChecks, WindowIds::MonsterChecks, open))
    {
        if (!m_checked)
        {
            RunChecks();
        }
        RenderCheckLine();
        IssueTable::RenderNotes(I18N::Editor::SaveMonstersRefusesDataWithErrorsAndSavesDataWithWarnings,
                                I18N::Editor::MonsterChecksClickNote);
        ImGui::Separator();
        MonsterIssueSubjects subjects;
        m_issueTable.RenderTable(subjects);
    }
    ToolWindow::End();
}

void MuMonsterChecksUI::RunChecks()
{
    std::vector<ItemDataIssue> issues;
    MonsterChecks::Run(issues);
    m_issueTable.SetIssues(std::move(issues));
    m_checked = true;
    m_checkedDataVersion = ToolWindow::GetDataVersion();
}

void MuMonsterChecksUI::RenderCheckLine()
{
    if (ImGui::Button(I18N::Editor::Check))
    {
        RunChecks();
    }
    ImGui::SameLine();
    m_issueTable.RenderKindBoxes();
    // As in Item Checks, a change does not run the checks again: Check does.
    if (ToolWindow::GetDataVersion() != m_checkedDataVersion)
    {
        IssueTable::RenderChangedNote(I18N::Editor::TheMonstersChangedSinceTheLastCheckCheckAgain);
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
