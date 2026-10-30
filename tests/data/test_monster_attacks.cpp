#include "stdafx.h"

#include "doctest.h"

// The attacks of a monster entry (monsters design, D48): which animation each
// skill plays, and how the list holds them.

#include "Core/Globals/_define.h"
#include "Data/GameData/MapData/MapRef.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonstersJson.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <string>
#include <vector>

using Data::Items::HasFatal;
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
    attacks.otherwise = {MONSTER01_ATTACK1};
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
    attacks.otherwise = {MONSTER01_ATTACK1};
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

TEST_CASE("Monster attacks: a rotation plays its animations in turn, generic the generic pattern (Q-MN69) "
          "[data][monsters]")
{
    MonsterAttacks attacks;
    attacks.monsterSkills = {{9, MONSTER01_ATTACK3}};
    attacks.otherwise = {MONSTER01_ATTACK2, MONSTER01_ATTACK1, MONSTER01_ATTACK4};
    CHECK(Data::Monsters::IsRotation(attacks));
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0, 0) ==
          MonsterAttackChoice{MONSTER01_ATTACK2, std::nullopt, false, false, true});
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0, 1).animation == MONSTER01_ATTACK1);
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0, 2).animation == MONSTER01_ATTACK4);
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0, 3).animation == MONSTER01_ATTACK2);
    // An entry plays its animation; the rotation waits.
    CHECK(ChooseMonsterAttack(attacks, 9, 0, 1) == MonsterAttackChoice{MONSTER01_ATTACK3, std::nullopt, false});

    attacks.otherwise = {MONSTER01_ATTACK2};
    CHECK_FALSE(Data::Monsters::IsRotation(attacks));
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0, 1) == MonsterAttackChoice{MONSTER01_ATTACK2, std::nullopt, false});

    attacks.otherwiseGeneric = true;
    CHECK(ChooseMonsterAttack(attacks, NoSkill, 0, 1) ==
          MonsterAttackChoice{NoAttackAnimation, std::nullopt, false, true, false});
    CHECK(ChooseMonsterAttack(attacks, 9, 0, 1).animation == MONSTER01_ATTACK3);
}

TEST_CASE("Monster attacks: a rotation and generic read and write, the rotation on one line [data][monsters]")
{
    MonsterDefinition rotation;
    rotation.number = 350;
    rotation.names.Set("en", "Berserker");
    rotation.attacks.emplace();
    rotation.attacks->otherwise = {MONSTER01_ATTACK2, MONSTER01_ATTACK1, NoAttackAnimation};
    MonsterDefinition generic;
    generic.number = 291;
    generic.names.Set("en", "Fire Golem");
    generic.attacks.emplace();
    generic.attacks->otherwiseGeneric = true;
    const std::vector<MonsterDefinition> monsters = {rotation, generic};

    const std::string text = Data::Monsters::WriteMonstersJson(monsters);
    CHECK(text.find(R"("otherwise": ["attack2", "attack1", "none"])") != std::string::npos);
    CHECK(text.find(R"("otherwise": "generic")") != std::string::npos);

    std::vector<MonsterDefinition> read;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", read, issues);
    CHECK(issues.empty());
    REQUIRE(read.size() == 2);
    CHECK(read[0] == generic);
    CHECK(read[1] == rotation);
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
    monster.attacks->otherwise = {MONSTER01_ATTACK4};
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
    trap.attacks->otherwise = {MONSTER01_STOP2};
    trap.attacks->sound = "SOUND_TRAP01";
    trap.attacks->soundAtMonster = true;
    MonsterDefinition fire;
    fire.number = 102;
    fire.names.Set("en", "Fire Trap");
    fire.attacks.emplace();
    fire.attacks->otherwise = {NoAttackAnimation};
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
    CHECK(result.monsters[0].attacks->otherwise == std::vector<int>{MONSTER01_ATTACK1});
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
             // A rotation has two animations at least, and generic is no animation.
             {R"({ "otherwise": ["attack1"] })", "monster 456 attacks.otherwise", "two or more animations"},
             {R"({ "otherwise": ["attack1", "jump"] })", "monster 456 attacks.otherwise", "two or more animations"},
             {R"({ "otherwise": ["generic", "attack1"] })", "monster 456 attacks.otherwise", "two or more animations"},
             {R"({ "otherwise": "generic", "otherwiseRandom": "rand" })", "monster 456 attacks", "not both"},
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
        CHECK(HasFatal(result.issues));
        CHECK(HasIssueAbout(result.issues, wrong.field, wrong.message));
        CHECK(result.monsters.empty());
    }
}

TEST_CASE("Monster attacks: an unknown field is a warning [data][monsters]")
{
    const ReadResult result = ReadMonster(R"({ "otherwise": "shock", "map": 30 })");
    CHECK_FALSE(HasFatal(result.issues));
    REQUIRE(result.issues.size() == 1);
    REQUIRE(result.monsters.size() == 1);
    CHECK(result.monsters[0].attacks->otherwise == std::vector<int>{MONSTER01_SHOCK});
}

namespace
{
// A monster entry with these fields after its kind.
ReadResult ReadMonsterWith(const std::string& fields)
{
    ReadResult result;
    Data::Monsters::ReadMonstersJson(
        R"({ "formatVersion": 1, "monsters": [ { "number": 291, "name": "Fire Golem", "kind": "monster", )" + fields +
            " } ] }",
        "Monsters.json", result.monsters, result.issues);
    return result;
}
} // namespace

TEST_CASE("Monster attacks: a map variant's attacks replace the monster's on its maps (Q-MN65, Q-MN68) "
          "[data][monsters]")
{
    MonsterDefinition golem;
    golem.number = 291;
    golem.names.Set("en", "Fire Golem");
    golem.attacks.emplace();
    golem.attacks->skills = {{50, MONSTER01_ATTACK1}};
    golem.attacks->otherwise = {MONSTER01_ATTACK2};
    Data::Monsters::MonsterMapVariant devilSquare;
    devilSquare.maps = {{32}};
    devilSquare.attacks.emplace();
    devilSquare.attacks->otherwiseGeneric = true;
    // A variant without attacks changes nothing.
    Data::Monsters::MonsterMapVariant empty;
    empty.maps = {{WD_9DEVILSQUARE}};
    golem.mapVariants = {devilSquare, empty};

    CHECK(&Data::Monsters::GetAttacksOnMap(golem, 32) == &golem.mapVariants[0].attacks);
    CHECK(&Data::Monsters::GetAttacksOnMap(golem, WD_9DEVILSQUARE) == &golem.attacks);
    CHECK(&Data::Monsters::GetAttacksOnMap(golem, WD_31HUNTING_GROUND) == &golem.attacks);

    // Without attacks of its own, the monster has attacks on the variant's maps only.
    MonsterDefinition variantOnly = golem;
    variantOnly.attacks.reset();
    CHECK(Data::Monsters::GetAttacksOnMap(variantOnly, 32).has_value());
    CHECK_FALSE(Data::Monsters::GetAttacksOnMap(variantOnly, WD_31HUNTING_GROUND).has_value());
}

TEST_CASE("Maps: two map list entries overlap on a shared map and stage, a map without a stage on all (D67) "
          "[data][maps]")
{
    using Data::Maps::MapRef;
    using Data::Maps::Overlaps;
    CHECK(Overlaps(MapRef{WD_9DEVILSQUARE}, MapRef{WD_9DEVILSQUARE}));
    CHECK(Overlaps(MapRef{WD_9DEVILSQUARE}, MapRef{WD_9DEVILSQUARE, 2}));
    CHECK(Overlaps(MapRef{WD_9DEVILSQUARE, 2}, MapRef{WD_9DEVILSQUARE}));
    CHECK(Overlaps(MapRef{WD_9DEVILSQUARE, 2}, MapRef{WD_9DEVILSQUARE, 2}));
    CHECK_FALSE(Overlaps(MapRef{WD_9DEVILSQUARE, 2}, MapRef{WD_9DEVILSQUARE, 3}));
    CHECK_FALSE(Overlaps(MapRef{WD_9DEVILSQUARE}, MapRef{32}));
}

TEST_CASE("Monster attacks: the maps of a map variant that another variant names too, stages included (Q-MN65) "
          "[data][monsters]")
{
    using Data::Maps::MapRef;
    MonsterDefinition golem;
    golem.number = 291;
    golem.mapVariants.resize(3);
    golem.mapVariants[0].maps = {{WD_9DEVILSQUARE}, {32}};
    golem.mapVariants[1].maps = {{WD_9DEVILSQUARE, 2}, {WD_31HUNTING_GROUND}};
    golem.mapVariants[2].maps = {{WD_0LORENCIA}};
    CHECK(Data::Monsters::FindMapsOfOtherVariants(golem, 0) == std::vector<MapRef>{{WD_9DEVILSQUARE}});
    CHECK(Data::Monsters::FindMapsOfOtherVariants(golem, 1) == std::vector<MapRef>{{WD_9DEVILSQUARE, 2}});
    CHECK(Data::Monsters::FindMapsOfOtherVariants(golem, 2).empty());
}

TEST_CASE("Monster attacks: a map variant names a stage of a map, a map without a stage all of them (D67) "
          "[data][monsters]")
{
    MonsterDefinition golem;
    golem.number = 291;
    golem.attacks.emplace();
    golem.attacks->otherwise = {MONSTER01_ATTACK2};
    Data::Monsters::MonsterMapVariant stage2;
    stage2.maps = {{WD_9DEVILSQUARE, 2}};
    stage2.attacks.emplace();
    stage2.attacks->otherwiseGeneric = true;
    golem.mapVariants = {stage2};

    CHECK(&Data::Monsters::GetAttacksOnMap(golem, WD_9DEVILSQUARE, 2) == &golem.mapVariants[0].attacks);
    CHECK(&Data::Monsters::GetAttacksOnMap(golem, WD_9DEVILSQUARE, 1) == &golem.attacks);
    CHECK(&Data::Monsters::GetAttacksOnMap(golem, WD_9DEVILSQUARE) == &golem.attacks);
    CHECK(&Data::Monsters::GetAttacksOnMap(golem, 32, 2) == &golem.attacks);

    golem.mapVariants[0].maps = {{32}};
    CHECK(&Data::Monsters::GetAttacksOnMap(golem, 32, 6) == &golem.mapVariants[0].attacks);
    CHECK(&Data::Monsters::GetAttacksOnMap(golem, 32) == &golem.mapVariants[0].attacks);
}

TEST_CASE("Monster attacks: map variants read and write, their maps on one line [data][monsters]")
{
    const ReadResult result =
        ReadMonsterWith(R"("attacks": { "skills": {"50": "attack1"}, "otherwise": "attack2" }, )"
                        R"("mapVariants": [ { "maps": [32, 9], "attacks": { "otherwise": "generic" } } ])");
    CHECK(result.issues.empty());
    REQUIRE(result.monsters.size() == 1);
    const MonsterDefinition& golem = result.monsters[0];
    REQUIRE(golem.mapVariants.size() == 1);
    CHECK(golem.mapVariants[0].maps == std::vector<Data::Maps::MapRef>{{WD_9DEVILSQUARE}, {32}});
    REQUIRE(golem.mapVariants[0].attacks);
    CHECK(golem.mapVariants[0].attacks->otherwiseGeneric);

    const std::string text = Data::Monsters::WriteMonstersJson(result.monsters);
    CHECK(text.find(R"("maps": [9, 32])") != std::string::npos);
    std::vector<MonsterDefinition> read;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", read, issues);
    CHECK(issues.empty());
    REQUIRE(read.size() == 1);
    CHECK(read[0] == golem);
}

TEST_CASE("Monster attacks: a stage in a map variant's maps reads and writes as {map, stage} (D67) [data][monsters]")
{
    const ReadResult result = ReadMonsterWith(R"("attacks": { "otherwise": "attack2" }, )"
                                              R"("mapVariants": [ { "maps": [32, {"stage": 2, "map": 9}], )"
                                              R"("attacks": { "otherwise": "generic" } } ])");
    CHECK(result.issues.empty());
    REQUIRE(result.monsters.size() == 1);
    const MonsterDefinition& golem = result.monsters[0];
    REQUIRE(golem.mapVariants.size() == 1);
    CHECK(golem.mapVariants[0].maps == std::vector<Data::Maps::MapRef>{{WD_9DEVILSQUARE, 2}, {32}});

    const std::string text = Data::Monsters::WriteMonstersJson(result.monsters);
    INFO(text);
    CHECK(text.find(R"("maps": [{"map": 9, "stage": 2}, 32])") != std::string::npos);
    std::vector<MonsterDefinition> read;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", read, issues);
    CHECK(issues.empty());
    REQUIRE(read.size() == 1);
    CHECK(read[0] == golem);
}

TEST_CASE("Monster attacks: wrong map variants are fatal, one without attacks a warning [data][monsters]")
{
    struct WrongValue
    {
        const char* variants;
        const char* field;
        const char* message;
    };
    for (const WrongValue& wrong : std::vector<WrongValue>{
             {R"({})", "monster 291 mapVariants", "must be a list of map variants"},
             {R"([3])", "monster 291 mapVariants[0]", "must be an object"},
             {R"([{ "attacks": {} }])", "monster 291 mapVariants[0].maps", "one or more maps"},
             {R"([{ "maps": [], "attacks": {} }])", "monster 291 mapVariants[0].maps", "one or more maps"},
             {R"([{ "maps": 9, "attacks": {} }])", "monster 291 mapVariants[0].maps", "a list of maps"},
             {R"([{ "maps": [256], "attacks": {} }])", "monster 291 mapVariants[0].maps[0]", "from 0 to 255"},
             {R"([{ "maps": [{"map": 9}], "attacks": {} }])", "monster 291 mapVariants[0].maps[0]",
              "\"stage\": 1 to 255"},
             {R"([{ "maps": [9, {"map": 9, "stage": 0}], "attacks": {} }])", "monster 291 mapVariants[0].maps[1]",
              "\"stage\": 1 to 255"},
             {R"([{ "maps": [{"map": 9, "stage": 2, "round": 1}], "attacks": {} }])",
              "monster 291 mapVariants[0].maps[0]", "\"stage\": 1 to 255"},
             {R"([{ "maps": [32], "attacks": {} }, { "maps": [9, 32], "attacks": {} }])",
              "monster 291 mapVariants[1].maps", "names map 32, which an earlier map variant names (map 32)"},
             {R"([{ "maps": [9], "attacks": {} }, { "maps": [{"map": 9, "stage": 2}], "attacks": {} }])",
              "monster 291 mapVariants[1].maps", "names map 9 stage 2, which an earlier map variant names (map 9)"},
             {R"([{ "maps": [{"map": 9, "stage": 2}], "attacks": {} }, { "maps": [9], "attacks": {} }])",
              "monster 291 mapVariants[1].maps", "names map 9, which an earlier map variant names (map 9 stage 2)"},
             {R"([{ "maps": [32], "attacks": { "otherwise": "jump" } }])",
              "monster 291 mapVariants[0].attacks.otherwise", "must be an animation"},
         })
    {
        INFO(wrong.variants);
        const ReadResult result = ReadMonsterWith(std::string(R"("mapVariants": )") + wrong.variants);
        CHECK(HasFatal(result.issues));
        CHECK(HasIssueAbout(result.issues, wrong.field, wrong.message));
        CHECK(result.monsters.empty());
    }

    const ReadResult nothing = ReadMonsterWith(R"("mapVariants": [ { "maps": [32] } ])");
    CHECK_FALSE(HasFatal(nothing.issues));
    CHECK(HasIssueAbout(nothing.issues, "monster 291 mapVariants[0]", "changes nothing"));
    REQUIRE(nothing.monsters.size() == 1);
    CHECK(nothing.monsters[0].mapVariants.size() == 1);
}
