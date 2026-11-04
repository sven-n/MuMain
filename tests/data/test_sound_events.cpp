#include "stdafx.h"

// Which sound the game's code plays when (Data/Audio/SoundEvents.json, sound
// editor design Q-SE1): the reader and writer, the rules' match, the shipped
// file, the pick and what a player holds.

#include "doctest.h"

#include "Audio/SoundEventPlay.h"
#include "Core/Utilities/Random.h"
#include "Data/GameData/SoundData/SoundEventDatabase.h"
#include "Data/GameData/SoundData/SoundEvents.h"
#include "Engine/Object/ZzzCharacter.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Sounds::HeldWeapon;
using Data::Sounds::SoundEvent;
using Data::Sounds::SoundEventContext;
using Data::Sounds::SoundEventRule;

namespace
{
std::vector<SoundEvent> Read(const std::string& text, std::vector<ItemDataIssue>& issues)
{
    std::vector<SoundEvent> events;
    Data::Sounds::ReadSoundEventsJson(text, "SoundEvents.json", events, issues);
    return events;
}

std::string File(const std::string& events)
{
    return R"({"formatVersion": 1, "events": [)" + events + "]}";
}

bool HasFatal(const std::vector<ItemDataIssue>& issues, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(), [&field](const ItemDataIssue& issue)
                       { return issue.severity == ItemDataIssueSeverity::Fatal && issue.field == field; });
}

// The table for the scope, put back after.
class EventScope
{
public:
    explicit EventScope(std::vector<SoundEvent> events) : m_previous(g_SoundEventDatabase.GetAll())
    {
        g_SoundEventDatabase.Build(events);
    }
    ~EventScope()
    {
        g_SoundEventDatabase.Build(m_previous);
    }
    EventScope(const EventScope&) = delete;
    EventScope& operator=(const EventScope&) = delete;

private:
    std::vector<SoundEvent> m_previous;
};
} // namespace

TEST_CASE("Sound events: a rule reads its worlds, tiles, safe zone, what is held and its sounds, and writes back "
          "the same, and wrong values are fatal and name the field (Q-SE1) [data][sounds]")
{
    std::vector<ItemDataIssue> issues;
    const std::vector<SoundEvent> events =
        Read(File(R"({"event": "heroFootstep", "rules": [)"
                  R"({"worlds": [7, 24], "tiles": [0, 3], "safeZone": false, "plays": ["SOUND_HUMAN_WALK_SWIM"]},)"
                  R"({"holds": "magicCrossbow", "plays": ["SOUND_BRANDISH_SWORD01", "SOUND_BRANDISH_SWORD02"]},)"
                  R"({"plays": ["SOUND_HUMAN_WALK_GROUND"]}]})"),
             issues);
    CHECK(issues.empty());
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].rules.size() == 3);
    const SoundEventRule& swim = events[0].rules[0];
    CHECK(swim.worlds == std::vector<int>{7, 24});
    CHECK(swim.tiles == std::vector<int>{0, 3});
    CHECK(swim.safeZone == false);
    CHECK_FALSE(swim.holds.has_value());
    CHECK(swim.numbers == std::vector<int>{static_cast<int>(SOUND_HUMAN_WALK_SWIM)});
    CHECK(events[0].rules[1].holds == HeldWeapon::MagicCrossbow);
    CHECK(events[0].rules[1].numbers.size() == 2);
    CHECK(events[0].rules[2].worlds.empty());
    const std::string text = Data::Sounds::WriteSoundEventsJson(events);
    CHECK(text.find(R"({"plays": ["SOUND_HUMAN_WALK_GROUND"]})") != std::string::npos);
    std::vector<ItemDataIssue> again;
    CHECK(Read(text, again) == events);
    CHECK(again.empty());

    const auto fatal = [](const std::string& rule, const std::string& field)
    {
        std::vector<ItemDataIssue> found;
        CHECK(Read(File(R"({"event": "e", "rules": [)" + rule + "]}"), found).empty());
        CHECK(HasFatal(found, field));
    };
    fatal(R"({"plays": []})", "events[0].rules[0].plays");
    fatal(R"({"plays": ["SOUND_NONE"]})", "events[0].rules[0].plays[0]");
    fatal(R"({"worlds": [-1], "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].worlds[0]");
    fatal(R"({"tiles": [256], "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].tiles[0]");
    fatal(R"({"safeZone": 1, "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].safeZone");
    fatal(R"({"holds": "sword", "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].holds");
    fatal(R"({"kinds": "confirm", "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].kinds");
    fatal(R"({"kinds": ["Confirm"], "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].kinds[0]");
    fatal(R"({"kinds": [""], "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].kinds[0]");
    fatal(R"({"kinds": ["menu", "menu"], "plays": ["SOUND_MAGIC"]})", "events[0].rules[0].kinds[1]");
    std::vector<ItemDataIssue> twice;
    CHECK(Read(File(R"({"event": "e", "rules": []}, {"event": "e", "rules": []})"), twice).size() == 1);
    CHECK(HasFatal(twice, "events[1].event"));
}

TEST_CASE("Sound events: a rule's click kinds read, write back and match the clicked button's kind, and a rule "
          "without kinds plays for any (Q-UI1) [data][sounds]")
{
    std::vector<ItemDataIssue> issues;
    const std::vector<SoundEvent> events = Read(File(R"({"event": "interfaceClick", "rules": [)"
                                                     R"({"kinds": ["menu", "tab-2"], "plays": ["SOUND_MENU01"]},)"
                                                     R"({"plays": ["SOUND_CLICK01"]}]})"),
                                                issues);
    CHECK(issues.empty());
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].rules.size() == 2);
    CHECK(events[0].rules[0].kinds == std::vector<std::string>{"menu", "tab-2"});
    CHECK(events[0].rules[1].kinds.empty());
    const std::string text = Data::Sounds::WriteSoundEventsJson(events);
    CHECK(text.find(R"({"kinds": ["menu", "tab-2"], "plays": ["SOUND_MENU01"]})") != std::string::npos);
    std::vector<ItemDataIssue> again;
    CHECK(Read(text, again) == events);
    const auto match = [&events](std::string_view kind)
    {
        SoundEventContext context;
        context.kind = kind;
        const SoundEventRule* rule = Data::Sounds::MatchRule(events[0], context);
        return rule == nullptr ? -1 : static_cast<int>(rule - events[0].rules.data());
    };
    CHECK(match("menu") == 0);
    CHECK(match("tab-2") == 0);
    CHECK(match("confirm") == 1);
    CHECK(match("") == 1);
    CHECK(Data::Sounds::IsClickKindWord("confirm"));
    CHECK(Data::Sounds::IsClickKindWord("tab-2"));
    CHECK_FALSE(Data::Sounds::IsClickKindWord(""));
    CHECK_FALSE(Data::Sounds::IsClickKindWord("Confirm"));
    CHECK_FALSE(Data::Sounds::IsClickKindWord("ok cancel"));
    CHECK_FALSE(Data::Sounds::IsClickKindWord("a_b"));
}

TEST_CASE("Sound events: the first rule that matches the world, tile, safe zone and what is held plays (Q-SE1) "
          "[data][sounds]")
{
    SoundEvent event;
    SoundEventRule devias;
    devias.worlds = {2};
    devias.tiles = {0, 1};
    SoundEventRule swim;
    swim.worlds = {7};
    swim.safeZone = false;
    SoundEventRule bow;
    bow.holds = HeldWeapon::Bow;
    SoundEventRule any;
    event.rules = {devias, swim, bow, any};
    const auto match = [&event](int world, int tile, bool safeZone, HeldWeapon holds)
    {
        const SoundEventRule* rule = Data::Sounds::MatchRule(event, SoundEventContext{world, tile, safeZone, holds});
        return rule == nullptr ? -1 : static_cast<int>(rule - event.rules.data());
    };
    CHECK(match(2, 1, false, HeldWeapon::None) == 0);
    CHECK(match(2, 3, false, HeldWeapon::None) == 3);
    CHECK(match(7, 9, false, HeldWeapon::None) == 1);
    CHECK(match(7, 9, true, HeldWeapon::None) == 3);
    CHECK(match(7, 9, true, HeldWeapon::Bow) == 2);
    event.rules.pop_back();
    CHECK(match(9, 0, true, HeldWeapon::Other) == -1);
    for (const HeldWeapon weapon : {HeldWeapon::None, HeldWeapon::Bow, HeldWeapon::Crossbow, HeldWeapon::MagicCrossbow,
                                    HeldWeapon::SaberOrSpear, HeldWeapon::Other})
    {
        CHECK(Data::Sounds::FindHeldWeapon(Data::Sounds::HeldWeaponWord(weapon)) == weapon);
    }
}

TEST_CASE("Sound events: the shipped file reads with no problem, has the footsteps and the swings, and writes back "
          "the same (Q-SE1) [data][sounds]")
{
    std::ifstream in(std::string(MU_TEST_DATA_DIR) + "/Audio/SoundEvents.json", std::ios::binary);
    REQUIRE(in.good());
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string text = buffer.str();
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    std::vector<ItemDataIssue> issues;
    const std::vector<SoundEvent> events = Read(text, issues);
    CHECK(issues.empty());
    REQUIRE(events.size() == 4);
    CHECK(events[0].event == Data::Sounds::HeroFootstepEvent);
    CHECK(events[1].event == Data::Sounds::PlayerAttackSwingEvent);
    CHECK(Data::Sounds::WriteSoundEventsJson(events) == text);
    // The clicks play the click the login's OK and Cancel played (Q-UI1).
    CHECK(events[2].event == Data::Sounds::InterfaceClickEvent);
    for (const std::string_view kind : {"confirm", "cancel", "select", "menu"})
    {
        SoundEventContext context;
        context.kind = kind;
        const SoundEventRule* rule = Data::Sounds::MatchRule(events[2], context);
        REQUIRE(rule != nullptr);
        CHECK(rule->kinds == std::vector<std::string>{std::string(kind)});
        CHECK(rule->numbers == std::vector<int>{static_cast<int>(SOUND_CLICK01)});
    }
    CHECK(events[3].event == Data::Sounds::CharacterSelectedEvent);
    REQUIRE(events[3].rules.size() == 1);
    CHECK(events[3].rules[0].numbers == std::vector<int>{static_cast<int>(SOUND_CLICK01)});
}

TEST_CASE("Sound events: PickEventSound plays a rule's one sound without a draw and picks one of more with "
          "Random::Rand(), and says nothing without a rule (Q-SE1) [data][sounds]")
{
    SoundEvent swing;
    swing.event = "testSwing";
    SoundEventRule one;
    one.holds = HeldWeapon::Bow;
    one.plays = {"SOUND_BOW01"};
    one.numbers = {static_cast<int>(SOUND_BOW01)};
    SoundEventRule two;
    two.holds = HeldWeapon::Other;
    two.plays = {"SOUND_BRANDISH_SWORD01", "SOUND_BRANDISH_SWORD02"};
    two.numbers = {static_cast<int>(SOUND_BRANDISH_SWORD01), static_cast<int>(SOUND_BRANDISH_SWORD02)};
    swing.rules = {one, two};
    const EventScope scope({swing});
    SoundEventContext context;
    context.holds = HeldWeapon::Bow;
    Random::SeedRand(7);
    const int first = Random::Rand();
    Random::SeedRand(7);
    CHECK(Audio::PickEventSound("testSwing", context) == SOUND_BOW01);
    // No draw for one sound: the sequence goes on where it was.
    CHECK(Random::Rand() == first);
    context.holds = HeldWeapon::Other;
    Random::SeedRand(7);
    const int draw = Random::Rand();
    Random::SeedRand(7);
    CHECK(Audio::PickEventSound("testSwing", context) ==
          static_cast<ESound>(static_cast<int>(SOUND_BRANDISH_SWORD01) + draw % 2));
    context.holds = HeldWeapon::None;
    CHECK_FALSE(Audio::PickEventSound("testSwing", context).has_value());
    CHECK_FALSE(Audio::PickEventSound("noSuchEvent", context).has_value());
}

TEST_CASE("Sound events: what a player holds at an attack's start: the bow group's items 13 and 14, a Light Saber "
          "or Spear, another weapon, nothing (Q-SE1) [data][sounds]")
{
    auto c = std::make_unique<CHARACTER>();
    c->Weapon[0].Type = -1;
    c->Weapon[1].Type = -1;
    CHECK(Audio::HeldWeaponOf(*c) == HeldWeapon::None);
    c->Weapon[0].Type = MODEL_BLUEWING_CROSSBOW;
    const HeldWeapon bluewing = Audio::HeldWeaponOf(*c);
    // Counted as a crossbow when the item data says so, else by its model.
    CHECK((bluewing == HeldWeapon::MagicCrossbow || bluewing == HeldWeapon::Crossbow));
    c->Weapon[0].Type = MODEL_LIGHT_SABER;
    CHECK(Audio::HeldWeaponOf(*c) == HeldWeapon::SaberOrSpear);
    c->Weapon[0].Type = MODEL_SPEAR;
    CHECK(Audio::HeldWeaponOf(*c) == HeldWeapon::SaberOrSpear);
    c->Weapon[0].Type = MODEL_SWORD;
    CHECK(Audio::HeldWeaponOf(*c) == HeldWeapon::Other);
    c->Weapon[0].Type = -1;
    c->Weapon[1].Type = MODEL_SHIELD;
    CHECK(Audio::HeldWeaponOf(*c) == HeldWeapon::Other);
}
