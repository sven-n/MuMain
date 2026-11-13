#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MapData/LightMood.h"
#include "Data/GameData/MapData/LookProfile.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Maps::BuiltInLookValues;
using Data::Maps::LookProfileData;
using Data::Maps::LookValue;
using Data::Maps::LookValues;

namespace
{
constexpr int Lorencia = 0;
constexpr int Dungeon = 1;
constexpr int Noria = 3;
constexpr int Icarus = 10;

struct ReadResult
{
    LookProfileData data;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Maps::ReadLookProfileJson(text, "Look.json", result.data, result.issues);
    return result;
}

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

bool Mentions(const std::vector<ItemDataIssue>& issues, const std::string& text)
{
    return Messages(issues).find(text) != std::string::npos;
}

// A rejected file leaves no partial profile: every world gets the built-in values.
void CheckBuiltInEverywhere(const ReadResult& result)
{
    CHECK(HasFatal(result.issues));
    CHECK(result.data == LookProfileData{});
    CHECK(result.data.ForWorld(Lorencia) == BuiltInLookValues());
    CHECK(result.data.ForWorld(Dungeon) == BuiltInLookValues());
}

const std::string ValidFile = R"({
  "formatVersion": 1,
  "default": { "lighting.rim.strength": 0.4, "bloom.strength": 0.9 },
  "maps": [ { "world": 1, "name": "Dungeon", "grading.strength": 0.8, "bloom.strength": 0.5 } ]
})";
} // namespace

TEST_CASE("every look value has a built-in value inside its range [data][look]")
{
    const LookValues builtIn = BuiltInLookValues();
    for (const Data::Maps::LookValueInfo& info : Data::Maps::LookValueInfos())
    {
        CAPTURE(info.name);
        CHECK(info.min <= info.max);
        CHECK(Data::Maps::IsInRange(info, builtIn[info.value]));
        REQUIRE(Data::Maps::FindLookValue(info.name).has_value());
        CHECK(*Data::Maps::FindLookValue(info.name) == info.value);
    }
    CHECK_FALSE(Data::Maps::FindLookValue("bloom.strenght").has_value());
}

TEST_CASE("a look profile file gives the default and each map's values [data][look]")
{
    const ReadResult result = Read(ValidFile);

    CHECK_MESSAGE(result.issues.empty(), Messages(result.issues));
    REQUIRE(result.data.maps.size() == 1);

    // Lorencia has no entry: built-in, then the default.
    const LookValues lorencia = result.data.ForWorld(Lorencia);
    CHECK(lorencia[LookValue::RimStrength] == doctest::Approx(0.4));
    CHECK(lorencia[LookValue::BloomStrength] == doctest::Approx(0.9));
    CHECK(lorencia[LookValue::GradingStrength] == doctest::Approx(1.0));
    CHECK(lorencia[LookValue::DiffuseScale] == doctest::Approx(0.8));
}

TEST_CASE("a map entry overrides only the values it names [data][look]")
{
    const ReadResult result = Read(ValidFile);
    REQUIRE_MESSAGE(result.issues.empty(), Messages(result.issues));

    const LookValues dungeon = result.data.ForWorld(Dungeon);
    // The map's own values.
    CHECK(dungeon[LookValue::GradingStrength] == doctest::Approx(0.8));
    CHECK(dungeon[LookValue::BloomStrength] == doctest::Approx(0.5));
    // The default's value where the map names none, the built-in value where neither does.
    CHECK(dungeon[LookValue::RimStrength] == doctest::Approx(0.4));
    CHECK(dungeon[LookValue::DynamicWrap] == doctest::Approx(0.5));

    // Another map keeps the default's value.
    CHECK(result.data.ForWorld(Noria)[LookValue::BloomStrength] == doctest::Approx(0.9));
}

TEST_CASE("a file without values gives the built-in values [data][look]")
{
    const ReadResult result = Read(R"({ "formatVersion": 1 })");

    CHECK_MESSAGE(result.issues.empty(), Messages(result.issues));
    CHECK(result.data.ForWorld(Lorencia) == BuiltInLookValues());
}

TEST_CASE("an unknown name makes the whole file unusable [data][look]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "bloom.strength": 0.9, "bloom.strenght": 0.9 }
    })");

    CHECK_MESSAGE(Mentions(result.issues, "Look.json: default.bloom.strenght: is not a look value"),
                  Messages(result.issues));
    CheckBuiltInEverywhere(result);
}

TEST_CASE("a mood value in a format 1 file makes the file unusable, on every layer [data][look]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "bloom.strength": 0.9, "mood.base": 0.6 }
    })");
    CHECK_MESSAGE(Mentions(result.issues, "Look.json: default.mood.base: is a format 2 value"),
                  Messages(result.issues));
    CheckBuiltInEverywhere(result);

    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "environments": { "dim": { "mood.sun": 0.1 } } })"));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": [ { "world": 1, "mood.figures": 0.9 } ] })"));
}

TEST_CASE("an unknown name in a map entry or at the top makes the file unusable [data][look]")
{
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": [ { "world": 1, "grading": 0.5 } ] })"));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "defaults": {} })"));
}

TEST_CASE("a value of the wrong type makes the whole file unusable [data][look]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "bloom.strength": "0.6" }
    })");

    CHECK_MESSAGE(Mentions(result.issues, "default.bloom.strength: must be a number"), Messages(result.issues));
    CheckBuiltInEverywhere(result);
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "default": [] })"));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": {} })"));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": [ { "world": 1, "name": 1 } ] })"));
}

TEST_CASE("a value out of range makes the whole file unusable [data][look]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "bloom.strength": 0.6 },
      "maps": [ { "world": 1, "grading.strength": -0.05 } ]
    })");

    CHECK_MESSAGE(Mentions(result.issues, "Look.json: maps[map 1].grading.strength: is -0.05, must be from 0 to 1"),
                  Messages(result.issues));
    CheckBuiltInEverywhere(result);

    // The range ends themselves are valid.
    const ReadResult ends = Read(R"({
      "formatVersion": 1,
      "default": { "dynamic.wrap": 0, "lighting.highlightKnee": 0.9, "bloom.levelFalloff": 0.25 },
      "maps": [ { "world": 1, "dynamic.wrap": 0.5, "lighting.highlightKnee": 0.5, "bloom.levelFalloff": 1,
                  "grading.strength": 0 } ]
    })");
    CHECK_MESSAGE(ends.issues.empty(), Messages(ends.issues));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "default": { "dynamic.wrap": 0.51 } })"));
    // 0.95 was the knee's end until the range-end check found it burns out bright faces (6.2).
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "default": { "lighting.highlightKnee": 0.95 } })"));
}

TEST_CASE("a missing or unsupported format version makes the file unusable [data][look]")
{
    const ReadResult unsupported = Read(R"({ "formatVersion": 3, "default": { "bloom.strength": 0.9 } })");
    CHECK_MESSAGE(Mentions(unsupported.issues, "formatVersion"), Messages(unsupported.issues));
    CheckBuiltInEverywhere(unsupported);

    CheckBuiltInEverywhere(Read(R"({ "default": { "bloom.strength": 0.9 } })"));
    CheckBuiltInEverywhere(Read("{ not json"));
}

TEST_CASE("two entries for one map make the whole file unusable [data][look]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "maps": [ { "world": 1, "bloom.strength": 0.5 }, { "world": 1, "bloom.strength": 0.7 } ]
    })");

    CHECK_MESSAGE(Mentions(result.issues, "Look.json: maps[map 1]: has map 1, which another entry has too"),
                  Messages(result.issues));
    CheckBuiltInEverywhere(result);
}

TEST_CASE("a world outside 0 to 255 makes the whole file unusable [data][look]")
{
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": [ { "world": 256 } ] })"));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": [ { "world": -1 } ] })"));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": [ { "world": 1.5 } ] })"));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "maps": [ { "name": "Lorencia" } ] })"));
}

TEST_CASE("a missing look profile file gives the built-in values [data][look]")
{
    const Data::Maps::LookProfileLoadResult missing =
        Data::Maps::LoadLookProfileFile(std::filesystem::temp_directory_path() / "mu_no_such_look_profile.json");

    CHECK_FALSE(HasFatal(missing.issues));
    CHECK_MESSAGE(Mentions(missing.issues, "not found"), Messages(missing.issues));
    CHECK(missing.data.ForWorld(Lorencia) == BuiltInLookValues());
}

#ifndef _WIN32
TEST_CASE("a look profile file whose existence cannot be checked is an error, not a missing file [data][look]")
{
    // A file name longer than any file system allows: the existence check itself fails (ENAMETOOLONG).
    const std::filesystem::path file = std::filesystem::temp_directory_path() / (std::string(300, 'x') + ".json");
    const Data::Maps::LookProfileLoadResult loaded = Data::Maps::LoadLookProfileFile(file);

    CHECK(HasFatal(loaded.issues));
    CHECK_MESSAGE(Mentions(loaded.issues, "could not be read"), Messages(loaded.issues));
    CHECK_FALSE(Mentions(loaded.issues, "not found"));
    CHECK(loaded.data.ForWorld(Lorencia) == BuiltInLookValues());
}
#endif

TEST_CASE("a look profile file is read from disk [data][look]")
{
    const std::filesystem::path file = std::filesystem::temp_directory_path() / "mu_look_profile_test.json";
    {
        std::ofstream out(file, std::ios::binary | std::ios::trunc);
        out << "\xEF\xBB\xBF" << ValidFile;
    }
    const Data::Maps::LookProfileLoadResult loaded = Data::Maps::LoadLookProfileFile(file);
    std::filesystem::remove(file);

    CHECK_MESSAGE(loaded.issues.empty(), Messages(loaded.issues));
    CHECK(loaded.data.ForWorld(Dungeon)[LookValue::GradingStrength] == doctest::Approx(0.8));
}

TEST_CASE("the shipped look profile holds the tuned Enhanced values [data][look]")
{
    // MU_TEST_DATA_DIR is the client folder's Data; the file name starts with it.
    const std::filesystem::path file =
        std::filesystem::path(MU_TEST_DATA_DIR).parent_path() / Data::Maps::LookProfileFile;
    const Data::Maps::LookProfileLoadResult loaded = Data::Maps::LoadLookProfileFile(file);

    CHECK_MESSAGE(loaded.issues.empty(), Messages(loaded.issues));
    // The tuned Enhanced look; unlisted values stay at their built-in value.
    const std::pair<LookValue, double> accepted[] = {
        {LookValue::DiffuseScale, 1.0},        {LookValue::DiffuseAmbient, 0.3},
        {LookValue::DiffuseFloor, 0.15},       {LookValue::RimStrength, 0.4},
        {LookValue::DynamicWrap, 0.3},         {LookValue::DynamicMapSmoothing, 0.5},
        {LookValue::DynamicEffectGain, 0.5},   {LookValue::DynamicEffectRange, 2.0},
        {LookValue::DynamicEffectRangeMax, 8}, {LookValue::DynamicEffectFade, 0.5},
        {LookValue::DynamicEffectCap, 2.5},    {LookValue::BloomStrength, 1.0},
        {LookValue::BloomLevelFalloff, 0.6},   {LookValue::GroundShadowAngle, 135},
        {LookValue::GroundShadowLights, 1},    {LookValue::AmbientOcclusionStrength, 0.8},
        {LookValue::SpecularScale, 0.8},       {LookValue::SunShadowStrength, 1.0},
        {LookValue::CloudStrength, 0.35},      {LookValue::SkyFill, 0.5},
        {LookValue::RimSunFollow, 1.0},
    };
    for (const Data::Maps::LookValueInfo& info : Data::Maps::LookValueInfos())
    {
        CAPTURE(info.name);
        if (Data::Maps::IsMoodValue(info.value))
        {
            continue;
        }
        // The file names every value but the mood's, which name only what differs from Classic.
        REQUIRE(loaded.data.defaultOverrides[info.value].has_value());
        double expected = info.builtIn;
        for (const auto& [value, number] : accepted)
        {
            if (value == info.value)
            {
                expected = number;
            }
        }
        CHECK(*loaded.data.defaultOverrides[info.value] == doctest::Approx(expected));
    }
    // Icarus: less bloom, which widened its fog clouds.
    CHECK(loaded.data.ForWorld(Icarus)[LookValue::BloomStrength] == doctest::Approx(0.3));
    CHECK(loaded.data.ForWorld(Lorencia)[LookValue::BloomStrength] == doctest::Approx(1.0));
    // Indoor and underground maps: no sun, and no flat sun shadow under characters.
    for (const int world : {1, 4, 38, 39})
    {
        CAPTURE(world);
        CHECK(loaded.data.ForWorld(world)[LookValue::SunShadowStrength] == 0.0);
        CHECK(loaded.data.ForWorld(world)[LookValue::FlatSunShadow] == 0.0);
    }
    CHECK(loaded.data.ForWorld(Lorencia)[LookValue::FlatSunShadow] == 1.0);
    // The tuned light mood: base, figures, dynamicGain, background, sun, shadowKeep, shadeCool, brightScale.
    using Data::Maps::LightMood;
    using Data::Maps::MoodOf;
    CHECK(MoodOf(loaded.data.ForWorld(Lorencia)) == LightMood{0.6, 0.7, 1.7, 1.0, 0.8, 0.8, 0.1, 0.8});
    CHECK(MoodOf(loaded.data.ForWorld(Icarus)) == LightMood{0.7, 0.7, 1.7, 0.7, 0.8, 0.8, 0.1, 0.95});
    CHECK(MoodOf(loaded.data.ForWorld(4)) == LightMood{0.45, 0.7, 1.7, 1.0, 1.0, 0.0, 0.1, 0.8});
    CHECK(MoodOf(loaded.data.ForWorld(7)) == LightMood{0.8, 0.8, 1.7, 1.0, 1.0, 0.8, 0.1, 1.0});
}

TEST_CASE("a map's environment applies between the default and the map's own values [data][look]")
{
    const ReadResult result = Read(R"({"formatVersion": 1,
        "default": {"bloom.strength": 1.0, "sun.shadow": 1},
        "environments": {"enclosed": {"sun.shadow": 0, "bloom.strength": 0.8}},
        "maps": [{"world": 1, "environment": "enclosed", "bloom.strength": 0.5},
                 {"world": 10, "environment": "enclosed"}]})");
    REQUIRE_MESSAGE(!HasFatal(result.issues), Messages(result.issues));
    CHECK(result.data.ForWorld(Lorencia)[LookValue::SunShadowStrength] == 1.0);
    CHECK(result.data.ForWorld(Dungeon)[LookValue::SunShadowStrength] == 0.0);
    // The map's own value wins over its environment's.
    CHECK(result.data.ForWorld(Dungeon)[LookValue::BloomStrength] == doctest::Approx(0.5));
    CHECK(result.data.ForWorld(Icarus)[LookValue::BloomStrength] == doctest::Approx(0.8));
}

TEST_CASE("an unknown environment makes the file unusable [data][look]")
{
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 1, "maps": [{"world": 1, "environment": "cave"}]})"));
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 1, "environments": {"cave": 1}})"));
}

namespace
{
std::string ReadText(const std::filesystem::path& file)
{
    std::ifstream stream(file, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

std::filesystem::path ShippedFile()
{
    return std::filesystem::path(MU_TEST_DATA_DIR).parent_path() / Data::Maps::LookProfileFile;
}
} // namespace

TEST_CASE("the light mood's values are look values with Classic built-ins and the mood file's ranges [data][look]")
{
    const auto& moods = Data::Maps::LightMoodValueInfos();
    for (std::size_t index = 0; index < moods.size(); ++index)
    {
        const Data::Maps::LookValueInfo& info = Data::Maps::Info(Data::Maps::MoodLookValue(index));
        CAPTURE(info.name);
        CHECK(std::string(info.name) == std::string("mood.") + moods[index].key);
        CHECK(Data::Maps::IsMoodValue(info.value));
        CHECK(info.min == moods[index].min);
        CHECK(info.max == moods[index].max);
    }
    CHECK_FALSE(Data::Maps::IsMoodValue(LookValue::RimSunFollow));
    CHECK(Data::Maps::MoodOf(BuiltInLookValues()) == Data::Maps::LightMood{});
}

TEST_CASE("a format 2 entry lists the maps it is for [data][look]")
{
    const ReadResult result = Read(R"({"formatVersion": 2,
        "default": {"bloom.strength": 1, "mood.base": 0.6, "mood.figures": 0.7},
        "environments": {"fixed": {"mood.sun": 1}},
        "maps": [{"maps": [11, 12, 52], "name": "Blood Castle", "environment": "fixed", "bloom.strength": 0.5}]})");
    REQUIRE_MESSAGE(!HasFatal(result.issues), Messages(result.issues));
    for (const int world : {11, 12, 52})
    {
        CAPTURE(world);
        CHECK(result.data.ForWorld(world)[LookValue::BloomStrength] == doctest::Approx(0.5));
        CHECK(result.data.ForWorld(world)[LookValue::MoodSun] == 1.0);
    }
    CHECK(result.data.ForWorld(13)[LookValue::BloomStrength] == 1.0);
    CHECK(result.data.maps[0].name == "Blood Castle");

    // A format 2 entry has "maps", not "world"; every number from 0 to 255; a map in one entry only.
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 2, "maps": [{"world": 1}]})"));
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 2, "maps": [{"maps": []}]})"));
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 2, "maps": [{"maps": [-1]}]})"));
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 2, "maps": [{"maps": [256]}]})"));
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 2, "maps": [{"maps": [1.5]}]})"));
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 2, "maps": [{"maps": 1}]})"));
    const ReadResult twice = Read(R"({"formatVersion": 2, "maps": [{"maps": [1, 2]}, {"maps": [3, 2]}]})");
    CHECK_MESSAGE(Mentions(twice.issues, "maps[map 3]: has map 2, which another entry has too"),
                  Messages(twice.issues));
    CheckBuiltInEverywhere(twice);
    const ReadResult listedTwice = Read(R"({"formatVersion": 2, "maps": [{"maps": [4, 4]}]})");
    CHECK_MESSAGE(Mentions(listedTwice.issues, "lists map 4 twice"), Messages(listedTwice.issues));
    CheckBuiltInEverywhere(Read(R"({"formatVersion": 2, "maps": [{"maps": [1], "environment": "fixed"}]})"));
}

TEST_CASE("mood figures below base make the file unusable, on every layer [data][look]")
{
    // The default.
    const ReadResult defaults = Read(R"({"formatVersion": 2, "default": {"mood.base": 0.8, "mood.figures": 0.7}})");
    CHECK_MESSAGE(Mentions(defaults.issues, "default.mood.figures: is 0.7, must not be below mood.base 0.8"),
                  Messages(defaults.issues));
    CheckBuiltInEverywhere(defaults);
    // An environment that names only figures, under a default with a higher base: no map uses it.
    const ReadResult environment = Read(R"({"formatVersion": 2, "default": {"mood.base": 0.8, "mood.figures": 0.9},
        "environments": {"dim": {"mood.figures": 0.75}}})");
    CHECK_MESSAGE(Mentions(environment.issues, "environments.dim.mood.figures"), Messages(environment.issues));
    CheckBuiltInEverywhere(environment);
    // A map, over its environment.
    const ReadResult map = Read(R"({"formatVersion": 2, "default": {"mood.base": 0.6, "mood.figures": 0.7},
        "environments": {"bright": {"mood.figures": 0.9}},
        "maps": [{"maps": [1], "environment": "bright", "mood.base": 0.95}]})");
    CHECK_MESSAGE(Mentions(map.issues, "maps[map 1].mood.figures"), Messages(map.issues));
    CheckBuiltInEverywhere(map);
    // A map that lowers base and figures under the default's is fine.
    const ReadResult lowered = Read(R"({"formatVersion": 2, "default": {"mood.base": 0.8, "mood.figures": 0.8},
        "maps": [{"maps": [1], "mood.base": 0.5, "mood.figures": 0.5}]})");
    CHECK_MESSAGE(lowered.issues.empty(), Messages(lowered.issues));
}

TEST_CASE("the writer gives one text per profile, in its fixed order [data][look]")
{
    const std::string canonical = R"({
  "formatVersion": 2,
  "default": {
    "bloom.strength": 0.9,
    "mood.base": 0.6
  },
  "environments": {
    "enclosed": {"sun.shadow": 0, "mood.base": 0.45},
    "outdoor": {}
  },
  "maps": [
    {"maps": [0], "name": "Lorencia", "environment": "outdoor"},
    {"maps": [11, 12, 52], "name": "Blood Castle", "environment": "enclosed", "grading.strength": 0.8}
  ]
}
)";
    const ReadResult read = Read(canonical);
    REQUIRE_MESSAGE(read.issues.empty(), Messages(read.issues));
    CHECK(Data::Maps::WriteLookProfileJson(read.data) == canonical);

    // Any order in, the fixed order out.
    const ReadResult shuffled = Read(R"({"maps": [
        {"grading.strength": 0.8, "environment": "enclosed", "name": "Blood Castle", "maps": [52, 11, 12]},
        {"environment": "outdoor", "maps": [0], "name": "Lorencia"}],
      "environments": {"outdoor": {}, "enclosed": {"mood.base": 0.45, "sun.shadow": 0}},
      "default": {"mood.base": 0.6, "bloom.strength": 0.9}, "formatVersion": 2})");
    REQUIRE_MESSAGE(shuffled.issues.empty(), Messages(shuffled.issues));
    CHECK(Data::Maps::WriteLookProfileJson(shuffled.data) == canonical);

    CHECK(Data::Maps::WriteLookProfileJson(LookProfileData{}) ==
          "{\n  \"formatVersion\": 2,\n  \"default\": {},\n  \"environments\": {},\n  \"maps\": []\n}\n");
}

TEST_CASE("a format 1 light mood upgrades onto the profile, keeping every value a map named [data][look]")
{
    const ReadResult look = Read(R"({"formatVersion": 1,
        "default": {"bloom.strength": 0.9},
        "maps": [{"world": 1, "name": "Dungeon", "grading.strength": 0.8}]})");
    REQUIRE_MESSAGE(look.issues.empty(), Messages(look.issues));
    Data::Maps::LightMoodData moods;
    std::vector<ItemDataIssue> moodIssues;
    Data::Maps::ReadLightMoodJson(R"({"formatVersion": 1,
        "default": {"base": 0.6, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 0.8},
        "maps": [{"world": 1, "sun": 0.3}, {"world": 10, "base": 0.6, "background": 0.7}]})",
                                  "LightMood.json", moods, moodIssues);
    REQUIRE_MESSAGE(moodIssues.empty(), Messages(moodIssues));

    LookProfileData upgraded = look.data;
    Data::Maps::UpgradeLightMood(upgraded, moods);
    std::vector<ItemDataIssue> issues;
    REQUIRE(Data::Maps::ValidateLookProfile(upgraded, "Look.json", issues));
    for (int world = 0; world <= Data::Maps::MaxLookProfileWorld; ++world)
    {
        CAPTURE(world);
        CHECK(Data::Maps::MoodOf(upgraded.ForWorld(world)) == moods.ForWorld(world));
        LookValues lookOnly = upgraded.ForWorld(world);
        const LookValues before = look.data.ForWorld(world);
        for (std::size_t index = 0; index < Data::Maps::LightMoodValueCount; ++index)
        {
            lookOnly[Data::Maps::MoodLookValue(index)] = before[Data::Maps::MoodLookValue(index)];
        }
        CHECK(lookOnly == before);
    }
    // Icarus named base 0.6, the default's value: it keeps it when the default changes.
    upgraded.defaultOverrides[LookValue::MoodBase] = 0.65;
    CHECK(upgraded.ForWorld(Icarus)[LookValue::MoodBase] == doctest::Approx(0.6));
    CHECK(upgraded.ForWorld(Lorencia)[LookValue::MoodBase] == doctest::Approx(0.65));
}

TEST_CASE("the shipped look profile gives every map what the last two files gave [data][look]")
{
    const Data::Maps::LookProfileLoadResult shipped = Data::Maps::LoadLookProfileFile(ShippedFile());
    REQUIRE_MESSAGE(shipped.issues.empty(), Messages(shipped.issues));

    std::vector<ItemDataIssue> issues;
    LookProfileData look;
    int version = 0;
    Data::Maps::ReadLookProfileJson(ReadText(std::filesystem::path(MU_TEST_LOOK_V1_DIR) / "Look.json"), "Look.json",
                                    look, issues, &version);
    Data::Maps::LightMoodData moods;
    Data::Maps::ReadLightMoodJson(ReadText(std::filesystem::path(MU_TEST_LOOK_V1_DIR) / "LightMood.json"),
                                  "LightMood.json", moods, issues);
    REQUIRE_MESSAGE(!HasFatal(issues), Messages(issues));
    CHECK(version == 1);

    for (int world = 0; world <= Data::Maps::MaxLookProfileWorld; ++world)
    {
        CAPTURE(world);
        const LookValues now = shipped.data.ForWorld(world);
        const LookValues before = look.ForWorld(world);
        for (const Data::Maps::LookValueInfo& info : Data::Maps::LookValueInfos())
        {
            if (!Data::Maps::IsMoodValue(info.value))
            {
                CAPTURE(info.name);
                CHECK(now[info.value] == before[info.value]);
            }
        }
        CHECK(Data::Maps::MoodOf(now) == moods.ForWorld(world));
    }
}

TEST_CASE("the shipped look profile saved unchanged is the same text [data][look]")
{
    const std::string text = ReadText(ShippedFile());
    const ReadResult read = Read(text);
    REQUIRE_MESSAGE(read.issues.empty(), Messages(read.issues));
    CHECK(Data::Maps::WriteLookProfileJson(read.data) == text);
}
