#include "stdafx.h"
#include "Engine/Object/Movement/MovementBones.h"

#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FootPlacement.h"
#include "Engine/Object/Movement/PoseFade.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"

namespace Engine::Characters
{
void AnimateBones(const OBJECT* o, BMD& model, float (*bones)[3][4], float animationFrame, bool translate,
                  short currentAction)
{
    const CHARACTER* character = CharacterOfObject(o);
    vec3_t angle = {o->Angle[0], o->Angle[1],
                    o->Angle[2] + (character != nullptr ? character->Motion.SnapTurnOffset : 0.f)};
    const float turnLag = character != nullptr ? character->Motion.TurnLag : 0.f;
    const bool mount = o >= Mounts && o < Mounts + MAX_MOUNTS;
    const float headTurn = mount ? MountHeadTurn[o - Mounts] : 0.f;
    const float headRoll = mount ? MountHeadRoll[o - Mounts] : 0.f;
    PoseBlend blend;
    model.Animation(bones, animationFrame, o->PriorAnimationFrame, o->PriorAction, angle, o->HeadAngle, false,
                    translate, nullptr, currentAction, MAX_BONES, turnLag, headTurn, headRoll,
                    CharacterPoseFade(character, blend));
    if (character != nullptr && !translate)
    {
        PlaceFeet(character, bones);
    }
}

MovementPoseKey CurrentMovementPoseKey(const OBJECT* o)
{
    MovementPoseKey key{};
    key[0] = EnhancedMovementEnabled() ? 1.f : 0.f;
    if (o >= Mounts && o < Mounts + MAX_MOUNTS)
    {
        key[1] = MountHeadTurn[o - Mounts];
        key[2] = MountHeadRoll[o - Mounts];
        return key;
    }
    const CHARACTER* character = CharacterOfObject(o);
    if (character == nullptr)
    {
        return key;
    }
    key[1] = character->Motion.SnapTurnOffset;
    key[2] = character->Motion.TurnLag;
    PoseBlend blend;
    if (const PoseBlend* fade = CharacterPoseFade(character, blend))
    {
        key[3] = static_cast<float>(fade->Action);
        key[4] = fade->Frame;
        key[5] = fade->Weight;
    }
    // The feet stand on the ground under the character.
    key[6] = o->Position[0];
    key[7] = o->Position[1];
    key[8] = o->Position[2];
    return key;
}
} // namespace Engine::Characters
