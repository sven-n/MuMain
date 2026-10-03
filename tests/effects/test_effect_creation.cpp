#include "stdafx.h"

#include "doctest.h"

#include "EffectRecorder.h"

#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"

#include <array>
#include <sstream>
#include <string>
#include <vector>

using namespace EffectRecorder;
using Data::Effects::EffectCreateParams;
using Data::Effects::EffectTypeCreateParams;

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
}
