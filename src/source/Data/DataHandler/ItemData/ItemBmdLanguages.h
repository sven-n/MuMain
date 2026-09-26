#pragma once

#include <span>
#include <string>

namespace Data::Items
{
// A language the item editor's "Export as bmd" writes an item file for.
struct ItemBmdLanguage
{
    std::wstring folder; // Data/Local/<folder>/Item_<folder>.bmd
    std::string locale;  // UI locale code, e.g. "pt"
};

// Eng (en), Por (pt), Spn (es).
std::span<const ItemBmdLanguage> GetItemBmdLanguages();
} // namespace Data::Items
