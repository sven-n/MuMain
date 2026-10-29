#include "stdafx.h"

#include "MonsterDefinition.h"

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
} // namespace Data::Monsters
