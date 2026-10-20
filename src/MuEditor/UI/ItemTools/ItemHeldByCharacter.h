#pragma once

#ifdef _EDITOR

#include <vector>

namespace MuEditor::Items
{
// Whether the character wears an item of that type or a window shows one (the
// inventory and its extensions, the vault, a trade, a shop, the cursor). Its
// size and slot cannot change then, and it cannot be moved or removed: an item
// grid frees the cells of an item by the size the item has when it is taken
// out, so it would free the wrong ones.
bool IsHeldByCharacter(int itemType);

// The item types IsHeldByCharacter is true for.
std::vector<int> FindHeldItemTypes();
} // namespace MuEditor::Items

#endif // _EDITOR
