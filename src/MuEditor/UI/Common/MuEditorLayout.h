#pragma once

#ifdef _EDITOR

#include "imgui.h"

// The editor window: where it first opens, and the tool windows docked in it.
namespace MuEditor::Layout
{
struct WindowRect
{
    ImVec2 position;
    ImVec2 size;
};

// Beside the game window where the game's monitor has room for it, else over
// the right of the game window; as tall as the game window.
WindowRect FirstEditorWindowRect(float uiScale);

// Submits the dock space of the tool windows, inside the editor window. Lays
// them out first when there is no layout yet or resetLayout is set: the console
// along the bottom, the other tool windows as tabs above it.
void SubmitDockSpace(bool resetLayout);

// Call right before a tool window's Begin(): floating, it keeps a tab to drag
// it back into a dock by (under the OS title bar when it is a window of its own).
void PrepareToolWindow();
} // namespace MuEditor::Layout

#endif // _EDITOR
