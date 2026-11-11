#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace Data::Effects
{
// The four kinds of effect types. Each kind has its own creation function
// (CreateEffect, CreateParticle, CreateJoint, CreateSprite), its own code and
// its own names: the same number is a different type in each kind.
enum class EffectKind
{
    Effect,
    Particle,
    Joint,
    Sprite,
};

constexpr std::array<EffectKind, 4> EffectKinds = {EffectKind::Effect, EffectKind::Particle, EffectKind::Joint,
                                                   EffectKind::Sprite};
constexpr size_t EffectKindCount = EffectKinds.size();

constexpr size_t ToIndex(EffectKind kind)
{
    return static_cast<size_t>(kind);
}

// The kind as the catalogue files write it: "effect", "particle", "joint", "sprite".
constexpr std::string_view GetEffectKindName(EffectKind kind)
{
    constexpr std::array<std::string_view, EffectKindCount> Names = {"effect", "particle", "joint", "sprite"};
    return Names[ToIndex(kind)];
}
} // namespace Data::Effects
