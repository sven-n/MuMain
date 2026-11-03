#include "stdafx.h"

#include "MonsterDefinition.h"

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
