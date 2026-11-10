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

std::span<const MonsterValueField> GetMonsterValueFields()
{
    static const std::vector<MonsterValueField> fields = []
    {
        std::vector<MonsterValueField> all(MonsterValueFields.begin(), MonsterValueFields.end());
        for (size_t index = 0; index < GetMonsterSetupFields().size(); ++index)
            all.push_back(GetSetupValueField(index));
        return all;
    }();
    return fields;
}

MonsterValueField GetSetupValueField(size_t index)
{
    return static_cast<MonsterValueField>(static_cast<int>(MonsterValueField::SetupFirst) + static_cast<int>(index));
}

std::optional<size_t> GetSetupFieldIndex(MonsterValueField field)
{
    const int index = static_cast<int>(field) - static_cast<int>(MonsterValueField::SetupFirst);
    if (index < 0 || static_cast<size_t>(index) >= GetMonsterSetupFields().size())
        return std::nullopt;
    return static_cast<size_t>(index);
}

bool HasSameValue(const MonsterDefinition& left, const MonsterDefinition& right, MonsterValueField field)
{
    if (const std::optional<size_t> setup = GetSetupFieldIndex(field))
        return GetMonsterSetupFields()[*setup].isSame(left.setup, right.setup);
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
    case MonsterValueField::SetupFirst:
        // A setup field, compared above.
        break;
    }
    return true;
}

void CopyValue(MonsterDefinition& to, const MonsterDefinition& from, MonsterValueField field)
{
    if (const std::optional<size_t> setup = GetSetupFieldIndex(field))
    {
        GetMonsterSetupFields()[*setup].copy(to.setup, from.setup);
        return;
    }
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
    case MonsterValueField::SetupFirst:
        // A setup field, copied above.
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
