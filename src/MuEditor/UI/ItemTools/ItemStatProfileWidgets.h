#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemUsers.h"
#include "ItemValueFormulaText.h"

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

// The terms of the profile storing `item` gives it, from the item stat file the
// editor has.
ExpandedStatProfile ExpandItemStatProfile(const Data::Items::ItemDefinition& item);

// A formula line, grey and wrapped at the window's edge, after `label` (none:
// nullptr), with its tooltip.
void RenderFormulaLine(const char* label, const FormulaLine& line);
} // namespace MuEditor::Items

#endif // _EDITOR
