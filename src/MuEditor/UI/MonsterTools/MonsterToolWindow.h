#pragma once

#ifdef _EDITOR

namespace MuEditor
{
class UndoTarget;
}

// What every monster tool window does: where it docks, that the editor counts
// the mouse over it, the undo and redo keys, and the line that names the
// selected monster.
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

// "Budge Dragon (2)" in the UI language, "Select a monster", or "(76) No
// monster at this number"; false when there is no monster to edit.
bool RenderSelectedMonster(int number);

// The data the tools show changed: the monster table.
int GetDataVersion();
} // namespace MuEditor::Monsters::ToolWindow

#endif // _EDITOR
