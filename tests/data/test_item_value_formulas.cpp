#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemValueFormulas.h"
#include "UI/ItemTools/ItemValueFormulaText.h"
#include "UI/ItemTools/ItemStatProfileWidgets.h"
#include "ClientDataScope.h"
#include "LevelBasisScrollProfile.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

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

TEST_CASE("The item tools show the formula of each value of an item [data][items][editor]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const auto stat = [](int group, int number, ItemStat stat)
    {
        const ItemDefinition& item = *g_ItemDatabase.Find(MakeItemType(group, number));
        return DescribeStatFormula(item, stat, false, ExpandItemStatProfile(item)).text;
    };
    const auto requirement = [](int group, int number, ItemRequirement requirement)
    {
        const ItemDefinition& item = *g_ItemDatabase.Find(MakeItemType(group, number));
        return DescribeRequirementFormula(item, requirement, ExpandItemStatProfile(item)).text;
    };

    // Kris
    CHECK(stat(0, 0, ItemStat::Damage) ==
          "6 + damageIncreaseWeapons; excellent +30 (6 × 25 / 6 + 5); ancient +5 (5 + 36 / 40) [default]");
    CHECK(requirement(0, 0, ItemRequirement::Strength) ==
          "20 + 40 × (6 + 3 × item level) × 3 / 100 [default]; + 5 × option level [weapon]");
    CHECK(requirement(0, 0, ItemRequirement::Energy) ==
          "none (weight 0); with a weight: 20 + weight × (6 + 3 × item level) × 4 / 100 [default]");
    CHECK(requirement(0, 0, ItemRequirement::Level) == "none");
    // Chaos Dragon Axe: a fixed excellent bonus.
    CHECK(stat(2, 6, ItemStat::Damage) == "102 + damageIncreaseWeapons [default]; excellent +15 [item: "
                                          "excellentBonus]; ancient +7 (5 + 105 / 40) [default]");
    // Small Shield
    CHECK(stat(6, 0, ItemStat::Defense) == "1 + defenseIncreaseShields; no excellent part; ancient +((1 + "
                                           "defenseIncreaseShields) × 20 / 33 + 2) [shield]");
    CHECK(stat(6, 0, ItemStat::BlockRate) == "3 + defenseRateIncreaseShields; excellent +30 (3 × 25 / 3 + 5); no "
                                             "ancient part [default], at most 255");
    // Dragon Helm
    CHECK(stat(7, 1, ItemStat::Defense) == "24 + defenseIncreaseArmors; excellent +20 (24 × 12 / 57 + 4 + 57 / 5); "
                                           "ancient +5 ((24 + 20) × 3 / 87 + 2 + 87 / 30) [default]");
    // Wings of Elf
    CHECK(stat(12, 0, ItemStat::Defense) ==
          "10 + defenseBonus1stWings [firstTierWing]; excellent +25 (10 × 12 / 100 + 4 + 100 / 5) only when ancient: "
          "a wing's excellent options are wing options; ancient +6 ((10 + 25) × 3 / 130 + 2 + 130 / 30) [default]");
    CHECK(requirement(12, 0, ItemRequirement::Level) ==
          "180 + 4 × item level [firstTierWing]; a wing's excellent options are wing options and do not raise it");
    // Skull Staff and Battle Scepter: magic power is halved.
    CHECK(stat(5, 0, ItemStat::MagicPower) ==
          "(6 + magicPowerIncrease; excellent +30 (6 × 25 / 6 + 5); ancient +2 (2 + 36 / 60) [default]) / 2 + 2 × "
          "item level [default]");
    CHECK(stat(2, 8, ItemStat::MagicPower) ==
          "(3 + magicPowerIncrease; excellent +6 (3 × 25 / 54 + 5); ancient +3 (2 + 84 / 60) [default]) / 2 "
          "[sceptre]");
    // Ring of Ice, the Lucky Dragon Helm and the Transformation Ring
    CHECK(requirement(13, 8, ItemRequirement::Level) == "20 + 4 × item level [helper]; excellent +20 [default]");
    CHECK(requirement(7, 67, ItemRequirement::Level) ==
          "150 at every item level [default]; excellent options do not raise it [item: excellentRaisesLevel]");
    CHECK(requirement(13, 10, ItemRequirement::Level) == "given by item level: 20 (+0 to +2), 50 (+3 to +15) [item: "
                                                         "requirementsByLevel]; excellent +20 [default]");
    // Orb of Summoning
    CHECK(requirement(12, 11, ItemRequirement::Energy) ==
          "given by item level: 30 (+0), 60 (+1), 90 (+2), 130 (+3), 170 (+4), 210 (+5), 300 (+6), 500 (+7), 0 (+8 "
          "to +15) [item: requirementsByLevel]");
    // Book of Samut
    CHECK(requirement(5, 21, ItemRequirement::Energy) == "20 + 135 × (52 + 1 × item level) × 3 / 100 [book]");
    // Scroll of Wizardry Enhance, Scroll of Fire Ball and Healing Orb: the energy as stored.
    CHECK(requirement(15, 28, ItemRequirement::Energy) == "1058, kept as is [scroll]");
    CHECK(requirement(15, 3, ItemRequirement::Energy) == "40, kept as is [scroll]");
    CHECK(requirement(12, 8, ItemRequirement::Energy) == "52, kept as is [orb]");
    // Scroll of Critical Damage: leadership kept as it is.
    CHECK(requirement(12, 23, ItemRequirement::Leadership) == "300, kept as is [default]");
    // The Horn of Fenrir, whose profile is picked.
    CHECK(requirement(13, 37, ItemRequirement::Level) ==
          "300 at every item level [default]; excellent options do not raise it [item: excellentRaisesLevel]");

    // The "levelRequirement" basis, which no shipped profile uses any more: the energy takes
    // the level requirement given by level where it is above 0.
    {
        const LevelBasisScrollProfile levelBasis(ClientDirectory);
        ItemDefinition poison = *g_ItemDatabase.Find(MakeItemType(15, 0));
        poison.requirements.energy = 100;
        RequirementByLevel levels{};
        poison.requirementsByLevel.Set(ItemRequirement::Level, levels);
        CHECK(DescribeRequirementFormula(poison, ItemRequirement::Energy,
                                         ExpandStatProfile(levelBasis.GetFile(), "scroll"))
                  .text ==
              "20 + 100 × (30 + 3 × item level) × 4 / 100 (no level requirement: from the drop level) [scroll]");
        levels.fill(50);
        levels[0] = 0;
        poison.requirementsByLevel.Set(ItemRequirement::Level, levels);
        CHECK(
            DescribeRequirementFormula(poison, ItemRequirement::Energy,
                                       ExpandStatProfile(levelBasis.GetFile(), "scroll"))
                .text ==
            "20 + 100 × level requirement × 4 / 100 (the level requirement given at each item level); where the level "
            "requirement is 0: 20 + 100 × (30 + 3 × item level) × 4 / 100 [scroll]");
        levels[0] = 50;
        poison.requirementsByLevel.Set(ItemRequirement::Level, levels);
        CHECK(DescribeRequirementFormula(poison, ItemRequirement::Energy,
                                         ExpandStatProfile(levelBasis.GetFile(), "scroll"))
                  .text ==
              "20 + 100 × level requirement × 4 / 100 (the level requirement given at each item level) [scroll]");
    }

    // Excellent options raise only a level requirement above 0.
    ItemDefinition krisByLevel = *g_ItemDatabase.Find(MakeItemType(0, 0));
    RequirementByLevel level{};
    krisByLevel.requirementsByLevel.Set(ItemRequirement::Level, level);
    CHECK(DescribeRequirementFormula(krisByLevel, ItemRequirement::Level, ExpandItemStatProfile(krisByLevel)).text ==
          "given by item level: 0 (+0 to +15) [item: requirementsByLevel]");
    level.fill(50);
    level[0] = 0;
    level[1] = 0;
    krisByLevel.requirementsByLevel.Set(ItemRequirement::Level, level);
    CHECK(DescribeRequirementFormula(krisByLevel, ItemRequirement::Level, ExpandItemStatProfile(krisByLevel)).text ==
          "given by item level: 0 (+0 to +1), 50 (+2 to +15) [item: requirementsByLevel]; excellent +20 where it is "
          "above 0 [default]");

    // A draft follows what is typed.
    ItemDefinition kris = *g_ItemDatabase.Find(MakeItemType(0, 0));
    kris.requirements.strength = 41;
    kris.statProfile = "armor";
    CHECK(DescribeRequirementFormula(kris, ItemRequirement::Strength, ExpandItemStatProfile(kris)).text ==
          "20 + 41 × (6 + 3 × item level) × 3 / 100 [default]; + 5 × option level [armor]");
}

#endif // _EDITOR
