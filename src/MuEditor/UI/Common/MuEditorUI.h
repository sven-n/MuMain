#pragma once

#ifdef _EDITOR

#include "MuEditorToolWindows.h"

struct SDL_Window;

namespace MuEditor
{
struct ToolGroup;
}

class CMuEditorUI
{
public:
    static CMuEditorUI& GetInstance();

    // With the editor open: the editor window, its menu bar and the dock space
    // of the tool windows. Closed: the Open Editor button over the game.
    void Render(bool& editorEnabled, MuEditor::ToolWindows& windows);

    // Whether the point (in ImGui's coordinates) is on the Open Editor button
    // as it was last drawn.
    bool IsOnOpenButton(float x, float y) const;

    // The editor window's bar in the current window's menu bar, for a tool
    // window that floats (MuEditor::Layout::ToolWindowBarFlags); nothing for a
    // window without a menu bar.
    void RenderToolWindowBar();

private:
    CMuEditorUI() = default;
    ~CMuEditorUI() = default;

    void RenderToolbarOpen(bool& editorEnabled);
    // In a menu bar: the Windows menu, Undo and Redo of the item tools, the UI
    // scale and the language.
    void RenderBar(MuEditor::ToolWindows& windows);
    void RenderEditorWindow(bool& editorEnabled, MuEditor::ToolWindows& windows);
    // osWindow is the OS window of the window the menu is in (Fullscreen), null
    // while it is inside the game window.
    void RenderWindowsMenu(MuEditor::ToolWindows& windows, SDL_Window* osWindow);
    // The sub-menu of a tool group: its browser, then its tools.
    void RenderToolGroupMenu(const MuEditor::ToolGroup& group, MuEditor::ToolWindows& windows);
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
