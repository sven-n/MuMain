#pragma once

#ifdef _EDITOR

#include "imgui.h"

#include <vector>

// The pieces the item file tools (Item Formulas, Item Options, the option links of Item Basics) draw alike.
namespace MuEditor::Items::ToolWidgets
{
inline constexpr ImVec4 WarningColor(1.0f, 0.8f, 0.3f, 1.0f);
inline constexpr ImVec4 ErrorColor(1.0f, 0.4f, 0.4f, 1.0f);

// A grey line that wraps at the window's edge.
void RenderNote(const char* text);

// A small button that is off while the tool's copy has unapplied changes, which its change of the stored files would
// drop.
bool SmallButtonUnlessUnapplied(const char* label, bool unapplied);

// The items as a list of a few lines; a click selects one.
void RenderItemList(const char* id, const std::vector<int>& items);
} // namespace MuEditor::Items::ToolWidgets

#endif // _EDITOR
