#pragma once

#ifdef _EDITOR

// The OS cursor's shape over the editor.
namespace MuEditor::Cursor
{
// Off Windows SDL has one cursor for all of the game's windows, which the game
// keeps blank, and the ImGui backend leaves it alone there (see
// CMuEditorCore::Initialize): while the mouse is over the editor this shows
// ImGui's shape. Does nothing on Windows, where the backend sets it.
void ShowImGuiShape();

// Frees the cursors, at shutdown.
void Release();
} // namespace MuEditor::Cursor

#endif // _EDITOR
