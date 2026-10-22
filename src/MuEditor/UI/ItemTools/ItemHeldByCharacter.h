#pragma once

#ifdef _EDITOR

#include <span>
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

// Drops the cloth of what the characters wear (DeleteCloth: the client's
// characters, the character creation's and the letters' photo characters):
// the cloth of the pants that have one (RenderPartObject) moves a mesh of its model's slot
// every frame and was made from the data the slot had, so it goes before the
// item tools change an opened model (CItemDataHandler::SetOpenedModelHooks).
// The game makes it again the next time it draws the part, from the data the
// slot has then.
void DropWornCloth();

// After model slots took other data (CItemDataHandler::SetOpenedModelHooks):
// mesh 2 of the Grand Soul pants is made the grid their cloth moves again, as
// OpenItems makes it, when their slot is among them (opened again, or data
// moved into it).
void PrepareClothMeshes(std::span<const int> itemTypes);
} // namespace MuEditor::Items

#endif // _EDITOR
