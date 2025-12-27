#pragma once

#ifdef _EDITOR

#include "stdafx.h"

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

    bool IsShowingItemEditor() const { return m_bShowItemEditor; }

private:
    CMuEditor();
    ~CMuEditor();

    bool m_bEditorMode;
    bool m_bInitialized;
    bool m_bFrameStarted;
    bool m_bShowItemEditor;
};

// Global accessor
#define g_MuEditor CMuEditor::GetInstance()

#endif // _EDITOR
