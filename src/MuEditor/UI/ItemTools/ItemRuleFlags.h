#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <span>

namespace MuEditor::Items
{
// A rule flag of the item data: what it allows, where the definition keeps
// it and its name in the editor.
struct ItemRuleFlag
{
    Data::Items::ItemAction action;
    bool Data::Items::ItemDefinition::* value;
    // The translated name (an I18N::Editor text), read when it is shown so it
    // follows the UI language.
    const char* const* label;
};

// The rule flags in the order of the item data (docs/item-data.md).
std::span<const ItemRuleFlag> GetItemRuleFlags();
} // namespace MuEditor::Items

#endif // _EDITOR
