#include "stdafx.h"

#include "doctest.h"

#include "Data/GameConfig/PersonalTuning.h"
#include "Render/Look/LookFileSystem.h"
#include "Render/Look/LookFiles.h"
#include "Render/Look/TuningSession.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using Data::Config::PersonalTuning;
using Data::Config::PersonalTuningSaveResult;
using Data::Maps::LookValue;
using Render::Look::TuningControlId;
using Render::Look::TuningGroup;
using Render::Look::TuningSaveState;
using Render::Look::TuningSession;

namespace
{
// Records what the session saves and applies; a save fails while `failSave` is set.
struct Recorder
{
    std::vector<PersonalTuning> saved;
    std::vector<Data::Maps::LookOverrides> looks;
    std::vector<std::optional<int>> moods;
    bool failSave = false;

    TuningSession Session(PersonalTuning persisted = {})
    {
        return TuningSession({[this](const PersonalTuning&, const PersonalTuning& tuning)
                              {
                                  if (failSave)
                                  {
                                      return PersonalTuningSaveResult{false, "disk full"};
                                  }
                                  saved.push_back(tuning);
                                  return PersonalTuningSaveResult{true, {}};
                              },
                              [this](const Data::Maps::LookOverrides& look) { looks.push_back(look); },
                              [this](std::optional<int> mood) { moods.push_back(mood); }},
                             persisted);
    }
};

std::string ReadFile(const std::filesystem::path& file)
{
    std::ifstream stream(file, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}
} // namespace

TEST_CASE("a tuning preview applies at once and is saved only when the interaction ends [render][look][tuning]")
{
    Recorder recorder;
    TuningSession session = recorder.Session();
    CHECK(session.SaveState() == TuningSaveState::Saved);

    // A drag: many previews, the renderer follows each one, nothing is written yet.
    for (const double value : {2.0, 1.8, 1.6, 1.5})
    {
        CHECK(session.Preview(TuningControlId::EffectGain, value));
    }
    CHECK(recorder.looks.size() == 4);
    CHECK(recorder.looks.back()[LookValue::DynamicEffectGain] == 1.5);
    CHECK(recorder.saved.empty());
    CHECK(session.SaveState() == TuningSaveState::Unsaved);
    CHECK(session.IsUnsaved(TuningControlId::EffectGain));

    // Released: one save of the final value.
    CHECK(session.Commit());
    REQUIRE(recorder.saved.size() == 1);
    CHECK(recorder.saved[0].look[LookValue::DynamicEffectGain] == 1.5);
    CHECK(session.SaveState() == TuningSaveState::Saved);
    // Nothing changed since: no further write.
    CHECK(session.Commit());
    CHECK(recorder.saved.size() == 1);
}

TEST_CASE("a value outside a control's range is refused [render][look][tuning]")
{
    Recorder recorder;
    TuningSession session = recorder.Session();
    CHECK_FALSE(session.Preview(TuningControlId::EffectGain, 4.1));
    CHECK_FALSE(session.Preview(TuningControlId::MoodStrength, -1));
    CHECK_FALSE(session.Override(TuningControlId::EffectGain));
    CHECK(recorder.looks.empty());
    // The endpoints are fine, zero cap included (No cap).
    CHECK(session.Preview(TuningControlId::EffectCap, 0.0));
    CHECK(session.Override(TuningControlId::EffectCap) == 0.0);
}

TEST_CASE("mood strength changes rebuild at most once per frame [render][look][tuning]")
{
    Recorder recorder;
    TuningSession session = recorder.Session();
    for (int percent = 100; percent >= 40; percent -= 5)
    {
        CHECK(session.Preview(TuningControlId::MoodStrength, percent));
    }
    // Look values are untouched by the mood strength.
    CHECK(recorder.looks.empty());
    CHECK(recorder.moods.empty());
    session.ApplyPending();
    REQUIRE(recorder.moods.size() == 1);
    CHECK(recorder.moods[0] == 40);
    // A frame without a change rebuilds nothing.
    session.ApplyPending();
    CHECK(recorder.moods.size() == 1);
    CHECK(session.Preview(TuningControlId::MoodStrength, 0));
    session.ApplyPending();
    CHECK(recorder.moods.back() == 0);
}

TEST_CASE("resets remove player values and are saved at once [render][look][tuning]")
{
    Recorder recorder;
    TuningSession session = recorder.Session();
    session.Preview(TuningControlId::BloomStrength, 0.5);
    session.Preview(TuningControlId::GradingStrength, 0.0);
    session.Preview(TuningControlId::EffectGain, 1.0);
    session.Preview(TuningControlId::MoodStrength, 60);
    session.ApplyPending();
    REQUIRE(session.Commit());

    // One control: only its key goes.
    session.Reset(TuningControlId::BloomStrength);
    CHECK_FALSE(session.Override(TuningControlId::BloomStrength));
    CHECK(session.Override(TuningControlId::GradingStrength) == 0.0);
    CHECK(recorder.saved.back() == session.Current());
    CHECK(session.SaveState() == TuningSaveState::Saved);

    // A group: only that group's keys go.
    session.ResetGroup(TuningGroup::PostProcessing);
    CHECK_FALSE(session.Override(TuningControlId::GradingStrength));
    CHECK(session.Override(TuningControlId::EffectGain) == 1.0);
    CHECK(session.Override(TuningControlId::MoodStrength) == 60);

    // Everything, mood strength included: saved at once, the mood handed on by the next frame.
    const std::size_t moods = recorder.moods.size();
    session.ResetAll();
    CHECK(session.Current().Empty());
    CHECK(recorder.saved.back().Empty());
    CHECK(recorder.moods.size() == moods);
    session.ApplyPending();
    CHECK(recorder.moods.back() == std::nullopt);
}

TEST_CASE("interactions completed within one frame rebuild the mood once [render][look][tuning]")
{
    Recorder recorder;
    TuningSession session = recorder.Session();

    // Two key presses with their releases, a reset and a group reset, all before the next frame.
    CHECK(session.Preview(TuningControlId::MoodStrength, 99));
    CHECK(session.Commit());
    CHECK(session.Preview(TuningControlId::MoodStrength, 98));
    CHECK(session.Commit());
    session.Reset(TuningControlId::MoodStrength);
    CHECK(session.Preview(TuningControlId::MoodStrength, 97));
    CHECK(session.Commit());
    session.ResetGroup(TuningGroup::LightMood);
    CHECK(session.Preview(TuningControlId::MoodStrength, 50));
    CHECK(session.Commit());
    // Each was saved; none rebuilt the map light.
    CHECK(recorder.saved.size() == 6);
    CHECK(recorder.moods.empty());

    // The frame: one rebuild, with the last value.
    session.ApplyPending();
    REQUIRE(recorder.moods.size() == 1);
    CHECK(recorder.moods[0] == 50);
    session.ApplyPending();
    CHECK(recorder.moods.size() == 1);
}

TEST_CASE("only the frame loop applies the pending mood strength [render][look][tuning]")
{
    // MU_TEST_DATA_DIR is src/bin/Data; the sources are src/source.
    const std::filesystem::path sources =
        std::filesystem::path(MU_TEST_DATA_DIR).parent_path().parent_path() / "source";
    REQUIRE(std::filesystem::is_directory(sources));
    std::vector<std::string> frameCalls;
    std::vector<std::string> sessionCalls;
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sources))
    {
        const std::string extension = entry.path().extension().string();
        if (!entry.is_regular_file() || (extension != ".cpp" && extension != ".h"))
        {
            continue;
        }
        const std::string text = ReadFile(entry.path());
        const std::string name = entry.path().lexically_relative(sources).generic_string();
        constexpr std::string_view Declaration = "void ";
        for (std::size_t at = text.find("ApplyPendingTuning();"); at != std::string::npos;
             at = text.find("ApplyPendingTuning();", at + 1))
        {
            // Not the declaration, `void ApplyPendingTuning();`.
            if (at < Declaration.size() || text.compare(at - Declaration.size(), Declaration.size(), Declaration) != 0)
            {
                frameCalls.push_back(name);
            }
        }
        for (const char* call : {".ApplyPending();", "->ApplyPending();"})
        {
            if (text.find(call) != std::string::npos)
            {
                sessionCalls.push_back(name);
            }
        }
    }
    CHECK(frameCalls == std::vector<std::string>{"App/Platform/Windows/Winmain.cpp"});
    CHECK(sessionCalls == std::vector<std::string>{"Render/Look/TuningSetting.cpp"});
}

TEST_CASE("a failed save keeps the preview, says so and is retried [render][look][tuning]")
{
    PersonalTuning persisted;
    persisted.look[LookValue::BloomStrength] = 0.7;
    Recorder recorder;
    TuningSession session = recorder.Session(persisted);

    recorder.failSave = true;
    session.Preview(TuningControlId::BloomStrength, 0.2);
    CHECK_FALSE(session.Commit());
    CHECK(session.SaveState() == TuningSaveState::Failed);
    CHECK(session.SaveError() == "disk full");
    // The preview still applies; what is saved is still the old value.
    CHECK(session.Override(TuningControlId::BloomStrength) == 0.2);
    CHECK(session.Persisted().look[LookValue::BloomStrength] == 0.7);
    CHECK(session.IsUnsaved(TuningControlId::BloomStrength));

    recorder.failSave = false;
    CHECK(session.Commit());
    CHECK(session.SaveState() == TuningSaveState::Saved);
    CHECK(session.SaveError().empty());
    CHECK(session.Persisted().look[LookValue::BloomStrength] == 0.2);
}

TEST_CASE("player tuning writes config.ini only, never the look or mood files [render][look][tuning]")
{
    const std::filesystem::path data = std::filesystem::path(MU_TEST_DATA_DIR);
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() /
        ("tuning_session_tests_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(dir);
    const std::filesystem::path look = dir / "Look.json";
    const std::filesystem::path mood = dir / "LightMood.json";
    const std::filesystem::path config = dir / "config.ini";
    std::filesystem::copy_file(data / "Render/Look.json", look);
    std::filesystem::copy_file(data / "Maps/LightMood.json", mood);
    {
        std::ofstream(config, std::ios::binary) << "[Render]\nLook=Enhanced\n";
    }
    const std::string lookBefore = ReadFile(look);
    const std::string moodBefore = ReadFile(mood);

    TuningSession session({[&config](const PersonalTuning& saved, const PersonalTuning& tuning)
                           { return Data::Config::SavePersonalTuningFile(config, saved, tuning); },
                           {},
                           {}},
                          {});
    session.Preview(TuningControlId::BloomStrength, 0.5);
    session.Preview(TuningControlId::MoodStrength, 30);
    session.ApplyPending();
    REQUIRE(session.Commit());
    std::vector<std::string> issues;
    CHECK(Data::Config::LoadPersonalTuningFile(config, issues) == session.Current());
    session.ResetAll();
    CHECK(ReadFile(config) == "[Render]\nLook=Enhanced\n");

    CHECK(ReadFile(look) == lookBefore);
    CHECK(ReadFile(mood) == moodBefore);

    // A developer's save writes the files' own documents: the player's values are not in them.
    Render::Look::DiskLookFileSystem fileSystem;
    Render::Look::LookFilesLoad loaded = Render::Look::LoadLookFiles(fileSystem, look, mood);
    REQUIRE(loaded.files.lookUsable);
    REQUIRE(Render::Look::SaveLookFiles(fileSystem, loaded.files, look, mood).saved);
    std::vector<Data::Items::ItemDataIssue> lookIssues;
    Data::Maps::LookProfileData reread;
    Data::Maps::ReadLookProfileJson(ReadFile(look), look.generic_string(), reread, lookIssues);
    CHECK(reread == loaded.files.lookData);
    CHECK(reread.defaultOverrides[LookValue::BloomStrength] == 1.0);

    std::error_code ignored;
    std::filesystem::remove_all(dir, ignored);
}
