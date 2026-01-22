#include "stdafx.h"

#ifdef _EDITOR

#include "../MuEditor/MuEditorCenterPane.h"
#include "MuEditor.h"
#include "../MuEditor/MuItemEditor/MuItemEditor.h"
#include "imgui.h"

CMuEditorCenterPane& CMuEditorCenterPane::GetInstance()
{
    static CMuEditorCenterPane instance;
    return instance;
}

void CMuEditorCenterPane::Render(bool& showItemEditor)
{
    // Simply render editor windows directly without a container
    // The container was causing an extra debug window to appear

    if (showItemEditor)
    {
        g_MuItemEditor.Render(showItemEditor);
    }

    // Future editor windows can be added here
    // if (showOtherEditor) { g_MuOtherEditor.Render(showOtherEditor); }
}

#endif // _EDITOR
