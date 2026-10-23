#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/IssueTable.h"

namespace MuEditor::Monsters
{
// Monster Checks: the errors and warnings Save Monsters finds in the monster
// list (MonsterChecks), errors first. A click on a problem of a monster
// selects the monster and opens the tool of its field. The checks run when the
// window is first shown and on Check, as Item Checks does.
class MuMonsterChecksUI
{
public:
    static MuMonsterChecksUI& GetInstance();

    void Render(bool* open);

private:
    MuMonsterChecksUI() = default;

    void RunChecks();
    void RenderCheckLine();

    IssueTable m_issueTable;
    bool m_checked = false;
    // ToolWindow::GetDataVersion() when the checks ran.
    int m_checkedDataVersion = 0;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterChecksUI MuEditor::Monsters::MuMonsterChecksUI::GetInstance()

#endif // _EDITOR
