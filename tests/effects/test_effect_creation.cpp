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
// The 8 types whose creation cases only set fields of CreateParams; FX1.3
// moved them into the catalogue.
const std::array<int, 8> CatalogueCreatedTypes = {MODEL_KENTAUROS_ARROW,
                                                  MODEL_WARP3,
                                                  MODEL_WARP6,
                                                  BITMAP_SPARK + 1,
                                                  BITMAP_SPARK + 2,
                                                  MODEL_1_STREAMBREATHFIRE,
                                                  MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2,
                                                  MODEL_EFFECT_SD_AURA};

// Sub types the callers pass (0 to 3) and one no case handles.
constexpr std::initializer_list<int> RecordedSubTypes = {0, 1, 2, 3, 99};

struct Recorded
{
    std::string description;
    Record record;
};

std::vector<Recorded> RecordAll(std::span<const int> types)
{
    std::vector<Recorded> all;
    for (const int type : types)
    {
        for (const EffectCall& call : CallsFor(type, RecordedSubTypes))
        {
            for (const Conditions& conditions : AllConditions())
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
    REQUIRE(expected.size() == CatalogueCreatedTypes.size());

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
    REQUIRE(scaleFromCaller.size() == CatalogueCreatedTypes.size());
    for (size_t i = 0; i < CatalogueCreatedTypes.size(); ++i)
    {
        EffectCall call = CallOf(CatalogueCreatedTypes[i]);
        call.scale = 2.5f;
        INFO(scaleFromCaller[i].path);
        CHECK(Find(RecordCall(call, {}), "Effects[0].Scale") == scaleFromCaller[i].value);
    }
}

namespace
{
// The digests of the records of the 8 FX1.3 types for every call RecordAll
// makes, taken with their old cases (the commit before the one that deleted
// them). Set MU_EFFECT_RECORDER_WRITE=1 to write the file anew from the
// current code.
const std::filesystem::path CatalogueCreatedRecords =
    std::filesystem::path(MU_EFFECT_RECORDINGS_DIR) / "CatalogueCreatedTypes.txt";

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
} // namespace

// The whole record of every call, not only the values the spot checks name:
// a change anywhere (another field, another slot, a sound, a trail) fails.
TEST_CASE("The 8 types of FX1.3 give the records of their old cases [effects][recorder]")
{
    BuildShippedRegistry();
    const std::vector<Recorded> records = RecordAll(CatalogueCreatedTypes);

    if (std::getenv("MU_EFFECT_RECORDER_WRITE") != nullptr)
    {
        std::ofstream out(CatalogueCreatedRecords, std::ios::binary);
        out << "# The digests of the records of the 8 types FX1.3 moved into the catalogue, one\n"
               "# line per call (tests/effects/test_effect_creation.cpp). Taken with their old\n"
               "# cases; written with MU_EFFECT_RECORDER_WRITE=1.\n";
        for (const Recorded& recorded : records)
        {
            out << recorded.description << '\t' << Digest(recorded.record) << '\n';
        }
        MESSAGE("wrote " << CatalogueCreatedRecords.string());
        return;
    }

    const std::map<std::string, std::string> expected = ReadDigests(CatalogueCreatedRecords);
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
