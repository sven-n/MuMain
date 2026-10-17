#include "stdafx.h"

#include "doctest.h"

#include "Data/GameConfig/PersonalTuning.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using Data::Config::PersonalTuning;
using Data::Maps::LookValue;

namespace
{
std::string Join(const std::vector<std::string>& lines)
{
    std::string text;
    for (const std::string& line : lines)
    {
        text += line + "\n";
    }
    return text;
}

struct TempDir
{
    TempDir()
    {
        path = std::filesystem::temp_directory_path() /
               ("personal_tuning_tests_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ignored;
        std::filesystem::permissions(path, std::filesystem::perms::owner_all, ignored);
        std::filesystem::remove_all(path, ignored);
    }
    std::filesystem::path path;
};

void WriteFile(const std::filesystem::path& file, const std::string& text)
{
    std::ofstream stream(file, std::ios::binary);
    stream << text;
}

std::string ReadFile(const std::filesystem::path& file)
{
    std::ifstream stream(file, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

const std::string ConfigWithoutTuning = "[Window]\nWidth=1280\nHeight=720\n\n[Render]\nLook=Enhanced\nLightMood=1\n\n";
} // namespace

TEST_CASE("missing tuning keys inherit and valid ones are read [data][config][tuning]")
{
    std::vector<std::string> issues;
    CHECK(Data::Config::ReadPersonalTuning(ConfigWithoutTuning, issues).Empty());
    CHECK(issues.empty());

    const PersonalTuning tuning = Data::Config::ReadPersonalTuning(
        ConfigWithoutTuning + "[enhancedtuning]\ndynamic.effectGain = 1.2\nbloom.strength=0.5\n"
                              "dynamic.effectCap=0\ngrading.strength=0\nmood.strengthPercent=70\n",
        issues);
    CHECK_MESSAGE(issues.empty(), Join(issues));
    CHECK(tuning.look[LookValue::DynamicEffectGain] == 1.2);
    CHECK(tuning.look[LookValue::BloomStrength] == 0.5);
    // No-cap and zero-grading endpoints are values like any other.
    CHECK(tuning.look[LookValue::DynamicEffectCap] == 0.0);
    CHECK(tuning.look[LookValue::GradingStrength] == 0.0);
    CHECK(tuning.moodStrengthPercent == 70);
    CHECK_FALSE(tuning.look[LookValue::RimStrength].has_value());
}

TEST_CASE("each invalid tuning key is ignored on its own [data][config][tuning]")
{
    std::vector<std::string> issues;
    const PersonalTuning tuning = Data::Config::ReadPersonalTuning("[EnhancedTuning]\n"
                                                                   "bloom.strength=0.5\n"
                                                                   "dynamic.effectGain=9\n"      // out of range
                                                                   "dynamic.effectRange=nan\n"   // not finite
                                                                   "lighting.rim.strength=inf\n" // not finite
                                                                   "dynamic.wrap=abc\n"          // not a number
                                                                   "bloom.strenght=1\n"          // unknown
                                                                   "mood.strengthPercent=50.5\n" // not whole
                                                                   "grading.strength=-0.1\n" // below the zero endpoint
                                                                   "dynamic.effectFade=3\n"  // out of range
                                                                   "lighting.diffuse.floor=2\n" // out of range
                                                                   "bloom.levelFalloff=0.5\n",
                                                                   issues);
    CHECK(tuning.look[LookValue::BloomStrength] == 0.5);
    CHECK(tuning.look[LookValue::BloomLevelFalloff] == 0.5);
    CHECK_FALSE(tuning.look[LookValue::DynamicEffectGain].has_value());
    CHECK_FALSE(tuning.look[LookValue::DynamicEffectRange].has_value());
    CHECK_FALSE(tuning.look[LookValue::RimStrength].has_value());
    CHECK_FALSE(tuning.look[LookValue::DynamicWrap].has_value());
    CHECK_FALSE(tuning.look[LookValue::GradingStrength].has_value());
    CHECK_FALSE(tuning.moodStrengthPercent.has_value());
    // Bounded: at most MaxPersonalTuningIssues lines, the last counts the rest.
    CHECK(issues.size() == Data::Config::MaxPersonalTuningIssues);
    CHECK_MESSAGE(issues.front().find("dynamic.effectGain") != std::string::npos, Join(issues));
    CHECK_MESSAGE(issues.back().find("more ignored keys") != std::string::npos, Join(issues));
}

TEST_CASE("the tuning section is replaced on its own and removed when empty [data][config][tuning]")
{
    PersonalTuning tuning;
    tuning.look[LookValue::BloomStrength] = 0.5;
    tuning.moodStrengthPercent = 40;

    const std::string added = Data::Config::ReplacePersonalTuningSection(ConfigWithoutTuning, tuning);
    CHECK(added == ConfigWithoutTuning + "[EnhancedTuning]\nbloom.strength=0.5\nmood.strengthPercent=40\n");

    // Every other line survives; a section in the middle is replaced in place.
    const std::string middle = "[Window]\nWidth=1280\n[EnhancedTuning]\nbloom.strength=1\n; note\n[Audio]\nMusic=3\n";
    const std::string replaced = Data::Config::ReplacePersonalTuningSection(middle, tuning);
    CHECK(replaced ==
          "[Window]\nWidth=1280\n[EnhancedTuning]\nbloom.strength=0.5\nmood.strengthPercent=40\n[Audio]\nMusic=3\n");

    // Reset all: no keys, no section.
    CHECK(Data::Config::ReplacePersonalTuningSection(replaced, {}) == "[Window]\nWidth=1280\n[Audio]\nMusic=3\n");
    // Line endings of the file are kept.
    CHECK(Data::Config::ReplacePersonalTuningSection("[Window]\r\nWidth=1\r\n", tuning) ==
          "[Window]\r\nWidth=1\r\n[EnhancedTuning]\r\nbloom.strength=0.5\r\nmood.strengthPercent=40\r\n");
}

TEST_CASE("saved tuning round-trips and resets remove only their keys [data][config][tuning]")
{
    TempDir dir;
    const std::filesystem::path config = dir.path / "config.ini";
    WriteFile(config, ConfigWithoutTuning);

    PersonalTuning tuning;
    tuning.look[LookValue::BloomStrength] = 0.5;
    tuning.look[LookValue::DynamicEffectGain] = 1.2;
    tuning.moodStrengthPercent = 0;
    REQUIRE(Data::Config::SavePersonalTuningFile(config, tuning).saved);

    std::vector<std::string> issues;
    CHECK(Data::Config::LoadPersonalTuningFile(config, issues) == tuning);
    CHECK(issues.empty());
    CHECK(ReadFile(config).starts_with(ConfigWithoutTuning));
    CHECK_FALSE(std::filesystem::exists(dir.path / "config.ini.tmp"));

    // A per-control reset removes that key only.
    tuning.look[LookValue::BloomStrength].reset();
    REQUIRE(Data::Config::SavePersonalTuningFile(config, tuning).saved);
    const std::string afterReset = ReadFile(config);
    CHECK(afterReset.find("bloom.strength") == std::string::npos);
    CHECK(afterReset.find("dynamic.effectGain=1.2") != std::string::npos);
    CHECK(afterReset.find("mood.strengthPercent=0") != std::string::npos);

    // Reset all leaves the file as it was before any tuning.
    REQUIRE(Data::Config::SavePersonalTuningFile(config, {}).saved);
    CHECK(ReadFile(config) == ConfigWithoutTuning);

    // A missing file is created with only the section.
    const std::filesystem::path fresh = dir.path / "fresh.ini";
    REQUIRE(Data::Config::SavePersonalTuningFile(fresh, tuning).saved);
    CHECK(Data::Config::LoadPersonalTuningFile(fresh, issues) == tuning);
}

TEST_CASE("a restart reads exactly the value a session applied [data][config][tuning]")
{
    for (const double value : {0.1, 0.3, 1.0 / 3.0, 0.15000000000000002})
    {
        const double kept = Data::Config::NormalizeTuningNumber(value);
        std::vector<std::string> issues;
        PersonalTuning tuning;
        tuning.look[LookValue::DiffuseFloor] = kept;
        const PersonalTuning read =
            Data::Config::ReadPersonalTuning(Data::Config::ReplacePersonalTuningSection("", tuning), issues);
        CHECK(read.look[LookValue::DiffuseFloor] == kept);
    }
}

TEST_CASE("a failed tuning save leaves the configuration as it was [data][config][tuning]")
{
    TempDir dir;
    const std::filesystem::path config = dir.path / "config.ini";
    const std::string before = ConfigWithoutTuning + "[EnhancedTuning]\nbloom.strength=0.7\n";
    WriteFile(config, before);

    PersonalTuning tuning;
    tuning.look[LookValue::BloomStrength] = 0.2;

    // The temporary file cannot be created: a folder of that name is in the way.
    std::filesystem::create_directory(dir.path / "config.ini.tmp");
    const Data::Config::PersonalTuningSaveResult blocked = Data::Config::SavePersonalTuningFile(config, tuning);
    CHECK_FALSE(blocked.saved);
    CHECK_MESSAGE(blocked.error.find("left as it was") != std::string::npos, blocked.error);
    CHECK(ReadFile(config) == before);
    CHECK(std::filesystem::is_directory(dir.path / "config.ini.tmp"));
    std::filesystem::remove(dir.path / "config.ini.tmp");

    // A UTF-16 file is never rewritten.
    const std::string utf16 = std::string("\xFF\xFE[\0W\0]\0", 8);
    WriteFile(config, utf16);
    CHECK_FALSE(Data::Config::SavePersonalTuningFile(config, tuning).saved);
    CHECK(ReadFile(config) == utf16);
}
