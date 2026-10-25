#include "stdafx.h"

// One-time comparison of MN2 (monsters design, D50): every monster with
// `attacks` in Data/Monsters/Monsters.json chooses its attack animation from
// them exactly as SetPlayerAttack's handlers chose it. Each case runs twice in
// one build: with the monster list without `attacks`, so the handlers run
// their old code, and with the shipped list. It goes with the old code.

#include "MonsterCreationScope.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#include <doctest.h>

#include <cstdlib>
#include <initializer_list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace
{
using Data::Monsters::MonsterDefinition;

// A skill no handler and no entry names.
constexpr int UnnamedMonsterSkill = 99;
constexpr int UnnamedSkill = 9999;
constexpr int NoMonsterSkill = 0xFFFF;
// An action no attack plays, to see "no animation".
constexpr unsigned short ActionBefore = MONSTER01_WALK;
constexpr float HeightBefore = 1234.0f;
constexpr std::initializer_list<unsigned int> Seeds = {7u, 12345u};
constexpr std::initializer_list<float> FpsFactors = {1.0f, 0.4f};

struct Case
{
    int number;
    int world;
    int monsterSkill;
    int skill;
    int swordCount;
    unsigned int seed;
    float fps;

    auto Key() const
    {
        return std::make_tuple(number, world, monsterSkill, skill, swordCount, seed, fps);
    }
    std::string Describe() const
    {
        std::ostringstream text;
        text << "monster " << number << " world " << world << " monsterSkill " << monsterSkill << " skill " << skill
             << " swordCount " << swordCount << " seed " << seed << " fps " << fps;
        return text.str();
    }
};

// What an attack left: the animation, the skills, the count of the generic
// pattern, the guard AI, the height (Selupan), and the next rand() (how many
// draws it took).
struct Step
{
    int action = 0;
    int monsterSkill = 0;
    int skill = 0;
    int swordCount = 0;
    int ai = 0;
    float height = 0.0f;
    int nextRand = 0;

    bool operator==(const Step&) const = default;
    std::string Describe() const
    {
        std::ostringstream text;
        text << "action " << action << " monsterSkill " << monsterSkill << " skill " << skill << " swordCount "
             << swordCount << " ai " << ai << " height " << height << " nextRand " << nextRand;
        return text.str();
    }
};

// A case is two attacks: the case's skills, then a normal attack (a rage the
// first started shows in the second).
using Result = std::pair<Step, Step>;

std::vector<MonsterDefinition> ShippedMonsters()
{
    const std::vector<MonsterDefinition> monsters =
        Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
            .monsters;
    REQUIRE(monsters.size() > 500);
    return monsters;
}

std::vector<MonsterDefinition> WithoutAttacks(std::vector<MonsterDefinition> monsters)
{
    for (MonsterDefinition& monster : monsters)
    {
        monster.attacks.reset();
    }
    return monsters;
}

// The effect pools, empty: the effects of a skill (Selupan's) would fill
// them over the cases, and a full pool draws other random numbers.
void ClearEffectPools()
{
    const auto clear = [](auto* pool, int count)
    {
        for (int index = 0; index < count; ++index)
        {
            pool[index].Live = false;
        }
    };
    clear(Effects, MAX_EFFECTS);
    clear(Sprites, MAX_SPRITES);
    clear(Joints, MAX_JOINTS);
    clear(Particles, MAX_PARTICLES);
    clear(Leaves, MAX_LEAVES);
    clear(Points, MAX_POINTS);
}

Step Attack(CHARACTER& character, int monsterSkill, int skill)
{
    OBJECT& object = character.Object;
    character.MonsterSkill = static_cast<WORD>(monsterSkill);
    character.Skill = static_cast<WORD>(skill);
    object.CurrentAction = ActionBefore;
    object.AnimationFrame = 0.0f;
    SetPlayerAttack(&character);
    Step step;
    step.action = object.CurrentAction;
    step.monsterSkill = character.MonsterSkill;
    step.skill = character.Skill;
    step.swordCount = character.SwordCount;
    step.ai = object.AI;
    step.height = object.Position[2];
    step.nextRand = std::rand();
    return step;
}

std::vector<Case> CasesOf(const MonsterDefinition& monster, int world)
{
    std::set<int> monsterSkills = {0, UnnamedMonsterSkill, NoMonsterSkill, ATMON_SKILL_EMPIREGUARDIAN_BERSERKER};
    std::set<int> skills = {0, UnnamedSkill};
    for (const auto& entry : monster.attacks->monsterSkills)
    {
        monsterSkills.insert(entry.first);
    }
    for (const auto& entry : monster.attacks->skills)
    {
        skills.insert(entry.first);
    }
    std::vector<Case> cases;
    for (const int monsterSkill : monsterSkills)
    {
        for (const int skill : skills)
        {
            for (const int swordCount : {0, 1})
            {
                for (const unsigned int seed : Seeds)
                {
                    for (const float fps : FpsFactors)
                    {
                        cases.push_back({monster.number, world, monsterSkill, skill, swordCount, seed, fps});
                    }
                }
            }
        }
    }
    return cases;
}

// Every case of every monster with attacks on one world, with the monster list
// `monsters`.
void Run(const std::vector<MonsterDefinition>& shipped, const std::vector<MonsterDefinition>& monsters, int world,
         std::map<decltype(Case{}.Key()), std::pair<Case, Result>>& results)
{
    g_MonsterDatabase.Build(monsters, "en");
    MonsterRecorder::CreationScope scope(world);
    // CheckMonsterSkill's table, as the game loads it at startup.
    OpenMonsterSkillScript(L"Data\\Local\\MonsterSkill.bmd");
    const float savedFps = FPS_ANIMATION_FACTOR;
    // The rage states of the Empire Guardian live in its map process: a new
    // one for each case starts them off.
    const bool empireGuardian = world >= WD_69EMPIREGUARDIAN1 && world <= WD_72EMPIREGUARDIAN4;
    for (const MonsterDefinition& monster : shipped)
    {
        if (!monster.attacks)
        {
            continue;
        }
        CHARACTER* character = scope.Create(monster.number);
        REQUIRE(character != nullptr);
        for (const Case& test : CasesOf(monster, world))
        {
            if (empireGuardian)
            {
                g_MapProcess = MapProcess::Make();
            }
            character->SwordCount = static_cast<BYTE>(test.swordCount);
            character->Object.AI = 0;
            character->Object.Position[2] = HeightBefore;
            FPS_ANIMATION_FACTOR = test.fps;
            ClearEffectPools();
            std::srand(test.seed);
            Random::Seed(test.seed);
            const Step first = Attack(*character, test.monsterSkill, test.skill);
            const Step second = Attack(*character, NoMonsterSkill, 0);
            results[test.Key()] = {test, {first, second}};
        }
    }
    FPS_ANIMATION_FACTOR = savedFps;
}

void Compare(std::initializer_list<int> worlds)
{
    const std::vector<MonsterDefinition> shipped = ShippedMonsters();
    std::map<decltype(Case{}.Key()), std::pair<Case, Result>> old;
    std::map<decltype(Case{}.Key()), std::pair<Case, Result>> data;
    for (const int world : worlds)
    {
        Run(shipped, WithoutAttacks(shipped), world, old);
        Run(shipped, shipped, world, data);
    }
    g_MonsterDatabase.Build(shipped, "en");
    REQUIRE(old.size() == data.size());
    REQUIRE(!old.empty());
    int differences = 0;
    for (const auto& [key, oldCase] : old)
    {
        const auto& dataCase = data.at(key);
        if (oldCase.second == dataCase.second)
        {
            continue;
        }
        if (++differences <= 20)
        {
            INFO(oldCase.first.Describe()
                 << "\n  old: " << oldCase.second.first.Describe() << " / " << oldCase.second.second.Describe()
                 << "\n data: " << dataCase.second.first.Describe() << " / " << dataCase.second.second.Describe());
            CHECK(false);
        }
    }
    MESSAGE(old.size() << " cases, " << differences << " differ");
    CHECK(differences == 0);
}
} // namespace

TEST_CASE("Monster attacks: the data plays what the handlers played, on maps without a gate and in Hellas "
          "[monsters][attacks][comparison]")
{
    Compare({WD_0LORENCIA, WD_24HELLAS, WD_30BATTLECASTLE, WD_31HUNTING_GROUND});
}

TEST_CASE("Monster attacks: the data plays what the handlers played, in Aida, Crywolf, Devil Square and Kanturu "
          "[monsters][attacks][comparison]")
{
    Compare({WD_33AIDA, WD_34CRYWOLF_1ST, WD_9DEVILSQUARE, WD_37KANTURU_1ST, WD_38KANTURU_2ND, WD_39KANTURU_3RD});
}

TEST_CASE("Monster attacks: the data plays what the handlers played, in the barracks, Illusion Temple, the Swamp "
          "and Raklion [monsters][attacks][comparison]")
{
    Compare({WD_41CHANGEUP3RD_1ST, WD_42CHANGEUP3RD_2ND, WD_45CURSEDTEMPLE_LV1, WD_56MAP_SWAMP_OF_QUIET, WD_57ICECITY,
             WD_58ICECITY_BOSS});
}

TEST_CASE("Monster attacks: the data plays what the handlers played, in Empire Guardian and Loren Market "
          "[monsters][attacks][comparison]")
{
    Compare({WD_69EMPIREGUARDIAN1, WD_70EMPIREGUARDIAN2, WD_71EMPIREGUARDIAN3, WD_72EMPIREGUARDIAN4,
             WD_79UNITEDMARKETPLACE});
}

// A comparison of two recorders that both do nothing finds no difference:
// the data must make the Ice Walker play attack 2 for its skill 29 in Raklion.
TEST_CASE("Monster attacks: the Ice Walker plays attack 2 for skill 29 in Raklion [monsters][attacks]")
{
    const std::vector<MonsterDefinition> shipped = ShippedMonsters();
    g_MonsterDatabase.Build(shipped, "en");
    MonsterRecorder::CreationScope scope(WD_57ICECITY);
    g_MapProcess = MapProcess::Make();
    CHARACTER* character = scope.Create(MONSTER_ICE_WALKER);
    REQUIRE(character != nullptr);
    CHECK(Attack(*character, 29, 0).action == MONSTER01_ATTACK2);
    CHECK(Attack(*character, NoMonsterSkill, 0).action == MONSTER01_ATTACK1);
}
