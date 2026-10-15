#pragma once

#ifdef _EDITOR

struct SDL_Window;

class CMuEditorUI
{
public:
    static CMuEditorUI& GetInstance();

    // With the editor open: the editor window, its menu bar and the dock space
    // of the tool windows. Closed: the Open Editor button over the game.
    void Render(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor,
                bool& showMapEditor, bool& showEffectBrowser, bool& showConsole);

    // Whether the point (in ImGui's coordinates) is on the Open Editor button
    // as it was last drawn.
    bool IsOnOpenButton(float x, float y) const;

private:
    CMuEditorUI() = default;
    ~CMuEditorUI() = default;

    void RenderToolbarOpen(bool& editorEnabled);
    void RenderEditorWindow(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor,
                            bool& showMapEditor, bool& showEffectBrowser, bool& showConsole);
    // editorOsWindow is the editor window's own OS window, null while it is
    // inside the game window.
    void RenderWindowsMenu(bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor, bool& showMapEditor,
                           bool& showEffectBrowser, bool& showConsole, SDL_Window* editorOsWindow);
    void RenderUIScale();
    void RenderLanguageSelector();
    void RenderCloseButton(bool& editorEnabled);

    // The Windows menu asked to lay the tool windows out as at first.
    bool m_bResetLayout = false;
    // The Open Editor button as last drawn; empty before.
    float m_openButtonMinX = 0.0f;
    float m_openButtonMinY = 0.0f;
    float m_openButtonMaxX = 0.0f;
    float m_openButtonMaxY = 0.0f;
};

#define g_MuEditorUI CMuEditorUI::GetInstance()

#endif // _EDITOR
