#pragma once

#ifdef _EDITOR

namespace MuEditor
{
struct ToolGroup;
class UndoTarget;
} // namespace MuEditor

// What every tool window of a tool group does: where it docks, the editor's
// bar when it floats, that the editor counts the mouse over it, and the undo
// and redo keys of its group's history.
namespace MuEditor::ToolWindowFrame
{
// Begins the window (always call End). `title` is the translated title,
// `windowId` its "###" id. While the window has the focus, the editor bar's
// Undo and Redo act on `undo`.
bool Begin(const char* title, const char* windowId, bool* open, const ToolGroup& group, UndoTarget& undo);
void End();

// Ctrl+Z, Ctrl+Y and Ctrl+Shift+Z act on `undo`; only while the keyboard is
// the editor's. Begin calls it for the window's own undo.
void HandleUndoKeys(UndoTarget& undo);

// The mouse over the current window (or one of its popups) is over the
// editor, not the game. Begin calls it.
void NoteHovered();
} // namespace MuEditor::ToolWindowFrame

#endif // _EDITOR
