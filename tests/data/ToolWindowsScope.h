#pragma once

#ifdef _EDITOR

#include "Core/MuEditorCore.h"
#include "UI/Common/MuEditorToolWindows.h"

// The editor's tool windows all closed while it lives, and as they were
// after: for a test that opens some.
class ToolWindowsScope
{
public:
    ToolWindowsScope() : m_previous(g_MuEditorCore.GetToolWindows())
    {
        g_MuEditorCore.GetToolWindows() = MuEditor::ToolWindows{};
    }

    ~ToolWindowsScope()
    {
        g_MuEditorCore.GetToolWindows() = m_previous;
    }

    ToolWindowsScope(const ToolWindowsScope&) = delete;
    ToolWindowsScope& operator=(const ToolWindowsScope&) = delete;

private:
    MuEditor::ToolWindows m_previous;
};

#endif // _EDITOR
