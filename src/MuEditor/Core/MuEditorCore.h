#pragma once

#ifdef _EDITOR

#include "stdafx.h"

class CMuEditorCore
{
public:
    static CMuEditorCore& GetInstance();

    void Initialize(HWND hwnd, HDC hdc);
    void Shutdown();
    void Update();
    void Render();

    bool IsEnabled() const { return m_bEditorMode; }
    void SetEnabled(bool enabled) { m_bEditorMode = enabled; }
    void ToggleEditor() { m_bEditorMode = !m_bEditorMode; }

    bool IsShowingItemEditor() const { return m_bShowItemEditor; }
    bool IsShowingSkillEditor() const { return m_bShowSkillEditor; }
    bool IsHoveringUI() const { return m_bHoveringUI; }
    void SetHoveringUI(bool hovering) { m_bHoveringUI = hovering; }

private:
    CMuEditorCore();
    ~CMuEditorCore();

    bool m_bEditorMode;
    bool m_bInitialized;
    bool m_bFrameStarted;
    bool m_bShowItemEditor;
    bool m_bShowSkillEditor;
    bool m_bHoveringUI;
    bool m_bPreviousFrameHoveringUI;  // Store previous frame's hover state for input blocking
};

// Global accessor
#define g_MuEditorCore CMuEditorCore::GetInstance()

#endif // _EDITOR
