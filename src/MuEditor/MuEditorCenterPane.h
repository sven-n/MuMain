#pragma once

#ifdef _EDITOR

class CMuEditorCenterPane
{
public:
    static CMuEditorCenterPane& GetInstance();

    void Render(bool& showItemEditor);

private:
    CMuEditorCenterPane() = default;
    CMuEditorCenterPane(const CMuEditorCenterPane&) = delete;
    CMuEditorCenterPane& operator=(const CMuEditorCenterPane&) = delete;
};

// Global accessor
#define g_MuEditorCenterPane CMuEditorCenterPane::GetInstance()

#endif // _EDITOR
