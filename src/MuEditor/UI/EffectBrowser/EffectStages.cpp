#include "stdafx.h"

#ifdef _EDITOR

#include "EffectStages.h"

#include "EffectLegacyCases.h"
#include "Core/Globals/_enum.h"
#include "Render/Effects/EffectDef.h"

namespace MuEditor::Effects
{
namespace
{
// CreateEffect applies the row and then runs the hook when the descriptor has
// either; only without both does it go to the switch.
CreateStage DescribeCreation(const Render::Effects::EffectDescriptor* descriptor, std::uint8_t legacyCases)
{
    const bool data = descriptor != nullptr && descriptor->create.has_value();
    const bool hook = descriptor != nullptr && descriptor->onCreate != nullptr;
    if (data && hook)
        return CreateStage::DataThenHook;
    if (data)
        return CreateStage::Data;
    if (hook)
        return CreateStage::Hook;
    return (legacyCases & CreateCase) != 0 ? CreateStage::Switch : CreateStage::SetupOnly;
}

MoveStage DescribeMove(const Render::Effects::EffectDescriptor* descriptor, std::uint8_t legacyCases)
{
    if (descriptor != nullptr && descriptor->move != nullptr)
        return MoveStage::Handler;
    return (legacyCases & MoveCase) != 0 ? MoveStage::Switch : MoveStage::SharedCodeOnly;
}

// RenderEffects' default draws the models of the skill range with
// RenderObject and nothing else.
RenderStage DescribeRendering(int type, const Render::Effects::EffectDescriptor* descriptor, std::uint8_t legacyCases)
{
    if (descriptor != nullptr && descriptor->render != nullptr)
        return RenderStage::Handler;
    if ((legacyCases & RenderCase) != 0)
        return RenderStage::Switch;
    if (type >= MODEL_SKILL_BEGIN && type < MODEL_SKILL_END)
        return RenderStage::DrawnAsModel;
    return (legacyCases & GroundCase) != 0 ? RenderStage::OnGround : RenderStage::NotDrawn;
}
} // namespace

EffectStages DescribeEffectStages(int type, const Render::Effects::EffectDescriptor* descriptor,
                                  std::uint8_t legacyCases)
{
    return {DescribeCreation(descriptor, legacyCases), DescribeMove(descriptor, legacyCases),
            DescribeRendering(type, descriptor, legacyCases), (legacyCases & GroundCase) != 0,
            (legacyCases & AfterCharactersCase) != 0};
}
} // namespace MuEditor::Effects

#endif // _EDITOR
