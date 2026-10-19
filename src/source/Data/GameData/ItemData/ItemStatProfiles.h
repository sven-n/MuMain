#pragma once

#include "Data/GameData/ItemData/ItemStatTerms.h"

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The profiles of the item stat file (Data/Items/Stats/ItemStats.json) with the
// default laid under each: what an item's + level, excellent options, ancient
// set and option level add to its stats and requirements. Items name their
// profile ("statProfile"). See docs/item-data.md.
namespace Data::Items
{
struct ItemStatFile;

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
