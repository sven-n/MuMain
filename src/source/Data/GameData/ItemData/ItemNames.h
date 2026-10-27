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
// The item's own name in the language: its English name for "en", else its
// translation, also one written like the English name. Empty when it has none
// (the game then shows the English name).
const std::string& GetOwnName(const ItemDefinition& definition, std::string_view locale);

// Whether the item has a name of its own in the language (GetOwnName).
bool HasNameIn(const ItemDefinition& definition, std::string_view locale);

// The language the game shows item names in: the UI language, or English when
// the player turned translated item names off in the options window.
std::string_view GetItemNameLocale(std::string_view uiLocale, bool translateItemNames);

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
