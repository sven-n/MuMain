#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

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

// A number of an offset; the effect multiplies it by the frame factor
// (FPS_ANIMATION_FACTOR) when timesFrameFactor is set.
struct EffectCreateNumber
{
    double value = 0.0;
    bool timesFrameFactor = false;

    bool operator==(const EffectCreateNumber&) const = default;
};

// The render types the creation cases set, by name: RENDER_DARK of the models
// and RENDER_TYPE_ALPHA_BLEND_MINUS of the textures.
enum class EffectRenderType
{
    Dark,
    AlphaBlendMinus,
};

struct EffectCreateVariant;

// The creation values of an effect type: the "create" object of its entry in
// EffectTypes.json (docs/effect-data.md). CreateEffect applies them on top of
// the setup it does for every effect: first the values, then the offsets, then
// the copies; an offset of a field a copy writes adds to the copy. An unset
// value keeps what that setup chose, or, for the fields it does not set
// (lifeTime, gravity, timer, distance, startPosition, ...), the value the
// slot's previous effect left, so an entry only states what differs (D34).
// The values are kept as read, so a written file shows them as they were
// written.
//
// A new field goes into the reader and the writer (EffectCreateParamsJson.cpp),
// ResolveVariant, the game's CreateParams with ToCreateParams, GroupsOf and
// ApplyCreateParams, and the test that applies each field alone. The functions
// next to ResolveVariant, ToCreateParams and GroupsOf name every field and
// check their count against EffectCreateFieldCount, and the test checks its
// list against it, so all of them stop compiling until they get the field.
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
    std::optional<double> alphaTarget;
    std::optional<EffectRenderType> renderType;
    std::optional<int> animation;
    EffectCreateVector position;
    EffectCreateVector angle;
    EffectCreateVector direction;
    EffectCreateVector startPosition;

    // Added to the field after the values.
    std::optional<EffectCreateNumber> lifeTimeOffset;
    EffectCreateVector positionOffset;
    EffectCreateVector angleOffset;
    EffectCreateVector startPositionOffset;

    // Copies of other fields, or of the arguments of the CreateEffect call,
    // after the offsets.
    bool copyLightToDirection = false;
    bool copyCallAngleToDirection = false;
    bool copyPositionToStartPosition = false;
    bool copyLightToStartPosition = false;
    bool copyCallPositionToStartPosition = false;
    bool copyCallLightToHeadTargetAngle = false;
    bool copyLightToEyeRight = false;
    bool copyCallAngleToDeadPosition = false;
    bool copyCallScaleToScale = false;

    // Values for some SubTypes on top of the ones above (D35); a SubType
    // without a variant gets the ones above. Only the "create" object has
    // variants, a variant has none.
    std::vector<EffectCreateVariant> variants;

    bool operator==(const EffectCreateParams&) const = default;
};

// The number of fields of EffectCreateParams, variants included (C++ cannot
// count the fields of a struct). The game's CreateParams has as many, with its
// groups in place of the variants.
inline constexpr std::size_t EffectCreateFieldCount = 38;

// The number of its arguments; only for counting names at compile time, in
// decltype.
template <typename... T> std::integral_constant<std::size_t, sizeof...(T)> CountNames(const T&...);

// One copy of "copy": { "<target>": "<source>" }; the sources starting with
// "call" are arguments of the CreateEffect call. The copies of a target are
// listed together, in the order they are written.
struct EffectCopyField
{
    const char* target;
    const char* source;
    bool EffectCreateParams::* copy;
};
inline constexpr std::array<EffectCopyField, 9> EffectCopyFields = {{
    {"direction", "light", &EffectCreateParams::copyLightToDirection},
    {"direction", "callAngle", &EffectCreateParams::copyCallAngleToDirection},
    {"startPosition", "position", &EffectCreateParams::copyPositionToStartPosition},
    {"startPosition", "light", &EffectCreateParams::copyLightToStartPosition},
    {"startPosition", "callPosition", &EffectCreateParams::copyCallPositionToStartPosition},
    {"headTargetAngle", "callLight", &EffectCreateParams::copyCallLightToHeadTargetAngle},
    {"eyeRight", "light", &EffectCreateParams::copyLightToEyeRight},
    {"deadPosition", "callAngle", &EffectCreateParams::copyCallAngleToDeadPosition},
    {"scale", "callScale", &EffectCreateParams::copyCallScaleToScale},
}};

// Whether `params` copies into `target`, and whether it sets the value of that
// field (only scale, direction and startPosition have one).
bool CopiesInto(const EffectCreateParams& params, std::string_view target);
bool SetsValueOf(const EffectCreateParams& params, std::string_view target);

// The values the SubTypes in `subTypes` get on top of the ones of the row.
struct EffectCreateVariant
{
    std::vector<int> subTypes;
    EffectCreateParams params;

    bool operator==(const EffectCreateVariant&) const = default;
};

// The creation values a SubType of a variant gets: the row's values with the
// variant's on top. A value replaces the row's value of its field and the
// row's copy into it; a copy replaces the row's value and copy of its field;
// vectors and offsets are replaced component by component. The result has no
// variants.
EffectCreateParams ResolveVariant(const EffectCreateParams& row, const EffectCreateParams& variant);

// The creation values of one effect type number.
struct EffectTypeCreateParams
{
    int type = 0;
    EffectCreateParams params;
};
} // namespace Data::Effects
