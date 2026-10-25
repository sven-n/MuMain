#include "stdafx.h"

#include "GameLogic/Monsters/MonsterAttackPlay.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <cstdlib>

namespace GameLogic::Monsters::Attacks
{
namespace
{
// The old code's draws, which call rand() and rand_fps_check() as it did.
int Draw(Data::Monsters::MonsterAttackRandom random)
{
    switch (random)
    {
    case Data::Monsters::MonsterAttackRandom::Rand:
        return MONSTER01_ATTACK1 + std::rand() % 2;
    case Data::Monsters::MonsterAttackRandom::RandFps:
        return rand_fps_check(2) ? MONSTER01_ATTACK1 : MONSTER01_ATTACK2;
    case Data::Monsters::MonsterAttackRandom::Bali:
    {
        const int action = std::rand() % 8;
        if (action > 2)
        {
            return MONSTER01_ATTACK1 + std::rand() % 2;
        }
        return action > 0 ? MONSTER01_ATTACK3 : MONSTER01_ATTACK4;
    }
    }
    return MONSTER01_ATTACK1;
}

const Data::Monsters::MonsterAttacks* FindAttacks(const CHARACTER& character)
{
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(character.MonsterIndex);
    return monster != nullptr && monster->attacks ? &*monster->attacks : nullptr;
}
} // namespace

bool Play(CHARACTER& character, OBJECT& object)
{
    const Data::Monsters::MonsterAttacks* attacks = FindAttacks(character);
    if (attacks == nullptr)
    {
        return false;
    }
    const Data::Monsters::MonsterAttackChoice choice =
        Data::Monsters::ChooseMonsterAttack(*attacks, character.MonsterSkill, character.Skill);
    const int animation = choice.random ? Draw(*choice.random) : choice.animation;
    if (animation != Data::Monsters::NoAttackAnimation)
    {
        SetAction(&object, animation);
    }
    if (choice.clearMonsterSkill)
    {
        character.MonsterSkill = -1;
    }
    if (const std::optional<int> sound = Data::Monsters::FindMonsterSoundId(attacks->sound))
    {
        PlayBuffer(static_cast<ESound>(*sound), attacks->soundAtMonster ? &object : nullptr);
    }
    return true;
}

bool PlaysSound(const CHARACTER& character)
{
    const Data::Monsters::MonsterAttacks* attacks = FindAttacks(character);
    return attacks != nullptr && !attacks->sound.empty();
}

void OnAttackEnded(CHARACTER& character)
{
    const Data::Monsters::MonsterAttacks* attacks = FindAttacks(character);
    if (attacks != nullptr && Data::Monsters::ClearsMonsterSkillAfterAttack(*attacks))
    {
        character.MonsterSkill = -1;
    }
}
} // namespace GameLogic::Monsters::Attacks
