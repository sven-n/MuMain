#pragma once

#ifdef _EDITOR

class CMuEditorCenterPaneUI
{
public:
    static CMuEditorCenterPaneUI& GetInstance();

    void Render(bool& showItemEditor);

private:
    CMuEditorCenterPaneUI() = default;
    CMuEditorCenterPaneUI(const CMuEditorCenterPaneUI&) = delete;
    CMuEditorCenterPaneUI& operator=(const CMuEditorCenterPaneUI&) = delete;
};

// Global accessor
#define g_MuEditorCenterPaneUI CMuEditorCenterPaneUI::GetInstance()

#endif // _EDITOR
