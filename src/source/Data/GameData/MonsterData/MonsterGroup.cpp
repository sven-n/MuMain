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

bool HasSameValue(const MonsterDefinition& left, const MonsterDefinition& right, MonsterValueField field)
{
    switch (field)
    {
    case MonsterValueField::Names:
        return left.names == right.names;
    case MonsterValueField::Kind:
        return left.kind == right.kind;
    case MonsterValueField::Model:
        return left.model == right.model;
    case MonsterValueField::SoundSet:
        return left.soundSet == right.soundSet;
    case MonsterValueField::Attacks:
        return left.attacks == right.attacks;
    case MonsterValueField::MapVariants:
        return left.mapVariants == right.mapVariants;
    case MonsterValueField::RandomVariants:
        return left.randomVariants == right.randomVariants;
    }
    return true;
}

void CopyValue(MonsterDefinition& to, const MonsterDefinition& from, MonsterValueField field)
{
    switch (field)
    {
    case MonsterValueField::Names:
        to.names = from.names;
        break;
    case MonsterValueField::Kind:
        to.kind = from.kind;
        break;
    case MonsterValueField::Model:
        to.model = from.model;
        break;
    case MonsterValueField::SoundSet:
        to.soundSet = from.soundSet;
        break;
    case MonsterValueField::Attacks:
        to.attacks = from.attacks;
        break;
    case MonsterValueField::MapVariants:
        to.mapVariants = from.mapVariants;
        break;
    case MonsterValueField::RandomVariants:
        to.randomVariants = from.randomVariants;
        break;
    }
}

MonsterDefinition MakeSharedValues(const MonsterDefinition& values)
{
    MonsterDefinition shared = values;
    shared.number = 0;
    shared.maps.clear();
    return shared;
}
} // namespace Data::Monsters
