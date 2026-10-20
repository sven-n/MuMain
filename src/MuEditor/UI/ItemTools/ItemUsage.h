#pragma once

#ifdef _EDITOR

#include "ItemCodeReferences.h"

#include "Data/GameData/ItemData/ItemTagSet.h"

#include <span>
#include <string>
#include <vector>

namespace MuEditor::Items
{
// What uses an item, shown before it is removed or moved to another number
// (section 7): those uses stay at its number.
struct ItemUsage
{
    std::vector<Data::Items::ItemTag> tags;
    bool hasModel = false;
    // The shared model its model entry names, and how many other items use it.
    std::string sharedModel;
    size_t sharedModelOtherUsers = 0;
    // Whether the sources were found; without them `code` is empty but the
    // code may still name the item.
    bool codeKnown = false;
    std::span<const ItemCodeReference> code;
    // Game data read by item type that the item tools do not edit.
    bool inAncientSet = false;
    bool hasAddOption = false;
    bool givesBuff = false;
    bool isPet = false;

    bool IsUsed() const
    {
        return !tags.empty() || hasModel || !code.empty() || inAncientSet || hasAddOption || givesBuff || isPet;
    }
};

ItemUsage FindItemUsage(int itemType);
} // namespace MuEditor::Items

#endif // _EDITOR
