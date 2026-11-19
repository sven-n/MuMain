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
    // The level has no name line of its own, or its line's text is empty: the name is the item's name and " +level",
    // as the tooltip writes it.
    StandardName,
    // The level's name line needs values only the game has (a skill's or monster's name, a bundle count): the text
    // keeps the reference as written. So do the three below.
    UnfillableLine,
    // The level's name line names its own level through name lines ("{item:12,31+0}" in 12,31's line at +0).
    LineNamesItself,
    // The level's name line has a shared text ("@x") the tooltip file doesn't have.
    MissingLineText,
    // The level's name line names an item, or a level, no item has.
    LineNamesNoItem,
    // The language has no form of the name for the case, or does not decline names in it (NameCases.json): the name
    // itself stands in.
    MissingForm,
    // The reference asks for a case, but the level's name line has placeholders without {name}: no form can show,
    // the line's text stands in.
    UndeclinedLine,
    // A UI text's name has a '%', which the text, a printf format, would read as a conversion (ResolveUiTextName): the
    // text keeps the reference as written.
    UnshowableName,
};

// Whether the problem leaves the reference without a name, so the text keeps it as written.
bool KeepsReferenceAsWritten(ItemReferenceProblem problem);

struct ReferencedName
{
    // The name, UTF-8; empty with the problems that keep the reference as written (KeepsReferenceAsWritten).
    std::string text;
    ItemReferenceProblem problem = ItemReferenceProblem::None;
};

// The name a text's item reference shows in `locale` (the item name language of the text, see GetItemNameLocale):
// the item's name, or at a level the text of the first name line shown at that level without conditions (the standard
// name when that line has no text, as in the tooltip), and the form of the case when the reference asks for one. Needs
// no game state, unlike the tooltip's name (GetItemNameText): a name line may fill in {name}, {level}, {levelPlusOne},
// the item's level values and other items' names ({item:14,13}, in the language the line is written in).
ReferencedName ResolveReferencedName(const ItemDatabase& database, const Translation::ItemReference& reference,
                                     std::string_view locale);

// The same for a UI text (the Game and Dialog texts), which is a printf format: a name with a '%' gives none
// (UnshowableName).
ReferencedName ResolveUiTextName(const ItemDatabase& database, const Translation::ItemReference& reference,
                                 std::string_view locale);
} // namespace Data::Items
