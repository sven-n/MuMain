#pragma once

#ifdef _EDITOR

class CMuEditorUI
{
public:
    static CMuEditorUI& GetInstance();

    // With the editor open: the editor window, its menu bar and the dock space
    // of the tool windows. Closed: the Open Editor button over the game.
    void Render(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor,
                bool& showMapEditor, bool& showEffectBrowser, bool& showConsole);

private:
    CMuEditorUI() = default;
    ~CMuEditorUI() = default;

    void RenderToolbarOpen(bool& editorEnabled);
    void RenderEditorWindow(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor,
                            bool& showMapEditor, bool& showEffectBrowser, bool& showConsole);
    void RenderWindowsMenu(bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor, bool& showMapEditor,
                           bool& showEffectBrowser, bool& showConsole);
    void RenderUIScale();
    void RenderLanguageSelector();
    void RenderCloseButton(bool& editorEnabled);

    // The Windows menu asked to lay the tool windows out as at first.
    bool m_bResetLayout = false;
};

#define g_MuEditorUI CMuEditorUI::GetInstance()

#endif // _EDITOR
