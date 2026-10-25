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
