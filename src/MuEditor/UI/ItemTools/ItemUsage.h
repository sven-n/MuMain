#pragma once

#ifdef _EDITOR

#include "ItemCodeReferences.h"

#include "Data/GameData/ItemData/ItemTagSet.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Items
{
// What uses an item, shown before it is removed or moved to another number
// (section 7): its tags and its model entry go with the item; the game data
// and the code that read it by its number stay there.
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
        return GoesWithItem() || IsUsedAtNumber();
    }
    // Its tags or its model entry, which move or go with the item.
    bool GoesWithItem() const
    {
        return !tags.empty() || hasModel;
    }
    // What reads the item by its number and stays there.
    bool IsUsedAtNumber() const
    {
        return !code.empty() || inAncientSet || hasAddOption || givesBuff || isPet;
    }
};

ItemUsage FindItemUsage(int itemType);

// What takes a number that is not free (Data::Items::IsFreeItemType), as the
// add, duplicate and move dialogs show it: the item on it, an entry without a
// name (Sphere 4 and 5: their models, but no item yet) or a model entry without
// an item entry (only in files the data checks report).
struct ItemNumberHolder
{
    // "Kris (0,0)"; empty when no item is on the number.
    std::string item;
    bool entryWithoutName = false;
    // Without an item: the shared model its model entry names, else its file.
    std::string model;
    // "ITEM_SPHERE_4"; empty when the sources were not found or name no constant for it.
    std::string_view codeName;
};

ItemNumberHolder FindItemNumberHolder(int itemType);

// The item's log name, "Kris (0,0)", or "Entry without a name (12,73)" in the UI
// language for an entry without a name.
std::string GetEntryLabel(int itemType);

// How the item tools name a number without an item, in the UI language: "Entry without a
// name" for an entry without a name, else "No item at this number".
const char* GetNumberWithoutItemLabel(int itemType);
} // namespace MuEditor::Items

#endif // _EDITOR
