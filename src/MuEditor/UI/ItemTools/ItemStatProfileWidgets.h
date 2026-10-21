#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemUsers.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

// The stat profile list of Item Basics and Item Requirements.
namespace MuEditor::Items
{
// "By the rules" (what the profile rules give), "default" and the profiles of
// the item stat file, each with how many items use it, and beside it where the
// item's profile comes from: the rule that gives it, or that it is picked for
// the item. A pick is one undo step.
void RenderStatProfileCombo(ItemEditDraft<Data::Items::ItemDefinition>& draft, Data::Items::ItemDefinition& item,
                            ItemStatProfileUseCounts& counts);
} // namespace MuEditor::Items

#endif // _EDITOR
