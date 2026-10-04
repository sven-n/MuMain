#pragma once

#include <array>
#include <cstdint>
#include <optional>

class OBJECT;

// Data-driven description of a single effect type.
//
// Historically every effect was hand-coded as a `case` in three giant switch
// statements (CreateEffect / MoveEffect / RenderEffects in ZzzEffect.cpp). The
// vast majority of those cases only assigned a handful of scalar fields at
// creation and rendered with a plain RenderObject(). EffectDescriptor captures
// that as data: the parameters come from the effect catalogue
// (Data/Effects/EffectTypes.json, see EffectRegistry), and only effects with
// genuine per-frame behaviour carry a handler function.
namespace Render::Effects
{
// A vector of creation values: the components in `components` (bit 0 x, bit 1
// y, bit 2 z) get `values`. An offset multiplies the components in
// `timesFrameFactor` by FPS_ANIMATION_FACTOR, as the old creation code did.
struct CreateVector
{
    std::array<float, 3> values{};
    std::uint8_t components = 0;
    std::uint8_t timesFrameFactor = 0;
};

// The arguments of the CreateEffect call that creation values can copy.
struct CreateCall
{
    const float* light = nullptr;
    float scale = 0.f;
};

// Creation parameters applied on top of the common initialisation that
// CreateEffect performs for every effect: first the values, then the offsets,
// then the copies; an offset of a field a copy writes adds to the copy. Every
// field is optional: an unset field keeps what the common initialisation
// chose, or, for the fields it does not set (lifeTime, gravity, timer,
// distance, startPosition, ...), the value the slot's previous effect left
// (D34), so a catalogue entry only states what differs for that effect.
// BuildRegistry converts them once from the values of the catalogue
// (Data::Effects::EffectCreateParams), so creating an effect only copies.
//
// These cover effects whose creation is plain data. Randomised creation in
// this codebase is almost always fused with angle/direction/matrix setup
// (e.g. a stone that picks a random spin, then rotates its launch vector by
// that angle), which isn't expressible as independent scalar parameters --
// those effects use an onCreate hook instead (see EffectDescriptor).
struct CreateParams
{
    std::optional<float> lifeTime;
    std::optional<float> scale;
    std::optional<float> velocity;
    std::optional<float> gravity;
    std::optional<int> hiddenMesh;
    std::optional<int> blendMesh;
    std::optional<float> blendMeshLight;
    std::optional<float> alpha;

    // When set, overrides o->Light (the colour the effect renders with).
    std::optional<std::array<float, 3>> light;

    // The groups below that a row sets. ApplyCreateParams tests only those,
    // so the rows that set only the values above cost what they did before
    // these fields came.
    enum Group : std::uint8_t
    {
        Flags = 1 << 0,
        Numbers = 1 << 1,
        Vectors = 1 << 2,
        Offsets = 1 << 3,
        Copies = 1 << 4,
    };
    std::uint8_t groups = 0;

    // Flags
    std::optional<bool> lightEnable;
    std::optional<bool> alphaEnable;
    std::optional<std::uint8_t> kind;
    std::optional<std::uint16_t> skill;

    // Numbers
    std::optional<float> pkKey;
    std::optional<float> timer;
    std::optional<float> distance;
    std::optional<float> collisionRange;

    // Vectors
    CreateVector position;
    CreateVector angle;
    CreateVector direction;

    // Offsets
    CreateVector positionOffset;
    CreateVector angleOffset;
    CreateVector startPositionOffset;

    // Copies. Many legacy cases finish with `VectorCopy(o->Light,
    // o->Direction)`, stashing the colour so MoveEffect can fade it back in.
    bool copyLightToDirection = false;
    bool copyPositionToStartPosition = false;
    bool copyCallLightToHeadTargetAngle = false;
    bool copyCallScaleToScale = false;
};

// Spawns sub-effects / joints or runs other one-shot setup that can't be
// expressed as plain parameters. Runs once, right after CreateParams are
// applied.
using CreateHook = void (*)(OBJECT* o);

// Per-frame update. `luminosity` is the per-frame flicker value MoveEffect
// computes once for every effect (so handlers don't draw an extra rand()).
// Returns true to run MoveEffect's shared tail (lifetime decrement, particle
// trail, destruction); false to skip it, mirroring the handful of legacy
// cases that `return` early out of the move switch.
using MoveHandler = bool (*)(OBJECT* o, int index, float luminosity);

// Per-frame draw. Defaults to RenderObject() when left null.
using RenderHandler = void (*)(OBJECT* o);

// A descriptor migrates each lifecycle stage independently: a type can have
// its rendering driven by the registry while its creation still runs through
// the legacy switch, or vice versa. CreateEffect treats creation as migrated
// only when `create` or `onCreate` is set; MoveEffect / RenderEffects gate on
// their respective handlers. An unset stage falls back to the legacy switch.
struct EffectDescriptor
{
    std::optional<CreateParams> create;
    CreateHook onCreate = nullptr;
    MoveHandler move = nullptr;
    RenderHandler render = nullptr;
};

// Applies the optional parameters to an already common-initialised effect.
void ApplyCreateParams(OBJECT* o, const CreateParams& params, const CreateCall& call);
} // namespace Render::Effects
