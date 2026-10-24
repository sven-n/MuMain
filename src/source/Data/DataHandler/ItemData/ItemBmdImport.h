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

class ItemStatProfiles;

// A stat requirement whose formula the item's stat profile (the one it names, or the one its rules
// give) keeps the base value of, where the default grows it, is kept as the value the game asks
// for, as OpenMU does: with the shipped formulas, those of the orbs, crystals and scrolls (the orb
// and scroll profiles). The original client's bmd files hold weights for them, which its code grew
// like any item's: 20 + weight * drop level * 3 / 100 (energy * 4), and a scroll's energy (group
// 15) from its level requirement where it has one: 20 + weight * level requirement * 4 / 100.
// Leadership is the value in both. An orb given the default profile holds weights like any item.
bool HasRequirementWeightsInBmd(const ItemStatProfiles& profiles, const ItemDefinition& definition);
// Import from bmd: the weights of such requirements become the values.
void ToRequirementValues(const ItemStatProfiles& profiles, ItemDefinition& definition);
// Export as bmd: the smallest weight that gives each value of such requirements, written into
// `attribute`. Returns the requirements no weight gives exactly; they get the nearest one.
std::vector<ItemRequirement> ToBmdRequirementWeights(const ItemStatProfiles& profiles, const ItemDefinition& definition,
                                                     ITEM_ATTRIBUTE& attribute);

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
