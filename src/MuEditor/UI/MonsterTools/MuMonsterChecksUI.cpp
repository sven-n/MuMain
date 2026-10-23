#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterChecksUI.h"

#include "MonsterSelection.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "I18N/All.h"

namespace MuEditor::Monsters
{
MuMonsterChecksUI& MuMonsterChecksUI::GetInstance()
{
    static MuMonsterChecksUI instance;
    return instance;
}

void MuMonsterChecksUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterChecks, WindowIds::MonsterChecks, open))
    {
        ToolWindow::RenderSelectedMonster(g_MonsterSelection.GetNumber());
    }
    ToolWindow::End();
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
