#pragma once

#ifdef _EDITOR

#include "stdafx.h"
#include <string>

class CMuEditor
{
public:
    static CMuEditor& GetInstance();

    void Initialize(HWND hwnd, HDC hdc);
    void Shutdown();
    void Update();
    void Render();

    bool IsEnabled() const { return m_bEditorMode; }
    void SetEnabled(bool enabled) { m_bEditorMode = enabled; }
    void ToggleEditor() { m_bEditorMode = !m_bEditorMode; }

private:
    CMuEditor();
    ~CMuEditor();

    void RenderTopToolbar();
    void RenderCenterViewport();
    void RenderBottomConsole();
    void RenderItemEditor();

    void StartGame();
    void StopGame();
    void LogEditor(const std::string& message);
    void LogGame(const std::string& message);

    bool m_bEditorMode;
    bool m_bInitialized;
    bool m_bGameRunning;
    bool m_bFrameStarted;
    bool m_bShowItemEditor;

    // Console buffers
    std::string m_strEditorConsole;
    std::string m_strGameConsole;

    // Window handles (if we need to embed separate game instance)
    HWND m_hGameWindow;
    HWND m_hGameConsole;
};

// Global accessor
#define g_MuEditor CMuEditor::GetInstance()

#endif // _EDITOR
