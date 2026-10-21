#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemUsers.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

// The stat profile list of Item Basics and Item Requirements.
namespace MuEditor::Items
{
// "None" (follow the profile rules), "default" and the profiles of the item
// stat file, each with how many items use it; a pick is one undo step.
void RenderStatProfileCombo(ItemEditDraft<Data::Items::ItemDefinition>& draft, Data::Items::ItemDefinition& item,
                            ItemStatProfileUseCounts& counts);
} // namespace MuEditor::Items

#endif // _EDITOR
