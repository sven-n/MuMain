#include "stdafx.h"

#include "doctest.h"

#include "EffectRecorder.h"
#include "EffectTestData.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/EffectData/EffectKind.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Render/Effects/EffectRegistry.h"

#include <algorithm>
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
const std::array<int, 8> Fx13Types = {MODEL_KENTAUROS_ARROW,
                                      MODEL_WARP3,
                                      MODEL_WARP6,
                                      BITMAP_SPARK + 1,
                                      BITMAP_SPARK + 2,
                                      MODEL_1_STREAMBREATHFIRE,
                                      MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2,
                                      MODEL_EFFECT_SD_AURA};

// Sub types the callers pass (0 to 3) and one no case handles.
constexpr std::array<int, 5> RecordedSubTypes = {0, 1, 2, 3, 99};

// More SubTypes for the types whose old case handled them or whose callers
// pass them, so the creation baseline holds what the old case did for them.
const std::map<int, std::vector<int>> MoreRecordedSubTypes = {
    {BITMAP_SKULL, {4, 5}},              // a branch for 4; callers pass 5
    {MODEL_CIRCLE_LIGHT, {4}},           // a branch for 4
    {BITMAP_FIRE_CURSEDLICH, {4, 12}},   // a branch for 12; callers pass 4 and 12
    {MODEL_ALICE_BUFFSKILL_EFFECT, {4}}, // a branch for 4
    {MODEL_CHANGE_UP_NASA, {4}},         // the end of its range of SubTypes 1 to 3
    {MODEL_WINDFOCE, {4, 5}},            // callers pass them
};

std::vector<int> RecordedSubTypesOf(int type)
{
    std::vector<int> subTypes(RecordedSubTypes.begin(), RecordedSubTypes.end());
    if (const auto more = MoreRecordedSubTypes.find(type); more != MoreRecordedSubTypes.end())
    {
        subTypes.insert(subTypes.end(), more->second.begin(), more->second.end());
    }
    return subTypes;
}

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

TEST_CASE("The second geometry of a call shows what a row copies [effects][recorder]")
{
    BuildShippedRegistry();
    // MODEL_INFINITY_ARROW4 sets its light and copies it into the direction;
    // the light of the call stays as it was.
    const std::vector<EffectCall> calls = SecondGeometryCallsFor(MODEL_INFINITY_ARROW4, std::array{0});
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
// The creation baseline (tests/effects/baseline/EffectCreation.txt): what
// CreateEffect does for every effect type whose creation moved from code into
// the catalogue, as their old code did it. One line per recorded call: the
// name of the type, the call, and the digest of its whole record.
const std::filesystem::path CreationBaseline = MU_EFFECT_CREATION_BASELINE;

constexpr const char* CreationBaselineHeader =
    "# What CreateEffect does for the effect types whose creation moved from code into\n"
    "# the effect catalogue: one line per recorded call, the name of the type and the\n"
    "# call, then a digest of its whole record (what tests/effects/EffectRecorder.h\n"
    "# records). Equal to their old code: their rows were compared with it before the\n"
    "# code was deleted. A deliberate change to a type's creation rewrites its lines\n"
    "# (MU_EFFECT_RECORDER_WRITE=1); the file and its test are removed once the\n"
    "# catalogue is edited on purpose (docs/effect-data.md).\n";

// Types whose creation code set nothing and was deleted without a row.
const std::array<int, 1> CreatedWithoutRow = {MODEL_PHOENIX_SHOT};

struct BaselineType
{
    std::string name;
    int type;
    std::vector<int> variantSubTypes;
};

// The types with creation values in the catalogue and the types created
// without a row, sorted by name.
std::vector<BaselineType> BaselineTypes()
{
    using Data::Effects::EffectKind;
    Data::Effects::EffectTypeCatalogue catalogue;
    catalogue.Build(EffectKind::Effect,
                    EffectTestData::ShippedTypes().types[Data::Effects::ToIndex(EffectKind::Effect)]);
    std::vector<BaselineType> types;
    for (const EffectTypeCreateParams& row : catalogue.GetCreateParams())
    {
        BaselineType& type = types.emplace_back(
            BaselineType{std::string(catalogue.GetName(EffectKind::Effect, row.type)), row.type, {}});
        for (const Data::Effects::EffectCreateVariant& variant : row.params.variants)
        {
            type.variantSubTypes.insert(type.variantSubTypes.end(), variant.subTypes.begin(), variant.subTypes.end());
        }
    }
    for (const int type : CreatedWithoutRow)
    {
        types.push_back({std::string(catalogue.GetName(EffectKind::Effect, type)), type});
    }
    std::sort(types.begin(), types.end(),
              [](const BaselineType& left, const BaselineType& right) { return left.name < right.name; });
    return types;
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

// The records of the creation baseline, with the registry as it is built.
std::vector<Recorded> RecordBaseline()
{
    std::vector<Recorded> records;
    for (const BaselineType& type : BaselineTypes())
    {
        // Every SubType a variant names is recorded.
        const std::vector<int> subTypes = RecordedSubTypesOf(type.type);
        for (const int subType : type.variantSubTypes)
        {
            INFO(type.name << " subType " << subType);
            CHECK(std::find(subTypes.begin(), subTypes.end(), subType) != subTypes.end());
        }
        for (const EffectCall& call : SecondGeometryCallsFor(type.type, subTypes))
        {
            for (const Conditions& conditions : AllConditions())
            {
                records.push_back(
                    {type.name + " " + DescribeArguments(call, conditions), RecordCall(call, conditions)});
            }
        }
    }
    return records;
}

void WriteBaseline(const std::vector<Recorded>& records)
{
    std::filesystem::create_directories(CreationBaseline.parent_path());
    std::ofstream out(CreationBaseline, std::ios::binary);
    out << CreationBaselineHeader;
    for (const Recorded& recorded : records)
    {
        out << recorded.description << '\t' << Digest(recorded.record) << '\n';
    }
    MESSAGE("wrote " << CreationBaseline.string());
}
} // namespace

// The whole record of every call, not only the values the spot checks name: a
// change anywhere (a row, how rows are applied, the common setup, another
// field, another slot, a sound, a trail) fails, and so does a type that gains
// or loses its row. MU_EFFECT_RECORDER_WRITE=1 writes the file anew from the
// current code.
TEST_CASE("The effect types in the catalogue create what their old code created [effects][recorder]")
{
    BuildShippedRegistry();
    const std::vector<Recorded> records = RecordBaseline();
    if (std::getenv("MU_EFFECT_RECORDER_WRITE") != nullptr)
    {
        WriteBaseline(records);
        return;
    }

    const std::map<std::string, std::string> expected = ReadDigests(CreationBaseline);
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

namespace
{
float RecordedFloat(const Record& record, std::string_view path)
{
    const std::optional<std::string> value = Find(record, path);
    REQUIRE(value.has_value());
    return std::stof(*value);
}
} // namespace

// FX1.4 moved the creation of these types from code into the catalogue; the
// commit before the one that deleted their cases compared both for every sub
// type, owner, argument set, slot pattern and the frame factors 1, 0.5 and
// 25/60, and without an owner also with a second position, angle and light.
// These are values the cases set that the new fields hold (the call's
// position is 13120.25, 12480.5, 140.75, its angle 11, 22, 33 and its light
// 0.9, 0.8, 0.7). The skill values and the PK key of the calls differ from
// the ones the rows set, which the common setup would copy otherwise.
TEST_CASE("The types of FX1.4 create from the catalogue what their cases set [effects][recorder]")
{
    BuildShippedRegistry();

    // Raised by 3400, then copied into the start position; one angle component.
    EffectCall dragonCall = CallOf(MODEL_DRAGON);
    dragonCall.skill = 43;
    const Record dragon = RecordCall(dragonCall, {});
    CHECK(Find(dragon, "Effects[0].Position[2]") == "3540.75");
    CHECK(Find(dragon, "Effects[0].StartPosition[0]") == "13120.25");
    CHECK(Find(dragon, "Effects[0].StartPosition[2]") == "3540.75");
    CHECK(Find(dragon, "Effects[0].Angle[0]") == "11");
    CHECK(Find(dragon, "Effects[0].Angle[1]") == "0");
    CHECK(Find(dragon, "Effects[0].Direction[1]") == "-35");
    CHECK(Find(dragon, "Effects[0].Kind") == "0");
    CHECK(Find(dragon, "Effects[0].Timer") == "0");
    CHECK(Find(dragon, "Effects[0].Distance") == "1");
    CHECK(Find(dragon, "Effects[0].CollisionRange") == "1");

    // Copied, then the copy raised by 800.
    const Record thunder = RecordCall(CallOf(BITMAP_JOINT_THUNDER), {});
    CHECK(Find(thunder, "Effects[0].StartPosition[2]") == "940.75");
    CHECK(Find(thunder, "Effects[0].Position[2]") == "140.75");

    // The light of the call (the summoner passes the target there), then its own light.
    EffectCall neilCall = CallOf(MODEL_SUMMONER_SUMMON_NEIL);
    neilCall.skillIndex = 41;
    const Record neil = RecordCall(neilCall, {});
    CHECK(Find(neil, "Effects[0].HeadTargetAngle[0]") == "0.899999976");
    CHECK(Find(neil, "Effects[0].HeadTargetAngle[2]") == "0.699999988");
    CHECK(Find(neil, "Effects[0].Light[0]") == "1");
    CHECK(Find(neil, "Effects[0].Skill") == "0");

    const Record rise = RecordCall(CallOf(BITMAP_FIRECRACKERRISE), {});
    CHECK(Find(rise, "Effects[0].Position[2]") == "100");
    CHECK(Find(rise, "Effects[0].Angle[0]") == "0");
    CHECK(Find(rise, "Effects[0].Angle[2]") == "0");

    // The call's scale, also when it passes none.
    EffectCall shiny = CallOf(BITMAP_SHINY + 4);
    CHECK(Find(RecordCall(shiny, {}), "Effects[0].Scale") == "0");
    shiny.scale = 1.75f;
    CHECK(Find(RecordCall(shiny, {}), "Effects[0].Scale") == "1.75");
    EffectCall piece = CallOf(MODEL_STATUE_CRUSH_EFFECT_PIECE04);
    piece.pkKey = 37;
    CHECK(Find(RecordCall(piece, {}), "Effects[0].PKKey") == "-1");

    // Offsets times the frame factor, at 25/60 where the products round; the
    // expected values are computed in the form of the old cases. volatile
    // keeps the compiler from computing them while compiling, in another way.
    volatile float frameFactor = 25.f / 60.f;
    const Conditions frameRate{frameFactor, SlotPattern::A};
    const Record staff = RecordCall(CallOf(MODEL_STAFF_OF_DESTRUCTION), frameRate);
    float z = 140.75f;
    z += (280.f) * frameFactor;
    float pitch = 11.f;
    pitch += (20.f) * frameFactor;
    CHECK(RecordedFloat(staff, "Effects[0].Position[2]") == z);
    CHECK(RecordedFloat(staff, "Effects[0].Angle[0]") == pitch);

    const Record cloud = RecordCall(CallOf(MODEL_CLOUD), frameRate);
    float y = 12480.5f;
    y += (200.f) * frameFactor;
    z = 140.75f;
    z -= (190.f) * frameFactor;
    CHECK(RecordedFloat(cloud, "Effects[0].Position[1]") == y);
    CHECK(RecordedFloat(cloud, "Effects[0].Position[2]") == z);
    // The flags of pattern A are false already.
    CHECK(Find(RecordCall(CallOf(MODEL_CLOUD), {1.f, SlotPattern::B}), "Effects[0].LightEnable") == "false");

    // Its case set nothing, so it has no row and creates with the common setup.
    const Render::Effects::EffectDescriptor* phoenix = Render::Effects::Lookup(MODEL_PHOENIX_SHOT);
    CHECK((phoenix == nullptr || !phoenix->create.has_value()));
    const Record shot = RecordCall(CallOf(MODEL_PHOENIX_SHOT), {});
    CHECK(Find(shot, "Effects[0].Live") == "true");
    CHECK_FALSE(Find(shot, "Effects[0].LifeTime").has_value());
}

// FX1.5 moved the creation of these types from code into the catalogue, with
// variants by SubType; the commit before the one that deleted their cases
// compared both for every SubType a caller passes or a branch handles, one no
// branch handles, every owner, argument set, slot pattern, both geometries
// and the frame factors 1, 0.5 and 25/60. These are values the cases chose by
// SubType.
TEST_CASE("The types of FX1.5 create from the catalogue what their cases chose by SubType [effects][recorder]")
{
    BuildShippedRegistry();
    const auto record = [](int type, int subType)
    {
        EffectCall call = CallOf(type);
        call.subType = subType;
        return RecordCall(call, {});
    };

    // The else branch is the row's value; the variants replace some of them.
    CHECK(Find(record(MODEL_MAGIC_CIRCLE1, 0), "Effects[0].Velocity") == "0.100000001");
    CHECK(Find(record(MODEL_MAGIC_CIRCLE1, 0), "Effects[0].LifeTime") == "30");
    CHECK(Find(record(MODEL_MAGIC_CIRCLE1, 1), "Effects[0].LifeTime") == "20");
    CHECK(Find(record(MODEL_MAGIC_CIRCLE1, 1), "Effects[0].HiddenMesh") == "0");
    CHECK(Find(record(MODEL_MAGIC_CIRCLE1, 2), "Effects[0].LifeTime") == "15");
    CHECK(Find(record(MODEL_MAGIC_CIRCLE1, 99), "Effects[0].LifeTime") == "30");

    // A row with only variants: a SubType without one keeps the slot's old
    // lifeTime, as the case did (callers pass 4).
    CHECK(Find(record(BITMAP_FIRE_CURSEDLICH, 12), "Effects[0].LifeTime") == "20");
    CHECK(Find(record(BITMAP_FIRE_CURSEDLICH, 0), "Effects[0].BlendMesh") == "-2");
    CHECK_FALSE(Find(record(BITMAP_FIRE_CURSEDLICH, 4), "Effects[0].LifeTime").has_value());
    CHECK_FALSE(Find(record(MODEL_CHAIN_LIGHTNING, 3), "Effects[0].LifeTime").has_value());

    // A variant with an offset times the frame factor (at 0.5, so the factor
    // shows), one with other values.
    EffectCall swordForceCall = CallOf(MODEL_SWORD_FORCE);
    swordForceCall.subType = 2;
    const Record swordForce = RecordCall(swordForceCall, {0.5f, SlotPattern::A});
    CHECK(Find(swordForce, "Effects[0].Scale") == "0");
    CHECK(Find(swordForce, "Effects[0].Position[2]") == "190.75");
    CHECK(Find(swordForce, "Effects[0].Velocity") == "0.25");
    CHECK(Find(record(MODEL_SWORD_FORCE, 3), "Effects[0].Scale") == "3.5");

    // A variant with a copy of the light it sets (blue 0.2; the call's is 0.7).
    CHECK(Find(record(MODEL_ARROW_AUTOLOAD, 1), "Effects[0].Direction[2]") == "0.200000003");

    // The row copies the call's scale; SubType 5 gets the row's values.
    CHECK(Find(record(MODEL_WINDFOCE, 5), "Effects[0].LifeTime") == "50");
    CHECK(Find(record(MODEL_WINDFOCE, 5), "Effects[0].Scale") == "0");
    CHECK(Find(record(MODEL_WINDFOCE, 1), "Effects[0].LifeTime") == "999");
}

// FX1.5b moved the creation of these types from code into the catalogue with
// the fields it added; the commit before the one that deleted their cases
// compared both for every SubType a caller passes or a branch handles, one no
// branch handles, every owner, argument set, slot pattern, both geometries
// and the frame factors 1, 0.5 and 25/60. These are values the cases set with
// those fields (the call's position is 13120.25, 12480.5, 140.75, its angle
// 11, 22, 33 and its light 0.9, 0.8, 0.7).
TEST_CASE("The types of FX1.5b create from the catalogue what their cases set [effects][recorder]")
{
    BuildShippedRegistry();
    const auto record = [](int type, int subType, float frameFactor = 1.f)
    {
        EffectCall call = CallOf(type);
        call.subType = subType;
        return RecordCall(call, {frameFactor, SlotPattern::A});
    };

    // 1000 minus 60 times the frame factor.
    CHECK(Find(record(BITMAP_SKULL, 1, 0.5f), "Effects[0].LifeTime") == "970");
    CHECK(Find(record(BITMAP_SKULL, 5), "Effects[0].LifeTime") == "1000");

    CHECK(Find(record(BITMAP_OUR_INFLUENCE_GROUND, 0), "Effects[0].AlphaTarget") == "0.75");
    CHECK(Find(record(BITMAP_SHINY + 6, 3), "Effects[0].RenderType") == std::to_string(RENDER_TYPE_ALPHA_BLEND_MINUS));
    CHECK(Find(record(MODEL_CIRCLE_LIGHT, 1), "Effects[0].RenderType") == "128");
    CHECK(Find(record(MODEL_MOONHARVEST_MOON, 0), "Effects[0].m_iAnimation") == "0");
    CHECK(Find(record(BITMAP_CRATER, 0), "Effects[0].StartPosition[0]") == "4.5");

    // The copies: the light, the call's position before the offset, the call's
    // angle before the variant zeroes it.
    CHECK(Find(record(MODEL_MAYAHANDSKILL, 1), "Effects[0].StartPosition[1]") == "0.800000012");
    CHECK(Find(record(BITMAP_TWLIGHT, 3), "Effects[0].EyeRight[2]") == "0.699999988");
    const Record piercing = record(MODEL_PIERCING2, 0);
    CHECK(Find(piercing, "Effects[0].StartPosition[2]") == "140.75");
    CHECK(Find(piercing, "Effects[0].Position[2]") == "270.75");
    const Record moon = record(MODEL_MOONHARVEST_MOON, 1);
    CHECK(Find(moon, "Effects[0].Direction[1]") == "22");
    CHECK(Find(moon, "Effects[0].Angle[1]") == "0");
    CHECK(Find(record(MODEL_ARROW_TANKER_HIT, 2), "Effects[0].m_vDeadPosition[2]") == "33");
}
