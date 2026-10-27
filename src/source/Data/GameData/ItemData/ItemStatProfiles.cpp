#include "stdafx.h"

#include "ItemStatProfiles.h"
#include "ItemStatFile.h"

#include <algorithm>

namespace Data::Items
{
namespace
{
int FindTable(const std::vector<LevelBonusTable>& tables, const std::string& name)
{
    for (size_t index = 0; index < tables.size(); ++index)
    {
        if (tables[index].name == name)
        {
            return static_cast<int>(index);
        }
    }
    return StatTerms::NoTable;
}

void LayTerm(const WrittenStatTerm& written, std::optional<StatBonusTerm>& term)
{
    switch (written.state)
    {
    case WrittenStatTerm::State::NotWritten:
        break;
    case WrittenStatTerm::State::None:
        term.reset();
        break;
    case WrittenStatTerm::State::Values:
        term = written.values;
        break;
    }
}

// Lays the written section over the terms: what it writes replaces them.
void LaySection(const WrittenStatSection& section, const std::vector<LevelBonusTable>& tables, StatTerms& terms)
{
    if (section.table)
    {
        terms.table = FindTable(tables, *section.table);
    }
    LayTerm(section.excellent, terms.excellent);
    LayTerm(section.ancient, terms.ancient);
    if (section.perLevelAfterHalving)
    {
        terms.perLevelAfterHalving = *section.perLevelAfterHalving;
    }
}

void LayRequirements(const WrittenRequirementSection& section, RequirementTerms& terms)
{
    if (section.levelPerItemLevel)
    {
        terms.levelPerItemLevel = *section.levelPerItemLevel;
    }
    if (section.excellentLevelAdd)
    {
        terms.excellentLevelAdd = *section.excellentLevelAdd;
    }
    for (size_t stat = 0; stat < ItemRequirementStatCount; ++stat)
    {
        if (section.stats[stat])
        {
            terms.stats[stat] = *section.stats[stat];
        }
    }
    if (section.strengthPerOptionLevel)
    {
        terms.strengthPerOptionLevel = *section.strengthPerOptionLevel;
    }
}

ItemStatProfile LayProfile(const ItemStatProfile& below, const std::array<WrittenStatSection, ItemStatCount>& sections,
                           const WrittenRequirementSection& requirements, const std::vector<LevelBonusTable>& tables)
{
    ItemStatProfile profile = below;
    for (size_t stat = 0; stat < ItemStatCount; ++stat)
    {
        LaySection(sections[stat], tables, profile.stats[stat]);
    }
    LayRequirements(requirements, profile.requirements);
    return profile;
}
} // namespace

ItemStatProfiles::ItemStatProfiles() : m_names{""}, m_profiles(1) {}

ItemStatProfiles::ItemStatProfiles(const ItemStatFile& file)
    : m_tables(file.tables), m_rules(file.profileRules), m_excellentDropLevelBonus(file.excellentDropLevelBonus),
      m_ancientDropLevelBonus(file.ancientDropLevelBonus)
{
    m_names.push_back("");
    m_profiles.push_back(LayProfile(ItemStatProfile{}, file.defaults, file.defaultRequirements, m_tables));
    for (const WrittenStatProfile& profile : file.profiles)
    {
        m_names.push_back(profile.name);
        m_profiles.push_back(LayProfile(m_profiles[DefaultProfile], profile.stats, profile.requirements, m_tables));
    }
    m_tableUsedByStats.resize(m_tables.size());
    for (const ItemStatProfile& profile : m_profiles)
    {
        for (const StatTerms& terms : profile.stats)
        {
            if (terms.table >= 0 && static_cast<size_t>(terms.table) < m_tables.size())
            {
                m_tableUsedByStats[static_cast<size_t>(terms.table)] = true;
            }
        }
    }
    for (const StatProfileRule& rule : m_rules)
    {
        m_ruleProfiles.push_back(Find(rule.profile) > DefaultProfile ? rule.profile : std::string());
    }
}

int ItemStatProfiles::Find(std::string_view name) const
{
    if (name == StatProfileNames::Default)
    {
        return DefaultProfile;
    }
    for (size_t index = 0; index < m_names.size(); ++index)
    {
        if (m_names[index] == name)
        {
            return static_cast<int>(index);
        }
    }
    return UnknownProfile;
}

const ItemStatProfile& ItemStatProfiles::Get(int index) const
{
    if (index < 0 || static_cast<size_t>(index) >= m_profiles.size())
    {
        return m_profiles[DefaultProfile];
    }
    return m_profiles[static_cast<size_t>(index)];
}

std::vector<std::string> ItemStatProfiles::GetProfileNames() const
{
    return std::vector<std::string>(m_names.begin() + 1, m_names.end());
}

int ItemStatProfiles::FindRule(const ItemDefinition& item) const
{
    return FindMatchingRule(m_rules, item);
}

const std::string& ItemStatProfiles::GetRuleProfile(int rule) const
{
    static const std::string defaultProfile;
    if (rule < 0 || static_cast<size_t>(rule) >= m_ruleProfiles.size())
    {
        return defaultProfile;
    }
    return m_ruleProfiles[static_cast<size_t>(rule)];
}

int ItemStatProfiles::GetLevelBonus(const StatTerms& terms, int level) const
{
    if (terms.table < 0 || static_cast<size_t>(terms.table) >= m_tables.size())
    {
        return 0;
    }
    return m_tables[static_cast<size_t>(terms.table)].GetBonus(level);
}

const LevelBonusTable* ItemStatProfiles::FindTable(std::string_view name) const
{
    const auto found = std::find_if(m_tables.begin(), m_tables.end(),
                                    [&](const LevelBonusTable& table) { return table.name == name; });
    return found != m_tables.end() ? &*found : nullptr;
}

bool ItemStatProfiles::IsTableUsedByStats(std::string_view name) const
{
    for (size_t index = 0; index < m_tables.size(); ++index)
    {
        if (m_tables[index].name == name)
        {
            return m_tableUsedByStats[index];
        }
    }
    return false;
}

std::vector<std::string> ItemStatProfiles::GetTableNames() const
{
    std::vector<std::string> names;
    names.reserve(m_tables.size());
    for (const LevelBonusTable& table : m_tables)
    {
        names.push_back(table.name);
    }
    return names;
}
} // namespace Data::Items
