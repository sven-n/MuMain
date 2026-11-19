#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

// The option links of Item Basics (design O18): the definitions the item links ("options" in its item file), each
// opening Item Options, a picker that adds one, and whether the list is the one the option rules give.
namespace MuEditor::Items
{
// The linked definitions with their type and English display name, a remove button each, a picker of the definitions
// the item doesn't link, and, when the list differs from what the rules give, those and "Use the rules". A change is
// one undo step, the list in the order the game takes it in (ItemOptionDatabase::OrderLinks).
void RenderOptionLinks(ItemEditDraft<Data::Items::ItemDefinition>& draft, Data::Items::ItemDefinition& item);
} // namespace MuEditor::Items

#endif // _EDITOR
