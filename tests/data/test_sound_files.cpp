#include "stdafx.h"

// The code's sounds and their files (Data/Audio/SoundFiles.json, sound editor
// design Q-SE1): the reader and writer, their checks, the shipped file, the
// SOUND_ id names and LoadGameSound.

#include "doctest.h"

#include "Audio/DSPlaySound.h"
#include "Audio/SoundIds.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Data/DataHandler/SoundData/SoundFileStorage.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "Data/GameData/SoundData/SoundFilesJson.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Sounds::SoundFile;

namespace
{
struct ReadResult
{
    std::vector<SoundFile> sounds;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Sounds::ReadSoundFilesJson(text, "SoundFiles.json", result.sounds, result.issues);
    return result;
}

std::string File(const std::string& entries)
{
    return R"({ "formatVersion": 1, "sounds": [ )" + entries + " ] }";
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field,
                   const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue)
                       {
                           return issue.severity == severity && issue.field == field &&
                                  issue.message.find(message) != std::string::npos;
                       });
}

std::string WithoutCarriageReturns(std::string text)
{
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    return text;
}

// An audio backend that notes what loads, with channels and 3D.
class LoadingAudio final : public mu::IPlatformAudio
{
public:
    std::vector<std::tuple<int, std::wstring, int, bool>> loads;

    bool Initialize() override
    {
        return true;
    }
    void Shutdown() override {}
    void LoadSound(ESound buffer, const wchar_t* filename, int channels, bool enable3D) override
    {
        loads.emplace_back(static_cast<int>(buffer), filename != nullptr ? filename : L"", channels, enable3D);
    }
    bool PlaySound(ESound, const void*, bool) override
    {
        return true;
    }
    bool PlaySoundAtLevel(ESound, float) override
    {
        return true;
    }
    void StopSound(ESound, bool) override {}
    void AllStopSound() override {}
    void ReleaseSound(ESound) override {}
    void Set3DSoundPosition() override {}
    void SetVolume(ESound, long) override {}
    void SetMasterVolume(long) override {}
    void PlayMusic(const char*, bool) override {}
    void StopMusic(const char*, bool) override {}
    bool IsEndMusic() override
    {
        return true;
    }
    int GetMusicPosition() override
    {
        return 0;
    }
    void SetBGMVolume(float) override {}
    void SetSFXVolume(float) override {}
    float GetBGMVolume() const override
    {
        return 1.0f;
    }
    float GetSFXVolume() const override
    {
        return 1.0f;
    }
};

// The table and a loading backend for the scope; both put back after.
class LoadScope
{
public:
    LoadScope() : m_previousAudio(g_platformAudio), m_previousSounds(g_SoundFileDatabase.GetAll())
    {
        g_platformAudio = &audio;
        ForgetLoadedSounds();
    }
    ~LoadScope()
    {
        g_platformAudio = m_previousAudio;
        g_SoundFileDatabase.Build(m_previousSounds);
        ForgetLoadedSounds();
    }
    LoadScope(const LoadScope&) = delete;
    LoadScope& operator=(const LoadScope&) = delete;

    LoadingAudio audio;

private:
    mu::IPlatformAudio* m_previousAudio;
    std::vector<SoundFile> m_previousSounds;
};
} // namespace

TEST_CASE("Sound files: an entry reads its id's number, its name, file, channels, 3D and type, and the optional fields "
          "have defaults (Q-SE1) [data][sounds]")
{
    const ReadResult result = Read(
        File(R"({"id": "SOUND_CLICK01", "name": "Button click", "file": "Data/Sound/iButtonClick.wav", "channels": 2,)"
             R"( "spatial": true, "type": "interface"},)"
             R"({"id": "SOUND_RAIN01", "file": "Data/Sound/aRain.wav"})"));
    REQUIRE_FALSE(HasFatal(result.issues));
    REQUIRE(result.sounds.size() == 2);
    const SoundFile& click = result.sounds[0];
    CHECK(click.id == "SOUND_CLICK01");
    CHECK(click.number == static_cast<int>(SOUND_CLICK01));
    CHECK(click.name == "Button click");
    CHECK(click.file == "Data/Sound/iButtonClick.wav");
    CHECK(click.channels == 2);
    CHECK(click.spatial);
    REQUIRE(click.type.has_value());
    CHECK(*click.type == Audio::SoundKind::Interface);
    const SoundFile& rain = result.sounds[1];
    CHECK(rain.number == static_cast<int>(SOUND_RAIN01));
    CHECK(rain.name.empty());
    CHECK(rain.channels == 1);
    CHECK_FALSE(rain.spatial);
    CHECK_FALSE(rain.type.has_value());

    // Written and read back the same, the optional fields only when set.
    const std::string text = Data::Sounds::WriteSoundFilesJson(result.sounds);
    CHECK(text.find(R"({"id": "SOUND_RAIN01", "file": "Data/Sound/aRain.wav", "channels": 1})") != std::string::npos);
    const ReadResult again = Read(text);
    CHECK(again.sounds == result.sounds);
    CHECK(again.issues.empty());
}

TEST_CASE("Sound files: wrong values are fatal problems that name the field, unknown fields warnings, and an id "
          "named twice fatal (Q-SE1) [data][sounds]")
{
    const auto fatalAbout = [](const std::string& entry, const std::string& field, const std::string& message)
    {
        const ReadResult result = Read(File(entry));
        CHECK(result.sounds.empty());
        CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Fatal, field, message));
    };
    fatalAbout(R"({"id": "SOUND_NONE", "file": "Data/Sound/a.wav"})", "sounds[0].id", "no SOUND_ id");
    fatalAbout(R"({"id": 7, "file": "Data/Sound/a.wav"})", "sounds[0].id", "must be a SOUND_ id");
    fatalAbout(R"({"id": "SOUND_CLICK01"})", "sounds[0]", "with id and file");
    fatalAbout(R"({"id": "SOUND_CLICK01", "file": ""})", "sounds[0].file", "below the client folder");
    fatalAbout(R"({"id": "SOUND_CLICK01", "file": "Data/Sound/a.wav", "channels": 0})", "sounds[0].channels",
               "from 1 to 4");
    fatalAbout(R"({"id": "SOUND_CLICK01", "file": "Data/Sound/a.wav", "channels": 5})", "sounds[0].channels",
               "from 1 to 4");
    fatalAbout(R"({"id": "SOUND_CLICK01", "file": "Data/Sound/a.wav", "spatial": 1})", "sounds[0].spatial",
               "true or false");
    fatalAbout(R"({"id": "SOUND_CLICK01", "file": "Data/Sound/a.wav", "type": "loud"})", "sounds[0].type",
               "must be one of music, mapAmbience");
    fatalAbout(R"({"id": "SOUND_CLICK01", "file": "Data/Sound/a.wav", "name": 3})", "sounds[0].name", "a text");

    const ReadResult unknown = Read(File(R"({"id": "SOUND_CLICK01", "file": "Data/Sound/a.wav", "loud": true})"));
    CHECK(unknown.sounds.size() == 1);
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(std::any_of(unknown.issues.begin(), unknown.issues.end(), [](const ItemDataIssue& issue)
                      { return issue.severity == ItemDataIssueSeverity::Warning && issue.field == "sounds[0].loud"; }));

    ReadResult twice = Read(File(R"({"id": "SOUND_CLICK01", "file": "Data/Sound/a.wav"},)"
                                 R"({"id": "SOUND_RAIN01", "file": "Data/Sound/b.wav"},)"
                                 R"({"id": "SOUND_CLICK01", "file": "Data/Sound/c.wav"})"));
    REQUIRE(twice.sounds.size() == 3);
    Data::Sounds::ValidateSoundFiles(twice.sounds, "SoundFiles.json", twice.issues);
    CHECK(HasIssueAbout(twice.issues, ItemDataIssueSeverity::Fatal, "sounds[2].id", "is in sounds[0] too"));
    CHECK_FALSE(HasIssueAbout(twice.issues, ItemDataIssueSeverity::Fatal, "sounds[1].id", ""));
}

TEST_CASE("Sound files: the shipped file names each sound the code loads once, reads with no fatal problem and "
          "writes back the same text (Q-SE1) [data][sounds]")
{
    std::ifstream in(std::string(MU_TEST_DATA_DIR) + "/Audio/SoundFiles.json", std::ios::binary);
    REQUIRE(in.good());
    std::stringstream buffer;
    buffer << in.rdbuf();
    const std::string text = WithoutCarriageReturns(buffer.str());
    ReadResult result = Read(text);
    Data::Sounds::ValidateSoundFiles(result.sounds, "SoundFiles.json", result.issues);
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(result.issues.empty());
    CHECK(result.sounds.size() == 389);
    std::set<int> numbers;
    for (const SoundFile& sound : result.sounds)
    {
        CHECK(numbers.insert(sound.number).second);
    }
    CHECK(Data::Sounds::WriteSoundFilesJson(result.sounds) == text);

    // The files fixed after the move.
    const auto fileOf = [&result](const std::string& id)
    {
        const auto found = std::find_if(result.sounds.begin(), result.sounds.end(),
                                        [&id](const SoundFile& sound) { return sound.id == id; });
        return found != result.sounds.end() ? found->file : std::string();
    };
    // Fenrir's two pain sounds, of which the second was never loaded.
    CHECK(fileOf("SOUND_FENRIR_DAMAGE_1") == "Data/Sound/pWpain1.wav");
    CHECK(fileOf("SOUND_FENRIR_DAMAGE_2") == "Data/Sound/pWpain2.wav");
    // Nightmare's skills, which named their files without ".wav".
    CHECK(fileOf("SOUND_KANTURU_3RD_NIGHTMARE_ATT2") == "Data/Sound/w39/nightmare_skill-01.wav");
    CHECK(fileOf("SOUND_KANTURU_3RD_NIGHTMARE_ATT3") == "Data/Sound/w39/nightmare_skill-02.wav");
    CHECK(fileOf("SOUND_KANTURU_3RD_NIGHTMARE_ATT4") == "Data/Sound/w39/nightmare_skill-03.wav");
    // Battle Castle's mana sound, whose file name had a typo.
    CHECK(fileOf("SOUND_BMS_MANA") == "Data/Sound/BattleCastle/sDSwellMana.wav");
}

TEST_CASE("Sound files: LoadGameSound loads the file the data gives the id, with its channels and 3D, and an id the "
          "data has not loads nothing (Q-SE1) [data][sounds]")
{
    LoadScope scope;
    SoundFile click;
    click.id = "SOUND_CLICK01";
    click.number = static_cast<int>(SOUND_CLICK01);
    click.file = "Data/Sound/w35/iButtonClick.wav";
    click.channels = 3;
    click.spatial = true;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{click});
    LoadGameSound(SOUND_CLICK01);
    LoadGameSound(SOUND_RAIN01);
    REQUIRE(scope.audio.loads.size() == 1);
    const auto& [number, file, channels, spatial] = scope.audio.loads[0];
    CHECK(number == static_cast<int>(SOUND_CLICK01));
    // As the code named it: backslashes.
    CHECK(file == L"Data\\Sound\\w35\\iButtonClick.wav");
    CHECK(channels == 3);
    CHECK(spatial);
    CHECK(IsSoundLoaded(SOUND_CLICK01, L"Data\\Sound\\w35\\iButtonClick.wav", 3, true));
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_CLICK01)) != nullptr);
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_RAIN01)) == nullptr);
}

TEST_CASE("Data paths: a data file's path becomes the loaders' path with backslashes, its UTF-8 letters wide "
          "[data][sounds]")
{
    CHECK(Data::Paths::ToLoaderPath("Data/Sound/w34/aida_ambi.wav") == L"Data\\Sound\\w34\\aida_ambi.wav");
    CHECK(Data::Paths::ToLoaderPath("Data/Sound/caf\xc3\xa9.wav") == L"Data\\Sound\\café.wav");
    CHECK(Data::Paths::ToLoaderPath("a.wav") == L"a.wav");
    CHECK(Data::Paths::ToLoaderPath("").empty());
}

TEST_CASE("Sound ids: a SOUND_ name finds its id and an id its name, and types have their words (Q-SE1, Q-SE2) "
          "[data][sounds]")
{
    CHECK(Audio::SoundIds::Find("SOUND_CLICK01") == SOUND_CLICK01);
    CHECK(Audio::SoundIds::Find("SOUND_WIND01") == SOUND_WIND01);
    CHECK(Audio::SoundIds::Find("SOUND_KARUTAN_KARDAMAHAL_ENV") == SOUND_KARUTAN_KARDAMAHAL_ENV);
    CHECK_FALSE(Audio::SoundIds::Find("SOUND_NONE").has_value());
    CHECK_FALSE(Audio::SoundIds::Find("MAX_BUFFER").has_value());
    CHECK(Audio::SoundIds::NameOf(SOUND_FENRIR_DAMAGE_2) == "SOUND_FENRIR_DAMAGE_2");
    CHECK(Audio::SoundIds::NameOf(MAX_BUFFER).empty());
    for (int kind = 0; kind < Audio::SoundKindCount; ++kind)
    {
        const auto type = static_cast<Audio::SoundKind>(kind);
        const std::string_view word = Data::Sounds::SoundTypeWord(type);
        CHECK_FALSE(word.empty());
        CHECK(Data::Sounds::FindSoundType(word) == type);
    }
    CHECK(Data::Sounds::SoundTypeWord(Audio::SoundKind::Hero) == "hero");
    CHECK_FALSE(Data::Sounds::FindSoundType("Hero").has_value());
}
