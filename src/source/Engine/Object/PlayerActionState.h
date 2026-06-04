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
}
