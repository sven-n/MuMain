#include "stdafx.h"

#include "EffectCreateParams.h"

#include <algorithm>

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

// The value of `target`, which a copy replaces.
const EffectCopyTargetValue& ValueOf(std::string_view target)
{
    for (const EffectCopyField& field : EffectCopyFields)
    {
        if (field.target == target)
        {
            return field.value;
        }
    }
    return NoValue;
}

void ClearValueOf(EffectCreateParams& params, std::string_view target)
{
    if (const EffectCopyTargetValue& value = ValueOf(target); value.clear != nullptr)
    {
        value.clear(params);
    }
}

void ClearCopiesInto(EffectCreateParams& params, std::string_view target)
{
    for (const EffectCopyField& field : EffectCopyFields)
    {
        if (field.target == target)
        {
            params.*field.copy = false;
        }
    }
}

// One name per field of EffectCreateParams: the binding stops compiling when
// the struct gets a field, and the count when the name is added without
// changing EffectCreateFieldCount, so ResolveVariant below is given it too.
#define EFFECT_CREATE_FIELD_NAMES                                                                                      \
    lifeTime, scale, velocity, gravity, hiddenMesh, blendMesh, blendMeshLight, alpha, light, lightEnable, alphaEnable, \
        kind, skill, pkKey, timer, distance, collisionRange, alphaTarget, renderType, animation, position, angle,      \
        direction, startPosition, lifeTimeOffset, positionOffset, angleOffset, startPositionOffset,                    \
        copyLightToDirection, copyCallAngleToDirection, copyPositionToStartPosition, copyLightToStartPosition,         \
        copyCallPositionToStartPosition, copyCallLightToHeadTargetAngle, copyLightToEyeRight,                          \
        copyCallAngleToDeadPosition, copyCallScaleToScale, variants
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
    Override(params.alphaTarget, variant.alphaTarget);
    Override(params.renderType, variant.renderType);
    Override(params.animation, variant.animation);
    OverrideComponents(params.position, variant.position);
    OverrideComponents(params.angle, variant.angle);
    OverrideComponents(params.direction, variant.direction);
    OverrideComponents(params.startPosition, variant.startPosition);
    Override(params.lifeTimeOffset, variant.lifeTimeOffset);
    OverrideComponents(params.positionOffset, variant.positionOffset);
    OverrideComponents(params.angleOffset, variant.angleOffset);
    OverrideComponents(params.startPositionOffset, variant.startPositionOffset);

    // A field gets a value or a copy: the variant's replaces the row's, and a
    // copy of the variant replaces the row's copies into the same field. The
    // reader makes sure a variant that sets a vector the row copies into sets
    // all three components.
    for (const EffectCopyField& field : EffectCopyFields)
    {
        if (CopiesInto(variant, field.target))
        {
            ClearCopiesInto(params, field.target);
            ClearValueOf(params, field.target);
        }
        else if (SetsValueOf(variant, field.target))
        {
            ClearCopiesInto(params, field.target);
        }
    }
    for (const EffectCopyField& field : EffectCopyFields)
    {
        params.*field.copy = params.*field.copy || variant.*field.copy;
    }
    return params;
}

bool CopiesInto(const EffectCreateParams& params, std::string_view target)
{
    return std::any_of(EffectCopyFields.begin(), EffectCopyFields.end(),
                       [&](const EffectCopyField& field) { return field.target == target && params.*field.copy; });
}

bool SetsValueOf(const EffectCreateParams& params, std::string_view target)
{
    const EffectCopyTargetValue& value = ValueOf(target);
    return value.isSet != nullptr && value.isSet(params);
}
} // namespace Data::Effects
