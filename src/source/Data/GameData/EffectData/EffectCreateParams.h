#pragma once

#include <array>
#include <optional>

namespace Data::Effects
{
// A vector of creation values, set component by component (x, y, z); a
// component left out keeps its value. Components of an offset can be
// multiplied by the frame factor (FPS_ANIMATION_FACTOR), as the old creation
// code multiplied them (sven-n/MuMain#681).
struct EffectCreateVector
{
    std::array<std::optional<double>, 3> components;
    std::array<bool, 3> timesFrameFactor{};

    bool IsSet() const
    {
        return components[0] || components[1] || components[2];
    }

    bool operator==(const EffectCreateVector&) const = default;
};

// The creation values of an effect type: the "create" object of its entry in
// EffectTypes.json (docs/effect-data.md). CreateEffect applies them on top of
// the setup it does for every effect: first the values, then the offsets, then
// the copies; an offset of a field a copy writes adds to the copy. An unset
// value keeps what that setup chose, or, for the fields it does not set
// (lifeTime, gravity, timer, distance, startPosition, ...), the value the
// slot's previous effect left, so an entry only states what differs (D34).
// The values are kept as read, so a written file shows them as they were
// written.
struct EffectCreateParams
{
    std::optional<double> lifeTime;
    std::optional<double> scale;
    std::optional<double> velocity;
    std::optional<double> gravity;
    std::optional<int> hiddenMesh;
    std::optional<int> blendMesh;
    std::optional<double> blendMeshLight;
    std::optional<double> alpha;

    // The color the effect is drawn with (red, green, blue).
    std::optional<std::array<double, 3>> light;

    std::optional<bool> lightEnable;
    std::optional<bool> alphaEnable;
    std::optional<int> kind;
    std::optional<int> skill;
    std::optional<double> pkKey;
    std::optional<double> timer;
    std::optional<double> distance;
    std::optional<double> collisionRange;
    EffectCreateVector position;
    EffectCreateVector angle;
    EffectCreateVector direction;

    // Added to the field after the values.
    EffectCreateVector positionOffset;
    EffectCreateVector angleOffset;
    EffectCreateVector startPositionOffset;

    // Copies of other fields, or of the arguments of the CreateEffect call,
    // after the offsets.
    bool copyLightToDirection = false;
    bool copyPositionToStartPosition = false;
    bool copyCallLightToHeadTargetAngle = false;
    bool copyCallScaleToScale = false;

    bool operator==(const EffectCreateParams&) const = default;
};

// The creation values of one effect type number.
struct EffectTypeCreateParams
{
    int type = 0;
    EffectCreateParams params;
};
} // namespace Data::Effects
