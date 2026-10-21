#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemValueFormulas.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <filesystem>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using namespace Data::Items;

// The formulas the item tools show for the values of an item, from the shipped item data.
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

ItemStatFile LoadShippedStatFile()
{
    const ItemStatsLoadResult stats = LoadItemStatsFile(ClientDirectory / GetItemStatsFile());
    REQUIRE(stats.issues.empty());
    return stats.file;
}

int TableIndex(const ItemStatFile& file, const std::optional<std::string>& name)
{
    for (size_t index = 0; index < file.tables.size(); ++index)
    {
        if (name && file.tables[index].name == *name)
        {
            return static_cast<int>(index);
        }
    }
    return StatTerms::NoTable;
}
} // namespace

TEST_CASE("The formula terms of a stat profile are the ones the game lays over the default [data][items][editor]")
{
    const ItemStatFile file = LoadShippedStatFile();
    const ItemStatProfiles profiles(file);
    std::vector<std::string> names = {"default"};
    for (const WrittenStatProfile& profile : file.profiles)
    {
        names.push_back(profile.name);
    }
    for (const std::string& name : names)
    {
        CAPTURE(name);
        const ExpandedStatProfile expanded = ExpandStatProfile(file, name);
        const ItemStatProfile& resolved = profiles.Get(profiles.Find(name));
        CHECK(expanded.name == name);
        for (size_t stat = 0; stat < ItemStatCount; ++stat)
        {
            CAPTURE(stat);
            const ExpandedStatTerms& terms = expanded.stats[stat];
            CHECK(TableIndex(file, terms.table.value) == resolved.stats[stat].table);
            CHECK(terms.excellent.value == resolved.stats[stat].excellent);
            CHECK(terms.ancient.value == resolved.stats[stat].ancient);
            CHECK(terms.perLevelAfterHalving.value == resolved.stats[stat].perLevelAfterHalving);
        }
        const ExpandedRequirementTerms& requirements = expanded.requirements;
        CHECK(requirements.levelPerItemLevel.value == resolved.requirements.levelPerItemLevel);
        CHECK(requirements.excellentLevelAdd.value == resolved.requirements.excellentLevelAdd);
        CHECK(requirements.strengthPerOptionLevel.value == resolved.requirements.strengthPerOptionLevel);
        for (size_t stat = 0; stat < ItemRequirementStatCount; ++stat)
        {
            CHECK(requirements.stats[stat].value == resolved.requirements.stats[stat]);
        }
    }

    // Where the terms come from.
    const ExpandedStatProfile shield = ExpandStatProfile(file, "shield");
    CHECK(shield.Get(ItemStat::Defense).table.source == FormulaSource{FormulaSource::Kind::Profile, "shield"});
    CHECK(shield.Get(ItemStat::Damage).table.source == FormulaSource{FormulaSource::Kind::Default, ""});
    const ExpandedStatProfile weapon = ExpandStatProfile(file, "weapon");
    CHECK(weapon.requirements.strengthPerOptionLevel.source == FormulaSource{FormulaSource::Kind::Profile, "weapon"});
    CHECK(weapon.requirements.stats[0].source == FormulaSource{FormulaSource::Kind::Default, ""});
    // An unknown name gives the default.
    CHECK(ExpandStatProfile(file, "nosuch").name == "default");
}

#endif // _EDITOR
