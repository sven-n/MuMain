#include "stdafx.h"

#include "MonsterDefinition.h"

#include "Data/GameData/MonsterData/MonsterModelSlots.h"

#include <algorithm>

namespace Data::Monsters
{
const std::optional<MonsterAttacks>& GetAttacksOnMap(const MonsterDefinition& monster, int map, int stage)
{
    for (const MonsterMapVariant& variant : monster.mapVariants)
    {
        if (variant.attacks && Data::Maps::Names(variant.maps, map, stage))
        {
            return variant.attacks;
        }
    }
    return monster.attacks;
}

const MonsterMapVariant* FindMapVariant(const MonsterDefinition& monster, int map, int stage)
{
    const auto found = std::ranges::find_if(monster.mapVariants, [&](const MonsterMapVariant& variant)
                                            { return Data::Maps::Names(variant.maps, map, stage); });
    return found != monster.mapVariants.end() ? &*found : nullptr;
}

MonsterSetup GetSetupOnMap(const MonsterDefinition& monster, int map, int stage)
{
    MonsterSetup setup = monster.setup;
    if (const MonsterMapVariant* variant = FindMapVariant(monster, map, stage))
    {
        for (const MonsterSetupField& field : GetMonsterSetupFields())
        {
            if (field.isSet(variant->setup))
                field.copy(setup, variant->setup);
        }
    }
    return setup;
}

const std::string& GetModelOnMap(const MonsterDefinition& monster, int map, int stage)
{
    for (const MonsterMapVariant& variant : monster.mapVariants)
    {
        if (variant.model && Data::Maps::Names(variant.maps, map, stage))
            return *variant.model;
    }
    return monster.model;
}

const std::string& GetSoundSetWithModel(const MonsterDefinition& monster, int slot)
{
    for (const MonsterRandomVariant& variant : monster.randomVariants)
    {
        if (variant.soundSet && FindMonsterModelReference(variant.model) == slot)
        {
            return *variant.soundSet;
        }
    }
    return monster.soundSet;
}

std::vector<Data::Maps::MapRef> FindMapsOfOtherVariants(const MonsterDefinition& monster, size_t variant)
{
    std::vector<Data::Maps::MapRef> shared;
    for (const Data::Maps::MapRef& ref : monster.mapVariants[variant].maps)
    {
        for (size_t other = 0; other < monster.mapVariants.size(); ++other)
        {
            const std::vector<Data::Maps::MapRef>& otherMaps = monster.mapVariants[other].maps;
            const bool named = std::any_of(otherMaps.begin(), otherMaps.end(), [&](const Data::Maps::MapRef& candidate)
                                           { return Data::Maps::Overlaps(ref, candidate); });
            if (other != variant && named)
            {
                shared.push_back(ref);
                break;
            }
        }
    }
    return shared;
}
} // namespace Data::Monsters
