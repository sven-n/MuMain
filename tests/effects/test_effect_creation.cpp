#include "stdafx.h"

#include "doctest.h"

#include "EffectRecorder.h"
#include "EffectTestData.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace EffectRecorder;
using Data::Effects::EffectCreateParams;
using Data::Effects::EffectTypeCreateParams;
using EffectTestData::BuildShippedRegistry;

namespace
{
// The 32 types whose creation values FX1.2 moved into the catalogue (it
// compared them with their cases then).
const std::array<int, 32> Fx12Types = {MODEL_BALGAS_SKILL,
                                       BATTLE_CASTLE_WALL1,
                                       BATTLE_CASTLE_WALL2,
                                       BATTLE_CASTLE_WALL3,
                                       BATTLE_CASTLE_WALL4,
                                       MODEL_BLOOD,
                                       MODEL_CURSEDTEMPLE_HOLYITEM,
                                       MODEL_CURSEDTEMPLE_PRODECTION_SKILL,
                                       MODEL_CURSEDTEMPLE_RESTRAINT_SKILL,
                                       MODEL_DESAIR,
                                       BITMAP_FIRE,
                                       BITMAP_FIRE_RED,
                                       MODEL_FISSURE,
                                       MODEL_FISSURE_LIGHT,
                                       BITMAP_IMPACT,
                                       MODEL_INFINITY_ARROW4,
                                       MODEL_CUNDUN_GHOST,
                                       BITMAP_LIGHT_MARKS,
                                       MODEL_SPEAR,
                                       MODEL_MAGIC1,
                                       MODEL_MAGIC_CAPSULE2,
                                       MODEL_MAYASTAR,
                                       MODEL_POISON,
                                       MODEL_PROTECT,
                                       MODEL_SKILL_FISSURE,
                                       MODEL_SUMMONER_SUMMON_NEIL_GROUND1,
                                       MODEL_SUMMONER_SUMMON_NEIL_GROUND2,
                                       MODEL_SUMMONER_SUMMON_NEIL_GROUND3,
                                       MODEL_SUMMONER_SUMMON_NEIL_NIFE1,
                                       MODEL_SUMMONER_SUMMON_NEIL_NIFE2,
                                       MODEL_SUMMONER_SUMMON_NEIL_NIFE3,
                                       BITMAP_SWORDEFF};

// The 8 types whose creation cases only set fields of CreateParams; FX1.3
// moved them into the catalogue.
const std::array<int, 8> Fx13Types = {MODEL_KENTAUROS_ARROW,
                                      MODEL_WARP3,
                                      MODEL_WARP6,
                                      BITMAP_SPARK + 1,
                                      BITMAP_SPARK + 2,
                                      MODEL_1_STREAMBREATHFIRE,
                                      MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2,
                                      MODEL_EFFECT_SD_AURA};

// The 28 types whose creation cases FX1.4 moved into the catalogue with the
// fields it added (vectors, offsets, copies and more values), and
// MODEL_PHOENIX_SHOT, whose case set nothing, last.
const std::array<int, 29> Fx14Types = {MODEL_DRAGON,
                                       MODEL_SHIELD_CRASH2,
                                       MODEL_TREE_ATTACK,
                                       MODEL__SPEAR,
                                       MODEL_SUMMONER_WRISTRING_EFFECT,
                                       MODEL_SUMMONER_CASTING_EFFECT4,
                                       MODEL_SUMMONER_SUMMON_NEIL,
                                       MODEL_ALICE_BUFFSKILL_EFFECT2,
                                       BITMAP_JOINT_THUNDER,
                                       MODEL_STAFF_OF_DESTRUCTION,
                                       MODEL_WAVE,
                                       MODEL_TAIL,
                                       MODEL_BOSS_ATTACK,
                                       MODEL_DARK_ELF_SKILL,
                                       MODEL_WATER_WAVE,
                                       BITMAP_FIRECRACKERRISE,
                                       BITMAP_FIRECRACKER0001,
                                       MODEL_CLOUD,
                                       MODEL_TOWER_GATE_PLANE,
                                       MODEL_KNIGHT_PLANCRACK_B,
                                       MODEL_PROJECTILE,
                                       BITMAP_SHINY + 4,
                                       MODEL_WINDFOCE_MIRROR,
                                       BITMAP_SWORD_EFFECT_MONO,
                                       MODEL_TARGETMON_EFFECT,
                                       BITMAP_EVENT_CLOUD,
                                       MODEL_STATUE_CRUSH_EFFECT_PIECE04,
                                       MODEL_DOOR_CRUSH_EFFECT_PIECE10,
                                       MODEL_PHOENIX_SHOT};
const std::span<const int> Fx14RowTypes(Fx14Types.data(), Fx14Types.size() - 1);

// Sub types the callers pass (0 to 3) and one no case handles.
constexpr std::initializer_list<int> RecordedSubTypes = {0, 1, 2, 3, 99};

struct Recorded
{
    std::string description;
    Record record;
};

using CallList = std::vector<EffectCall> (*)(int type, std::initializer_list<int> subTypes);

std::vector<Recorded> RecordAll(std::span<const int> types, CallList calls = CallsFor,
                                std::span<const Conditions> allConditions = AllConditions())
{
    std::vector<Recorded> all;
    for (const int type : types)
    {
        for (const EffectCall& call : calls(type, RecordedSubTypes))
        {
            for (const Conditions& conditions : allConditions)
            {
                all.push_back({Describe(call, conditions), RecordCall(call, conditions)});
            }
        }
    }
    return all;
}

// The number of calls whose records differ; their differences go to `log`.
int CompareAll(const std::vector<Recorded>& expected, const std::vector<Recorded>& actual, std::ostream& log)
{
    REQUIRE(expected.size() == actual.size());
    int differing = 0;
    for (size_t i = 0; i < expected.size(); ++i)
    {
        const std::vector<Difference> differences = Compare(expected[i].record, actual[i].record);
        if (!differences.empty())
        {
            ++differing;
            log << "## " << expected[i].description << "\n" << ToText(differences);
        }
    }
    return differing;
}

EffectCall CallOf(int type, Owner owner = Owner::None)
{
    EffectCall call;
    call.type = type;
    call.owner = owner;
    return call;
}
} // namespace

TEST_CASE("The effect recorder gives the same record when a call is repeated [effects][recorder]")
{
    BuildShippedRegistry();
    const std::array<int, 4> types = {MODEL_KENTAUROS_ARROW, MODEL_WARP, MODEL_GHOST, BITMAP_SWORD_FORCE};
    for (const int type : types)
    {
        for (const Conditions& conditions : AllConditions())
        {
            const EffectCall call = CallOf(type);
            INFO(Describe(call, conditions));
            const Record first = RecordCall(call, conditions);
            const Record second = RecordCall(call, conditions);
            CHECK(Compare(first, second).empty());
            CHECK_FALSE(first.empty());
        }
    }
}

TEST_CASE("The frame factor and the slot pattern show in the record [effects][recorder]")
{
    BuildShippedRegistry();
    // MODEL_GHOST multiplies random offsets by the frame factor.
    const Record full = RecordCall(CallOf(MODEL_GHOST), {1.f, SlotPattern::A});
    const Record half = RecordCall(CallOf(MODEL_GHOST), {0.5f, SlotPattern::A});
    CHECK_FALSE(Compare(full, half).empty());

    // BITMAP_SPARK + 1 sets only LifeTime: Gravity, Timer, ... keep what the slot had.
    const Record a = RecordCall(CallOf(BITMAP_SPARK + 1), {1.f, SlotPattern::A});
    const Record b = RecordCall(CallOf(BITMAP_SPARK + 1), {1.f, SlotPattern::B});
    CHECK(Find(a, "Effects[0].LifeTime") == "10");
    CHECK(Find(b, "Effects[0].LifeTime") == "10");
    CHECK_FALSE(Find(a, "Effects[0].Gravity").has_value());
    CHECK_FALSE(Compare(a, b).empty());
}

TEST_CASE("The effect recorder counts the values rand() and Random:: give [effects][recorder]")
{
    BuildShippedRegistry();
    Record record = RecordCall(CallOf(MODEL_KENTAUROS_ARROW), {});
    CHECK(Find(record, "draws.rand") == "0");
    CHECK(Find(record, "draws.Random") == "0");

    // Three rand() in its case.
    record = RecordCall(CallOf(MODEL_WARP), {});
    CHECK(Find(record, "draws.rand") == "3");
    CHECK(Find(record, "draws.Random") == "0");

    // rand_fps_check(2), which takes two values of Random::, then nine rand().
    record = RecordCall(CallOf(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN), {});
    CHECK(Find(record, "draws.rand") == "9");
    CHECK(Find(record, "draws.Random") == "2");
}

TEST_CASE("A skill effect of the hero goes into the skill effect pool [effects][recorder]")
{
    BuildShippedRegistry();
    const Record hero = RecordCall(CallOf(BITMAP_SWORD_FORCE, Owner::Hero), {1.f, SlotPattern::A});
    CHECK(Find(hero, "SkillEffects[0].Live") == "true");
    CHECK(Find(hero, "SkillEffects[0].Owner") == "hero");
    CHECK_FALSE(Find(hero, "Effects[0].Live").has_value());

    const Record monster = RecordCall(CallOf(BITMAP_SWORD_FORCE, Owner::Monster), {1.f, SlotPattern::A});
    CHECK(Find(monster, "Effects[0].Live") == "true");
    CHECK(Find(monster, "Effects[0].Owner") == "monster");
}

TEST_CASE("Sounds, terrain light and trails a creation changes show in the record [effects][recorder]")
{
    BuildShippedRegistry();
    // BITMAP_BLIZZARD plays a sound.
    const Record blizzard = RecordCall(CallOf(BITMAP_BLIZZARD), {});
    CHECK(Find(blizzard, "sounds[0]") == "play " + std::to_string(static_cast<int>(SOUND_METEORITE01)) + " at null");

    // BITMAP_LIGHT_RED with sub type 3 lights the terrain.
    EffectCall light = CallOf(BITMAP_LIGHT_RED);
    light.subType = 3;
    CHECK(Find(RecordCall(light, {}), "terrainLight").has_value());
    CHECK_FALSE(Find(blizzard, "terrainLight").has_value());

    // The Gaion swords take away the trails of the hero (sub types 113 to 155).
    const Record free = RecordCall(CallOf(MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE), {1.f, SlotPattern::A});
    CHECK_FALSE(Find(free, "objectBlurs[0].Live").has_value());
    const Record live = RecordCall(CallOf(MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE), {1.f, SlotPattern::B});
    CHECK(Find(live, "objectBlurs[0].Live") == "false");
    CHECK_FALSE(Find(live, "objectBlurs[1].Live").has_value());
}

TEST_CASE("The second geometry of a call shows what a row copies [effects][recorder]")
{
    BuildShippedRegistry();
    // MODEL_INFINITY_ARROW4 sets its light and copies it into the direction;
    // the light of the call stays as it was.
    const std::vector<EffectCall> calls = SecondGeometryCallsFor(MODEL_INFINITY_ARROW4, {0});
    REQUIRE(calls.size() == 8);
    const EffectCall& second = calls.back();
    CHECK(second.owner == Owner::None);
    CHECK(second.position != calls.front().position);
    CHECK(second.angle != calls.front().angle);
    CHECK(second.light != calls.front().light);
    CHECK(Describe(second, {}).find(" position 12345.5/13579.25/260.5 angle -17/101/271 light ") != std::string::npos);
    CHECK(Describe(calls.front(), {}).find(" position ") == std::string::npos);

    const Record record = RecordCall(second, {});
    CHECK(Find(record, "Effects[0].Position[0]") == "12345.5");
    CHECK(Find(record, "Effects[0].Angle[1]") == "101");
    CHECK(Find(record, "Effects[0].Light[0]") == "1");
    CHECK(Find(record, "Effects[0].Direction[1]") == "0.5");
}

TEST_CASE("A creation row that differs from the old one is caught [effects][recorder]")
{
    BuildShippedRegistry();
    const std::array<int, 1> arrow = {MODEL_KENTAUROS_ARROW};
    const std::vector<Recorded> shipped = RecordAll(arrow);

    // Without alpha and with a lifeTime of 35 instead of 34.
    const std::array<EffectTypeCreateParams, 1> wrongRow = {EffectTypeCreateParams{
        MODEL_KENTAUROS_ARROW,
        EffectCreateParams{.lifeTime = 35, .scale = 0.7, .velocity = 70, .light = std::array<double, 3>{1, 1, 1}}}};
    BuildShippedRegistry({}, wrongRow);
    const std::vector<Recorded> wrong = RecordAll(arrow);
    BuildShippedRegistry();

    std::ostringstream log;
    CHECK(CompareAll(shipped, wrong, log) == static_cast<int>(shipped.size()));
    const std::vector<Difference> differences = Compare(shipped[0].record, wrong[0].record);
    REQUIRE(differences.size() == 2);
    CHECK(differences[0].path == "Effects[0].Alpha");
    CHECK(differences[0].expected == "0");
    CHECK(differences[1].path == "Effects[0].LifeTime");
    CHECK(differences[1].expected == "34");
    CHECK(differences[1].actual == "35");
}

// FX1.3 moved the creation of these types from code into the catalogue; the
// commit before the one that deleted their cases compared both for every sub
// type, owner, frame factor and slot pattern. These are the values the cases
// set (floats as the recorder writes them).
TEST_CASE("The 8 types of FX1.3 create from the catalogue what their cases set [effects][recorder]")
{
    struct Expected
    {
        int type;
        std::vector<RecordedValue> values;
    };
    const std::vector<Expected> expected = {
        {MODEL_KENTAUROS_ARROW,
         {{"Effects[0].LifeTime", "34"},
          {"Effects[0].Scale", "0.699999988"},
          {"Effects[0].Velocity", "70"},
          {"Effects[0].Alpha", "0"},
          {"Effects[0].Light[0]", "1"},
          {"Effects[0].Light[1]", "1"},
          {"Effects[0].Light[2]", "1"}}},
        {MODEL_WARP3,
         {{"Effects[0].LifeTime", "16777215"}, {"Effects[0].Scale", "0.600000024"}, {"Effects[0].BlendMesh", "-2"}}},
        {MODEL_WARP6,
         {{"Effects[0].LifeTime", "16777215"}, {"Effects[0].Scale", "0.600000024"}, {"Effects[0].BlendMesh", "-2"}}},
        {BITMAP_SPARK + 1, {{"Effects[0].LifeTime", "10"}}},
        {BITMAP_SPARK + 2, {{"Effects[0].LifeTime", "100"}}},
        {MODEL_1_STREAMBREATHFIRE, {{"Effects[0].LifeTime", "30"}}},
        {MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2,
         {{"Effects[0].LifeTime", "20"}, {"Effects[0].Scale", "0.899999976"}}},
        {MODEL_EFFECT_SD_AURA, {{"Effects[0].LifeTime", "1000"}, {"Effects[0].Scale", "1"}}},
    };
    REQUIRE(expected.size() == Fx13Types.size());

    BuildShippedRegistry();
    for (const Expected& type : expected)
    {
        for (const Conditions& conditions : AllConditions())
        {
            const EffectCall call = CallOf(type.type);
            INFO(Describe(call, conditions));
            const Record record = RecordCall(call, conditions);
            CHECK(Find(record, "Effects[0].Live") == "true");
            for (const RecordedValue& value : type.values)
            {
                INFO(value.path);
                CHECK(Find(record, value.path) == value.value);
            }
        }
    }

    // With a scale from the caller: the types whose cases set a scale keep
    // theirs, the others keep the caller's.
    const std::vector<RecordedValue> scaleFromCaller = {
        {"MODEL_KENTAUROS_ARROW", "0.699999988"},
        {"MODEL_WARP3", "0.600000024"},
        {"MODEL_WARP6", "0.600000024"},
        {"BITMAP_SPARK+1", "2.5"},
        {"BITMAP_SPARK+2", "2.5"},
        {"MODEL_1_STREAMBREATHFIRE", "2.5"},
        {"MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2", "0.899999976"},
        {"MODEL_EFFECT_SD_AURA", "1"},
    };
    REQUIRE(scaleFromCaller.size() == Fx13Types.size());
    for (size_t i = 0; i < Fx13Types.size(); ++i)
    {
        EffectCall call = CallOf(Fx13Types[i]);
        call.scale = 2.5f;
        INFO(scaleFromCaller[i].path);
        CHECK(Find(RecordCall(call, {}), "Effects[0].Scale") == scaleFromCaller[i].value);
    }
}

namespace
{
// The digests of the records of the types a phase moved, one file per phase.
// Set MU_EFFECT_RECORDER_WRITE=1 to write the files anew from the current code.
std::filesystem::path RecordingsOf(const char* phase)
{
    return std::filesystem::path(MU_EFFECT_RECORDINGS_DIR) / (std::string(phase) + ".txt");
}

std::map<std::string, std::string> ReadDigests(const std::filesystem::path& file)
{
    std::map<std::string, std::string> digests;
    std::ifstream in(file);
    std::string line;
    while (std::getline(in, line))
    {
        const size_t tab = line.rfind('\t');
        if (line.empty() || line[0] == '#' || tab == std::string::npos)
        {
            continue;
        }
        digests[line.substr(0, tab)] = line.substr(tab + 1);
    }
    return digests;
}

// Checks the digests of `records` against the file of `phase`, or writes them
// there with MU_EFFECT_RECORDER_WRITE=1, below the comment `header`.
void CheckDigests(const std::vector<Recorded>& records, const char* phase, const char* header)
{
    const std::filesystem::path file = RecordingsOf(phase);
    if (std::getenv("MU_EFFECT_RECORDER_WRITE") != nullptr)
    {
        std::ofstream out(file, std::ios::binary);
        out << header;
        for (const Recorded& recorded : records)
        {
            out << recorded.description << '\t' << Digest(recorded.record) << '\n';
        }
        MESSAGE("wrote " << file.string());
        return;
    }

    const std::map<std::string, std::string> expected = ReadDigests(file);
    REQUIRE(expected.size() == records.size());
    for (const Recorded& recorded : records)
    {
        INFO(recorded.description);
        const auto digest = expected.find(recorded.description);
        REQUIRE(digest != expected.end());
        INFO(ToText(recorded.record));
        CHECK(digest->second == std::to_string(Digest(recorded.record)));
    }
}
} // namespace

// The whole record of every call, not only the values the spot checks name:
// a change anywhere (another field, another slot, a sound, a trail) fails.
TEST_CASE("The 32 types of FX1.2 give the records of their rows [effects][recorder]")
{
    BuildShippedRegistry();
    CheckDigests(RecordAll(Fx12Types), "FX1.2",
                 "# The digests of the records of the 32 types FX1.2 moved into the catalogue, one\n"
                 "# line per call (tests/effects/test_effect_creation.cpp). Taken with their rows\n"
                 "# before FX1.4 added creation fields; written with MU_EFFECT_RECORDER_WRITE=1.\n");
}

TEST_CASE("The 8 types of FX1.3 give the records of their old cases [effects][recorder]")
{
    BuildShippedRegistry();
    CheckDigests(RecordAll(Fx13Types), "FX1.3",
                 "# The digests of the records of the 8 types FX1.3 moved into the catalogue, one\n"
                 "# line per call (tests/effects/test_effect_creation.cpp). Taken with their old\n"
                 "# cases; written with MU_EFFECT_RECORDER_WRITE=1.\n");
}

namespace
{
constexpr const char* Fx14Header = "# The digests of the records of the 29 types of FX1.4, one line per call\n"
                                   "# (tests/effects/test_effect_creation.cpp), with a second position, angle and\n"
                                   "# light. Taken with their old cases; written with MU_EFFECT_RECORDER_WRITE=1.\n";
} // namespace

TEST_CASE("The 29 types of FX1.4 give the records of their old cases [effects][recorder]")
{
    BuildShippedRegistry();
    CheckDigests(RecordAll(Fx14Types, SecondGeometryCallsFor), "FX1.4", Fx14Header);
}

// TEMPORARY: the next commit deletes the cases and this test. With
// MU_EFFECT_RECORDER_WRITE=1 it writes the digests of FX1.4 from the cases.
TEST_CASE("TEMP: the rows of FX1.4 give the records of their cases [effects][recorder]")
{
    // At 60 frames per second the frame factor is 25/60, where a product with
    // it is rounded, unlike at 1 and 0.5.
    const Conditions frameRate[] = {{25.f / 60.f, SlotPattern::A}, {25.f / 60.f, SlotPattern::B}};

    BuildShippedRegistry(Fx14RowTypes);
    const std::vector<Recorded> cases = RecordAll(Fx14Types, SecondGeometryCallsFor);
    const std::vector<Recorded> casesAtFrameRate = RecordAll(Fx14Types, SecondGeometryCallsFor, frameRate);
    if (std::getenv("MU_EFFECT_RECORDER_WRITE") != nullptr)
    {
        CheckDigests(cases, "FX1.4", Fx14Header);
    }

    BuildShippedRegistry();
    const std::vector<Recorded> rows = RecordAll(Fx14Types, SecondGeometryCallsFor);
    const std::vector<Recorded> rowsAtFrameRate = RecordAll(Fx14Types, SecondGeometryCallsFor, frameRate);

    std::ostringstream log;
    const int differing = CompareAll(cases, rows, log);
    const int differingAtFrameRate = CompareAll(casesAtFrameRate, rowsAtFrameRate, log);
    INFO(log.str());
    CHECK(differing == 0);
    CHECK(differingAtFrameRate == 0);
    MESSAGE("FX1.4 cases against the rows: " << cases.size() << " calls, " << differing
                                             << " differ; at 25/60: " << casesAtFrameRate.size() << " calls, "
                                             << differingAtFrameRate << " differ");
}
