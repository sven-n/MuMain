#pragma once

#ifdef _EDITOR

#include "imgui.h"

namespace MuEditor
{
struct ToolGroup;
}

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

// The flags a tool window adds to its Begin() for the editor's bar: a menu bar
// while it is not docked in the editor window, where the editor window's bar
// is not at hand; one per floating window, on the tab shown at its top left
// when several tool windows are docked there.
ImGuiWindowFlags ToolWindowBarFlags(const char* windowName);
// Right after a tool window's Begin() returned true: the editor window's bar
// in its menu bar, when it has one (ToolWindowBarFlags).
void RenderToolWindowBar();

// For an item tool that has never been open (no place in imgui.ini): it docks
// as a tab beside the other item tools, also in a layout made before it
// existed. Call before PrepareToolWindow and Begin().
void DockBesideItemTools(const char* windowId);

// The same for a tool of any group: beside the group's tools, else beside its
// browser, else where its fallback group docks. The browser of a group with a
// fallback docks beside the fallback's browser first.
void DockInGroup(const char* windowId, const ToolGroup& group);
} // namespace MuEditor::Layout

#endif // _EDITOR
