#pragma once

#ifdef _EDITOR

#include "ItemUsage.h"

namespace MuEditor::Items::UsageView
{
// What uses the item, as the remove and move dialogs show it: its tags, its
// model and shared model, the source lines that name it (the first of them,
// with the count) and the game data read by its item type.
void Render(const ItemUsage& usage);
} // namespace MuEditor::Items::UsageView

#endif // _EDITOR
