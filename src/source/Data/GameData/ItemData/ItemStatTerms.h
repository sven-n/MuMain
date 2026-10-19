#pragma once

#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

// The parts of the item stat file (Data/Items/Stats/ItemStats.json) that the
// file model (ItemStatFile.h) and the resolved profiles (ItemStatProfiles.h)
// share: the stats, the stat requirements, the level bonus tables and the
// terms. See docs/item-data.md.
namespace Data::Items
{
// The stats the file has terms for. Damage stands for the minimum and the
// maximum damage.
enum class ItemStat : uint8_t
{
    Damage,
    MagicPower,
    BlockRate,
    Defense,
    MagicDefense,

    Count
};

constexpr size_t ItemStatCount = static_cast<size_t>(ItemStat::Count);

// The stat requirements the file has formulas for, in the order of
// ItemRequirements after the level.
enum class ItemRequirementStat : uint8_t
{
    Strength,
    Dexterity,
    Energy,
    Vitality,
    Leadership,

    Count
};

constexpr size_t ItemRequirementStatCount = static_cast<size_t>(ItemRequirementStat::Count);

// What the + level adds to a stat: the value of the level (above +15 the +15
// value) plus perLevel for every level.
struct LevelBonusTable
{
    std::string name;
    LocalizedString caption;
    std::array<int, ItemLevelCount> values{};
    int perLevel = 0;

    int GetBonus(int level) const
    {
        return values[std::clamp(level, 0, ItemLevelCount - 1)] + perLevel * (level > 0 ? level : 0);
    }

    bool operator==(const LevelBonusTable& other) const = default;
};

// The excellent or ancient part of a stat. A divisor of 0 leaves its part out.
//   excellent: base * times / dropLevel + add + dropLevel / dropLevelDivisor
//   ancient:   value * times / d + add + d / dropLevelDivisor
// d is the drop level plus the ancient drop level bonus; the value is the base
// and the excellent part, and with afterLevelBonus also the level bonus.
struct StatBonusTerm
{
    int times = 0;
    int add = 0;
    int dropLevelDivisor = 0;
    bool afterLevelBonus = false;

    bool operator==(const StatBonusTerm& other) const = default;
};

// How a stat requirement grows with the item:
//   add + base * (drop level + level * levelWeight) * percent / 100
// where the drop level has the excellent or ancient drop level bonus. keepBase
// keeps the base value. levelRequirementBasis takes the item's level
// requirement (its base value, or the one given by level at the item's level)
// instead of the drop level (and no level weight) when it has one.
struct RequirementFormula
{
    bool keepBase = true;
    int add = 0;
    int levelWeight = 0;
    int percent = 0;
    bool levelRequirementBasis = false;

    bool operator==(const RequirementFormula& other) const = default;
};
} // namespace Data::Items
