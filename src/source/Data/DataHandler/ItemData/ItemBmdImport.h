#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"

#include <span>
#include <string>
#include <vector>

// Imports the legacy per-language item.bmd files into item definitions:
// the stats come from the English file, and each language adds its names.
//
// The legacy format has 30 bytes for a name. Longer names ran on into the
// following fields (two-handed flag, level, slot, ...), so those fields hold
// name bytes instead of values. The import reads the full name up to its
// terminating null byte, and takes every field a name ran into from the
// next language whose name did not reach that field.
//
// Names are UTF-8 when they are valid UTF-8, otherwise Windows-1252 (the
// encoding of the Portuguese and Spanish files).
namespace Data::Items
{
struct ItemBmdLanguage
{
    std::wstring folder; // Data/Local/<folder>/Item_<folder>.bmd
    std::string locale;  // UI locale code, e.g. "pt"
};

// Eng (en), Por (pt), Spn (es). English first: it provides the stats.
std::span<const ItemBmdLanguage> GetItemBmdLanguages();

// A field whose value was replaced because a name ran into it.
struct ItemBmdRepair
{
    int group = 0;
    int number = 0;
    std::string field;
    long long oldValue = 0;
    long long newValue = 0;
    std::string fromLocale; // empty when no language had the value; the field's default is used then
};

struct ItemBmdImportResult
{
    std::vector<ItemDefinition> items;
    std::vector<std::string> importedLocales;
    std::vector<ItemBmdRepair> repairs;
    // Names that were longer than the 30-byte name field.
    int recoveredNameCount = 0;
    // Names that were not UTF-8 and were read as Windows-1252.
    int reencodedNameCount = 0;
    std::vector<ItemDataIssue> issues;
};

// Reads Data/Local/<folder>/Item_<folder>.bmd for every language, relative
// to the working directory. Missing languages are skipped; English is
// required.
ItemBmdImportResult ImportItemBmdFiles(std::span<const ItemBmdLanguage> languages = GetItemBmdLanguages());
} // namespace Data::Items
