#include "stdafx.h"

#include "doctest.h"

// The attacks of a monster entry (monsters design, D48): which animation each
// skill plays, and how the list holds them.

#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonstersJson.h"

#include <algorithm>
#include <string>
#include <vector>

using Data::Items::HasErrors;
using Data::Items::ItemDataIssue;
using Data::Monsters::ChooseMonsterAttack;
using Data::Monsters::MonsterAttackChoice;
using Data::Monsters::MonsterAttackRandom;
using Data::Monsters::MonsterAttacks;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterSkillClear;
using Data::Monsters::NoAttackAnimation;

namespace
{
// The skill numbers the packets carry when there is none.
constexpr int NoSkill = 0xFFFF;

MonsterAttacks IceWalker()
{
    MonsterAttacks attacks;
    attacks.monsterSkills = {{29, MONSTER01_ATTACK2}};
    attacks.otherwise = MONSTER01_ATTACK1;
    attacks.clearSkill = MonsterSkillClear::OnMatch;
    return attacks;
}

struct ReadResult
{
    std::vector<MonsterDefinition> monsters;
    std::vector<ItemDataIssue> issues;
};

ReadResult ReadMonster(const std::string& attacks)
{
    ReadResult result;
    Data::Monsters::ReadMonstersJson(
        R"({ "formatVersion": 1, "monsters": [ { "number": 456, "name": "Ice Walker", "kind": "monster", )"
        R"("attacks": )" +
            attacks + " } ] }",
        "Monsters.json", result.monsters, result.issues);
    return result;
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, const std::string& field, const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                       { return issue.field == field && issue.message.find(message) != std::string::npos; });
}
} // namespace

TEST_CASE("Monster attacks: a monster skill entry plays its animation, any other skill otherwise [data][monsters]")
{
    const MonsterAttacks attacks = IceWalker();
    CHECK(ChooseMonsterAttack(attacks, 29, 0) == MonsterAttackChoice{MONSTER01_ATTACK2, std::nullopt, true});
    CHECK(ChooseMonsterAttack(attacks, 30, 0) == MonsterAttackChoice{MONSTER01_ATTACK1, std::nullopt, false});
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 29) == MonsterAttackChoice{MONSTER01_ATTACK1, std::nullopt, false});
}

TEST_CASE("Monster attacks: a player skill entry plays when no monster skill entry does [data][monsters]")
{
    MonsterAttacks attacks;
    attacks.monsterSkills = {{1, MONSTER01_ATTACK3}};
    attacks.skills = {{17, MONSTER01_ATTACK2}, {200, NoAttackAnimation}};
    attacks.otherwise = MONSTER01_ATTACK1;
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 17).animation == MONSTER01_ATTACK2);
    CHECK(ChooseMonsterAttack(attacks, 1, 17).animation == MONSTER01_ATTACK3);
    // None: the animation that plays goes on.
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 200).animation == NoAttackAnimation);
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0).animation == MONSTER01_ATTACK1);
}

TEST_CASE("Monster attacks: a random draw replaces otherwise, not an entry [data][monsters]")
{
    MonsterAttacks attacks;
    attacks.monsterSkills = {{9, MONSTER01_ATTACK2}};
    attacks.otherwiseRandom = MonsterAttackRandom::RandFps;
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0).random == MonsterAttackRandom::RandFps);
    CHECK_FALSE(ChooseMonsterAttack(attacks, 9, 0).random);
    CHECK(ChooseMonsterAttack(attacks, 9, 0).animation == MONSTER01_ATTACK2);
}

TEST_CASE("Monster attacks: the monster skill is cleared never, on a match, on no match, always or after the attack "
          "[data][monsters]")
{
    MonsterAttacks attacks = IceWalker();
    const auto clears = [&](MonsterSkillClear clear, int monsterSkill)
    {
        attacks.clearSkill = clear;
        return ChooseMonsterAttack(attacks, monsterSkill, 0).clearMonsterSkill;
    };
    CHECK_FALSE(clears(MonsterSkillClear::Never, 29));
    CHECK_FALSE(clears(MonsterSkillClear::Never, 30));
    CHECK(clears(MonsterSkillClear::OnMatch, 29));
    CHECK_FALSE(clears(MonsterSkillClear::OnMatch, 30));
    CHECK_FALSE(clears(MonsterSkillClear::OnNoMatch, 29));
    CHECK(clears(MonsterSkillClear::OnNoMatch, 30));
    CHECK(clears(MonsterSkillClear::Always, 29));
    CHECK(clears(MonsterSkillClear::Always, 30));
    // afterAttack keeps it for the attack's effects and clears it when the
    // animation ends.
    CHECK_FALSE(clears(MonsterSkillClear::AfterAttack, 29));
    CHECK(Data::Monsters::ClearsMonsterSkillAfterAttack(attacks));
    attacks.clearSkill = MonsterSkillClear::Always;
    CHECK_FALSE(Data::Monsters::ClearsMonsterSkillAfterAttack(attacks));
}

TEST_CASE("Monster attacks: the list reads and writes them, the skill tables on one line [data][monsters]")
{
    MonsterDefinition monster;
    monster.number = 459;
    monster.names.Set("en", "Selupan");
    monster.attacks.emplace();
    monster.attacks->monsterSkills = {{34, MONSTER01_ATTACK2}, {37, MONSTER01_APEAR}, {41, NoAttackAnimation}};
    monster.attacks->otherwise = MONSTER01_ATTACK4;
    MonsterDefinition hellas;
    hellas.number = 294;
    hellas.names.Set("en", "Schriker");
    hellas.attacks.emplace();
    hellas.attacks->skills = {{17, MONSTER01_ATTACK2}};
    hellas.attacks->otherwiseRandom = MonsterAttackRandom::Rand;
    hellas.attacks->clearSkill = MonsterSkillClear::Always;
    MonsterDefinition trap;
    trap.number = 101;
    trap.names.Set("en", "Iron Stick");
    trap.attacks.emplace();
    trap.attacks->otherwise = MONSTER01_STOP2;
    trap.attacks->sound = "SOUND_TRAP01";
    trap.attacks->soundAtMonster = true;
    MonsterDefinition fire;
    fire.number = 102;
    fire.names.Set("en", "Fire Trap");
    fire.attacks.emplace();
    fire.attacks->otherwise = NoAttackAnimation;
    fire.attacks->sound = "SOUND_FLAME";
    const std::vector<MonsterDefinition> monsters = {monster, hellas, trap, fire};

    const std::string text = Data::Monsters::WriteMonstersJson(monsters);
    CHECK(text.find(R"("monsterSkills": {"34": "attack2", "37": "appear", "41": "none"})") != std::string::npos);
    CHECK(text.find(R"("otherwise": "attack4")") != std::string::npos);
    CHECK(text.find(R"("otherwiseRandom": "rand")") != std::string::npos);
    CHECK(text.find(R"("clearSkill": "always")") != std::string::npos);
    CHECK(text.find(R"("sound": "SOUND_TRAP01",)") != std::string::npos);
    CHECK(text.find(R"("soundAtMonster": true)") != std::string::npos);
    CHECK(text.find(R"("sound": "SOUND_FLAME")") != std::string::npos);
    // A sound everywhere alike leaves soundAtMonster out.
    CHECK(text.find("soundAtMonster") == text.rfind("soundAtMonster"));

    std::vector<MonsterDefinition> read;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", read, issues);
    CHECK(issues.empty());
    REQUIRE(read.size() == 4);
    CHECK(read[0] == trap);
    CHECK(read[1] == fire);
    CHECK(read[2] == hellas);
    CHECK(read[3] == monster);
}

TEST_CASE("Monster attacks: without otherwise they play attack 1, and nothing is cleared [data][monsters]")
{
    const ReadResult result = ReadMonster(R"({ "monsterSkills": {"29": "attack2"} })");
    CHECK(result.issues.empty());
    REQUIRE(result.monsters.size() == 1);
    REQUIRE(result.monsters[0].attacks);
    CHECK(result.monsters[0].attacks->otherwise == MONSTER01_ATTACK1);
    CHECK(result.monsters[0].attacks->clearSkill == MonsterSkillClear::Never);
}

TEST_CASE("Monster attacks: wrong values are errors that name the monster and the field [data][monsters]")
{
    struct WrongValue
    {
        const char* attacks;
        const char* field;
        const char* message;
    };
    for (const WrongValue& wrong : std::vector<WrongValue>{
             {R"([])", "monster 456 attacks", "must be an object"},
             {R"({ "monsterSkills": [29] })", "monster 456 attacks.monsterSkills", "must be an object"},
             {R"({ "monsterSkills": {"x": "attack2"} })", "monster 456 attacks.monsterSkills.x", "skill number"},
             {R"({ "monsterSkills": {"70000": "attack2"} })", "monster 456 attacks.monsterSkills.70000",
              "skill number"},
             // "7" and "007" would be the same skill.
             {R"({ "monsterSkills": {"007": "attack2"} })", "monster 456 attacks.monsterSkills.007",
              "without leading zeros"},
             {R"({ "skills": {"07": "attack2"} })", "monster 456 attacks.skills.07", "without leading zeros"},
             {R"({ "skills": {"17": "jump"} })", "monster 456 attacks.skills.17", "must be an animation"},
             {R"({ "otherwise": 3 })", "monster 456 attacks.otherwise", "must be an animation"},
             {R"({ "otherwiseRandom": "dice" })", "monster 456 attacks.otherwiseRandom", "rand, randFps or bali"},
             {R"({ "otherwise": "attack1", "otherwiseRandom": "rand" })", "monster 456 attacks", "not both"},
             {R"({ "clearSkill": "sometimes" })", "monster 456 attacks.clearSkill", "never, onMatch"},
             {R"({ "sound": "SOUND_NOBODY_KNOWS" })", "monster 456 attacks.sound", "SOUND_ id"},
             {R"({ "sound": 3 })", "monster 456 attacks.sound", "SOUND_ id"},
             {R"({ "sound": "SOUND_TRAP01", "soundAtMonster": 1 })", "monster 456 attacks.soundAtMonster",
              "true or false"},
         })
    {
        INFO(wrong.attacks);
        const ReadResult result = ReadMonster(wrong.attacks);
        CHECK(HasErrors(result.issues));
        CHECK(HasIssueAbout(result.issues, wrong.field, wrong.message));
        CHECK(result.monsters.empty());
    }
}

TEST_CASE("Monster attacks: an unknown field is a warning [data][monsters]")
{
    const ReadResult result = ReadMonster(R"({ "otherwise": "shock", "map": 30 })");
    CHECK_FALSE(HasErrors(result.issues));
    REQUIRE(result.issues.size() == 1);
    REQUIRE(result.monsters.size() == 1);
    CHECK(result.monsters[0].attacks->otherwise == MONSTER01_SHOCK);
}
