#pragma once

#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The item stat file (Data/Items/Stats/ItemStats.json): what an item's + level,
// excellent options and ancient set add to its stats and requirements, as named
// level bonus tables, a default term set and named profiles that differ from
// it. Items name their profile ("statProfile"). See docs/item-data.md.
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

// How a stat requirement grows with the item:
//   add + base * (drop level + level * levelWeight) * percent / 100
// where the drop level has the excellent or ancient drop level bonus. keepBase
// keeps the base value. levelRequirementBasis takes the item's level
// requirement instead of the drop level (and no level weight) when it has one.
struct RequirementFormula
{
    bool keepBase = true;
    int add = 0;
    int levelWeight = 0;
    int percent = 0;
    bool levelRequirementBasis = false;

    bool operator==(const RequirementFormula& other) const = default;
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

    bool operator==(const ItemStatFile& other) const = default;
};

// The terms of one stat with the profile laid over the default.
struct StatTerms
{
    // Index into the tables of ItemStatProfiles; NoTable adds nothing.
    static constexpr int NoTable = -1;

    int table = NoTable;
    std::optional<StatBonusTerm> excellent;
    std::optional<StatBonusTerm> ancient;
    int perLevelAfterHalving = 0;
};

// The requirement terms with the profile laid over the default.
struct RequirementTerms
{
    int levelPerItemLevel = 0;
    int excellentLevelAdd = 0;
    std::array<RequirementFormula, ItemRequirementStatCount> stats;
    int strengthPerOptionLevel = 0;

    const RequirementFormula& Get(ItemRequirementStat stat) const
    {
        return stats[static_cast<size_t>(stat)];
    }
};

struct ItemStatProfile
{
    std::array<StatTerms, ItemStatCount> stats;
    RequirementTerms requirements;

    const StatTerms& Get(ItemStat stat) const
    {
        return stats[static_cast<size_t>(stat)];
    }
};

// The profiles of the stat file with every name resolved: index 0 is the
// default, the others follow the file's profiles. Built when the items load,
// so the stat code never looks a name up.
class ItemStatProfiles
{
public:
    static constexpr int DefaultProfile = 0;
    static constexpr int UnknownProfile = -1;

    // Only the default, without tables or terms: every stat and requirement keeps
    // its base value.
    ItemStatProfiles();
    // The file as ReadItemStatsJson gives it; a name it does not know adds nothing.
    explicit ItemStatProfiles(const ItemStatFile& file);

    // The default for "", UnknownProfile for a name the file does not have.
    int Find(std::string_view name) const;
    // The default for an index the file does not have.
    const ItemStatProfile& Get(int index) const;
    // The names of the file's profiles, without the default.
    std::vector<std::string> GetProfileNames() const;

    int GetLevelBonus(const StatTerms& terms, int level) const;
    int GetExcellentDropLevelBonus() const
    {
        return m_excellentDropLevelBonus;
    }
    int GetAncientDropLevelBonus() const
    {
        return m_ancientDropLevelBonus;
    }

private:
    std::vector<LevelBonusTable> m_tables;
    std::vector<std::string> m_names;
    std::vector<ItemStatProfile> m_profiles;
    int m_excellentDropLevelBonus = 0;
    int m_ancientDropLevelBonus = 0;
};
} // namespace Data::Items
