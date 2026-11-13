#include "stdafx.h"

// Talking to an NPC (MOVEMENT_TALK, MN5): the shipped monster data's talk
// fields and the map rules that stay code, recorded through a TalkWorld that
// stands in for the game (D50).

#include "doctest.h"

#include "MonsterModelRecorder.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterSetup.h"
#include "Engine/Object/w_CharacterInfo.h"
#include "GameLogic/Monsters/MonsterSetupApply.h"
#include "GameLogic/NPCs/NpcTalk.h"
#include "I18N/All.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "World/GameMaps/MonsterSpawnWorld.h"

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
using GameLogic::Npcs::Talk;
using GameLogic::Npcs::TalkWorld;
using Dialog = mu::ui::window::GenericDialogConfig;

constexpr int NpcKey = 77;
constexpr unsigned long RefugeZen = 3000000;
// An NPC no rule names, and a model no rule names.
constexpr int PlainNpc = MONSTER_POTION_GIRL_AMY;
constexpr int PlainModel = MODEL_SMITH;

// A game text as it shows: "%%" was a format's "%".
std::wstring Shown(std::wstring text)
{
    for (size_t at = text.find(L"%%"); at != std::wstring::npos; at = text.find(L"%%", at + 1))
        text.erase(at, 1);
    return text;
}

// The shipped monster list, as the game loads it.
const Data::Monsters::MonsterDataLoadResult& ShippedMonsters()
{
    static const Data::Monsters::MonsterDataLoadResult loaded =
        Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json");
    return loaded;
}

void LoadShippedMonsters()
{
    REQUIRE(ShippedMonsters().monsters.size() > 500);
    g_MonsterDatabase.Build(ShippedMonsters().monsters, "en");
    g_MonsterDatabase.SetTalkTexts(ShippedMonsters().talkTexts);
}

// The game's state a talk reads, and what it did, in order.
struct TalkRecorder
{
    TalkRecorder()
    {
        LoadShippedMonsters();
    }

    int level = 400;
    bool crywolfElfTime = false;
    bool crywolfMap = false;
    bool kanturu2nd = false;
    bool cursedTemple = false;
    // The quests' states by quest (QUEST_3RD_CHANGE_UP_2, ...); QUEST_NONE for
    // the others.
    std::map<int, int> questStates;
    unsigned long gold = 0;
    bool darkHorse = false;
    bool darkRaven = false;

    std::vector<std::wstring> log;
    std::optional<Dialog> dialog;
    GameLogic::Npcs::CrywolfAltarTexts altarTexts;

    TalkWorld World()
    {
        TalkWorld world;
        // The setup in the states that are on, crywolfElfTime by the recorder.
        world.talkSetup = [this](const CHARACTER& npc)
        {
            const Data::Monsters::MonsterDefinition* entry = g_MonsterDatabase.Find(npc.MonsterIndex);
            REQUIRE(entry != nullptr);
            GameLogic::Monsters::Setup::SpawnWorld spawnWorld = World::Monsters::GetSpawnWorld();
            spawnWorld.isStateOn = [this](std::string_view state)
            { return state == "crywolfElfTime" ? crywolfElfTime : World::Monsters::GetSpawnWorld().isStateOn(state); };
            return GameLogic::Monsters::Setup::SetupNow(*entry, spawnWorld);
        };
        world.locale = [] { return std::string("en"); };
        world.findTalkTexts = [](const std::string& name) -> const Data::Monsters::MonsterAltarContract*
        {
            const auto found = g_MonsterDatabase.GetTalkTexts().find(name);
            return found != g_MonsterDatabase.GetTalkTexts().end() ? &found->second : nullptr;
        };
        world.heroLevel = [this] { return level; };
        world.isCrywolfElfTime = [this] { return crywolfElfTime; };
        world.isCrywolfMap = [this] { return crywolfMap; };
        world.isKanturu2nd = [this] { return kanturu2nd; };
        world.isCursedTemple = [this] { return cursedTemple; };
        world.questState = [this](int quest)
        {
            const auto found = questStates.find(quest);
            return found != questStates.end() ? found->second : static_cast<int>(QUEST_NONE);
        };
        world.gold = [this] { return gold; };
        world.hasDarkHorse = [this] { return darkHorse; };
        world.hasDarkRaven = [this] { return darkRaven; };
        world.sendTalk = [this](int key) { log.push_back(L"talk " + std::to_wstring(key)); };
        world.sendEnterRefuge = [this] { log.emplace_back(L"enter refuge"); };
        world.sendEnterBarracks = [this] { log.emplace_back(L"enter barracks"); };
        world.sendDarkHorseInfo = [this] { log.emplace_back(L"dark horse info"); };
        world.sendDarkRavenInfo = [this] { log.emplace_back(L"dark raven info"); };
        world.showDialog = [this](Dialog config)
        {
            log.push_back(L"dialog " + (config.lines.empty() ? std::wstring() : config.lines.front().text));
            dialog = std::move(config);
        };
        world.systemMessage = [this](const std::wstring& text) { log.push_back(L"message " + text); };
        world.errorMessage = [this](const std::wstring& text) { log.push_back(L"error " + text); };
        world.setRepairShop = [this](bool repair) { log.push_back(repair ? L"repair shop" : L"no repair"); };
        world.setNoQuestTitleFor = [this](const std::vector<int>& classes)
        { log.push_back(L"no quest title for " + std::to_wstring(classes.size())); };
        world.beforeTalk = [this] { log.emplace_back(L"before talk"); };
        world.crywolfAltarTalk = [this](CHARACTER&, const GameLogic::Npcs::CrywolfAltarTexts& texts)
        {
            log.emplace_back(L"crywolf altar");
            altarTexts = texts;
        };
        world.kanturu2ndTalk = [this](CHARACTER&) { log.emplace_back(L"kanturu 2nd"); };
        world.cursedTempleTalk = [this](CHARACTER&) { log.emplace_back(L"cursed temple"); };
        return world;
    }
};

std::unique_ptr<CHARACTER> MakeNpc(int number, int model = PlainModel)
{
    auto npc = std::make_unique<CHARACTER>();
    npc->MonsterIndex = static_cast<EMonsterType>(number);
    npc->Object.Type = model;
    npc->Key = NpcKey;
    return npc;
}

// What every talking NPC does before its own steps.
std::vector<std::wstring> Around(std::vector<std::wstring> steps, bool repair = false)
{
    std::vector<std::wstring> log{repair ? L"repair shop" : L"no repair", L"before talk", L"no quest title for 0"};
    log.insert(log.end(), steps.begin(), steps.end());
    return log;
}
} // namespace

TEST_CASE("NPC talk: an NPC talks unless its entry says it does not [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    const auto statue = MakeNpc(MONSTER_WOLF_STATUS);
    CHECK(Talk(*statue, world));
    CHECK(recorder.log.empty());

    const auto npc = MakeNpc(PlainNpc);
    CHECK(Talk(*npc, world));
    CHECK(recorder.log == Around({L"talk 77"}));
}

TEST_CASE("NPC talk: the Chaos Goblin talks from its level on [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    const auto goblin = MakeNpc(MONSTER_CHAOS_GOBLIN);
    recorder.level = CHAOS_MIX_LEVEL - 1;
    CHECK_FALSE(Talk(*goblin, world));
    // The game's text, with the level for its "%d".
    std::wstring expected = I18N::Game::OnlyLevelAboveDCanDoTheChaosCombination;
    expected.replace(expected.find(L"%d"), 2, std::to_wstring(CHAOS_MIX_LEVEL));
    CHECK(recorder.log == std::vector<std::wstring>{L"message " + expected});

    recorder.log.clear();
    recorder.level = CHAOS_MIX_LEVEL;
    CHECK(Talk(*goblin, world));
    CHECK(recorder.log == Around({L"talk 77"}));
}

TEST_CASE("NPC talk: the repair shops show their repair buttons [monsters][talk]")
{
    for (const int number : {MONSTER_EO_THE_CRAFTSMAN, MONSTER_ZIENNA_THE_WEAPONS_MERCHANT,
                             MONSTER_HANZO_THE_BLACKSMITH, MONSTER_RHEA, MONSTER_WEAPONS_MERCHANT_BOLO})
    {
        INFO(number);
        TalkRecorder recorder;
        const auto npc = MakeNpc(number);
        CHECK(Talk(*npc, recorder.World()));
        CHECK(recorder.log == Around({L"talk 77"}, true));
    }
}

TEST_CASE("NPC talk: Crywolf's altars and its Werewolf Guardsman [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    const auto altar = MakeNpc(MONSTER_WOLF_ALTAR1, MODEL_CRYWOLF_ALTAR1);
    const auto guardsman = MakeNpc(MONSTER_WEREWOLF_QUARREL, MODEL_NPC_QUARREL);

    // While only elves may contract: the altar's hook, a plain talk else.
    recorder.crywolfElfTime = true;
    recorder.crywolfMap = true;
    CHECK(Talk(*altar, world));
    CHECK(recorder.log == Around({L"crywolf altar"}));
    // Its texts are its talk step's, from the game's texts.
    CHECK(recorder.altarTexts.canContract == I18N::Game::ContractCanBeMadeForDTimes);
    CHECK(recorder.altarTexts.ongoing == I18N::Game::ContractIsOngoingThereforeDualCompactIsNotPossible);
    recorder.log.clear();
    CHECK(Talk(*guardsman, world));
    CHECK(recorder.log == Around({L"talk 77"}));

    // Otherwise an altar does nothing; the guardsman shows his dialog, then talks.
    recorder.crywolfElfTime = false;
    recorder.log.clear();
    CHECK(Talk(*altar, world));
    CHECK(recorder.log == Around({}));
    // Away from Crywolf it talks.
    recorder.crywolfMap = false;
    recorder.log.clear();
    CHECK(Talk(*altar, world));
    CHECK(recorder.log == Around({L"talk 77"}));
    recorder.crywolfMap = true;
    recorder.log.clear();
    CHECK(Talk(*guardsman, world));
    CHECK(recorder.log == Around({std::wstring(L"dialog ") + I18N::Game::WerewolfGuardsman, L"talk 77"}));
    REQUIRE(recorder.dialog.has_value());
    REQUIRE(recorder.dialog->lines.size() == 5);
    CHECK(recorder.dialog->lines[0].bold);
    CHECK(recorder.dialog->lines[1].text == I18N::Game::DoYouEvenKnowAboutMe);
    CHECK(recorder.dialog->lines[4].text == I18N::Game::InOrderToReceiveHelpFrom);

    // His OK: the quest's step, then the zen. Another quest's step does not
    // count.
    recorder.log.clear();
    recorder.gold = RefugeZen;
    recorder.questStates = {{QUEST_3RD_CHANGE_UP_1, QUEST_ING}, {QUEST_3RD_CHANGE_UP_3, QUEST_END}};
    recorder.dialog->onPrimary();
    CHECK(recorder.log.empty());
    recorder.questStates = {{QUEST_3RD_CHANGE_UP_2, QUEST_ING}};
    recorder.gold = RefugeZen - 1;
    recorder.dialog->onPrimary();
    CHECK(recorder.log == std::vector<std::wstring>{std::wstring(L"error ") + I18N::Game::YouAreShortOfZen});
    recorder.log.clear();
    recorder.questStates = {{QUEST_3RD_CHANGE_UP_2, QUEST_END}};
    recorder.gold = RefugeZen;
    recorder.dialog->onPrimary();
    CHECK(recorder.log == std::vector<std::wstring>{L"enter refuge"});
}

// The barracks' rule is the Gatekeeper's own (Q-MN46d): OpenMU places only
// him there, so other NPCs no longer show his dialog.
TEST_CASE("NPC talk: the Gatekeeper talks and shows his dialog [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    const auto gatekeeper = MakeNpc(MONSTER_GATEKEEPER);
    CHECK(Talk(*gatekeeper, world));
    CHECK(recorder.log == Around({L"talk 77", std::wstring(L"dialog ") + I18N::Game::Gatekeeper}));
    REQUIRE(recorder.dialog.has_value());
    REQUIRE(recorder.dialog->lines.size() == 4);
    CHECK(recorder.dialog->lines[1].text == I18N::Game::HmmWhoAreYouIMConfusedAreYouEvenApprovedOfBalgass);

    recorder.log.clear();
    recorder.questStates = {{QUEST_3RD_CHANGE_UP_3, QUEST_END}, {QUEST_3RD_CHANGE_UP_2, QUEST_ING}};
    recorder.dialog->onPrimary();
    CHECK(recorder.log.empty());
    recorder.questStates = {{QUEST_3RD_CHANGE_UP_3, QUEST_ING}};
    recorder.dialog->onPrimary();
    CHECK(recorder.log == std::vector<std::wstring>{L"enter barracks"});
}

// The messages are whole (P23) and only the red and the blue one have one.
TEST_CASE("NPC talk: the Little Santas say what they gave [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    const auto red = MakeNpc(MONSTER_LITTLE_SANTA_RED);
    CHECK(Talk(*red, world));
    CHECK(recorder.log == Around({L"talk 77", L"message " + Shown(I18N::Game::HealthHasBeenRecoveredOf100)}));
    CHECK(Shown(I18N::Game::HealthHasBeenRecoveredOf100).size() > 31);

    recorder.log.clear();
    const auto blue = MakeNpc(MONSTER_LITTLE_SANTA_BLUE);
    CHECK(Talk(*blue, world));
    CHECK(recorder.log == Around({L"talk 77", L"message " + Shown(I18N::Game::ManaHasBeenRecoveredOf100)}));

    recorder.log.clear();
    const auto yellow = MakeNpc(MONSTER_LITTLE_SANTA_YELLOW);
    CHECK(Talk(*yellow, world));
    CHECK(recorder.log == Around({L"talk 77"}));
}

// Every NPC's talk request goes through the map's rule, Delgado's, Lugard's,
// Julia's and David's too (Q-MN46: none of them stands on those maps).
TEST_CASE("NPC talk: the map rules of Kanturu 2nd and the Illusion Temple [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    const auto npc = MakeNpc(PlainNpc);
    const auto delgado = MakeNpc(MONSTER_DELGADO);
    recorder.kanturu2nd = true;
    CHECK(Talk(*npc, world));
    CHECK(recorder.log == Around({L"kanturu 2nd"}));
    recorder.log.clear();
    CHECK(Talk(*delgado, world));
    CHECK(recorder.log == Around({L"kanturu 2nd"}));

    recorder.kanturu2nd = false;
    recorder.cursedTemple = true;
    recorder.log.clear();
    CHECK(Talk(*npc, world));
    CHECK(recorder.log == Around({L"cursed temple"}));
}

TEST_CASE("NPC talk: Marlon's quest window and the Pet Trainer's pet information [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    const auto marlon = MakeNpc(MONSTER_MARLON);
    CHECK(Talk(*marlon, world));
    CHECK(recorder.log == Around({L"talk 77", L"no quest title for 3"}));

    recorder.log.clear();
    recorder.darkHorse = true;
    recorder.darkRaven = true;
    const auto trainer = MakeNpc(MONSTER_PET_TRAINER);
    CHECK(Talk(*trainer, world));
    CHECK(recorder.log == Around({L"talk 77", L"dark horse info", L"dark raven info"}));

    // While only elves may make the altar contracts (the trainer is on
    // Crywolf too), it still asks for the pets' information.
    recorder.log.clear();
    recorder.crywolfElfTime = true;
    recorder.crywolfMap = true;
    CHECK(Talk(*trainer, world));
    CHECK(recorder.log == Around({L"talk 77", L"dark horse info", L"dark raven info"}));
}

// The altar contract step on an NPC that is no altar only talks: in Crywolf's
// elf time, on Crywolf after it and elsewhere.
TEST_CASE("NPC talk: an altar contract step on an NPC that is no altar talks [monsters][talk]")
{
    TalkRecorder recorder;
    const TalkWorld world = recorder.World();
    // The shipped list with the step on the plain NPC, the shipped one built
    // back however the test ends (Build: editor-OFF builds have no Set).
    struct ShippedBack
    {
        ~ShippedBack()
        {
            g_MonsterDatabase.Build(ShippedMonsters().monsters, "en");
        }
    } const back;
    std::vector<Data::Monsters::MonsterDefinition> monsters = ShippedMonsters().monsters;
    const auto plain = std::ranges::find(monsters, PlainNpc, &Data::Monsters::MonsterDefinition::number);
    REQUIRE(plain != monsters.end());
    Data::Monsters::MonsterTalkBlock contract;
    contract.kind = Data::Monsters::MonsterTalkBlock::Kind::CrywolfAltarContract;
    contract.altarTexts = "crywolfAltar";
    plain->setup.onTalk = {contract};
    g_MonsterDatabase.Build(monsters, "en");
    const auto npc = MakeNpc(PlainNpc);

    recorder.crywolfMap = true;
    recorder.crywolfElfTime = true;
    CHECK(Talk(*npc, world));
    CHECK(recorder.log == Around({L"talk 77"}));
    recorder.crywolfElfTime = false;
    recorder.log.clear();
    CHECK(Talk(*npc, world));
    CHECK(recorder.log == Around({L"talk 77"}));
    recorder.crywolfMap = false;
    recorder.log.clear();
    CHECK(Talk(*npc, world));
    CHECK(recorder.log == Around({L"talk 77"}));
}

// The level message and the altar contract's count put their number in each
// "%d" the same way; another "%" stays.
TEST_CASE("NPC talk: a text's %d is the number, every one [monsters][talk]")
{
    CHECK(GameLogic::Npcs::WithNumber(L"%d left, %d in all, 100%", 3) == L"3 left, 3 in all, 100%");
    CHECK(GameLogic::Npcs::WithNumber(L"No number", 3) == L"No number");
}
