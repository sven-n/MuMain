#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorCenterPaneUI.h"
#include "imgui.h"
#include "../MuEditor/UI/ItemEditor/MuItemEditorUI.h"
#include "../MuEditor/UI/SkillEditor/MuSkillEditorUI.h"

CMuEditorCenterPaneUI& CMuEditorCenterPaneUI::GetInstance()
{
    static CMuEditorCenterPaneUI instance;
    return instance;
}

void CMuEditorCenterPaneUI::Render(bool& showItemEditor, bool& showSkillEditor)
{
    // Simply render editor windows directly without a container
    // The container was causing an extra debug window to appear

    if (showItemEditor)
    {
        g_MuItemEditorUI.Render(showItemEditor);
    }

    if (showSkillEditor)
    {
        g_MuSkillEditorUI.Render(showSkillEditor);
    }
}

#endif // _EDITOR
