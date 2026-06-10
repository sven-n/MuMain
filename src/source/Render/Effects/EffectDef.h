#pragma once

#include <array>
#include <optional>

class OBJECT;

// Data-driven description of a single effect type.
//
// Historically every effect was hand-coded as a `case` in three giant switch
// statements (CreateEffect / MoveEffect / RenderEffects in ZzzEffect.cpp). The
// vast majority of those cases only assigned a handful of scalar fields at
// creation and rendered with a plain RenderObject(). EffectDescriptor captures
// that as data: the parameters live in a table (see EffectRegistry), and only
// effects with genuine per-frame behaviour carry a handler function.
namespace Render::Effects
{
    // Creation parameters applied on top of the common initialisation that
    // CreateEffect performs for every effect. Every field is optional: an unset
    // field means "keep whatever the common initialisation chose", so a table
    // row only states what actually differs from the default for that effect.
    struct CreateParams
    {
        std::optional<float> lifeTime;
        std::optional<float> scale;
        std::optional<float> velocity;
        std::optional<float> gravity;
        std::optional<int>   hiddenMesh;
        std::optional<int>   blendMesh;
        std::optional<float> blendMeshLight;
        std::optional<float> alpha;

        // When set, overrides o->Light (the colour the effect renders with).
        std::optional<std::array<float, 3>> light;

        // Many legacy cases finish with `VectorCopy(o->Light, o->Direction)`,
        // stashing the colour so MoveEffect can fade it back in. Opt in here.
        bool copyLightToDirection = false;
    };

    // Spawns sub-effects / joints or runs other one-shot setup that can't be
    // expressed as plain parameters. Runs once, right after CreateParams are
    // applied.
    using CreateHook = void (*)(OBJECT* o);

    // Per-frame update. Returns true to run MoveEffect's shared tail (lifetime
    // decrement, particle trail, destruction); false to skip it, mirroring the
    // handful of legacy cases that `return` early out of the move switch.
    using MoveHandler = bool (*)(OBJECT* o, int index);

    // Per-frame draw. Defaults to RenderObject() when left null.
    using RenderHandler = void (*)(OBJECT* o);

    struct EffectDescriptor
    {
        CreateParams  create;
        CreateHook    onCreate = nullptr;
        MoveHandler   move     = nullptr;
        RenderHandler render   = nullptr;
    };

    // Applies the optional parameters to an already common-initialised effect.
    void ApplyCreateParams(OBJECT* o, const CreateParams& params);
}
