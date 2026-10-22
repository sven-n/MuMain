#pragma once

#ifdef _EDITOR

#include "imgui.h"

// The arrow keys in the editor's lists and grids: while the keyboard is the
// editor's (CMuEditorCore::UpdateKeyboardNavigation), Up and Down (and
// PageUp, PageDown, Home, End) move through a list, and the row they move onto
// is selected as a click would select it.
namespace MuEditor::KeyboardSelection
{
// For a list row: Selectable() then also returns true when the keyboard moves
// onto the row (not with Ctrl held, which only moves the cursor).
constexpr ImGuiSelectableFlags RowFlags = ImGuiSelectableFlags_SelectOnNav;

// Whether the keyboard has just moved onto the last item, not with Ctrl held:
// a grid of buttons selects on it, and a list tells a step from a click.
bool MovedOnto();
} // namespace MuEditor::KeyboardSelection

#endif // _EDITOR
