#pragma once

#include <array>
#include <optional>

namespace Data::Effects
{
// The creation values of an effect type: the "create" object of its entry in
// EffectTypes.json (docs/effect-data.md). CreateEffect applies them on top of
// the setup it does for every effect; an unset value keeps what that setup
// chose, or, for lifeTime and gravity, which it does not set, the value the
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

    // Many legacy cases end with VectorCopy(o->Light, o->Direction), keeping
    // the color so the move code can fade it back in.
    bool copyLightToDirection = false;

    bool operator==(const EffectCreateParams&) const = default;
};

// The creation values of one effect type number.
struct EffectTypeCreateParams
{
    int type = 0;
    EffectCreateParams params;
};
} // namespace Data::Effects
