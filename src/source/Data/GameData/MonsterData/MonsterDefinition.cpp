#include "stdafx.h"

#include "MonsterDefinition.h"

#include <algorithm>

namespace Data::Monsters
{
const std::optional<MonsterAttacks>& GetAttacksOnMap(const MonsterDefinition& monster, int map)
{
    for (const MonsterMapVariant& variant : monster.mapVariants)
    {
        if (variant.attacks && std::find(variant.maps.begin(), variant.maps.end(), map) != variant.maps.end())
        {
            return variant.attacks;
        }
    }
    return monster.attacks;
}
} // namespace Data::Monsters
