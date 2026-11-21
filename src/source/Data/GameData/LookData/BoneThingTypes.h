#pragma once

#include "Data/GameData/EffectData/EffectKind.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <array>
#include <optional>
#include <string_view>
#include <utility>

// The effect catalogue's types of the things on bones (design L7, step 3): a thing names its type as FX1 named them
// (D30), and the game creates it by the type's number.
namespace Data::Looks
{
// The catalogue's kind of a thing on bones.
Effects::EffectKind CatalogueKindOf(BoneThingKind kind);

// The key a thing on bones names its kind with in the files; its value is the type's name in the effect catalogue.
inline constexpr std::array<std::pair<BoneThingKind, const char*>, 4> BoneThingKeys{{
    {BoneThingKind::Sprite, "sprite"},
    {BoneThingKind::Particle, "particle"},
    {BoneThingKind::Joint, "joint"},
    {BoneThingKind::Effect, "effect"},
}};

// The key of a kind ("sprite", "particle", "joint", "effect").
const char* BoneThingKey(BoneThingKind kind);

// The number of the thing's type in the effect catalogue, or nullopt for a name it doesn't have.
std::optional<int> FindBoneThingType(BoneThingKind kind, std::string_view type);
} // namespace Data::Looks
