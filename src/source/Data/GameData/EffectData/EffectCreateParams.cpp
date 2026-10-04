#include "stdafx.h"

#include "EffectCreateParams.h"

namespace Data::Effects
{
namespace
{
template <typename T> void Override(std::optional<T>& value, const std::optional<T>& variant)
{
    if (variant)
    {
        value = variant;
    }
}

void OverrideComponents(EffectCreateVector& vector, const EffectCreateVector& variant)
{
    for (size_t i = 0; i < vector.components.size(); ++i)
    {
        if (variant.components[i])
        {
            vector.components[i] = variant.components[i];
            vector.timesFrameFactor[i] = variant.timesFrameFactor[i];
        }
    }
}
// One name per field of EffectCreateParams: the binding stops compiling when
// the struct gets a field, and the count when the name is added without
// changing EffectCreateFieldCount, so ResolveVariant below is given it too.
#define EFFECT_CREATE_FIELD_NAMES                                                                                      \
    lifeTime, scale, velocity, gravity, hiddenMesh, blendMesh, blendMeshLight, alpha, light, lightEnable, alphaEnable, \
        kind, skill, pkKey, timer, distance, collisionRange, position, angle, direction, positionOffset, angleOffset,  \
        startPositionOffset, copyLightToDirection, copyPositionToStartPosition, copyCallLightToHeadTargetAngle,        \
        copyCallScaleToScale, variants
[[maybe_unused]] void NameEveryField(const EffectCreateParams& params)
{
    [[maybe_unused]] const auto& [EFFECT_CREATE_FIELD_NAMES] = params;
    static_assert(decltype(CountNames(EFFECT_CREATE_FIELD_NAMES))::value == EffectCreateFieldCount);
}
#undef EFFECT_CREATE_FIELD_NAMES
} // namespace

EffectCreateParams ResolveVariant(const EffectCreateParams& row, const EffectCreateParams& variant)
{
    EffectCreateParams params = row;
    params.variants.clear();
    Override(params.lifeTime, variant.lifeTime);
    Override(params.scale, variant.scale);
    Override(params.velocity, variant.velocity);
    Override(params.gravity, variant.gravity);
    Override(params.hiddenMesh, variant.hiddenMesh);
    Override(params.blendMesh, variant.blendMesh);
    Override(params.blendMeshLight, variant.blendMeshLight);
    Override(params.alpha, variant.alpha);
    Override(params.light, variant.light);
    Override(params.lightEnable, variant.lightEnable);
    Override(params.alphaEnable, variant.alphaEnable);
    Override(params.kind, variant.kind);
    Override(params.skill, variant.skill);
    Override(params.pkKey, variant.pkKey);
    Override(params.timer, variant.timer);
    Override(params.distance, variant.distance);
    Override(params.collisionRange, variant.collisionRange);
    OverrideComponents(params.position, variant.position);
    OverrideComponents(params.angle, variant.angle);
    OverrideComponents(params.direction, variant.direction);
    OverrideComponents(params.positionOffset, variant.positionOffset);
    OverrideComponents(params.angleOffset, variant.angleOffset);
    OverrideComponents(params.startPositionOffset, variant.startPositionOffset);

    // A field gets a value or a copy: the variant's replaces the row's. The
    // reader makes sure a variant that sets direction while the row copies
    // the light into it sets all three components.
    if (variant.scale)
    {
        params.copyCallScaleToScale = false;
    }
    if (variant.copyCallScaleToScale)
    {
        params.scale.reset();
        params.copyCallScaleToScale = true;
    }
    if (variant.direction.IsSet())
    {
        params.copyLightToDirection = false;
    }
    if (variant.copyLightToDirection)
    {
        params.direction = {};
        params.copyLightToDirection = true;
    }
    params.copyPositionToStartPosition = params.copyPositionToStartPosition || variant.copyPositionToStartPosition;
    params.copyCallLightToHeadTargetAngle =
        params.copyCallLightToHeadTargetAngle || variant.copyCallLightToHeadTargetAngle;
    return params;
}
} // namespace Data::Effects
