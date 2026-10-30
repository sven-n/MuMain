#include "stdafx.h"

#include "GameLogic/Monsters/MonsterAttackPlay.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"

#include <cstdlib>

namespace GameLogic::Monsters::Attacks
{
namespace
{
// The monster whose skill packet starts the attack being chosen.
const CHARACTER* g_skillAttacker = nullptr;

// The attacks of the monster's entry, or of its map variant on this map.
const Data::Monsters::MonsterAttacks* FindAttacks(const CHARACTER& character)
{
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(character.MonsterIndex);
    if (monster == nullptr)
    {
        return nullptr;
    }
    const std::optional<Data::Monsters::MonsterAttacks>& attacks =
        Data::Monsters::GetAttacksOnMap(*monster, gMapManager.GetServerMap(), gMapManager.GetServerStage());
    return attacks ? &*attacks : nullptr;
}
} // namespace

bool Play(CHARACTER& character, OBJECT& object)
{
    const Data::Monsters::MonsterAttacks* attacks = FindAttacks(character);
    if (attacks == nullptr)
    {
        return false;
    }
    if (Data::Monsters::ClearsMonsterSkillAfterAttack(*attacks) && &character != g_skillAttacker)
    {
        // Not the attack its skill packet started: a normal attack, also
        // while the skill's attack plays, which would play the skill again
        // with its sounds and effects (P4).
        character.MonsterSkill = -1;
    }
    const Data::Monsters::MonsterAttackChoice choice = Data::Monsters::ChooseMonsterAttack(
        *attacks, character.MonsterSkill, character.Skill, character.AttackRotation);
    const int animation = ResolveAttackAnimation(*attacks, choice, character.SwordCount, character.AttackRotation);
    if (animation != Data::Monsters::NoAttackAnimation)
    {
        SetAction(&object, animation);
    }
    else if (Data::Monsters::ClearsMonsterSkillAfterAttack(*attacks))
    {
        // No attack plays, so none ends that would clear the skill (P4).
        character.MonsterSkill = -1;
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

void OnAttackEnded(CHARACTER& character)
{
    const Data::Monsters::MonsterAttacks* attacks = FindAttacks(character);
    if (attacks != nullptr && Data::Monsters::ClearsMonsterSkillAfterAttack(*attacks))
    {
        character.MonsterSkill = -1;
    }
}

SkillAttackScope::SkillAttackScope(const CHARACTER& character) : m_previous(g_skillAttacker)
{
    g_skillAttacker = &character;
}

SkillAttackScope::~SkillAttackScope()
{
    g_skillAttacker = m_previous;
}

bool IsAttackAnimation(int animation)
{
    switch (animation)
    {
    case MONSTER01_ATTACK1:
    case MONSTER01_ATTACK2:
    case MONSTER01_ATTACK3:
    case MONSTER01_ATTACK4:
    case MONSTER01_ATTACK5:
    case MONSTER01_APEAR:
        return true;
    default:
        return false;
    }
}

int NextGenericAttack(CHARACTER& character)
{
    return NextGenericAttack(character.SwordCount);
}

int NextGenericAttack(std::uint8_t& swordCount)
{
    const int animation = swordCount % 3 == 0 ? MONSTER01_ATTACK1 : MONSTER01_ATTACK2;
    swordCount++;
    return animation;
}

int ResolveAttackAnimation(const Data::Monsters::MonsterAttacks& attacks,
                           const Data::Monsters::MonsterAttackChoice& choice, std::uint8_t& swordCount, int& rotation)
{
    int animation = choice.animation;
    if (choice.random)
    {
        animation = DrawAttack(*choice.random);
    }
    else if (choice.generic)
    {
        animation = NextGenericAttack(swordCount);
    }
    if (choice.advancesRotation)
    {
        rotation = (rotation + 1) % static_cast<int>(attacks.otherwise.size());
    }
    return animation;
}

// The old code's draws, which call rand() and rand_fps_check() as it did.
int DrawAttack(Data::Monsters::MonsterAttackRandom random)
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
} // namespace GameLogic::Monsters::Attacks
