#include "stdafx.h"
#include "Engine/Object/Movement/MovementBones.h"

#include "Engine/Object/Movement/FootPlacement.h"
#include "Engine/Object/Movement/PoseFade.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"

namespace Engine::Characters
{
void AnimateBones(OBJECT* o, BMD& model, float (*bones)[3][4], bool translate, short currentAction)
{
    const CHARACTER* character = CharacterOfObject(o);
    vec3_t angle = {o->Angle[0], o->Angle[1],
                    o->Angle[2] + (character != nullptr ? character->Motion.SnapTurnOffset : 0.f)};
    const float turnLag = character != nullptr ? character->Motion.TurnLag : 0.f;
    const bool mount = o >= Mounts && o < Mounts + MAX_MOUNTS;
    const float headTurn = mount ? MountHeadTurn[o - Mounts] : 0.f;
    const float headRoll = mount ? MountHeadRoll[o - Mounts] : 0.f;
    PoseBlend blend;
    model.Animation(bones, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, angle, o->HeadAngle, false,
                    translate, nullptr, currentAction, MAX_BONES, turnLag, headTurn, headRoll,
                    CharacterPoseFade(character, blend));
    if (character != nullptr && !translate)
    {
        PlaceFeet(character, bones);
    }
}
} // namespace Engine::Characters
