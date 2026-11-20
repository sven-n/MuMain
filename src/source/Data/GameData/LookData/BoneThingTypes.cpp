#include "stdafx.h"

#include "BoneThingTypes.h"

#include "Data/GameData/EffectData/EffectTypeCatalogue.h"

namespace Data::Looks
{
Effects::EffectKind CatalogueKindOf(BoneThingKind kind)
{
    switch (kind)
    {
    case BoneThingKind::Sprite:
        return Effects::EffectKind::Sprite;
    case BoneThingKind::Particle:
        return Effects::EffectKind::Particle;
    case BoneThingKind::Joint:
        return Effects::EffectKind::Joint;
    case BoneThingKind::Effect:
        return Effects::EffectKind::Effect;
    }
    return Effects::EffectKind::Effect;
}

const char* BoneThingKey(BoneThingKind kind)
{
    for (const auto& [named, key] : BoneThingKeys)
    {
        if (named == kind)
        {
            return key;
        }
    }
    return "";
}

std::optional<int> FindBoneThingType(BoneThingKind kind, std::string_view type)
{
    return g_EffectTypeCatalogue.FindType(CatalogueKindOf(kind), type);
}
} // namespace Data::Looks
