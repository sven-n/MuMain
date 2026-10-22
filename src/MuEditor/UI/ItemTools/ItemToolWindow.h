#pragma once

#ifdef _EDITOR

// What every item tool window does: where it docks, that the editor counts
// the mouse over it, the undo and redo keys, and the line that names the
// selected item.
namespace MuEditor::Items::ToolWindow
{
// Begins the window (always call End). `title` is the translated title,
// `windowId` its "###" id.
bool Begin(const char* title, const char* windowId, bool* open);
void End();

// The undo and redo buttons with what they would undo or redo, for the
// editor's bar; a click does not take the focus from the item tool.
void RenderUndoRedo();

// "Kris (0,0)" in the item name language, with what tells it apart from items of the
// same name (ItemNamesakes), or "Select an item"; false when there is no item
// to edit.
bool RenderSelectedItem(int itemType);

// The data the tools show changed: the item database or the item models.
long long GetDataVersion();
} // namespace MuEditor::Items::ToolWindow

#endif // _EDITOR
