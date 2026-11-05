#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "UI/MonsterTools/MonsterAttackChecks.h"
#include "UI/MonsterTools/MonsterCheckTargets.h"
#include "World/MapInfra/MapManager.h"

#include <array>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterAttackRandom;
using Data::Monsters::MonsterAttacks;
using Data::Monsters::MonsterDefinition;
using MuEditor::Monsters::GetToolForField;
using MuEditor::Monsters::MonsterTool;
namespace AttackChecks = MuEditor::Monsters::AttackChecks;

namespace
{
const std::filesystem::path ClientData = MU_TEST_DATA_DIR;

MonsterDefinition Monster(int number, const char* model, MonsterAttacks attacks)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", "Monster");
    monster.model = model;
    monster.attacks = std::move(attacks);
    return monster;
}

MonsterAttacks Attacks(std::vector<Data::Monsters::MonsterAttackEntry> monsterSkills, int otherwise)
{
    MonsterAttacks attacks;
    attacks.monsterSkills = std::move(monsterSkills);
    attacks.otherwise = {otherwise};
    return attacks;
}

// The test binary opens no models.
std::optional<int> NoneOpen(int)
{
    return std::nullopt;
}

std::vector<ItemDataIssue> Check(std::span<const MonsterDefinition> monsters,
                                 const AttackChecks::OpenAnimationCount& open = NoneOpen)
{
    g_MonsterDatabase.Build(monsters, "en");
    std::vector<ItemDataIssue> issues;
    AttackChecks::Validate(issues, open);
    g_MonsterDatabase.Build({}, "en");
    return issues;
}

std::vector<std::string> Messages(const std::vector<ItemDataIssue>& issues, int number)
{
    std::vector<std::string> messages;
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.number == number)
        {
            messages.push_back(issue.message);
        }
    }
    return messages;
}

bool HasMessage(const std::vector<ItemDataIssue>& issues, int number, const std::string& part)
{
    for (const std::string& message : Messages(issues, number))
    {
        if (message.find(part) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}
} // namespace

TEST_CASE("Monster attack checks: the attacks of a player-shaped monster; any other plays them on every map "
          "(Q-MN64) [editor][monsters]")
{
    const std::array monsters = {
        // Attacks play on every map: no handler has to answer for the Budge Dragon.
        Monster(MONSTER_BUDGE_DRAGON, "MONSTER_MODEL_BUDGE_DRAGON", Attacks({}, MONSTER01_ATTACK1)),
        Monster(MONSTER_DEATH_CENTURION_1, "player", Attacks({}, MONSTER01_ATTACK1)),
        // Ice Walker's own attacks: nothing to say.
        Monster(MONSTER_ICE_WALKER, "MONSTER_MODEL_ICE_WALKER", Attacks({{29, MONSTER01_ATTACK2}}, MONSTER01_ATTACK1)),
        // A monster without attacks plays the generic pattern: not checked.
        MonsterDefinition{},
    };

    const std::vector<ItemDataIssue> issues = Check(monsters);

    REQUIRE(issues.size() == 1);
    CHECK(Messages(issues, MONSTER_BUDGE_DRAGON).empty());
    CHECK(HasMessage(issues, MONSTER_DEATH_CENTURION_1, "the player's shape"));
    for (const ItemDataIssue& issue : issues)
    {
        CHECK(issue.severity == ItemDataIssueSeverity::Warning);
        CHECK(issue.field == "monster " + std::to_string(issue.number) + " attacks");
        CHECK(GetToolForField(issue.field) == MonsterTool::Attacks);
        CHECK(issue.source == Data::Monsters::GetMonstersFile().generic_string());
    }
}

TEST_CASE("Monster attack checks: monster skills that packet 0x69 plays no attack for [editor][monsters]")
{
    const std::array monsters = {Monster(
        MONSTER_ICE_WALKER, "MONSTER_MODEL_ICE_WALKER",
        Attacks({{0, MONSTER01_ATTACK2}, {77, MONSTER01_ATTACK2}, {78, MONSTER01_ATTACK2}}, MONSTER01_ATTACK1))};

    const std::vector<ItemDataIssue> issues = Check(monsters);

    CHECK(
        Messages(issues, MONSTER_ICE_WALKER) ==
        std::vector<std::string>{"monster skill 0: packet 0x69 plays no attack for it (only for the skills 1 to 77)",
                                 "monster skill 78: packet 0x69 plays no attack for it (only for the skills 1 to 77)"});
}

// Selupan and the Empire Guardian bosses show it today (monsters design,
// section 6): appear, attack 3 and attack 4.
TEST_CASE("Monster attack checks: animations other than attack 1 and 2, and appear without a way back to stop 1 "
          "[editor][monsters]")
{
    const std::array monsters = {
        Monster(MONSTER_SELUPAN, "MONSTER_MODEL_SELUPAN",
                Attacks({{36, MONSTER01_ATTACK3},
                         {37, MONSTER01_APEAR},
                         {38, MONSTER01_ATTACK3},
                         {41, Data::Monsters::NoAttackAnimation}},
                        MONSTER01_ATTACK1)),
        Monster(MONSTER_RAYMOND, "MONSTER_MODEL_RAYMOND", Attacks({{59, MONSTER01_APEAR}}, MONSTER01_ATTACK1)),
    };

    const std::vector<ItemDataIssue> issues = Check(monsters);

    // Selupan goes back to stop 1 after appear.
    CHECK(Messages(issues, MONSTER_SELUPAN) ==
          std::vector<std::string>{
              "appear (7) is played by monster skill 37: the game counts only attack1 and attack2 as attacks; a hit "
              "plays shock over any other animation, and only they have an impact frame",
              "attack3 (8) is played by monster skills 36, 38: the game counts only attack1 and attack2 as attacks; "
              "a hit plays shock over any other animation, and only they have an impact frame"});
    CHECK(HasMessage(issues, MONSTER_RAYMOND,
                     "appear (7) is played by monster skill 59: the model does not go back "
                     "to stop1 after it"));
    CHECK(HasMessage(issues, MONSTER_RAYMOND, "appear (7) is played by monster skill 59: the game counts only"));
    CHECK(Messages(issues, MONSTER_RAYMOND).size() == 2);
}

TEST_CASE("Monster attack checks: the draw's animations, an animation the open model lacks and one at speed 0 "
          "[editor][monsters]")
{
    MonsterAttacks bali;
    bali.otherwiseRandom = MonsterAttackRandom::Bali;
    MonsterAttacks iceWalker = Attacks({{29, MONSTER01_ATTACK2}, {30, MONSTER01_ATTACK3}}, MONSTER01_ATTACK1);
    iceWalker.skills = {{12, MONSTER01_ATTACK2}};
    const std::array monsters = {Monster(MONSTER_BALI, "MONSTER_MODEL_BALI", bali),
                                 Monster(MONSTER_ICE_WALKER, "MONSTER_MODEL_ICE_WALKER", iceWalker)};
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_ICE_WALKER";
    model.load.file = "Data/Monster/Monster146.bmd";
    model.load.speeds = {{MONSTER01_ATTACK2, 0.0f}};
    g_MonsterModelDatabase.Build(std::array{model});
    const int iceWalkerSlot = *Data::Monsters::FindMonsterModelReference("MONSTER_MODEL_ICE_WALKER");
    const auto open = [&](int slot) -> std::optional<int>
    {
        // The Ice Walker's model is open with animations 0 to 7.
        return slot == iceWalkerSlot ? std::optional<int>(MONSTER01_ATTACK3) : std::nullopt;
    };

    const std::vector<ItemDataIssue> issues = Check(monsters, open);
    g_MonsterModelDatabase.Build({});

    CHECK(HasMessage(issues, MONSTER_BALI, "attack3 (8) is played by the draw: the game counts only"));
    CHECK(HasMessage(issues, MONSTER_BALI, "attack4 (9) is played by the draw: the game counts only"));
    CHECK(Messages(issues, MONSTER_BALI).size() == 2);
    CHECK(Messages(issues, MONSTER_ICE_WALKER) ==
          std::vector<std::string>{
              "attack2 (4) is played by monster skill 29 and player skill 12, whose speed in MONSTER_MODEL_ICE_WALKER "
              "is 0: the monster stands still in it until its next action",
              "attack3 (8) is played by monster skill 30, which MONSTER_MODEL_ICE_WALKER does not have: the "
              "animation that plays goes on"});
}

TEST_CASE("Monster attack checks: each animation of a rotation, and the generic pattern's speed (Q-MN69) "
          "[editor][monsters]")
{
    MonsterAttacks rotation;
    rotation.otherwise = {MONSTER01_ATTACK1, MONSTER01_ATTACK3};
    MonsterAttacks generic;
    generic.otherwiseGeneric = true;
    const std::array monsters = {Monster(MONSTER_ICE_WALKER, "MONSTER_MODEL_ICE_WALKER", rotation),
                                 Monster(MONSTER_ICE_GIANT, "MONSTER_MODEL_ICE_WALKER", generic)};
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_ICE_WALKER";
    model.load.file = "Data/Monster/Monster146.bmd";
    model.load.speeds = {{MONSTER01_ATTACK2, 0.0f}, {MONSTER01_ATTACK3, 0.25f}};
    g_MonsterModelDatabase.Build(std::array{model});

    const std::vector<ItemDataIssue> issues = Check(monsters);
    g_MonsterModelDatabase.Build({});

    CHECK(Messages(issues, MONSTER_ICE_WALKER).size() == 1);
    CHECK(HasMessage(issues, MONSTER_ICE_WALKER, "attack3 (8) is played by any other attack: the game counts only"));
    CHECK(Messages(issues, MONSTER_ICE_GIANT).size() == 1);
    CHECK(HasMessage(issues, MONSTER_ICE_GIANT, "attack2 (4) is played by the generic pattern, whose speed"));
}

TEST_CASE("Monster attack checks: a map variant's attacks, under the variant's field (Q-MN65) [editor][monsters]")
{
    MonsterDefinition iceWalker =
        Monster(MONSTER_ICE_WALKER, "MONSTER_MODEL_ICE_WALKER", Attacks({{29, MONSTER01_ATTACK2}}, MONSTER01_ATTACK1));
    Data::Monsters::MonsterMapVariant hatchery;
    hatchery.maps = {{WD_58ICECITY_BOSS}};
    hatchery.attacks = Attacks({{0, MONSTER01_ATTACK2}}, MONSTER01_ATTACK1);
    iceWalker.mapVariants = {hatchery};
    const std::array monsters = {iceWalker};

    const std::vector<ItemDataIssue> issues = Check(monsters);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].field == "monster 454 mapVariants[0].attacks");
    CHECK(issues[0].message.find("monster skill 0: packet 0x69 plays no attack") != std::string::npos);
    CHECK(GetToolForField(issues[0].field) == MonsterTool::Attacks);
}

// What Data Checks shows for the shipped lists while no model is open
// (docs/monster-data.md): the attacks that play other animations than attack 1
// and 2, as the converter took them from the code. Every animation they play
// has a speed (P20 gave the Empire Guardian bosses' appear one).
TEST_CASE("Monster attack checks: the shipped lists' attacks [editor][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult monsters =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");
    const Data::Monsters::MonsterModelsLoadResult models =
        Data::Monsters::LoadMonsterModelsFile(ClientData / "Monsters" / "Models.json");
    REQUIRE(monsters.monsters.size() > 0);
    REQUIRE(models.models.size() > 0);
    g_MonsterModelDatabase.Build(models.models);

    const std::vector<ItemDataIssue> issues = Check(monsters.monsters);
    g_MonsterModelDatabase.Build({});

    std::vector<std::string> stillAnimations;
    for (const ItemDataIssue& issue : issues)
    {
        INFO(issue.number << ": " << issue.message);
        CHECK(issue.message.find("packet 0x69") == std::string::npos);
        if (issue.message.find(" is 0:") != std::string::npos)
        {
            stillAnimations.push_back(std::to_string(issue.number) + " " +
                                      issue.message.substr(0, issue.message.find(" is played")));
        }
    }
    CHECK(stillAnimations.empty());
    CHECK(issues.size() == 48);
}

#endif // _EDITOR
