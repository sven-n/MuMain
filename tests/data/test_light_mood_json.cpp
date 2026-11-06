#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MapData/LightMood.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Maps::LightMood;
using Data::Maps::LightMoodData;

namespace
{
constexpr int Lorencia = 0;
constexpr int LostTower = 4;
constexpr int Atlans = 7;
constexpr int Icarus = 10;

struct ReadResult
{
    LightMoodData data;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Maps::ReadLightMoodJson(text, "LightMood.json", result.data, result.issues);
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

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, const std::string& field, const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                       { return issue.field == field && issue.message.find(message) != std::string::npos; });
}

const std::string ValidFile = R"({
  "formatVersion": 1,
  "default": { "base": 0.7, "figures": 0.8, "dynamicGain": 1.2, "background": 1, "sun": 0.5 },
  "maps": [
    { "world": 10, "name": "Icarus", "background": 0.7 },
    { "world": 1, "base": 0.5, "figures": 0.6, "dynamicGain": 0.5, "background": 0, "sun": 0.3 }
  ]
})";
} // namespace

TEST_CASE("a light mood file gives the default and each map's mood [data][lightmood]")
{
    const ReadResult result = Read(ValidFile);

    CHECK_MESSAGE(result.issues.empty(), Messages(result.issues));
    CHECK(result.data.defaultMood == LightMood{0.7, 0.8, 1.2, 1.0, 0.5});
    REQUIRE(result.data.maps.size() == 2);
    CHECK(result.data.ForWorld(1) == LightMood{0.5, 0.6, 0.5, 0.0, 0.3});
}

TEST_CASE("a map entry overrides only the values it names [data][lightmood]")
{
    const ReadResult result = Read(ValidFile);

    CHECK(result.data.ForWorld(Icarus) == LightMood{0.7, 0.8, 1.2, 0.7, 0.5});
}

TEST_CASE("a map without an entry uses the default mood [data][lightmood]")
{
    const ReadResult result = Read(ValidFile);

    CHECK(result.data.ForWorld(Lorencia) == result.data.defaultMood);
}

TEST_CASE("a value out of range makes the whole file unusable [data][lightmood]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 1 },
      "maps": [ { "world": 10, "base": 0.1 } ]
    })");

    CHECK(HasFatal(result.issues));
    // The file, the map and the value are named.
    CHECK_MESSAGE(Messages(result.issues).find("LightMood.json: maps[world 10].base: is 0.1, must be from 0.2 to 1") !=
                      std::string::npos,
                  Messages(result.issues));
    // Classic everywhere, not a partial mood.
    CHECK(result.data.defaultMood == LightMood{});
    CHECK(result.data.maps.empty());
    CHECK(result.data.ForWorld(Icarus) == LightMood{});
}

TEST_CASE("every light mood value has its range [data][lightmood]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "base": 1.1, "figures": 1, "dynamicGain": 2.5, "background": -0.1, "sun": 1.5 }
    })");

    CHECK(HasIssueAbout(result.issues, "default.base", "must be from 0.2 to 1"));
    CHECK(HasIssueAbout(result.issues, "default.dynamicGain", "must be from 0.5 to 2"));
    CHECK(HasIssueAbout(result.issues, "default.background", "must be from 0 to 1"));
    CHECK(HasIssueAbout(result.issues, "default.sun", "must be from 0 to 1"));
    CHECK(result.data.defaultMood == LightMood{});
}

TEST_CASE("figures are never darker than the base [data][lightmood]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "base": 0.7, "figures": 0.6, "dynamicGain": 1, "background": 1, "sun": 1 }
    })");

    CHECK(HasIssueAbout(result.issues, "default.figures", "must be from 0.7 to 1"));
}

TEST_CASE("the default needs all five values [data][lightmood]")
{
    const ReadResult result = Read(R"({ "formatVersion": 1, "default": { "base": 0.7 } })");

    CHECK(HasIssueAbout(result.issues, "default.figures", "missing"));
    CHECK(HasIssueAbout(result.issues, "default.dynamicGain", "missing"));
    CHECK(HasIssueAbout(result.issues, "default.background", "missing"));
    CHECK(HasIssueAbout(result.issues, "default.sun", "missing"));
}

TEST_CASE("a map entry needs a world, once [data][lightmood]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 1 },
      "maps": [ { "base": 0.8 }, { "world": 256 }, { "world": 3 }, { "world": 3 } ]
    })");

    CHECK(HasIssueAbout(result.issues, "maps[0].world", "whole number from 0 to 255"));
    CHECK(HasIssueAbout(result.issues, "maps[1].world", "whole number from 0 to 255"));
    CHECK(HasIssueAbout(result.issues, "maps[world 3]", "more than once"));
    CHECK(result.data.maps.empty());
}

TEST_CASE("an unknown field is a warning and keeps the file usable [data][lightmood]")
{
    const ReadResult result = Read(R"({
      "formatVersion": 1,
      "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 1, "fog": 1 }
    })");

    REQUIRE(result.issues.size() == 1);
    CHECK(result.issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(result.issues[0].field == "default.fog");
    CHECK(result.data.defaultMood.base == doctest::Approx(0.7));
}

TEST_CASE("a broken or missing light mood file is an error [data][lightmood]")
{
    CHECK(HasFatal(Read("{ not json").issues));
    CHECK(HasFatal(Read(R"({ "formatVersion": 2, "default": {} })").issues));

    const Data::Maps::LightMoodLoadResult missing =
        Data::Maps::LoadLightMoodFile(std::filesystem::temp_directory_path() / "mu_no_such_light_mood.json");
    CHECK(HasFatal(missing.issues));
    CHECK_MESSAGE(Messages(missing.issues).find("could not be read") != std::string::npos, Messages(missing.issues));
    CHECK(missing.data.defaultMood == LightMood{});
}

TEST_CASE("the shipped light mood file has the tuned values [data][lightmood]")
{
    // MU_TEST_DATA_DIR is the client folder's Data; the file name starts with it.
    const std::filesystem::path file =
        std::filesystem::path(MU_TEST_DATA_DIR).parent_path() / Data::Maps::LightMoodFile;
    const Data::Maps::LightMoodLoadResult loaded = Data::Maps::LoadLightMoodFile(file);

    CHECK_MESSAGE(loaded.issues.empty(), Messages(loaded.issues));
    // The tuned Enhanced look: base, figures, dynamicGain, background, sun,
    // shadowKeep, shadeCool, brightScale.
    CHECK(loaded.data.defaultMood == LightMood{0.45, 0.7, 1.7, 1.0, 0.8, 0.8, 0.1, 0.8});
    CHECK(loaded.data.ForWorld(Icarus) == LightMood{0.7, 0.7, 1.7, 0.7, 0.8, 0.8, 0.1, 0.95});
    CHECK(loaded.data.ForWorld(LostTower) == LightMood{0.45, 0.7, 1.7, 1.0, 1.0, 0.8, 0.1, 0.8});
    CHECK(loaded.data.ForWorld(Atlans) == LightMood{0.8, 0.8, 1.7, 1.0, 1.0, 0.8, 0.1, 1.0});
    CHECK(loaded.data.ForWorld(Lorencia) == loaded.data.defaultMood);
}

TEST_CASE("the shadow keep and the cool shade are optional and range-checked [data][lightmood]")
{
    const ReadResult result = Read(R"({
  "formatVersion": 1,
  "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 0.6 },
  "maps": [ { "world": 0, "name": "Lorencia", "shadowKeep": 0.9, "shadeCool": 0.1, "brightScale": 0.85 } ]
})");
    REQUIRE_FALSE(HasFatal(result.issues));
    CHECK(result.data.defaultMood.shadowKeep == 0.0);
    CHECK(result.data.ForWorld(Lorencia).shadowKeep == doctest::Approx(0.9));
    CHECK(result.data.ForWorld(Lorencia).shadeCool == doctest::Approx(0.1));
    CHECK(result.data.ForWorld(Lorencia).brightScale == doctest::Approx(0.85));
    CHECK(result.data.defaultMood.brightScale == 1.0);

    const ReadResult outOfRange = Read(R"({
  "formatVersion": 1,
  "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 0.6, "shadowKeep": 2, "shadeCool": -0.1, "brightScale": 1.2 }
})");
    CHECK(HasFatal(outOfRange.issues));
    CHECK(Messages(outOfRange.issues).find("default.shadowKeep") != std::string::npos);
    CHECK(Messages(outOfRange.issues).find("default.shadeCool") != std::string::npos);
    CHECK(Messages(outOfRange.issues).find("default.brightScale") != std::string::npos);
}
