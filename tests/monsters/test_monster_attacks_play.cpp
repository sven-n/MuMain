#include "stdafx.h"

// The attack animations of monsters in the game (monsters design, MN2): the
// attacks of their entries, as SetPlayerAttack plays them on their maps, and
// the fixes of the problems the old code had.

#include "MonsterCreationScope.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#include <doctest.h>

#include <cstdlib>
#include <vector>

// The end of a monster's animation (MoveCharacter calls it).
void EtcStopAnimationSetting(CHARACTER* c, OBJECT* o);

namespace
{
constexpr int NoMonsterSkill = 0xFFFF;

// The shipped monster list and a creation scope on the world.
class AttackScope
{
public:
    explicit AttackScope(int world) : m_scope(world)
    {
        const std::vector<Data::Monsters::MonsterDefinition> monsters =
            Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
                .monsters;
        REQUIRE(monsters.size() > 500);
        g_MonsterDatabase.Build(monsters, "en");
        g_MapProcess = MapProcess::Make();
    }

    CHARACTER& Create(int number)
    {
        CHARACTER* character = m_scope.Create(number);
        REQUIRE(character != nullptr);
        return *character;
    }

private:
    MonsterRecorder::CreationScope m_scope;
};

// An attack with these skills; the animation it plays.
int Attack(CHARACTER& character, int monsterSkill, int skill = 0)
{
    character.MonsterSkill = static_cast<WORD>(monsterSkill);
    character.Skill = static_cast<WORD>(skill);
    character.Object.CurrentAction = MONSTER01_WALK;
    character.Object.AnimationFrame = 0.0f;
    SetPlayerAttack(&character);
    return character.Object.CurrentAction;
}
} // namespace

TEST_CASE("Monster attacks: the Kanturu 1st warriors play attack 2 for their skill at any count (P3) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_37KANTURU_1ST);
    struct Warrior
    {
        int number;
        int skill;
    };
    for (const Warrior warrior : {Warrior{MONSTER_KENTAUROS, ATMON_SKILL_NUM9},
                                  Warrior{MONSTER_BERSERKER_WARRIOR, ATMON_SKILL_EX_BERSERKERWARRIOR_ATTACKSKILL},
                                  Warrior{MONSTER_KENTAUROS_WARRIOR, ATMON_SKILL_EX_KENTAURUSWARRIOR_ATTACKSKILL},
                                  Warrior{MONSTER_GENOCIDER_WARRIOR, ATMON_SKILL_EX_GENOSIDEWARRIOR_ATTACKSKILL}})
    {
        INFO("monster " << warrior.number);
        CHARACTER& character = scope.Create(warrior.number);
        // The generic pattern played attack 1 at counts 0 and 3 and
        // overwrote the skill's animation.
        for (const BYTE count : {BYTE{0}, BYTE{1}, BYTE{3}})
        {
            character.SwordCount = count;
            CHECK(Attack(character, warrior.skill) == MONSTER01_ATTACK2);
            CHECK(character.MonsterSkill == NoMonsterSkill);
            character.SwordCount = count;
            CHECK(Attack(character, NoMonsterSkill) == MONSTER01_ATTACK1);
        }
    }
}

TEST_CASE("Monster attacks: Selupan and the Dark Ghost clear their skill when the attack ends (P4) "
          "[monsters][attacks]")
{
    {
        AttackScope scope(WD_58ICECITY_BOSS);
        CHARACTER& selupan = scope.Create(MONSTER_SELUPAN);
        CHECK(Attack(selupan, 34) == MONSTER01_ATTACK2);
        // The effects of the skill read it while the attack plays.
        CHECK(selupan.MonsterSkill == 34);
        EtcStopAnimationSetting(&selupan, &selupan.Object);
        CHECK(selupan.MonsterSkill == NoMonsterSkill);
        // The next normal attack does not repeat the skill (attack 2).
        CHECK(Attack(selupan, selupan.MonsterSkill) == MONSTER01_ATTACK4);
    }
    {
        AttackScope scope(WD_58ICECITY_BOSS);
        CHARACTER& selupan = scope.Create(MONSTER_SELUPAN);
        // A hit's shock ends attack 3 before its end, which clears the skill.
        CHECK(Attack(selupan, 36) == MONSTER01_ATTACK3);
        SetPlayerShock(&selupan, 1);
        CHECK(selupan.Object.CurrentAction == MONSTER01_SHOCK);
        CHECK(selupan.MonsterSkill == NoMonsterSkill);
        CHECK(Attack(selupan, selupan.MonsterSkill) == MONSTER01_ATTACK4);
        // Attack 1 and 2 are not shocked: the skill stays until they end.
        CHECK(Attack(selupan, 34) == MONSTER01_ATTACK2);
        SetPlayerShock(&selupan, 1);
        CHECK(selupan.Object.CurrentAction == MONSTER01_ATTACK2);
        CHECK(selupan.MonsterSkill == 34);
        // Skill 41 plays no attack, so it is cleared at once.
        CHECK(Attack(selupan, 41) == MONSTER01_WALK);
        CHECK(selupan.MonsterSkill == NoMonsterSkill);
        CHECK(Attack(selupan, selupan.MonsterSkill) == MONSTER01_ATTACK4);
    }
    {
        AttackScope scope(WD_70EMPIREGUARDIAN2);
        CHARACTER& ghost = scope.Create(MONSTER_DARK_GHOST);
        CHECK(Attack(ghost, 51) == MONSTER01_ATTACK2);
        CHECK(ghost.MonsterSkill == 51);
        EtcStopAnimationSetting(&ghost, &ghost.Object);
        CHECK(ghost.MonsterSkill == NoMonsterSkill);
        CHECK(Attack(ghost, ghost.MonsterSkill) == MONSTER01_ATTACK1);
    }
}

TEST_CASE("Monster attacks: the Dark Iron Knight plays attack 2 for skill 77, the Dark Coolutin for 32 (P5) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_57ICECITY);
    CHARACTER& knight = scope.Create(MONSTER_DARK_IRON_KNIGHT);
    CHECK(Attack(knight, 77) == MONSTER01_ATTACK2);
    CHECK(Attack(knight, 32) == MONSTER01_ATTACK1);
    CHARACTER& coolutin = scope.Create(MONSTER_DARK_COOLUTIN);
    CHECK(Attack(coolutin, 32) == MONSTER01_ATTACK2);
    CHECK(Attack(coolutin, 77) == MONSTER01_ATTACK1);
}

TEST_CASE("Monster attacks: the random Kanturu 1st monsters draw 50/50 at any frame rate (P18) [monsters][attacks]")
{
    AttackScope scope(WD_37KANTURU_1ST);
    const float savedFps = FPS_ANIMATION_FACTOR;
    for (const int number : {MONSTER_BERSERKER, MONSTER_GIGANTIS, MONSTER_GENOCIDER, MONSTER_SPLINTER_WOLF,
                             MONSTER_IRON_RIDER, MONSTER_GIGANTIS_WARRIOR})
    {
        INFO("monster " << number);
        CHARACTER& character = scope.Create(number);
        for (const unsigned int seed : {1u, 2u, 3u, 4u, 5u, 6u})
        {
            std::srand(seed);
            const int expected = MONSTER01_ATTACK1 + std::rand() % 2;
            // Before, the chance of attack 1 fell with the frame rate.
            for (const float fps : {1.0f, 0.25f})
            {
                FPS_ANIMATION_FACTOR = fps;
                std::srand(seed);
                Random::Seed(seed);
                CHECK(Attack(character, NoMonsterSkill) == expected);
            }
        }
    }
    FPS_ANIMATION_FACTOR = savedFps;
}
