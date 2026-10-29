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
// What takes a number that is not free, below "This number is not free" of
// the add, duplicate and move dialogs: the item on it, or the entry without a
// name or model entry without an item with what the code calls the number, how
// the Item Browser lists it and what uses the number (`usage`, FindItemUsage of
// the number; not read for an item). Nothing for a free number. The dialogs
// draw it with CItemNumberHolderView, which finds both once per number.
void RenderNumberHolder(const ItemNumberHolder& holder, const ItemUsage& usage);
} // namespace MuEditor::Items::UsageView

#endif // _EDITOR
