#pragma once

#ifdef _EDITOR

namespace MuEditor
{
class UndoTarget;
}

// What every monster tool window does: where it docks, that the editor counts
// the mouse over it, and the undo and redo keys.
namespace MuEditor::Monsters::ToolWindow
{
// Begins the window (always call End). `title` is the translated title,
// `windowId` its "###" id.
bool Begin(const char* title, const char* windowId, bool* open);
void End();

// The monster edit history, which the editor bar's Undo and Redo act on while
// a monster tool is the tool worked in last and a monster tool is open
// (UndoTargets).
UndoTarget& GetUndoTarget();

// The data the tools show changed: the monster table or the model table.
int GetDataVersion();
} // namespace MuEditor::Monsters::ToolWindow

#endif // _EDITOR
