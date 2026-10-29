#pragma once

#include "Data/GameData/ItemData/ItemStatTerms.h"
#include "Data/GameData/ItemData/StatProfileRule.h"

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

// The one profile name of the formats: "default" names the default, in an item
// file for an item that keeps it although its data gives a profile, and in a
// profile rule; no profile may be called so. The other names are data.
namespace StatProfileNames
{
constexpr const char* Default = "default";
} // namespace StatProfileNames

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

    // The default for "" and "default", UnknownProfile for a name the file does
    // not have.
    int Find(std::string_view name) const;
    // The default for an index the file does not have.
    const ItemStatProfile& Get(int index) const;
    // The names of the file's profiles, without the default.
    std::vector<std::string> GetProfileNames() const;

    // The first profile rule `item` matches, or NoStatProfileRule.
    int FindRule(const ItemDefinition& item) const;
    // The profile a rule names: "" for the default, also for "default" and a
    // name the file does not have.
    const std::string& GetRuleProfile(int rule) const;
    const std::vector<StatProfileRule>& GetRules() const
    {
        return m_rules;
    }

    int GetLevelBonus(const StatTerms& terms, int level) const;
    // The level bonus table of that name, or nullptr: what a description's placeholder names.
    const LevelBonusTable* FindTable(std::string_view name) const;
    std::vector<std::string> GetTableNames() const;
    // Whether the default or a profile gives a stat that table; the others are only for descriptions.
    bool IsTableUsedByStats(std::string_view name) const;
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
    std::vector<bool> m_tableUsedByStats;
    std::vector<std::string> m_names;
    std::vector<ItemStatProfile> m_profiles;
    std::vector<StatProfileRule> m_rules;
    std::vector<std::string> m_ruleProfiles;
    int m_excellentDropLevelBonus = 0;
    int m_ancientDropLevelBonus = 0;
};
} // namespace Data::Items
