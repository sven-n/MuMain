#include "stdafx.h"

#include "doctest.h"

// The setup fields of the monster list (monsters design, D63, MN4): read and
// written flat beside a monster's other values, and as a number's own.

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterGroup.h"
#include "Data/GameData/MonsterData/MonsterSetup.h"
#include "Data/GameData/MonsterData/MonstersJson.h"

#include <algorithm>
#include <array>
#include <span>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterGroup;
using Data::Monsters::MonsterItem;
using Data::Monsters::MonsterSetup;
using Data::Monsters::MonsterSpawnEffect;

namespace
{
struct GroupsResult
{
    std::vector<MonsterGroup> monsters;
    std::vector<ItemDataIssue> issues;
};

GroupsResult ReadGroups(const std::string& text)
{
    GroupsResult result;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", result.monsters, result.issues);
    return result;
}

std::string File(const std::string& monster)
{
    return R"({ "formatVersion": 2, "monsters": [ )" + monster + " ] }";
}

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
        text += issue.ToString() + "\n";
    return text;
}

bool Has(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::ranges::any_of(issues, [&](const ItemDataIssue& issue)
                               { return issue.severity == severity && issue.field == field; });
}
} // namespace

TEST_CASE("Monster data: reads and writes the setup fields of a monster (D63) [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Magic Skeleton", "kind": "monster", "model": "MONSTER_MODEL_MAGIC_SKELETON",
        "scale": 1.2, "outline": 1.5, "subType": 1, "lookLevel": 9, "glowMesh": 1, "glowLight": 0.5,
        "hiddenMesh": -2, "shadow": false, "hideShadow": false, "alphaTarget": 0.4, "light": false,
        "pkColor": "murderer2", "helperPlate": true,
        "equipment": {"weapons": [null, {"group": 5, "number": 0, "level": 11}],
                      "helm": {"group": 7, "number": 2, "level": 13, "excellent": 1}, "wings": {"group": 12, "number": 0}},
        "selectable": false,
        "spawnEffects": [{"eyes": [16, 17]}, {"joint": "BITMAP_JOINT_ENERGY", "subType": 2, "scale": 30},
                         {"joint": "BITMAP_JOINT_ENERGY", "subType": 24, "scale": 10, "color": [1.5, 0.01, 0]},
                         {"effect": "MODEL_AURORA", "key": 120}, {"object": "MODEL_BC_BOX"}],
        "spawnBuffs": ["poison"], "spawnSound": "SOUND_BC_EROHIM_ENTER", "randomLook": [10, 11], "randomStart": true,
        "heightOffset": 140,
        "startAction": 0, "startFrame": 10, "deathEffect": 100, "live": false, "noTurnOnHit": true,
        "moveSpeed": 15, "noBloodPool": true,
        "numbers": [{"number": 89}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& setup = read.monsters[0].numbers[0].setup;
    CHECK(setup.scale == 1.2f);
    CHECK_FALSE(setup.scaleByCharacter);
    CHECK(setup.outline == 1.5f);
    CHECK(setup.subType == 1);
    CHECK(setup.lookLevel == 9);
    CHECK(setup.glowMesh == 1);
    CHECK(setup.glowLight == 0.5f);
    CHECK(setup.hiddenMesh == -2);
    CHECK(setup.shadow == false);
    CHECK(setup.hideShadow == false);
    CHECK(setup.alphaTarget == 0.4f);
    CHECK(setup.light == false);
    CHECK(setup.pkColor == Data::Monsters::FindPkColor("murderer2"));
    CHECK(setup.helperPlate == true);
    CHECK_FALSE(setup.equipment.weapons[0]);
    CHECK(setup.equipment.weapons[1] == MonsterItem{5, 0, 11, 0});
    CHECK(setup.equipment.helm == MonsterItem{7, 2, 13, 1});
    CHECK(setup.equipment.wings == MonsterItem{12, 0, 0, 0});
    CHECK_FALSE(setup.equipment.armor);
    CHECK(setup.selectable == false);
    REQUIRE(setup.spawnEffects.size() == 5);
    CHECK(setup.spawnEffects[0].kind == MonsterSpawnEffect::Kind::Eyes);
    CHECK(setup.spawnEffects[0].bones == std::vector{16, 17});
    CHECK(setup.spawnEffects[1] ==
          MonsterSpawnEffect{MonsterSpawnEffect::Kind::Joint, "BITMAP_JOINT_ENERGY", 2, 30.0f});
    CHECK(setup.spawnEffects[2].color == std::array<float, 3>{1.5f, 0.01f, 0.0f});
    CHECK(setup.spawnEffects[3].kind == MonsterSpawnEffect::Kind::Effect);
    CHECK(setup.spawnEffects[3].type == "MODEL_AURORA");
    CHECK(setup.spawnEffects[3].key == 120);
    CHECK(setup.spawnEffects[4] == MonsterSpawnEffect{MonsterSpawnEffect::Kind::Object, "MODEL_BC_BOX", 0, 0.0f});
    CHECK(setup.spawnBuffs == std::vector<std::string>{"poison"});
    CHECK(setup.spawnSound == "SOUND_BC_EROHIM_ENTER");
    CHECK(setup.randomLook == std::vector{10, 11});
    CHECK(setup.randomStart);
    CHECK(setup.heightOffset == 140.0f);
    CHECK(setup.startAction == 0);
    CHECK(setup.startFrame == 10.0f);
    CHECK(setup.deathEffect == 100);
    CHECK(setup.live == false);
    CHECK(setup.noTurnOnHit == true);
    CHECK(setup.moveSpeed == 15);
    CHECK(setup.noBloodPool == true);

    // Written back, it reads the same.
    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    CHECK(text.find("\"scale\": 1.2") != std::string::npos);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);
}

TEST_CASE("Monster data: a number's own setup values replace the monster's, null clears them [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Red Skeleton Knight", "kind": "monster", "scale": "character",
        "equipment": {"weapons": [{"group": 1, "number": 2, "level": 8}]}, "glowMesh": 1,
        "numbers": [{"number": 88}, {"number": 111, "equipment": {"weapons": [{"group": 1, "number": 2}]}},
                    {"number": 123, "glowMesh": null, "scale": 1.3}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const std::vector<Data::Monsters::MonsterDefinition>& numbers = read.monsters[0].numbers;
    REQUIRE(numbers.size() == 3);
    CHECK(numbers[0].setup.scaleByCharacter);
    CHECK_FALSE(numbers[0].setup.scale);
    CHECK(numbers[0].setup.equipment.weapons[0] == MonsterItem{1, 2, 8, 0});
    CHECK(numbers[1].setup.equipment.weapons[0] == MonsterItem{1, 2, 0, 0});
    CHECK(numbers[1].setup.glowMesh == 1);
    CHECK_FALSE(numbers[2].setup.glowMesh);
    CHECK(numbers[2].setup.scale == 1.3f);
    CHECK_FALSE(numbers[2].setup.scaleByCharacter);

    // The file writes only what each number has differently.
    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    CHECK(text.find("{\"number\": 88}") != std::string::npos);
    CHECK(text.find("\"glowMesh\": null") != std::string::npos);
    CHECK(ReadGroups(text).monsters == read.monsters);

    // Each setup field is a value a number may have as its own (Q-MN50).
    const auto scale = Data::Monsters::GetSetupValueField(0);
    CHECK_FALSE(Data::Monsters::HasSameValue(numbers[0], numbers[2], scale));
    CHECK(std::ranges::find(Data::Monsters::GetMonsterValueFields(), scale) !=
          Data::Monsters::GetMonsterValueFields().end());
}

// The Cursed Wizard has a player's size, and 1.0 in Devil Square (Q-MN35): a
// map variant's setup fields replace the monster's one by one on its maps.
TEST_CASE("Monster data: a map variant changes the model and setup fields on its maps (Q-MN35) [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Cursed Wizard", "kind": "monster", "model": "player", "scale": "character", "pkColor": "murderer2",
        "mapVariants": [{"maps": [9], "scale": 1.0}, {"maps": [32], "pkColor": "hero1"}],
        "numbers": [{"number": 34}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const Data::Monsters::MonsterDefinition& entry = read.monsters[0].numbers[0];
    REQUIRE(entry.mapVariants.size() == 2);
    CHECK(entry.mapVariants[0].setup.scale == 1.0f);

    const MonsterSetup lostTower = Data::Monsters::GetSetupOnMap(entry, 4, 0);
    CHECK(lostTower.scaleByCharacter);
    CHECK_FALSE(lostTower.scale);
    const MonsterSetup devilSquare = Data::Monsters::GetSetupOnMap(entry, 9, 2);
    CHECK_FALSE(devilSquare.scaleByCharacter);
    CHECK(devilSquare.scale == 1.0f);
    // The fields the variant does not name stay the monster's.
    CHECK(devilSquare.pkColor == Data::Monsters::FindPkColor("murderer2"));
    CHECK(Data::Monsters::GetSetupOnMap(entry, 32, 5).pkColor == Data::Monsters::FindPkColor("hero1"));
    CHECK(Data::Monsters::GetSetupOnMap(entry, 32, 5).scaleByCharacter);

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    // A variant changes fields and clears none.
    const GroupsResult cleared = ReadGroups(File(R"({
        "name": "Cursed Wizard", "kind": "monster", "scale": "character",
        "mapVariants": [{"maps": [9], "scale": null}], "numbers": [{"number": 34}] })"));
    CHECK(Has(cleared.issues, ItemDataIssueSeverity::Fatal, "monster 34 mapVariants[0].scale"));
    CHECK(cleared.monsters.empty());

    const GroupsResult wrong = ReadGroups(File(R"({
        "name": "Cursed Wizard", "kind": "monster",
        "mapVariants": [{"maps": [9], "glowMesh": "one"}], "numbers": [{"number": 34}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 34 mapVariants[0].glowMesh"));
    CHECK(wrong.monsters.empty());
    // Balram has another body on Crywolf 2nd: a variant may name its model.
    const GroupsResult balram = ReadGroups(File(R"({
        "name": "Balram", "kind": "monster", "model": "MONSTER_MODEL_BALRAM", "scale": 1.25,
        "mapVariants": [{"maps": [35], "model": "MONSTER_MODEL_VALAM", "scale": 1.2}], "numbers": [{"number": 316}] })"));
    CHECK_MESSAGE(balram.issues.empty(), Messages(balram.issues));
    REQUIRE(balram.monsters.size() == 1);
    const Data::Monsters::MonsterDefinition& balramEntry = balram.monsters[0].numbers[0];
    CHECK(Data::Monsters::GetModelOnMap(balramEntry, 35, 0) == "MONSTER_MODEL_VALAM");
    CHECK(Data::Monsters::GetModelOnMap(balramEntry, 34, 0) == "MONSTER_MODEL_BALRAM");
    CHECK(Data::Monsters::GetSetupOnMap(balramEntry, 35, 0).scale == 1.2f);
    const std::string balramText = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(balram.monsters));
    INFO(balramText);
    CHECK(ReadGroups(balramText).monsters == balram.monsters);

    const GroupsResult noModel = ReadGroups(File(R"({
        "name": "Balram", "kind": "monster",
        "mapVariants": [{"maps": [35], "model": "MONSTER_MODEL_NOTHING"}], "numbers": [{"number": 316}] })"));
    CHECK(Has(noModel.issues, ItemDataIssueSeverity::Fatal, "monster 316 mapVariants[0].model"));
    CHECK(noModel.monsters.empty());
}

// The crown switches sink by 100 while no siege runs and keep the ground
// height for the map's code; Selupan waits at 1000 (Q-MN36c).
TEST_CASE("Monster data: state variants and the height fields (Q-MN36c) [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Crown Switch", "kind": "npc", "keepGroundHeight": true, "positionOffset": [-20, -200],
        "stateVariants": [{"state": "castleSiegeNotRunning", "heightOffset": -100},
                          {"state": "raklionBossWaiting", "height": 1000, "scale": 2}],
        "numbers": [{"number": 217}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& setup = read.monsters[0].numbers[0].setup;
    CHECK(setup.keepGroundHeight == true);
    CHECK(setup.positionOffset == std::array<float, 2>{-20.0f, -200.0f});
    REQUIRE(setup.stateVariants.size() == 2);
    CHECK(setup.stateVariants[0].state == "castleSiegeNotRunning");
    CHECK(setup.stateVariants[0].setup.heightOffset == -100.0f);
    CHECK(setup.stateVariants[1].setup.height == 1000.0f);
    CHECK(setup.stateVariants[1].setup.scale == 2.0f);

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    const GroupsResult unknown = ReadGroups(File(R"({ "name": "X", "kind": "npc",
        "stateVariants": [{"state": "rainy", "scale": 2}], "numbers": [{"number": 217}] })"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 217 stateVariants[0].state"));

    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "npc", "positionOffset": [1],
        "keepGroundHeight": 1, "height": "high",
        "stateVariants": [{"state": "castleSiegeRunning", "stateVariants": []},
                          {"state": "castleSiegeRunning", "scale": null}, {"scale": 2}],
        "numbers": [{"number": 217}] })"));
    CHECK(HasFatal(wrong.issues));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 217 positionOffset"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 217 keepGroundHeight"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 217 height"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 217 stateVariants[0].stateVariants"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 217 stateVariants[1].scale"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 217 stateVariants[2]"));
    CHECK(wrong.monsters.empty());
}

// The hooks hand over to the maps' own code with the values it fixed
// (Q-MN36d); a joint carries its key like an effect.
// An NPC's talk (MN5, D66): whether it talks, its minimum level and its
// message, its repair shop and its talk's steps, with texts by language.
TEST_CASE("Monster data: reads and writes the talk fields of an NPC (D66) [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Gatekeeper", "kind": "npc",
        "talks": true, "repairShop": true,
        "talkLevel": {"level": 10, "message": {"en": "Only level %d or above", "de": "Erst ab Stufe %d"}},
        "onTalk": [
          {"send": "talk"},
          {"dialog": {"title": "Gatekeeper", "lines": ["Who are you?", {"en": "Line two", "de": "Zeile zwei"}],
                      "ok": {"request": "enterRefuge", "if": {"quest": "thirdChangeUp2", "is": ["ongoing", "done"]},
                             "zen": 3000000, "failure": "You are short of Zen."}}},
          {"send": "petInfo"},
          {"message": "Health has been recovered."},
          {"questWindow": {"noQuestTitleFor": ["darkLord", "magicGladiator"]}}
        ],
        "numbers": [{"number": 408}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& setup = read.monsters[0].numbers[0].setup;
    CHECK(setup.talks == true);
    CHECK(setup.repairShop == true);
    REQUIRE(setup.talkLevel.has_value());
    CHECK(setup.talkLevel->level == 10);
    CHECK(setup.talkLevel->message.Get("de") == "Erst ab Stufe %d");
    using Kind = Data::Monsters::MonsterTalkBlock::Kind;
    REQUIRE(setup.onTalk.size() == 5);
    CHECK(setup.onTalk[0].kind == Kind::SendTalk);
    CHECK(setup.onTalk[1].kind == Kind::Dialog);
    CHECK(setup.onTalk[1].dialog.title.GetNeutral() == "Gatekeeper");
    REQUIRE(setup.onTalk[1].dialog.lines.size() == 2);
    REQUIRE(setup.onTalk[1].dialog.ok.has_value());
    CHECK(setup.onTalk[1].dialog.ok->request == "enterRefuge");
    CHECK(setup.onTalk[1].dialog.ok->quest == "thirdChangeUp2");
    CHECK(setup.onTalk[1].dialog.ok->questStates == std::vector<std::string>{"ongoing", "done"});
    CHECK(setup.onTalk[1].dialog.ok->zen == 3000000);
    CHECK(setup.onTalk[2].kind == Kind::SendPetInfo);
    CHECK(setup.onTalk[3].kind == Kind::Message);
    CHECK(setup.onTalk[4].kind == Kind::QuestWindow);
    CHECK(setup.onTalk[4].noQuestTitleFor.size() == 2);

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    // Names the client does not know are fatal.
    const GroupsResult wrong = ReadGroups(File(R"({
        "name": "X", "kind": "npc",
        "onTalk": [{"send": "shop"}, {"dialog": {"lines": [], "ok": {"request": "fly"}}},
                   {"questWindow": {"noQuestTitleFor": ["bard"]}}, {"wave": 1}],
        "numbers": [{"number": 538}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 onTalk[0].send"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 onTalk[1].dialog.ok.request"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 onTalk[2].questWindow.noQuestTitleFor[0]"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 onTalk[3]"));

    // A quest without states: the OK never sends, a warning.
    const GroupsResult noStates = ReadGroups(File(R"({
        "name": "X", "kind": "npc",
        "onTalk": [{"dialog": {"lines": [], "ok": {"request": "enterRefuge", "if": {"quest": "changeUp1", "is": []}}}}],
        "numbers": [{"number": 538}] })"));
    CHECK(Has(noStates.issues, ItemDataIssueSeverity::Warning, "monster 538 onTalk[0].dialog.ok.if.is"));
}

// A Crywolf altar's contract step names its six texts in the list's
// talkTexts, which the five altars share (Q-MN46); a name the list does not
// have and a set missing a text are fatal.
TEST_CASE("Monster data: a Crywolf altar's contract step and its shared texts [data][monsters]")
{
    const std::string talkTexts = R"("talkTexts": {"crywolfAltar": {"canContract": "Left: %d", "proceed": "Go?",
        "onMount": "No mount.", "needGuardian": "Guard!", "destroyed": {"en": "Gone.", "de": "Weg."},
        "ongoing": "Busy."}},)";
    const std::string altar = R"({"name": "Wolf Altar1", "kind": "npc",
        "onTalk": [{"crywolfAltarContract": "crywolfAltar"}], "numbers": [{"number": 205}]})";
    const std::string file = R"({ "formatVersion": 2, )" + talkTexts + R"( "monsters": [ )" + altar + " ] }";
    std::vector<MonsterGroup> groups;
    Data::Monsters::UnknownMonsterLook unknown;
    Data::Monsters::MonsterTalkTexts texts;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonstersJson(file, "Monsters.json", groups, unknown, texts, issues);
    CHECK_MESSAGE(issues.empty(), Messages(issues));
    REQUIRE(groups.size() == 1);
    const MonsterSetup& setup = groups[0].numbers[0].setup;
    REQUIRE(setup.onTalk.size() == 1);
    CHECK(setup.onTalk[0].kind == Data::Monsters::MonsterTalkBlock::Kind::CrywolfAltarContract);
    CHECK(setup.onTalk[0].altarTexts == "crywolfAltar");
    REQUIRE(texts.contains("crywolfAltar"));
    CHECK(texts.at("crywolfAltar").canContract.GetNeutral() == "Left: %d");
    CHECK(texts.at("crywolfAltar").destroyed.Get("de") == "Weg.");

    // Written and read again, the same.
    const std::string written = Data::Monsters::WriteMonstersJson(groups, unknown, texts);
    INFO(written);
    std::vector<MonsterGroup> again;
    Data::Monsters::MonsterTalkTexts againTexts;
    std::vector<ItemDataIssue> againIssues;
    Data::Monsters::ReadMonstersJson(written, "Monsters.json", again, unknown, againTexts, againIssues);
    CHECK_MESSAGE(againIssues.empty(), Messages(againIssues));
    CHECK(again == groups);
    CHECK(againTexts == texts);

    // A name the list does not have.
    const GroupsResult unnamed = ReadGroups(File(altar));
    CHECK(Has(unnamed.issues, ItemDataIssueSeverity::Fatal, "monster 205 onTalk[0].crywolfAltarContract"));
    // A set without one of its texts.
    const GroupsResult missing = ReadGroups(R"({ "formatVersion": 2,
        "talkTexts": {"crywolfAltar": {"canContract": "Left: %d"}}, "monsters": [ )" +
                                            altar + " ] }");
    CHECK(Has(missing.issues, ItemDataIssueSeverity::Fatal, "talkTexts.crywolfAltar.ongoing"));
    // That one problem only: the step still finds its set.
    CHECK_FALSE(Has(missing.issues, ItemDataIssueSeverity::Fatal, "monster 205 onTalk[0].crywolfAltarContract"));
    // The same for a set that is no object.
    const GroupsResult wrongSet = ReadGroups(R"({ "formatVersion": 2, "talkTexts": {"crywolfAltar": "oops"},
        "monsters": [ )" + altar + " ] }");
    CHECK(Has(wrongSet.issues, ItemDataIssueSeverity::Fatal, "talkTexts.crywolfAltar"));
    CHECK_FALSE(Has(wrongSet.issues, ItemDataIssueSeverity::Fatal, "monster 205 onTalk[0].crywolfAltarContract"));
}

TEST_CASE("Monster data: spawn hooks with their values, and a joint's key [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Daesuler", "kind": "monster",
        "spawnEffects": [{"joint": "BITMAP_JOINT_ENERGY", "subType": 54, "scale": 100, "key": 2}],
        "hooks": [{"hook": "EmpireGuardianRageReset", "boss": "Daesuler"}, {"hook": "RaklionBossDrop"}],
        "numbers": [{"number": 508}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& setup = read.monsters[0].numbers[0].setup;
    REQUIRE(setup.spawnEffects.size() == 1);
    CHECK(setup.spawnEffects[0].key == 2);
    REQUIRE(setup.hooks.size() == 2);
    CHECK(setup.hooks[0].name == "EmpireGuardianRageReset");
    CHECK(setup.hooks[0].Value("boss") == "Daesuler");
    CHECK(setup.hooks[1].values.empty());

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    const GroupsResult unknown = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "hooks": [{"hook": "Fireworks"}, {"hook": "IllusionTempleStorageReset", "storage": "cellar"}],
        "numbers": [{"number": 508}] })"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 508 hooks[0].hook"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 508 hooks[1].storage"));

    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "hooks": [{"hook": "EmpireGuardianRageReset"}, "RaklionBossDrop"], "numbers": [{"number": 508}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 508 hooks[0].boss"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 508 hooks[1]"));
    CHECK(wrong.monsters.empty());
}

// Player-shaped monsters (MN4c): a class and its step, bodies that are no
// items, and the Dark Horse's look (Q-MN57).
TEST_CASE("Monster data: a player-shaped monster's class, class step, bodies and horse look [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Doppelganger", "kind": "monster", "model": "player", "class": "darkLord", "classStep": 1,
        "equipment": {"helm": {"part": "MODEL_BODY_HELM+15"}, "armor": {"part": "MODEL_MERCHANT_MAN_UPPER+1"},
                      "helper": {"group": 13, "number": 4, "mountLook": "doppelganger"}},
        "numbers": [{"number": 538}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& setup = read.monsters[0].numbers[0].setup;
    CHECK(setup.playerClass == CLASS_DARK_LORD);
    CHECK(setup.classStep == 1);
    REQUIRE(setup.equipment.helm.has_value());
    CHECK(setup.equipment.helm->part == "MODEL_BODY_HELM+15");
    CHECK(Data::Monsters::FindMonsterBodyPartSlot("MODEL_BODY_HELM+15") == MODEL_BODY_HELM + 15);
    CHECK(Data::Monsters::GetMonsterBodyPartSymbol(MODEL_BODY_HELM + 15) == "MODEL_BODY_HELM+15");
    REQUIRE(setup.equipment.helper.has_value());
    CHECK(setup.equipment.helper->mountLook == "doppelganger");
    // The helper is the Dark Horse; a body in its place is none.
    CHECK(Data::Monsters::IsDarkHorse(*setup.equipment.helper));
    CHECK_FALSE(Data::Monsters::IsDarkHorse(*setup.equipment.armor));
    Data::Monsters::MonsterItem body = *setup.equipment.helper;
    body.part = "MODEL_MERCHANT_MAN_UPPER+1";
    CHECK_FALSE(Data::Monsters::IsDarkHorse(body));

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    const GroupsResult unknown = ReadGroups(File(R"({ "name": "X", "kind": "monster", "class": "bard",
        "equipment": {"helm": {"part": "MODEL_NOTHING"}, "helper": {"group": 13, "number": 4, "mountLook": "zebra"}},
        "numbers": [{"number": 538}] })"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 538 class"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 538 equipment.helm.part"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 538 equipment.helper.mountLook"));

    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "monster", "class": 4, "classStep": 3,
        "moveSpeed": -1, "lookLevel": 70000,
        "equipment": {"helm": {"part": 15}}, "numbers": [{"number": 538}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 class"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 classStep"));
    // The message names the reader's range.
    const auto classStep = std::ranges::find_if(wrong.issues, [](const ItemDataIssue& issue)
                                                { return issue.field == "monster 538 classStep"; });
    REQUIRE(classStep != wrong.issues.end());
    CHECK(classStep->message.find("to " + std::to_string(Data::Monsters::MaxClassStep)) != std::string::npos);
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 equipment.helm.part"));
    // The character's fields are 16-bit: a move speed or look level past them
    // would wrap.
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 moveSpeed"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 538 lookLevel"));
    CHECK(wrong.monsters.empty());
}

// How a monster the server sends faces (Q-MN38): fixed, by its step, by tile.
TEST_CASE("Monster data: the facing fields (Q-MN38) [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Ballista", "kind": "monster", "facingStep": 30,
        "facingByTile": [{"tileY": 90, "facingStep": 0}, {"tileX": 62, "facingStep": -135}],
        "spawnEffects": [{"object": "MODEL_BC_BOX", "angle": 90}],
        "numbers": [{"number": 348}, {"number": 465, "facing": 20}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& ballista = read.monsters[0].numbers[0].setup;
    CHECK(ballista.facingStep == 30.0f);
    REQUIRE(ballista.facingByTile.size() == 2);
    CHECK(ballista.facingByTile[0].tileY == 90);
    CHECK_FALSE(ballista.facingByTile[0].tileX);
    CHECK(ballista.facingByTile[1].step == -135.0f);
    CHECK(ballista.spawnEffects[0].angle == 90.0f);
    CHECK(read.monsters[0].numbers[1].setup.facing == 20.0f);

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "monster", "facing": "north",
        "facingByTile": [{"tileX": 62, "tileY": 90, "facingStep": 0}, {"tileX": 300, "facingStep": 0}, {"tileX": 1}],
        "numbers": [{"number": 348}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 348 facing"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 348 facingByTile[0]"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 348 facingByTile[1].tileX"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 348 facingByTile[2]"));
    CHECK(wrong.monsters.empty());
}

// The Dark Elf 412's crater, blue blurs and red shock waves at its bone 27
// (Q-MN38c): particles, counts, lights and bones of spawn effects.
TEST_CASE("Monster data: spawn effects at a bone, particles, counts and lights [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Dark Elf", "kind": "monster",
        "spawnEffects": [{"effect": "BITMAP_CRATER", "subType": 2, "bone": 27},
                         {"particle": "BITMAP_BLUE_BLUR", "subType": 1, "scale": 2, "color": [1, 0.2, 0.2],
                          "bone": 27, "count": 2},
                         {"effect": "BITMAP_SHOCK_WAVE", "subType": 8, "color": [1, 0.2, 0.2], "count": 5},
                         {"joint": "BITMAP_JOINT_ENERGY", "bone": 3}],
        "numbers": [{"number": 412}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const std::vector<MonsterSpawnEffect>& effects = read.monsters[0].numbers[0].setup.spawnEffects;
    REQUIRE(effects.size() == 4);
    CHECK(effects[0].kind == MonsterSpawnEffect::Kind::Effect);
    CHECK(effects[0].bone == 27);
    CHECK(effects[0].count == 1);
    CHECK_FALSE(effects[0].color);
    CHECK(effects[1].kind == MonsterSpawnEffect::Kind::Particle);
    CHECK(effects[1].type == "BITMAP_BLUE_BLUR");
    CHECK(effects[1].subType == 1);
    CHECK(effects[1].scale == 2.0f);
    CHECK(effects[1].color == std::array<float, 3>{1.0f, 0.2f, 0.2f});
    CHECK(effects[1].count == 2);
    CHECK(effects[2].color == std::array<float, 3>{1.0f, 0.2f, 0.2f});
    CHECK_FALSE(effects[2].bone);
    CHECK(effects[2].count == 5);
    CHECK(effects[3].bone == 3);

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "spawnEffects": [{"particle": "BITMAP_BLUE_BLUR", "bone": -1}, {"effect": "BITMAP_CRATER", "count": 0},
                         {"particle": "BITMAP_BLUE_BLUR", "effect": "BITMAP_CRATER"}],
        "numbers": [{"number": 412}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 412 spawnEffects[0].bone"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 412 spawnEffects[1].count"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 412 spawnEffects[2]"));

    const GroupsResult unknown = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "spawnEffects": [{"particle": "MODEL_AURORA"}], "numbers": [{"number": 412}] })"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 412 spawnEffects[0].particle"));
}

// How a monster comes in that the server has just spawned, or that teleported
// (Q-MN38c): Crywolf's thunder, the Maya hands' blended appear, the
// Nightmare's sparks.
TEST_CASE("Monster data: the appear and teleport fields (Q-MN38c) [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Balram", "kind": "monster",
        "appear": {"action": "appear", "blend": true, "startFrame": 0, "fadeIn": true, "appearTime": 60,
                   "sound": "SOUND_ASSASSIN",
                   "effects": [{"joint": "BITMAP_JOINT_THUNDER+1", "subType": 7, "scale": 50, "scaleRandom": 10,
                                "offset": [20, 20, 0], "count": 2},
                               {"joint": "BITMAP_JOINT_THUNDER", "subType": 17, "scale": 10,
                                "targetOffset": [0, 0, 120]},
                               {"particle": "BITMAP_SMOKE+4", "subType": 1, "scale": 5}]},
        "stateVariants": [{"state": "crywolfReady", "appear": {"fadeIn": false}}],
        "numbers": [{"number": 344},
                    {"number": 361, "appear": null,
                     "teleport": {"effects": [{"effect": "BITMAP_SPARK+1", "color": [1, 1, 1], "count": 2}],
                                  "sound": "SOUND_KANTURU_3RD_NIGHTMARE_TELE"}}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& balram = read.monsters[0].numbers[0].setup;
    REQUIRE(balram.appear);
    CHECK(balram.appear->action == "appear");
    CHECK(balram.appear->blend);
    CHECK(balram.appear->startFrame == 0.0f);
    CHECK(balram.appear->fadeIn == true);
    CHECK(balram.appear->appearTime == 60);
    CHECK(balram.appear->sound == "SOUND_ASSASSIN");
    REQUIRE(balram.appear->effects.size() == 3);
    CHECK(balram.appear->effects[0].scaleRandom == 10);
    CHECK(balram.appear->effects[0].offset == std::array<float, 3>{20.0f, 20.0f, 0.0f});
    CHECK(balram.appear->effects[0].count == 2);
    CHECK(balram.appear->effects[1].targetOffset == std::array<float, 3>{0.0f, 0.0f, 120.0f});
    CHECK(balram.appear->effects[2].kind == MonsterSpawnEffect::Kind::Particle);
    REQUIRE(balram.stateVariants.size() == 1);
    REQUIRE(balram.stateVariants[0].setup.appear);
    CHECK(balram.stateVariants[0].setup.appear->fadeIn == false);
    const MonsterSetup& nightmare = read.monsters[0].numbers[1].setup;
    CHECK_FALSE(nightmare.appear);
    REQUIRE(nightmare.teleport);
    CHECK(nightmare.teleport->effects[0].color == std::array<float, 3>{1.0f, 1.0f, 1.0f});
    CHECK(nightmare.teleport->sound == "SOUND_KANTURU_3RD_NIGHTMARE_TELE");

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "appear": {"action": 7, "fadeIn": "yes", "effects": [{"object": "MODEL_BC_BOX"},
                   {"joint": "BITMAP_JOINT_THUNDER", "offset": [1, 2]}]},
        "teleport": "sparks", "numbers": [{"number": 344}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 344 appear.action"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 344 appear.fadeIn"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 344 appear.effects[0]"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 344 appear.effects[1].offset"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 344 teleport"));

    const GroupsResult unknown = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "appear": {"action": "dance", "sound": "SOUND_NOTHING"}, "numbers": [{"number": 344}] })"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 344 appear.action"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 344 appear.sound"));
}

// A castle gate's animations and how an open one comes into view (Q-MN38c).
TEST_CASE("Monster data: the gate field [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Castle Gate", "kind": "monster",
        "gate": {"closed": "stop1", "opening": "stop2", "open": "walk", "arrival": "open"},
        "numbers": [{"number": 277}, {"number": 278, "gate": {}}] })"));
    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterSetup& gate = read.monsters[0].numbers[0].setup;
    REQUIRE(gate.gate);
    CHECK(gate.gate->open == "walk");
    CHECK_FALSE(gate.gate->arrivalOpening);
    // Values it leaves out are Castle Gate 1's.
    REQUIRE(read.monsters[0].numbers[1].setup.gate);
    CHECK(*read.monsters[0].numbers[1].setup.gate == Data::Monsters::MonsterGate{});

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "gate": {"open": 2, "arrival": "slowly"}, "numbers": [{"number": 277}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 277 gate.open"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 277 gate.arrival"));
    const GroupsResult unknown = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "gate": {"open": "swing"}, "numbers": [{"number": 277}] })"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 277 gate.open"));
}

// How the game shows a monster it cannot show as itself (D64): the section
// unknownMonsters, its defaults without it, and its problems.
TEST_CASE("Monster data: the unknown monsters' look [data][monsters]")
{
    std::vector<MonsterGroup> groups;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::UnknownMonsterLook look;
    Data::Monsters::ReadMonstersJson(File(R"({"name": "X", "kind": "monster", "numbers": [{"number": 7}]})"),
                                     "Monsters.json", groups, look, issues);
    CHECK_MESSAGE(issues.empty(), Messages(issues));
    CHECK(look == Data::Monsters::UnknownMonsterLook{});
    CHECK(look.model == "MONSTER_MODEL_GHOST");

    const std::string own = R"({ "formatVersion": 2,
        "unknownMonsters": {"model": "MONSTER_MODEL_HOUND", "scale": 1.5, "glow": [0, 1, 0], "marks": false,
                            "messageSeconds": 0, "for": ["noEntry"]},
        "monsters": [{"name": "X", "kind": "monster", "numbers": [{"number": 7}]}] })";
    groups.clear();
    Data::Monsters::ReadMonstersJson(own, "Monsters.json", groups, look, issues);
    CHECK_MESSAGE(issues.empty(), Messages(issues));
    CHECK(look.model == "MONSTER_MODEL_HOUND");
    CHECK(look.scale == 1.5f);
    CHECK(look.glow == std::array<float, 3>{0.0f, 1.0f, 0.0f});
    CHECK_FALSE(look.marks);
    CHECK(look.messageSeconds == 0);
    CHECK(look.forNoEntry);
    CHECK_FALSE(look.forNoModel);
    CHECK_FALSE(look.forMissingFile);

    // Written and read back, it is the same.
    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(groups), look, {});
    INFO(text);
    std::vector<MonsterGroup> again;
    Data::Monsters::UnknownMonsterLook readBack;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", again, readBack, issues);
    CHECK_MESSAGE(issues.empty(), Messages(issues));
    CHECK(readBack == look);

    std::vector<ItemDataIssue> wrong;
    Data::Monsters::UnknownMonsterLook kept;
    Data::Monsters::ReadMonstersJson(R"({ "formatVersion": 2,
        "unknownMonsters": {"model": "MONSTER_MODEL_NOTHING", "scale": 0, "glow": [1, 0], "messageSeconds": -1,
                            "for": ["nobody"], "colour": 1},
        "monsters": [] })",
                                     "Monsters.json", groups, kept, wrong);
    CHECK(Has(wrong, ItemDataIssueSeverity::Error, "unknownMonsters.model"));
    CHECK(Has(wrong, ItemDataIssueSeverity::Fatal, "unknownMonsters.scale"));
    CHECK(Has(wrong, ItemDataIssueSeverity::Fatal, "unknownMonsters.glow"));
    CHECK(Has(wrong, ItemDataIssueSeverity::Fatal, "unknownMonsters.messageSeconds"));
    CHECK(Has(wrong, ItemDataIssueSeverity::Error, "unknownMonsters.for"));
    CHECK(kept.model == "MONSTER_MODEL_GHOST");
}

// The Chaos Castle soldiers appear as an elf with a crossbow or a wizard with a
// staff (Q-MN83): each look names the equipment it wears instead.
TEST_CASE("Monster data: a random variant wears its own equipment (Q-MN83) [data][monsters]")
{
    const GroupsResult read = ReadGroups(File(R"({
        "name": "Chaos Castle 2", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF",
        "equipment": {"weapons": [{"group": 4, "number": 19}]},
        "randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD",
                            "equipment": {"weapons": [{"group": 5, "number": 5}]}}],
        "numbers": [{"number": 163}] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const Data::Monsters::MonsterDefinition& entry = read.monsters[0].numbers[0];
    CHECK(entry.setup.equipment.weapons[0] == MonsterItem{4, 19, 0, 0});
    REQUIRE(entry.randomVariants.size() == 1);
    REQUIRE(entry.randomVariants[0].equipment.has_value());
    CHECK(entry.randomVariants[0].equipment->weapons[0] == MonsterItem{5, 5, 0, 0});
    CHECK_FALSE(entry.randomVariants[0].equipment->weapons[1]);

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    const GroupsResult again = ReadGroups(text);
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.monsters == read.monsters);

    // Without its own equipment a look wears the entry's.
    const GroupsResult plain = ReadGroups(File(R"({
        "name": "Chaos Castle 2", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF",
        "randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD"}], "numbers": [{"number": 163}] })"));
    REQUIRE(plain.monsters.size() == 1);
    CHECK_FALSE(plain.monsters[0].numbers[0].randomVariants[0].equipment);

    const GroupsResult wrong = ReadGroups(File(R"({
        "name": "Chaos Castle 2", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF",
        "randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD", "equipment": {"weapons": 3}}],
        "numbers": [{"number": 163}] })"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 163 randomVariants[0].equipment.weapons"));
    CHECK(wrong.monsters.empty());
}

TEST_CASE("Monster data: setup values of the wrong kind are fatal, names the code lacks errors [data][monsters]")
{
    const GroupsResult wrong = ReadGroups(File(R"({ "name": "X", "kind": "monster", "scale": "big",
        "glowMesh": 1.5, "shadow": 0, "equipment": {"weapons": [{"group": 99, "number": 0}]}, "selectable": 0,
        "spawnEffects": [{"eyes": [16]}, {"joint": "BITMAP_JOINT_ENERGY", "color": [1, 2]}, {"effect": 5},
                         {"joint": "BITMAP_JOINT_ENERGY", "object": "MODEL_BC_BOX"}],
        "numbers": [{"number": 7}] })"));
    CHECK(HasFatal(wrong.issues));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 scale"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 glowMesh"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 shadow"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 equipment.weapons[0].group"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 selectable"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 spawnEffects[0].eyes"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 spawnEffects[1].color"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 spawnEffects[2].effect"));
    CHECK(Has(wrong.issues, ItemDataIssueSeverity::Fatal, "monster 7 spawnEffects[3]"));
    CHECK(wrong.monsters.empty());

    const GroupsResult unknown = ReadGroups(File(R"({ "name": "X", "kind": "monster",
        "spawnEffects": [{"joint": "BITMAP_NOTHING"}, {"object": "MODEL_NOTHING"}, {"effect": "MODEL_NOTHING"}],
        "spawnBuffs": ["sleepy"], "spawnSound": "SOUND_NOTHING", "pkColor": "villain", "numbers": [{"number": 7}] })"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 7 spawnEffects[0].joint"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 7 spawnEffects[1].object"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 7 spawnBuffs"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 7 pkColor"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 7 spawnEffects[2].effect"));
    CHECK(Has(unknown.issues, ItemDataIssueSeverity::Error, "monster 7 spawnSound"));
    REQUIRE(unknown.monsters.size() == 1);
    CHECK(unknown.monsters[0].numbers[0].setup.spawnEffects.size() == 3);
}
