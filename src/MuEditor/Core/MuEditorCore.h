#pragma once

#ifdef _EDITOR

#include "stdafx.h"

struct SDL_Window;

class CMuEditorCore
{
public:
    static CMuEditorCore& GetInstance();

    // window/glContext are the SDL window and GL context for the ImGui SDL3
    // backend (issue #442).
    void Initialize(SDL_Window* window, void* glContext);
    void Shutdown();
    void Update();
    void Render();

    bool IsEnabled() const { return m_bEditorMode; }
    void SetEnabled(bool enabled) { m_bEditorMode = enabled; }
    void ToggleEditor() { m_bEditorMode = !m_bEditorMode; }

    bool IsShowingItemEditor() const { return m_bShowItemEditor; }
    bool IsShowingSkillEditor() const { return m_bShowSkillEditor; }
    bool IsShowingDevEditor() const { return m_bShowDevEditor; }
    bool IsShowingMapEditor() const { return m_bShowMapEditor; }
    bool IsShowingConsole() const { return m_bShowConsole; }
    bool IsHoveringUI() const { return m_bHoveringUI; }
    void SetHoveringUI(bool hovering) { m_bHoveringUI = hovering; }

    // Global scale for ALL editor UI (fonts + widget/padding sizes). Driven by the
    // -/+ buttons in the toolbar. Applied between frames, not mid-frame.
    void  SetUIScale(float scale);
    float GetUIScale() const { return m_UIScale; }

private:
    CMuEditorCore();
    ~CMuEditorCore();

    void ApplyUIScale();

    bool m_bEditorMode;
    bool m_bInitialized;
    bool m_bFrameStarted;
    bool m_bShowItemEditor;
    bool m_bShowSkillEditor;
    bool m_bShowDevEditor;
    bool m_bShowMapEditor;
    bool m_bShowConsole;
    bool m_bHoveringUI;
    bool m_bPreviousFrameHoveringUI;  // Store previous frame's hover state for input blocking

    float m_UIScale;        // 1.0 = default ImGui size
    bool  m_bScaleDirty;    // apply the new scale at the start of the next frame
};

// Global accessor
#define g_MuEditorCore CMuEditorCore::GetInstance()

#endif // _EDITOR
