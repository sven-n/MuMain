#include "stdafx.h"

#include "doctest.h"

#include "Audio/DSPlaySound.h"
#include "Audio/SoundNumbers.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Data/DataHandler/MapData/MapObjectSoundStorage.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MapData/MapObjectSoundsJson.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/w_CharacterInfo.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "World/MapInfra/MapObjectSounds.h"
#ifdef _EDITOR
#include "Audio/SoundMixer.h"
#include "Audio/SoundOverlay.h"
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Maps::MapObjectSound;

namespace
{
struct ReadResult
{
    std::vector<MapObjectSound> sounds;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Maps::ReadMapObjectSoundsJson(text, "ObjectSounds.json", result.sounds, result.issues);
    return result;
}

std::string File(const std::string& entries)
{
    return R"({ "formatVersion": 1, "objectSounds": [ )" + entries + " ] }";
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

MapObjectSound Waterfall()
{
    MapObjectSound sound;
    sound.maps = {{24}, {25}, {26}, {27}, {28}, {29}, {36}};
    sound.objects = {37, 38};
    sound.file = "Data/Sound/aKalimaWaterFall.wav";
    sound.channels = 3;
    return sound;
}

// An audio backend that notes what loads and what plays on a free channel.
class RecordingAudio final : public mu::IPlatformAudio
{
public:
    std::vector<std::pair<int, std::wstring>> loads;
    std::vector<int> freePlays;
    std::vector<float> freeLevels;
    std::vector<int> plays;

    bool Initialize() override
    {
        return true;
    }
    void Shutdown() override {}
    void LoadSound(ESound buffer, const wchar_t* filename, int, bool) override
    {
        loads.emplace_back(static_cast<int>(buffer), filename != nullptr ? filename : L"");
    }
    bool PlaySound(ESound buffer, const void*, bool) override
    {
        plays.push_back(static_cast<int>(buffer));
        return true;
    }
    bool PlaySoundOnFreeChannel(ESound buffer, float level) override
    {
        freePlays.push_back(static_cast<int>(buffer));
        freeLevels.push_back(level);
        return true;
    }
    // Every free-channel play starts, for the Sound Mixer's placed starts.
    std::uint32_t GetSoundStarts(ESound) const override
    {
        return static_cast<std::uint32_t>(freePlays.size());
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
        return 0.f;
    }
    float GetSFXVolume() const override
    {
        return 0.f;
    }
};

// The recording backend for one test, and the table as it was.
class AudioScope
{
public:
    AudioScope() : m_previous(g_platformAudio), m_table(g_MapObjectSoundDatabase.GetAll())
    {
        g_platformAudio = &audio;
        ForgetLoadedSounds();
    }
    ~AudioScope()
    {
        g_platformAudio = m_previous;
        ForgetLoadedSounds();
        g_MapObjectSoundDatabase.Build(m_table);
    }
    AudioScope(const AudioScope&) = delete;
    AudioScope& operator=(const AudioScope&) = delete;

    RecordingAudio audio;

private:
    mu::IPlatformAudio* m_previous;
    std::vector<MapObjectSound> m_table;
};
} // namespace

TEST_CASE("Map object sounds: entries read back as written, each on its own line (MP1) [data][maps]")
{
    MapObjectSound stage = Waterfall();
    stage.maps = {{9, 2}};
    stage.objects = {5};
    stage.channels = 1;
    const std::vector<MapObjectSound> sounds = {Waterfall(), stage};

    const std::string text = Data::Maps::WriteMapObjectSoundsJson(sounds);
    const ReadResult read = Read(text);

    INFO(text);
    INFO(Messages(read.issues));
    CHECK(read.issues.empty());
    CHECK(read.sounds == sounds);
    CHECK(text.find(R"({"maps": [24, 25, 26, 27, 28, 29, 36], "objects": [37, 38], "sound": {"file": )"
                    R"("Data/Sound/aKalimaWaterFall.wav", "channels": 3}, "plays": "whileInView"})") !=
          std::string::npos);
}

TEST_CASE("Map object sounds: a missing plays is while in view, a missing channel count one [data][maps]")
{
    const ReadResult read =
        Read(File(R"({"maps": [24], "objects": [37], "sound": {"file": "Data/Sound/aKalimaWaterFall.wav"}})"));
    INFO(Messages(read.issues));
    REQUIRE(read.sounds.size() == 1);
    CHECK(read.sounds[0].channels == 1);
    CHECK(read.sounds[0].plays == Data::Maps::MapObjectSoundPlay::WhileInView);
}

TEST_CASE("Map object sounds: wrong values are fatal and name the entry and field [data][maps]")
{
    struct Case
    {
        std::string entry;
        std::string field;
        std::string message;
    };
    const std::string sound = R"("sound": {"file": "Data/Sound/a.wav"})";
    const std::vector<Case> cases = {
        {R"("not an object")", "objectSounds[0]", "maps and sound"},
        {R"({"objects": [37], )" + sound + "}", "objectSounds[0]", "maps and sound"},
        {R"({"maps": [], "objects": [37], )" + sound + "}", "objectSounds[0].maps", "one map or more"},
        {R"({"maps": [24], "objects": [], )" + sound + "}", "objectSounds[0].objects", "left out: any object"},
        {R"({"maps": [24], "objects": [160], )" + sound + "}", "objectSounds[0].objects[0]", "from 0 to 159"},
        {R"({"maps": [24], "objects": [37, 37], )" + sound + "}", "objectSounds[0].objects", "37 more than once"},
        {R"({"maps": [24], "objects": [37], "sound": {"channels": 3}})", "objectSounds[0].sound", "with file"},
        {R"({"maps": [24], "objects": [37], "sound": {"file": "C:/a.wav"}})", "objectSounds[0].sound.file",
         "relative path"},
        {R"({"maps": [24], "objects": [37], "sound": {"file": "Data/Sound/a.wav", "channels": 0}})",
         "objectSounds[0].sound.channels", "from 1 to 4"},
        {R"({"maps": [24], "objects": [37], "sound": {"file": "Data/Sound/a.wav", "channels": 5}})",
         "objectSounds[0].sound.channels", "from 1 to 4"},
        {R"({"maps": [24], "objects": [37], )" + sound + R"(, "plays": "always"})", "objectSounds[0].plays",
         "whileInView"},
    };
    for (const Case& test : cases)
    {
        const ReadResult read = Read(File(test.entry));
        INFO(test.entry);
        INFO(Messages(read.issues));
        CHECK(read.sounds.empty());
        CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, test.field, test.message));
    }

    // Unknown fields are warnings; the entry stays.
    const ReadResult read = Read(R"({ "formatVersion": 1, "extra": 1, "objectSounds": [ {"maps": [24], )"
                                 R"("objects": [37], "sound": {"file": "Data/Sound/a.wav", "loop": true}, )"
                                 R"("volume": 1} ] })");
    INFO(Messages(read.issues));
    CHECK_FALSE(HasFatal(read.issues));
    CHECK(read.sounds.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "extra", "unknown"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "objectSounds[0].volume", "unknown"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "objectSounds[0].sound.loop", "unknown"));
}

TEST_CASE("Map object sounds: an entry without objects plays for any object of its maps, at full volume (Q-MP8) "
          "[data][maps]")
{
    const ReadResult read = Read(
        File(R"({"maps": [37], "sound": {"file": "Data/Sound/w37/kan_ruin_global.wav"}, "plays": "whileInView"})"));
    INFO(Messages(read.issues));
    CHECK(read.issues.empty());
    REQUIRE(read.sounds.size() == 1);
    const MapObjectSound hum = read.sounds[0];
    CHECK(hum.anyObject);
    CHECK(hum.objects.empty());
    const std::string text = Data::Maps::WriteMapObjectSoundsJson(std::vector<MapObjectSound>{hum});
    INFO(text);
    CHECK(text.find("objects") == std::string::npos);
    CHECK(Read(text).sounds == std::vector<MapObjectSound>{hum});

    // A type's own sound and the hum on one map are no clash.
    MapObjectSound wheel = Waterfall();
    wheel.maps = {{37}};
    wheel.objects = {46};
    wheel.file = "Data/Sound/w37/kan_ruin_wheel.wav";
    wheel.channels = 1;
    std::vector<ItemDataIssue> issues;
    Data::Maps::ValidateMapObjectSounds(std::vector<MapObjectSound>{wheel, hum, hum}, "ObjectSounds.json", issues);
    INFO(Messages(issues));
    CHECK(issues.empty());

    AudioScope scope;
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{wheel, hum});
    World::MapObjectSounds::LoadForMap(37, 0);
    const int wheelNumber = static_cast<int>(Audio::SoundNumbers::ForFile(L"Data\\Sound\\w37\\kan_ruin_wheel.wav"));
    const int humNumber = static_cast<int>(Audio::SoundNumbers::ForFile(L"Data\\Sound\\w37\\kan_ruin_global.wav"));
    CHECK(scope.audio.loads.size() == 2);
    CHARACTER hero;
    CHARACTER* const previousHero = Hero;
    Hero = &hero;
    OBJECT wheelObject;
    wheelObject.Type = 46;
    OBJECT farObject;
    farObject.Type = 5;
    farObject.Position[0] = 5000.0f;
    World::MapObjectSounds::OnObjectDrawn(wheelObject);
    World::MapObjectSounds::OnObjectDrawn(farObject);
    World::MapObjectSounds::PlayDrawnSounds();
    // The hum once a frame, however many objects are drawn.
    CHECK(scope.audio.freePlays == std::vector<int>{wheelNumber, humNumber});
    CHECK(scope.audio.freeLevels == std::vector<float>{1.0f, 1.0f});
    scope.audio.freePlays.clear();
    World::MapObjectSounds::OnObjectDrawn(farObject);
    World::MapObjectSounds::OnObjectDrawn(farObject);
    World::MapObjectSounds::PlayDrawnSounds();
    CHECK(scope.audio.freePlays == std::vector<int>{humNumber});
    // Not a type of its own.
    CHECK_FALSE(World::MapObjectSounds::FindSound(5));

    // Another map: no hum.
    scope.audio.freePlays.clear();
    World::MapObjectSounds::LoadForMap(38, 0);
    World::MapObjectSounds::OnObjectDrawn(farObject);
    World::MapObjectSounds::PlayDrawnSounds();
    CHECK(scope.audio.freePlays.empty());
    Hero = previousHero;
    World::MapObjectSounds::LoadForMap(-1, 0);
}

TEST_CASE("Map object sounds: an object type on a map in two entries is fatal [data][maps]")
{
    MapObjectSound first = Waterfall();
    first.maps = {{24}, {25}};
    first.objects = {37};
    MapObjectSound second = Waterfall();
    second.maps = {{25}};
    second.objects = {37, 40};
    MapObjectSound otherMap = Waterfall();
    otherMap.maps = {{26}};
    otherMap.objects = {37};
    std::vector<ItemDataIssue> issues;

    Data::Maps::ValidateMapObjectSounds(std::vector<MapObjectSound>{first, second, otherMap}, "ObjectSounds.json",
                                        issues);

    INFO(Messages(issues));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "objectSounds[1].objects",
                        "object 37 on map 25 is in objectSounds[0] too"));
    CHECK(issues.size() == 1);

    // A whole map and one of its stages overlap; two stages do not.
    MapObjectSound wholeMap = Waterfall();
    wholeMap.maps = {{9}};
    MapObjectSound stageOne = Waterfall();
    stageOne.maps = {{9, 1}};
    MapObjectSound stageTwo = Waterfall();
    stageTwo.maps = {{9, 2}};
    issues.clear();
    Data::Maps::ValidateMapObjectSounds(std::vector<MapObjectSound>{stageOne, stageTwo}, "ObjectSounds.json", issues);
    CHECK(issues.empty());
    Data::Maps::ValidateMapObjectSounds(std::vector<MapObjectSound>{wholeMap, stageTwo}, "ObjectSounds.json", issues);
    CHECK(HasFatal(issues));
}

TEST_CASE("Map object sounds: a file that is not in the client folder is a warning [data][maps]")
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu-map-object-sounds";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder / "Data" / "Sound");
    std::ofstream(folder / "Data" / "Sound" / "here.wav") << "RIFF";
    const std::filesystem::path previous = std::filesystem::current_path();
    std::filesystem::current_path(folder);
    MapObjectSound here = Waterfall();
    here.file = "Data/Sound/here.wav";
    MapObjectSound missing = Waterfall();
    missing.file = "Data/Sound/missing.wav";
    std::vector<ItemDataIssue> issues;

    Data::Maps::ValidateMapObjectSoundFiles(std::vector<MapObjectSound>{here, missing}, "ObjectSounds.json", issues);

    std::filesystem::current_path(previous);
    std::filesystem::remove_all(folder);
    INFO(Messages(issues));
    REQUIRE(issues.size() == 1);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "objectSounds[1].sound.file", "not in the client"));
}

TEST_CASE("Map object sounds: the shipped file has the Kalima waterfalls and Kanturu's object sounds, and is written "
          "as the writer writes it (MP1, MP1b) [data][maps]")
{
    const std::filesystem::path file = std::filesystem::path(MU_TEST_DATA_DIR) / "Maps" / "ObjectSounds.json";
    std::ifstream stream(file, std::ios::binary);
    REQUIRE(stream);
    const std::string text((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    const ReadResult read = Read(text);
    std::vector<ItemDataIssue> issues = read.issues;
    Data::Maps::ValidateMapObjectSounds(read.sounds, "ObjectSounds.json", issues);

    INFO(Messages(issues));
    CHECK(issues.empty());
    // What Kanturu Ruins' and Relics' object code played, compared once for
    // every object type of the two maps when it moved (MP1b).
    const auto kanturu = [](int map, std::vector<int> objects, const std::string& file)
    {
        MapObjectSound sound;
        sound.maps = {{map}};
        sound.anyObject = objects.empty();
        sound.objects = std::move(objects);
        sound.file = file;
        return sound;
    };
    const std::vector<MapObjectSound> shipped = {
        Waterfall(),
        kanturu(37, {46}, "Data/Sound/w37/kan_ruin_wheel.wav"),
        kanturu(37, {77}, "Data/Sound/w37/kan_ruin_waterfall.wav"),
        kanturu(37, {92}, "Data/Sound/w37/kan_ruin_elec.wav"),
        kanturu(37, {98}, "Data/Sound/w37/kan_ruin_plant.wav"),
        kanturu(37, {}, "Data/Sound/w37/kan_ruin_global.wav"),
        kanturu(38, {9}, "Data/Sound/w38/kan_relic_gear.wav"),
        kanturu(38, {31, 35, 36, 37}, "Data/Sound/w38/kan_relic_incubator.wav"),
        kanturu(38, {}, "Data/Sound/w38/kan_relic_global.wav"),
    };
    CHECK(read.sounds == shipped);
    std::string unixText = text;
    std::erase(unixText, '\r');
    CHECK(unixText == Data::Maps::WriteMapObjectSoundsJson(read.sounds));
}

TEST_CASE("Map object sounds: a drawn object of an entry's type plays its sound on a free channel on its maps only "
          "(D-MP3) [data][maps]")
{
    AudioScope scope;
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{Waterfall()});
    const ESound number = Audio::SoundNumbers::ForFile(L"Data\\Sound\\aKalimaWaterFall.wav");
    OBJECT waterfall;
    waterfall.Type = 37;
    OBJECT other;
    other.Type = 36;
    // Before a map loads, no type has a sound.
    CHECK_FALSE(World::MapObjectSounds::FindSound(37));

    World::MapObjectSounds::LoadForMap(24, 0);
    REQUIRE(scope.audio.loads.size() == 1);
    CHECK(scope.audio.loads[0].first == static_cast<int>(number));
    CHECK(scope.audio.loads[0].second == L"Data\\Sound\\aKalimaWaterFall.wav");
    World::MapObjectSounds::OnObjectDrawn(waterfall);
    World::MapObjectSounds::OnObjectDrawn(waterfall);
    World::MapObjectSounds::OnObjectDrawn(other);
    // They start after the drawn objects.
    CHECK(scope.audio.freePlays.empty());
    World::MapObjectSounds::PlayDrawnSounds();
    CHECK(scope.audio.freePlays == std::vector<int>{static_cast<int>(number), static_cast<int>(number)});
    CHECK(scope.audio.plays.empty());
    CHECK(World::MapObjectSounds::FindSound(38) == number);
    CHECK_FALSE(World::MapObjectSounds::FindSound(36));

    // Another map: nothing loads or plays.
    scope.audio.freePlays.clear();
    World::MapObjectSounds::LoadForMap(2, 0);
    World::MapObjectSounds::OnObjectDrawn(waterfall);
    World::MapObjectSounds::PlayDrawnSounds();
    CHECK(scope.audio.freePlays.empty());
    CHECK_FALSE(World::MapObjectSounds::FindSound(37));

    // A change of the table (the Map Editor) reaches the map shown.
    World::MapObjectSounds::LoadForMap(24, 0);
    MapObjectSound edited = Waterfall();
    edited.objects = {36};
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{edited});
    scope.audio.freePlays.clear();
    World::MapObjectSounds::OnObjectDrawn(other);
    World::MapObjectSounds::PlayDrawnSounds();
    CHECK(scope.audio.freePlays == std::vector<int>{static_cast<int>(number)});
    World::MapObjectSounds::OnObjectDrawn(waterfall);
    World::MapObjectSounds::PlayDrawnSounds();
    CHECK(scope.audio.freePlays.size() == 1);
    World::MapObjectSounds::LoadForMap(-1, 0);
}

TEST_CASE("Map object sounds: a sound's copies play quieter the farther its nearest drawn object is from the hero "
          "(D-MP7, a test) [data][maps]")
{
    using World::MapObjectSounds::DistanceLevel;
    CHECK(DistanceLevel(0.0f) == 1.0f);
    CHECK(DistanceLevel(300.0f) == 1.0f);
    CHECK(DistanceLevel(900.0f) == doctest::Approx(0.625f));
    CHECK(DistanceLevel(1500.0f) == doctest::Approx(0.25f));
    CHECK(DistanceLevel(5000.0f) == doctest::Approx(0.25f));

    AudioScope scope;
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{Waterfall()});
    World::MapObjectSounds::LoadForMap(24, 0);
    CHARACTER hero;
    hero.Object.Position[0] = 1000.0f;
    hero.Object.Position[1] = 1000.0f;
    CHARACTER* const previousHero = Hero;
    Hero = &hero;
    OBJECT nearObject;
    nearObject.Type = 37;
    nearObject.Position[0] = 1100.0f;
    nearObject.Position[1] = 1000.0f;
    OBJECT farObject;
    farObject.Type = 38;
    farObject.Position[0] = 1000.0f;
    farObject.Position[1] = 1900.0f;

    // Only the far one drawn: its level.
    World::MapObjectSounds::OnObjectDrawn(farObject);
    World::MapObjectSounds::PlayDrawnSounds();
    // The far one drawn first, then the near one: both copies at the near
    // one's level, whichever finds a free channel.
    World::MapObjectSounds::OnObjectDrawn(farObject);
    World::MapObjectSounds::OnObjectDrawn(nearObject);
    World::MapObjectSounds::PlayDrawnSounds();

    Hero = previousHero;
    World::MapObjectSounds::LoadForMap(-1, 0);
    REQUIRE(scope.audio.freeLevels.size() == 3);
    CHECK(scope.audio.freeLevels[0] == doctest::Approx(0.625f));
    CHECK(scope.audio.freeLevels[1] == 1.0f);
    CHECK(scope.audio.freeLevels[2] == 1.0f);
}

#ifdef _EDITOR
TEST_CASE("Map object sounds: the Sound Mixer hears them as map object sounds of the object's type (Q-SM2) "
          "[data][maps][editor]")
{
    AudioScope scope;
    Audio::Mixer::Clear();
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{Waterfall()});
    World::MapObjectSounds::LoadForMap(24, 0);
    const std::optional<ESound> sound = World::MapObjectSounds::FindSound(38);
    REQUIRE(sound);
    OBJECT waterfall;
    waterfall.Type = 38;
    World::MapObjectSounds::OnObjectDrawn(waterfall);
    World::MapObjectSounds::PlayDrawnSounds();
    const std::vector<Audio::Mixer::HeardSound> heard = Audio::Mixer::Heard();
    World::MapObjectSounds::LoadForMap(-1, 0);
    Audio::Mixer::Clear();
    REQUIRE(heard.size() == 1);
    CHECK(heard[0].number == static_cast<int>(*sound));
    CHECK(heard[0].kind == Audio::SoundKind::MapObject);
    CHECK(heard[0].sourceNumber == 38);
}

TEST_CASE("Map object sounds: the Sound Mixer's overlay marks a drawn object with its volume rings while it is on, "
          "and its start with the object's place (Q-SM9) [data][maps][editor]")
{
    AudioScope scope;
    Audio::Mixer::Clear();
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{Waterfall()});
    World::MapObjectSounds::LoadForMap(24, 0);
    const std::optional<ESound> sound = World::MapObjectSounds::FindSound(38);
    REQUIRE(sound);
    OBJECT waterfall;
    waterfall.Type = 38;
    waterfall.Position[0] = 1200.0f;
    waterfall.Position[1] = 3400.0f;
    waterfall.Position[2] = 50.0f;
    Audio::Overlay::SetOn(false);
    World::MapObjectSounds::OnObjectDrawn(waterfall);
    World::MapObjectSounds::PlayDrawnSounds();
    const bool notedWhileOff = !Audio::Overlay::TakeDrawn().empty();
    Audio::Overlay::SetOn(true);
    World::MapObjectSounds::OnObjectDrawn(waterfall);
    World::MapObjectSounds::PlayDrawnSounds();
    const std::vector<Audio::Overlay::DrawnSource> drawn = Audio::Overlay::TakeDrawn();
    Audio::Overlay::SetOn(false);
    const std::vector<Audio::Mixer::PlacedStart> starts = Audio::Mixer::RecentStarts(Audio::Mixer::Now(), 60.0);
    World::MapObjectSounds::LoadForMap(-1, 0);
    Audio::Mixer::Clear();
    CHECK_FALSE(notedWhileOff);
    REQUIRE(drawn.size() == 1);
    CHECK(drawn[0].kind == Audio::SoundKind::MapObject);
    CHECK(drawn[0].sourceNumber == 38);
    CHECK(drawn[0].soundNumber == static_cast<int>(*sound));
    CHECK(drawn[0].position == std::array<float, 3>{1200.0f, 3400.0f, 50.0f});
    CHECK(drawn[0].inner == 300.0f);
    CHECK(drawn[0].outer == 1500.0f);
    REQUIRE_FALSE(starts.empty());
    CHECK(starts.back().kind == Audio::SoundKind::MapObject);
    CHECK(starts.back().sourceNumber == 38);
    CHECK(starts.back().position == std::array<float, 3>{1200.0f, 3400.0f, 50.0f});
}
#endif // _EDITOR
