#pragma once

#ifdef _EDITOR

class CMuEditorUI
{
public:
    static CMuEditorUI& GetInstance();

    void RenderToolbar(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor);
    void RenderCenterViewport();

private:
    CMuEditorUI() = default;
    ~CMuEditorUI() = default;

    void RenderToolbarOpen(bool& editorEnabled);
    void RenderToolbarFull(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor);
};

#define g_MuEditorUI CMuEditorUI::GetInstance()

#endif // _EDITOR
