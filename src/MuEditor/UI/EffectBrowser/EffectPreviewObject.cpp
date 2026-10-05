#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewObject.h"

#include "EffectPreviewSubject.h"
#include "Core/Globals/_enum.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Effects/EffectDef.h"

namespace MuEditor::Effects
{
namespace
{
// CreateEffect gives an effect this scale when the call passes none.
constexpr float DefaultScale = 0.9f;
constexpr float DefaultVelocity = 0.3f;
// Below this alpha RenderObject draws nothing; below this scale a model is
// too small to see.
constexpr float InvisibleAlpha = 0.01f;
constexpr float InvisibleScale = 0.01f;
// HiddenMesh -2 hides the whole model.
constexpr int HideAllMeshes = -2;

// The setup CreateEffect gives every effect before its creation values
// (ZzzEffect.cpp).
void SetUpLikeCreateEffect(OBJECT& o, int type, int subType)
{
    o.Initialize();
    o.Live = true;
    o.Type = type;
    o.SubType = subType;
    o.LightEnable = true;
    o.HiddenMesh = -1;
    o.BlendMesh = -1;
    o.BlendMeshLight = 1.0f;
    o.BlendMeshTexCoordU = 0.0f;
    o.BlendMeshTexCoordV = 0.0f;
    o.AnimationFrame = 0.0f;
    o.PriorAnimationFrame = 0.0f;
    o.AlphaEnable = false;
    o.Alpha = 1.0f;
    o.Scale = DefaultScale;
    o.Owner = nullptr;
    o.Velocity = DefaultVelocity;
    o.PKKey = -1;
    o.Kind = 0;
    o.Skill = 0;
    o.RenderType = 0;
    o.AttackPoint[0] = 0;
    o.CurrentAction = 0;
    Vector(1.0f, 1.0f, 1.0f, o.Light);
    Vector(0.0f, 0.0f, 0.0f, o.Angle);
    Vector(0.0f, 0.0f, 0.0f, o.Position);
    Vector(0.0f, 0.0f, 0.0f, o.Direction);
}
} // namespace

std::uint16_t BuildPreviewEffectObject(OBJECT& o, int type, int subType,
                                       const Render::Effects::EffectDescriptor* descriptor)
{
    SetUpLikeCreateEffect(o, type, subType);
    if (descriptor != nullptr && (descriptor->create || descriptor->onCreate))
    {
        // A scale of 1 for the rows that copy the call's scale, which the
        // call does not pass here.
        if (const Render::Effects::CreateParams* params = descriptor->CreateParamsFor(subType))
            Render::Effects::ApplyCreateParams(&o, *params,
                                               {{1.0f, 1.0f, 1.0f}, 1.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}});
    }
    // The preview places the effect itself.
    Vector(0.0f, 0.0f, 0.0f, o.Position);

    std::uint16_t notes = 0;
    if (o.Alpha < InvisibleAlpha)
    {
        o.Alpha = 1.0f;
        notes |= NoteStartsInvisible;
    }
    if (o.Scale < InvisibleScale)
    {
        o.Scale = 1.0f;
        notes |= NoteStartsInvisible;
    }
    if (o.HiddenMesh == HideAllMeshes)
        notes |= NoteHiddenModel;
    return notes;
}

// The conditions of the code MoveEffect runs after its switch (ZzzEffect.cpp),
// and the cases that return before it; test_effect_browser.cpp reads both from
// the source and fails when this copy no longer agrees.
bool IsAnimatedByMoveEffect(int type, int subType)
{
    // MoveEffect's case of the big meteors returns before that code.
    const bool returnsFirst = type == MODEL_BIG_METEO1 || type == MODEL_BIG_METEO2 || type == MODEL_BIG_METEO3;
    const bool notPlayed = type == MODEL_SKILL_WHEEL1 || type == MODEL_SKILL_WHEEL2 ||
                           type == MODEL_SKILL_FURY_STRIKE ||
                           ((type == MODEL_STONE1 || type == MODEL_STONE2) && subType == 5) ||
                           (type == MODEL_ARROW_DRILL && subType == 3) || type == MODEL_PIER_PART ||
                           type == MODEL_DEATH_SPI_SKILL || type == MODEL_CHANGE_UP_EFF;
    return !returnsFirst && !notPlayed && type >= MODEL_BIRD01 && type < MODEL_SKILL_END;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
