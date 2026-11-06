#pragma once

#include "Data/Translation/ItemReference.h"

#include <string>
#include <string_view>

namespace Data::Items
{
class ItemDatabase;

// What naming an item from a text ({item:14,25}, Data/Translation/ItemReference.h) found besides the name.
enum class ItemReferenceProblem : unsigned char
{
    None,
    // No item has the group and number: the text keeps the reference as written.
    UnknownItem,
    // The level has no name line of its own: the name is the item's name and " +level", as the tooltip writes it.
    StandardName,
    // The level's name line needs values only the game has (a skill's or monster's name, a bundle count): the text
    // keeps the reference as written.
    UnfillableLine,
    // The language has no form of the name for the case: the name itself stands in.
    MissingForm,
};

struct ReferencedName
{
    // The name, UTF-8; empty with UnknownItem and UnfillableLine.
    std::string text;
    ItemReferenceProblem problem = ItemReferenceProblem::None;
};

// The name a text's item reference shows in `locale` (the item name language of the text, see GetItemNameLocale):
// the item's name, or at a level the text of the first name line shown at that level without conditions, and the
// form of the case when the reference asks for one. Needs no game state, unlike the tooltip's name (GetItemNameText):
// a name line may fill in {name}, {level}, {levelPlusOne} and the item's level values.
ReferencedName ResolveReferencedName(const ItemDatabase& database, const Translation::ItemReference& reference,
                                     std::string_view locale);
} // namespace Data::Items
