#pragma once

#include "Data/Translation/ItemReference.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>

// Puts item names into the texts that name items by number ({item:14,25}; docs/translation-system.md, "Texts that
// name items"). The UI texts of the item text groups (Game, Dialog) are expanded when the locale switches, when the
// item data is loaded, when "Translate item names" changes and when the editor changes names (Refresh); each text's
// pointer then shows its expansion, so the code that shows a text needs no change. Main thread only.
namespace Data::Translation::ItemTexts
{
// The name a reference shows in a text of the locale `textLocale` (a UI locale code), UTF-8, or nothing to keep the
// reference as written (no such item).
using NameResolver =
    std::function<std::optional<std::string>(const ItemReference& reference, std::string_view textLocale)>;

// Sets the resolver the expansion asks for names; without one (the default, and before the item data is loaded) texts
// show their references as written. Takes effect with the next Refresh or locale switch.
void SetNameResolver(NameResolver resolver);

// The UTF-8 text of the locale with each reference it can resolve replaced by the name.
std::string ExpandText(std::string_view text, std::string_view textLocale);

// Expands the UI texts again for the current locale and, when one changed, calls the locale observers so the copies
// of texts refresh as on a language switch. Returns whether a text changed.
bool Refresh();
} // namespace Data::Translation::ItemTexts
