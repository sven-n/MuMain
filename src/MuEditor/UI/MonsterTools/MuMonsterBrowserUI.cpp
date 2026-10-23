#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterBrowserUI.h"

#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "I18N/All.h"

#include <string>

namespace MuEditor::Monsters
{
MuMonsterBrowserUI& MuMonsterBrowserUI::GetInstance()
{
    static MuMonsterBrowserUI instance;
    return instance;
}

void MuMonsterBrowserUI::Render(bool* open)
{
    // A mark in the title while there are changes that are not saved.
    std::string title = I18N::Editor::MonsterBrowser;
    if (g_MonsterEdits.GetHistory().IsModifiedSinceSave())
    {
        title += " *";
    }
    if (ToolWindow::Begin(title.c_str(), WindowIds::MonsterBrowser, open))
    {
        ToolWindow::RenderSelectedMonster(g_MonsterSelection.GetNumber());
    }
    ToolWindow::End();
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
