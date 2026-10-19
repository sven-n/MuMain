#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// Item names by language (an item's "name" in the item files, D17): which
// items have a name of their own in a language.
namespace Data::Items
{
// Whether the item has a name of its own in the language: its English name for
// "en", else a translation, also one written like the English name.
bool HasNameIn(const ItemDefinition& definition, std::string_view locale);

struct ItemNameCount
{
    std::string locale;
    int named = 0;
    int missing = 0;
};

// For each language, how many of the items that exist have a name of their own
// in it and how many show the English name instead.
std::vector<ItemNameCount> CountItemNames(std::span<const ItemDefinition> items, std::span<const char* const> locales);
} // namespace Data::Items
