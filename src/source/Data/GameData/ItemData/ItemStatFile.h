#pragma once

#include "Data/GameData/ItemData/ItemStatTerms.h"
#include "Data/GameData/ItemData/StatProfileRule.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// The item stat file (Data/Items/Stats/ItemStats.json) as written: named level
// bonus tables, a default term set, named profiles that differ from it and the
// rules that give items their profile.
// ItemStatsJson reads and writes it; ItemStatProfiles resolves it. See
// docs/item-data.md.
namespace Data::Items
{
// An excellent or ancient term as the file has it: left out (a profile then
// keeps the default's), "none", or its values.
struct WrittenStatTerm
{
    enum class State : uint8_t
    {
        NotWritten,
        None,
        Values,
    };

    State state = State::NotWritten;
    StatBonusTerm values;

    bool operator==(const WrittenStatTerm& other) const = default;
};

// One stat of the default or of a profile, as the file has it. A term a
// profile writes replaces the default's as a whole.
struct WrittenStatSection
{
    std::optional<std::string> table;
    WrittenStatTerm excellent;
    WrittenStatTerm ancient;
    // Magic power only: added for every level after the sum is halved.
    std::optional<int> perLevelAfterHalving;

    bool operator==(const WrittenStatSection& other) const = default;
};

// The requirement terms of the default or of a profile, as the file has them.
// A term a profile writes replaces the default's.
struct WrittenRequirementSection
{
    // What the level requirement grows by per item level.
    std::optional<int> levelPerItemLevel;
    // What an excellent item needs more (see ItemDefinition::excellentRaisesLevel).
    std::optional<int> excellentLevelAdd;
    std::array<std::optional<RequirementFormula>, ItemRequirementStatCount> stats;
    // What the strength requirement grows by per option level.
    std::optional<int> strengthPerOptionLevel;

    bool operator==(const WrittenRequirementSection& other) const = default;
};

struct WrittenStatProfile
{
    std::string name;
    std::array<WrittenStatSection, ItemStatCount> stats;
    WrittenRequirementSection requirements;

    bool operator==(const WrittenStatProfile& other) const = default;
};

// The whole file as written; reading and writing it keeps it exactly.
struct ItemStatFile
{
    // Added to the drop level for the requirements of excellent items.
    int excellentDropLevelBonus = 0;
    // Added to the drop level for the ancient terms and the requirements of
    // ancient items that are not excellent.
    int ancientDropLevelBonus = 0;
    std::vector<LevelBonusTable> tables;
    std::array<WrittenStatSection, ItemStatCount> defaults;
    WrittenRequirementSection defaultRequirements;
    std::vector<WrittenStatProfile> profiles;
    // In order: the first rule an item matches gives its profile.
    std::vector<StatProfileRule> profileRules;

    bool operator==(const ItemStatFile& other) const = default;
};
} // namespace Data::Items
