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
    // No item has the level (above +15): the text keeps the reference as written.
    NoSuchLevel,
    // The level has no name line of its own: the name is the item's name and " +level", as the tooltip writes it.
    StandardName,
    // The level's name line needs values only the game has (a skill's or monster's name, a bundle count), or names
    // its own level through name lines: the text keeps the reference as written.
    UnfillableLine,
    // The language has no form of the name for the case, or does not decline names in it (NameCases.json): the name
    // itself stands in.
    MissingForm,
    // The reference asks for a case, but the level's name line has placeholders without {name}: no form can show,
    // the line's text stands in.
    UndeclinedLine,
    // The name has a '%', '{' or '}', which a text would read as its own (the UI texts are printf formats, the item
    // data's texts have placeholders): the text keeps the reference as written.
    UnshowableName,
};

struct ReferencedName
{
    // The name, UTF-8; empty with UnknownItem, NoSuchLevel, UnfillableLine and UnshowableName.
    std::string text;
    ItemReferenceProblem problem = ItemReferenceProblem::None;
};

// The name a text's item reference shows in `locale` (the item name language of the text, see GetItemNameLocale):
// the item's name, or at a level the text of the first name line shown at that level without conditions (the standard
// name when that line has no text, as in the tooltip), and the form of the case when the reference asks for one. Needs
// no game state, unlike the tooltip's name (GetItemNameText): a name line may fill in {name}, {level}, {levelPlusOne},
// the item's level values and other items' names ({item:14,13}, in the language the line is written in). A name with
// '%', '{' or '}' gives none (UnshowableName).
ReferencedName ResolveReferencedName(const ItemDatabase& database, const Translation::ItemReference& reference,
                                     std::string_view locale);
} // namespace Data::Items
