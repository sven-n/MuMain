#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorCenterPaneUI.h"

#include "../MuEditor/UI/ItemEditor/MuItemEditorUI.h"

CMuEditorCenterPaneUI& CMuEditorCenterPaneUI::GetInstance()
{
    static CMuEditorCenterPaneUI instance;
    return instance;
}

void CMuEditorCenterPaneUI::Render(bool& showItemEditor)
{
    // Simply render editor windows directly without a container
    // The container was causing an extra debug window to appear

    if (showItemEditor)
    {
        g_MuItemEditorUI.Render(showItemEditor);
    }

    // Future editor windows can be added here
    // if (showOtherEditor) { g_MuOtherEditor.Render(showOtherEditor); }
}

#endif // _EDITOR
