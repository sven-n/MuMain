#pragma once

#include "Core/Globals/_enum.h"

namespace Engine::Object
{
    // True while a player's action is one of the attack/skill swing animations
    // (PLAYER_ATTACK_FIST .. PLAYER_RIDE_SKILL). The swing animation's playback
    // speed scales with AttackSpeed (see SetAttackSpeed in ZzzCharacter.cpp), so
    // this predicate doubles as the natural attack-cadence gate: hold off the
    // next action until the current swing finishes, and the rate follows attack
    // speed instead of any fixed timer.
    inline bool IsAttackAction(int currentAction)
    {
        return currentAction >= PLAYER_ATTACK_FIST && currentAction <= PLAYER_RIDE_SKILL;
    }

    // True while a player is firing an equipped bow or crossbow. Elves shoot from
    // four different poses - on foot, on wings, on a Uniria/Dinorant mount and on a
    // Fenrir - and each pose has a separate animation track per weapon type (see
    // SetPlayerBow in WSclient.cpp), so the set is not contiguous in the enum.
    inline bool IsBowAttackAction(int currentAction)
    {
        switch (currentAction)
        {
        case PLAYER_ATTACK_BOW:
        case PLAYER_ATTACK_CROSSBOW:
        case PLAYER_ATTACK_FLY_BOW:
        case PLAYER_ATTACK_FLY_CROSSBOW:
        case PLAYER_ATTACK_RIDE_BOW:
        case PLAYER_ATTACK_RIDE_CROSSBOW:
        case PLAYER_FENRIR_ATTACK_BOW:
        case PLAYER_FENRIR_ATTACK_CROSSBOW:
            return true;
        default:
            return false;
        }
    }

    // True while a player is firing an equipped bow or crossbow in the raised-shot
    // pose (see SetPlayerHighBow in WSclient.cpp). Same four poses as
    // IsBowAttackAction, on the contiguous "_UP" animation tracks.
    inline bool IsRaisedBowAttackAction(int currentAction)
    {
        return currentAction >= PLAYER_ATTACK_BOW_UP && currentAction <= PLAYER_ATTACK_RIDE_CROSSBOW_UP;
    }

    // True while a player is sitting or holding a pose (PLAYER_SIT1 ..
    // PLAYER_POSE_FEMALE1). Used to keep these animations from being reset to the
    // stand pose when equipment or class changes.
    inline bool IsSitOrPoseAction(int currentAction)
    {
        return currentAction >= PLAYER_SIT1 && currentAction <= PLAYER_POSE_FEMALE1;
    }
}
