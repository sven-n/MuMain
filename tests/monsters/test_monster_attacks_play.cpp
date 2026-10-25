#include "stdafx.h"

// The attack animations of monsters in the game (monsters design, MN2): the
// attacks of their entries, as SetPlayerAttack plays them on their maps, and
// the fixes of the problems the old code had.

#include "MonsterCreationScope.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#include <doctest.h>

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
        AttackScope scope(WD_70EMPIREGUARDIAN2);
        CHARACTER& ghost = scope.Create(MONSTER_DARK_GHOST);
        CHECK(Attack(ghost, 51) == MONSTER01_ATTACK2);
        CHECK(ghost.MonsterSkill == 51);
        EtcStopAnimationSetting(&ghost, &ghost.Object);
        CHECK(ghost.MonsterSkill == NoMonsterSkill);
        CHECK(Attack(ghost, ghost.MonsterSkill) == MONSTER01_ATTACK1);
    }
}
