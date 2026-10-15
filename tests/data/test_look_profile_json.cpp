#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MapData/LookProfile.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

using Data::Items::HasErrors;
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
    CHECK(HasErrors(result.issues));
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
      "maps": [ { "world": 1, "grading.strength": 0.05 } ]
    })");

    CHECK_MESSAGE(Mentions(result.issues, "Look.json: maps[world 1].grading.strength: is 0.05, must be from 0.1 to 1"),
                  Messages(result.issues));
    CheckBuiltInEverywhere(result);

    // The range ends themselves are valid.
    const ReadResult ends = Read(R"({
      "formatVersion": 1,
      "default": { "dynamic.wrap": 0, "lighting.highlightKnee": 0.9, "bloom.levelFalloff": 0.25 },
      "maps": [ { "world": 1, "dynamic.wrap": 0.5, "lighting.highlightKnee": 0.5, "bloom.levelFalloff": 1,
                  "grading.strength": 0.1 } ]
    })");
    CHECK_MESSAGE(ends.issues.empty(), Messages(ends.issues));
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "default": { "dynamic.wrap": 0.51 } })"));
    // 0.95 was the knee's end until the range-end check found it burns out bright faces (6.2).
    CheckBuiltInEverywhere(Read(R"({ "formatVersion": 1, "default": { "lighting.highlightKnee": 0.95 } })"));
}

TEST_CASE("a missing or unsupported format version makes the file unusable [data][look]")
{
    const ReadResult unsupported = Read(R"({ "formatVersion": 2, "default": { "bloom.strength": 0.9 } })");
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

    CHECK_MESSAGE(Mentions(result.issues, "Look.json: maps[world 1]: is in the list more than once"),
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

    CHECK_FALSE(HasErrors(missing.issues));
    CHECK_MESSAGE(Mentions(missing.issues, "not found"), Messages(missing.issues));
    CHECK(missing.data.ForWorld(Lorencia) == BuiltInLookValues());
}

#ifndef _WIN32
TEST_CASE("a look profile file whose existence cannot be checked is an error, not a missing file [data][look]")
{
    // A file name longer than any file system allows: the existence check itself fails (ENAMETOOLONG).
    const std::filesystem::path file = std::filesystem::temp_directory_path() / (std::string(300, 'x') + ".json");
    const Data::Maps::LookProfileLoadResult loaded = Data::Maps::LoadLookProfileFile(file);

    CHECK(HasErrors(loaded.issues));
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
        {LookValue::DynamicEffectGain, 2.5},   {LookValue::DynamicEffectRange, 3.0},
        {LookValue::DynamicEffectRangeMax, 8}, {LookValue::DynamicEffectFade, 0.5},
        {LookValue::DynamicEffectCap, 2.5},    {LookValue::BloomStrength, 1.0},
        {LookValue::BloomLevelFalloff, 0.6},
    };
    for (const Data::Maps::LookValueInfo& info : Data::Maps::LookValueInfos())
    {
        CAPTURE(info.name);
        // The file names every value.
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
}
