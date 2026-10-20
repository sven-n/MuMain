#pragma once

#ifdef _EDITOR

#include "ItemUsage.h"

namespace MuEditor::Items::UsageView
{
// What goes with the item when it is moved or removed: its tags, its model
// entry and the shared model it names. Nothing when it has none.
void RenderGoingWithItem(const ItemUsage& usage);
// What uses the item by its number and stays there, as the remove and move
// dialogs show it: the game data read by its item type and the source lines
// that name it (the first of them, with the count).
void RenderUsesAtNumber(const ItemUsage& usage);
} // namespace MuEditor::Items::UsageView

#endif // _EDITOR
