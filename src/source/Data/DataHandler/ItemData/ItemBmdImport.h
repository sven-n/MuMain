#pragma once

#include "Data/DataHandler/LegacyLanguages.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStructs.h"

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
// Data/Local/<folder>/Item_<folder>.bmd holds the items in the language.
using ItemBmdLanguage = LegacyLanguage;

// The original client's languages (GetLegacyLanguages). English first: it
// provides the stats.
std::span<const ItemBmdLanguage> GetItemBmdLanguages();

// The orbs, crystals and scrolls (items without a slot in groups 12 and 15) keep their stat
// requirements as the values the game asks for, as OpenMU does. The original client's bmd files
// hold weights for them, which its code grew like any item's: 20 + weight * drop level * 3 / 100
// (energy * 4), and a scroll's energy (group 15) from its level requirement where it has one:
// 20 + weight * level requirement * 4 / 100. Leadership is the value in both.
bool HasRequirementWeightsInBmd(const ItemDefinition& definition);
// Import from bmd: the weights of such an item become the values.
void ToRequirementValues(ItemDefinition& definition);
// Export as bmd: the smallest weight that gives each value of such an item, written into
// `attribute`. Returns the requirements no weight gives exactly; they get the nearest one.
std::vector<ItemRequirement> ToBmdRequirementWeights(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute);

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
    // Problems reading the files; with errors nothing was imported.
    std::vector<ItemDataIssue> issues;

    // Filled by CItemDataHandler::ImportFromBmd: English names kept from the
    // current data for items the English file lacks, and the problems the
    // imported data still has (Save refuses it while they are errors).
    int keptEnglishNameCount = 0;
    std::vector<ItemDataIssue> validationIssues;
};

// Reads Data/Local/<folder>/Item_<folder>.bmd for every language, relative
// to the working directory. Missing languages are skipped; English is
// required.
ItemBmdImportResult ImportItemBmdFiles(std::span<const ItemBmdLanguage> languages = GetItemBmdLanguages());
} // namespace Data::Items
