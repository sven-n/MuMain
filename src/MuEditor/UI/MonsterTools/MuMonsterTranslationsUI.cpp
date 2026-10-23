#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterTranslationsUI.h"

#include "MonsterSelection.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "I18N/All.h"

namespace MuEditor::Monsters
{
MuMonsterTranslationsUI& MuMonsterTranslationsUI::GetInstance()
{
    static MuMonsterTranslationsUI instance;
    return instance;
}

void MuMonsterTranslationsUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterTranslations, WindowIds::MonsterTranslations, open))
    {
        ToolWindow::RenderSelectedMonster(g_MonsterSelection.GetNumber());
    }
    ToolWindow::End();
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
