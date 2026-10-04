#pragma once

#ifdef _EDITOR

#include <array>
#include <cstdint>

namespace Render::Effects
{
struct EffectDescriptor;
}

namespace MuEditor::Effects
{
// Where the creation of an effect type is: the registry row (data), its
// creation hook, both, the case of CreateEffect's switch, or none of them, so
// the effect gets only the setup every effect gets.
enum class CreateStage
{
    Data,
    Hook,
    DataThenHook,
    Switch,
    SetupOnly,
};

// The move code: a registry handler, the case of MoveEffect's switch, or only
// the code MoveEffect runs for every effect after the switch.
enum class MoveStage
{
    Handler,
    Switch,
    SharedCodeOnly,
};

// The drawing: a registry handler, the case of RenderEffects' switch, the
// switch's default that draws the skill models, only the case of
// RenderEffectShadows that draws on the ground, or not drawn (most texture
// effects only create sprites, particles or joints).
enum class RenderStage
{
    Handler,
    Switch,
    DrawnAsModel,
    OnGround,
    NotDrawn,
};

constexpr std::array<CreateStage, 5> CreateStages = {CreateStage::Data, CreateStage::Hook, CreateStage::DataThenHook,
                                                     CreateStage::Switch, CreateStage::SetupOnly};
constexpr std::array<MoveStage, 3> MoveStages = {MoveStage::Handler, MoveStage::Switch, MoveStage::SharedCodeOnly};
constexpr std::array<RenderStage, 5> RenderStages = {
    RenderStage::Handler, RenderStage::Switch, RenderStage::DrawnAsModel, RenderStage::OnGround, RenderStage::NotDrawn};

struct EffectStages
{
    CreateStage create = CreateStage::SetupOnly;
    MoveStage move = MoveStage::SharedCodeOnly;
    RenderStage render = RenderStage::NotDrawn;
    // RenderEffectShadows has a case: it draws the effect on the ground, also
    // when a handler or RenderEffects draws it too.
    bool drawnOnGround = false;

    bool operator==(const EffectStages&) const = default;
};

// The stages of the effect type `type` from its registry entry (`descriptor`,
// null without one) and its LegacyCase flags.
EffectStages DescribeEffectStages(int type, const Render::Effects::EffectDescriptor* descriptor,
                                  std::uint8_t legacyCases);
} // namespace MuEditor::Effects

#endif // _EDITOR
