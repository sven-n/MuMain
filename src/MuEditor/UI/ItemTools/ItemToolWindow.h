#pragma once

#ifdef _EDITOR

namespace MuEditor
{
class UndoTarget;
}

// What every item tool window does: where it docks, that the editor counts
// the mouse over it, the undo and redo keys, and the line that names the
// selected item.
namespace MuEditor::Items::ToolWindow
{
// Begins the window (always call End). `title` is the translated title,
// `windowId` its "###" id.
bool Begin(const char* title, const char* windowId, bool* open);
void End();

// The item edit history, which the editor bar's Undo and Redo act on while
// an item tool is the tool worked in last (UndoTargets).
UndoTarget& GetUndoTarget();

// "Kris (0,0)" in the item name language, with what tells it apart from items of the
// same name (ItemNamesakes), or "Select an item"; false when there is no item
// to edit.
bool RenderSelectedItem(int itemType);

// The data the tools show changed: the item database or the item models.
long long GetDataVersion();
} // namespace MuEditor::Items::ToolWindow

#endif // _EDITOR
