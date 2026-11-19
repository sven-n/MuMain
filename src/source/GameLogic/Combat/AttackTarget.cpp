#include "stdafx.h"
#include "AttackTarget.h"

#include "Engine/Object/ZzzCharacter.h"

namespace GameLogic::Combat
{
OBJECT* AttackTargetObject(const CHARACTER& attacker)
{
    const int slot = attacker.TargetCharacter;
    if (slot < 0 || slot >= MAX_CHARACTERS_CLIENT)
        return nullptr;

    return &CharactersClient[slot].Object;
}
} // namespace GameLogic::Combat
