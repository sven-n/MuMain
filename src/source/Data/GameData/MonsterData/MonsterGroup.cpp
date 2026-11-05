#include "stdafx.h"

#include "MonsterGroup.h"

namespace Data::Monsters
{
const MonsterDefinition& GetSharedValues(const MonsterGroup& group)
{
    return group.numbers.size() == 1 ? group.numbers.front() : group.shared;
}

std::vector<MonsterDefinition> CollectNumberEntries(std::span<const MonsterGroup> groups)
{
    std::vector<MonsterDefinition> entries;
    for (const MonsterGroup& group : groups)
    {
        entries.insert(entries.end(), group.numbers.begin(), group.numbers.end());
    }
    return entries;
}

std::vector<MonsterGroup> MakeOneNumberGroups(std::span<const MonsterDefinition> entries)
{
    std::vector<MonsterGroup> groups;
    groups.reserve(entries.size());
    for (const MonsterDefinition& entry : entries)
    {
        groups.push_back({entry, {entry}});
    }
    return groups;
}
} // namespace Data::Monsters
