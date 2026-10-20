#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorCenterPaneUI.h"
#include "../MuEditor/UI/ItemEditor/MuItemEditorUI.h"
#include "../MuEditor/UI/ItemTools/MuItemBrowserUI.h"
#include "../MuEditor/UI/SkillEditor/MuSkillEditorUI.h"

CMuEditorCenterPaneUI& CMuEditorCenterPaneUI::GetInstance()
{
    static CMuEditorCenterPaneUI instance;
    return instance;
}

void CMuEditorCenterPaneUI::Render(bool& showItemBrowser, bool& showItemTable, bool& showSkillEditor)
{
    // Simply render editor windows directly without a container
    // The container was causing an extra debug window to appear

    if (showItemBrowser)
    {
        g_MuItemBrowserUI.Render(&showItemBrowser);
    }

    if (showItemTable)
    {
        g_MuItemEditorUI.Render(showItemTable);
    }

    if (showSkillEditor)
    {
        g_MuSkillEditorUI.Render(showSkillEditor);
    }
}

#endif // _EDITOR
