#include "App/stdafx.h"

// The editor's Sound Mixer (docs/superpowers/specs/2026-11-01-sound-mixer-design.md):
// its rules, the kinds of the sounds' sources, the heard list, the reminder's
// counts, the gains it gives the backend and the PlayBuffer hook.

#include "doctest.h"

#ifdef _EDITOR
#include "App/Platform/Windows/Winmain.h"
#include "Audio/DSPlaySound.h"
#include "Audio/SoundLoads.h"
#include "Audio/SoundMixer.h"
#include "Audio/SoundOverlay.h"
#include "Audio/SoundPlaces.h"
#include "Audio/SoundSource.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Effects/ZzzEffect.h"
#include "Camera/CameraState.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Core/Utilities/DrawSpawnCheck.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "TestImGuiContext.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/SoundEditor/SoundSelection.h"
#include "UI/SoundMixer/MuSoundMixerUI.h"
#include "UI/SoundMixer/MuSoundOverlay.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace
{
using Audio::SoundKind;
using Audio::Mixer::HeardSound;

// A backend that records the mixer's gains and counts a start for each play
// while `starts` is true.
class GainAudio : public mu::IPlatformAudio
{
public:
    bool Initialize() override
    {
        return true;
    }
    void Shutdown() override {}
    void LoadSound(ESound, const wchar_t*, int, bool) override {}
    bool PlaySound(ESound buffer, const void*, bool) override
    {
        plays.push_back(static_cast<int>(buffer));
        if (starts)
        {
            ++started[static_cast<int>(buffer)];
        }
        return true;
    }
    std::uint32_t GetSoundStarts(ESound buffer) const override
    {
        const auto found = started.find(static_cast<int>(buffer));
        return found != started.end() ? found->second : 0;
    }
    bool IsSoundPlaying(ESound buffer) const override
    {
        return playing.contains(static_cast<int>(buffer));
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
    void PlayMusic(const char* name, bool) override
    {
        if (music != name)
        {
            music = name;
            ++musicStarts;
        }
    }
    std::uint32_t GetMusicStarts() const override
    {
        return musicStarts;
    }
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
    void SetSoundMixGain(ESound buffer, int group, float gain) override
    {
        gains.emplace_back(static_cast<int>(buffer), gain);
        gainGroups.push_back(group);
    }
    void SetMusicMixGain(float gain) override
    {
        musicGains.push_back(gain);
    }

    std::vector<int> plays;
    bool starts = true;
    std::map<int, std::uint32_t> started;
    std::set<int> playing;
    std::vector<std::pair<int, float>> gains;
    // The group of each of `gains`.
    std::vector<int> gainGroups;
    std::vector<float> musicGains;
    std::string music;
    std::uint32_t musicStarts = 0;
};

// The mixer cleared and a recording backend for the life of the scope.
class MixerScope
{
public:
    MixerScope() : m_previous(g_platformAudio)
    {
        Audio::Mixer::Clear();
        g_platformAudio = &audio;
    }
    ~MixerScope()
    {
        g_platformAudio = m_previous;
        Audio::Mixer::Clear();
    }
    MixerScope(const MixerScope&) = delete;
    MixerScope& operator=(const MixerScope&) = delete;

    GainAudio audio;

private:
    mu::IPlatformAudio* m_previous;
};

const HeardSound* FindHeard(const std::vector<HeardSound>& heard, int number)
{
    for (const HeardSound& sound : heard)
    {
        if (sound.number == number)
        {
            return &sound;
        }
    }
    return nullptr;
}
} // namespace

TEST_CASE("Sound Mixer: a sound plays at its kind's volume times its own, mute wins, solo plays only what is solo "
          "(Q-SM3) [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    CHECK(GainFor(1, SoundKind::Other) == 1.0f);
    KindControl(SoundKind::Other).volume = 0.5f;
    SoundControl(1).volume = 0.5f;
    CHECK(GainFor(1, SoundKind::Other) == doctest::Approx(0.25f));
    // Another kind keeps its volume.
    CHECK(GainFor(1, SoundKind::Monster) == doctest::Approx(0.5f));
    SoundControl(1).muted = true;
    CHECK(GainFor(1, SoundKind::Other) == 0.0f);
    SoundControl(1).muted = false;

    // A solo kind: only its sounds play, and not the music.
    KindControl(SoundKind::Monster).solo = true;
    CHECK(GainFor(1, SoundKind::Other) == 0.0f);
    CHECK(GainFor(2, SoundKind::Monster) == 1.0f);
    CHECK(MusicGain() == 0.0f);
    // A solo sound plays too, at its volumes.
    SoundControl(1).solo = true;
    CHECK(GainFor(1, SoundKind::Other) == doctest::Approx(0.25f));
    // Mute wins over solo.
    KindControl(SoundKind::Other).muted = true;
    CHECK(GainFor(1, SoundKind::Other) == 0.0f);

    // The music: its kind's control.
    KindControl(SoundKind::Music).solo = true;
    KindControl(SoundKind::Music).volume = 0.3f;
    CHECK(MusicGain() == doctest::Approx(0.3f));
    KindControl(SoundKind::Music).muted = true;
    CHECK(MusicGain() == 0.0f);

    // Out-of-range volumes count as their ends.
    ResetAll();
    SoundControl(3).volume = 2.0f;
    KindControl(SoundKind::Other).volume = -1.0f;
    CHECK(GainFor(3, SoundKind::Monster) == 1.0f);
    CHECK(GainFor(3, SoundKind::Other) == 0.0f);
}

TEST_CASE("Sound Mixer: a sound's type is its source's, a loop's map ambience, else everything else (Q-SM2, "
          "Q-SM13) [audio][mixer][editor]")
{
    MixerScope scope;
    using Audio::Mixer::OnPlay;
    OnPlay(5, false);
    OnPlay(6, true);
    {
        const Audio::ScopedSoundSource monster(SoundKind::Monster, "Death Rider", 306);
        OnPlay(7, false);
        {
            const Audio::ScopedSoundSource object(SoundKind::MapObject, {}, 37);
            OnPlay(8, true);
        }
        OnPlay(9, false);
    }
    OnPlay(10, false);
    CHECK(Audio::CurrentSoundSource() == nullptr);

    const std::vector<HeardSound> heard = Audio::Mixer::Heard();
    REQUIRE(heard.size() == 6);
    const auto kindOf = [&](int number)
    {
        const HeardSound* sound = FindHeard(heard, number);
        REQUIRE(sound != nullptr);
        return sound->kind;
    };
    CHECK(kindOf(5) == SoundKind::Other);
    CHECK(kindOf(6) == SoundKind::MapAmbience);
    CHECK(kindOf(7) == SoundKind::Monster);
    CHECK(kindOf(8) == SoundKind::MapObject);
    CHECK(kindOf(9) == SoundKind::Monster);
    CHECK(kindOf(10) == SoundKind::Other);
    CHECK(FindHeard(heard, 7)->name == "Death Rider");
    CHECK(FindHeard(heard, 7)->sourceNumber == 306);
    CHECK(FindHeard(heard, 8)->sourceNumber == 37);
    CHECK(FindHeard(heard, 10)->sourceNumber == -1);
    CHECK(MuEditor::SoundMixer::SourceText(*FindHeard(heard, 7)) == "Monsters: Death Rider (306)");
    CHECK(MuEditor::SoundMixer::SourceText(*FindHeard(heard, 8)) == "Map object sounds: object 37");
}

TEST_CASE("Sound Mixer: the heard list keeps each sound where it was first heard and counts the backend's starts, and "
          "Clear all also unmutes and drops the changed sounds [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    const auto play = [&scope](int number, double now, bool starts)
    {
        scope.audio.starts = starts;
        OnPlay(number, false);
        if (starts)
        {
            ++scope.audio.started[number];
        }
        OnPlayed(number, now);
    };
    play(1, 0.0, true);
    play(2, 5.0, true);
    play(1, 6.0, true);
    // A call while it plays (a loop, a map object's sound each frame) is no
    // start.
    play(2, 7.0, false);
    std::vector<HeardSound> heard = Heard();
    REQUIRE(heard.size() == 2);
    CHECK(heard[0].number == 1);
    CHECK(heard[0].plays == 2);
    CHECK(heard[0].lastStarted == 6.0);
    CHECK(heard[1].number == 2);
    CHECK(heard[1].plays == 1);
    CHECK(heard[1].lastStarted == 5.0);
    // A start that no OnPlay announced is not counted.
    ++scope.audio.started[1];
    OnPlayed(1, 8.0);
    CHECK(Heard()[0].plays == 2);

    // Clear all: every row goes, a muted and a lowered one too, and their
    // controls are back to normal; the types' controls stay.
    SoundControl(1).muted = true;
    SoundControl(2).volume = 0.4f;
    KindControl(SoundKind::Monster).muted = true;
    scope.audio.gains.clear();
    ClearHeard();
    CHECK(Heard().empty());
    CHECK_FALSE(SoundControl(1).muted);
    CHECK(SoundControl(2).volume == 1.0f);
    CHECK(KindControl(SoundKind::Monster).muted);
    CHECK(CountChanges().muted == 1);
    // The backend hears it at once: sound 1 plays again.
    REQUIRE_FALSE(scope.audio.gains.empty());
    CHECK(std::find(scope.audio.gains.begin(), scope.audio.gains.end(), std::pair<int, float>{1, 1.0f}) !=
          scope.audio.gains.end());
    KindControl(SoundKind::Monster).muted = false;
    // The sounds heard after it are listed as new.
    play(3, 9.0, true);
    play(1, 10.0, true);
    heard = Heard();
    REQUIRE(heard.size() == 2);
    CHECK(heard[0].number == 3);
    CHECK(heard[1].number == 1);
    CHECK(heard[1].plays == 1);

    // A cleared sound still gets its gain: it may play on.
    ClearHeard();
    CHECK(Heard().empty());
    scope.audio.gains.clear();
    KindControl(SoundKind::Other).muted = true;
    Apply();
    CHECK(scope.audio.gains == std::vector<std::pair<int, float>>{{1, 0.0f}, {2, 0.0f}, {3, 0.0f}});
}

TEST_CASE("Sound Mixer: a row says how long its sound is and when it last started [audio][mixer][editor]")
{
    using namespace MuEditor::SoundMixer;
    CHECK(LengthText(1.42f, false) == "1.4 s");
    CHECK(LengthText(6.0f, true) == "6.0 s loop");
    CHECK(LengthText(0.0f, true).empty());
    HeardSound sound;
    CHECK(LastText(sound, 10.0, false).empty());
    CHECK(LastText(sound, 10.0, true) == "playing");
    sound.lastStarted = 7.5;
    CHECK(LastText(sound, 10.0, false) == "2.5 s ago");
    CHECK(LastText(sound, 10.0, true) == "playing");
}

namespace
{
using MuEditor::SoundMixer::HeardRow;

HeardRow Row(int number, const std::string& file, const std::string& source, int plays)
{
    HeardRow row;
    row.sound.number = number;
    row.sound.plays = plays;
    row.file = file;
    row.source = source;
    return row;
}

std::vector<int> NumbersOf(const std::vector<HeardRow>& rows)
{
    std::vector<int> numbers;
    for (const HeardRow& row : rows)
    {
        numbers.push_back(row.sound.number);
    }
    return numbers;
}
} // namespace

TEST_CASE("Sound Mixer: the filter bar leaves the rows that pass every filter set (Q-SM7) [audio][mixer][editor]")
{
    using namespace MuEditor::SoundMixer;
    HeardRow waterfall = Row(1, "aKalimaWaterFall.wav", "Map object sounds: object 37", 30);
    waterfall.sound.kind = SoundKind::MapObject;
    HeardRow idle = Row(2, "aida/dr_idle01.wav", "Monster sounds: Death Rider (306)", 12);
    idle.sound.kind = SoundKind::Monster;
    idle.playing = true;
    HeardRow wind = Row(3, "aWind.wav", "Map ambience (loops)", 1);
    wind.sound.kind = SoundKind::MapAmbience;
    wind.sound.looped = true;
    wind.changed = true;
    const std::vector<HeardRow> rows{waterfall, idle, wind};

    HeardFilter filter;
    CHECK_FALSE(filter.IsActive());
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{1, 2, 3});
    // Text: the file or the source, whatever the case.
    filter.text = "WATERFALL";
    CHECK(filter.IsActive());
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{1});
    filter.text = "death rider";
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{2});
    filter = {};
    filter.kind = SoundKind::MapAmbience;
    CHECK(filter.IsActive());
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{3});
    filter = {};
    filter.show = ShowFilter::Playing;
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{2});
    filter.show = ShowFilter::Changed;
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{3});
    filter = {};
    filter.loops = LoopFilter::Loops;
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{3});
    filter.loops = LoopFilter::OneShots;
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{1, 2});
    filter = {};
    filter.minimumPlays = 12;
    CHECK(filter.IsActive());
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{1, 2});
    // Every filter at once.
    filter.text = "a";
    filter.show = ShowFilter::Playing;
    CHECK(NumbersOf(FilterRows(rows, filter)) == std::vector<int>{2});
}

TEST_CASE("Sound Mixer: a header sorts by its column, again reverses, none keeps the order of first hearing "
          "(Q-SM7) [audio][mixer][editor]")
{
    using namespace MuEditor::SoundMixer;
    HeardRow first = Row(1, "B.wav", "Monster sounds", 5);
    first.length = 2.0f;
    first.sound.lastStarted = 9.0;
    HeardRow second = Row(2, "a.wav", "everything else", 5);
    second.length = 0.5f;
    second.sound.lastStarted = 2.0;
    HeardRow third = Row(3, "c.wav", "Map object sounds", 1);
    third.length = 1.0f;
    third.playing = true;
    HeardRow fourth = Row(4, "d.wav", "map ambience", 7);
    fourth.length = 3.0f;
    const std::vector<HeardRow> rows{first, second, third, fourth};
    const auto sorted = [&rows](std::optional<HeardColumn> column, bool descending)
    {
        std::vector<HeardRow> copy = rows;
        SortRows(copy, HeardSort{column, descending}, 10.0);
        return NumbersOf(copy);
    };
    CHECK(sorted(std::nullopt, false) == std::vector<int>{1, 2, 3, 4});
    // Text without case.
    CHECK(sorted(HeardColumn::Sound, false) == std::vector<int>{2, 1, 3, 4});
    CHECK(sorted(HeardColumn::Sound, true) == std::vector<int>{4, 3, 1, 2});
    CHECK(sorted(HeardColumn::Source, false) == std::vector<int>{2, 4, 3, 1});
    // Texts alike but their case keep their order, both ways.
    std::vector<HeardRow> alike{Row(5, "e.wav", "Monster sounds", 0), Row(6, "E.wav", "monster sounds", 0)};
    for (const bool descending : {false, true})
    {
        for (const HeardColumn column : {HeardColumn::Sound, HeardColumn::Source})
        {
            std::vector<HeardRow> copy = alike;
            SortRows(copy, HeardSort{column, descending}, 10.0);
            CHECK(NumbersOf(copy) == std::vector<int>{5, 6});
        }
    }
    // Equal rows keep their order.
    CHECK(sorted(HeardColumn::Plays, false) == std::vector<int>{3, 1, 2, 4});
    CHECK(sorted(HeardColumn::Plays, true) == std::vector<int>{4, 1, 2, 3});
    CHECK(sorted(HeardColumn::Length, false) == std::vector<int>{2, 3, 1, 4});
    // Last played: playing, then the most recent, never last.
    CHECK(sorted(HeardColumn::LastPlayed, false) == std::vector<int>{3, 1, 2, 4});
    CHECK(sorted(HeardColumn::LastPlayed, true) == std::vector<int>{4, 2, 1, 3});
}

TEST_CASE("Sound Mixer: Mute all and Unmute all act on the rows given, at once (Q-SM8) [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace MuEditor::SoundMixer;
    Audio::Mixer::OnPlay(1, false);
    Audio::Mixer::OnPlay(2, false);
    Audio::Mixer::OnPlay(3, false);
    scope.audio.gains.clear();
    SetMuted({Row(1, "", "", 0), Row(3, "", "", 0)}, true);
    CHECK(Audio::Mixer::SoundControl(1).muted);
    CHECK_FALSE(Audio::Mixer::SoundControl(2).muted);
    CHECK(Audio::Mixer::SoundControl(3).muted);
    CHECK(scope.audio.gains == std::vector<std::pair<int, float>>{{1, 0.0f}, {2, 1.0f}, {3, 0.0f}});
    CHECK(MakeRow(Audio::Mixer::Heard()[0]).changed);
    CHECK_FALSE(MakeRow(Audio::Mixer::Heard()[1]).changed);
    SetMuted({Row(3, "", "", 0)}, false);
    CHECK(Audio::Mixer::SoundControl(1).muted);
    CHECK_FALSE(Audio::Mixer::SoundControl(3).muted);
    CHECK(scope.audio.gains.back() == std::pair{3, 1.0f});
}

TEST_CASE("Sound Mixer: a start whose source has a place is kept a few seconds for the overlay's flash (Q-SM9) "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    const float place[3] = {100.0f, 200.0f, 30.0f};
    const auto play = [&scope](int number, double now, bool starts)
    {
        OnPlay(number, false);
        if (starts)
        {
            ++scope.audio.started[number];
        }
        OnPlayed(number, now);
    };
    {
        const Audio::ScopedSoundSource source(SoundKind::Monster, "Death Rider", 306, place);
        play(7, 10.0, true);
        // A call that starts nothing flashes nothing.
        play(7, 10.2, false);
    }
    {
        // A source without a place neither.
        const Audio::ScopedSoundSource source(SoundKind::MapObject, {}, 37);
        play(8, 10.5, true);
    }
    std::vector<PlacedStart> recent = RecentStarts(10.8, 1.0);
    REQUIRE(recent.size() == 1);
    CHECK(recent[0].number == 7);
    CHECK(recent[0].kind == SoundKind::Monster);
    CHECK(recent[0].name == "Death Rider");
    CHECK(recent[0].sourceNumber == 306);
    CHECK(recent[0].position == std::array<float, 3>{100.0f, 200.0f, 30.0f});
    CHECK(recent[0].time == 10.0);
    CHECK(RecentStarts(11.5, 1.0).empty());
    // A few seconds are kept: a new start drops the older ones.
    {
        const Audio::ScopedSoundSource source(SoundKind::Monster, "Death Rider", 306, place);
        play(7, 20.0, true);
    }
    recent = RecentStarts(20.0, 100.0);
    REQUIRE(recent.size() == 1);
    CHECK(recent[0].time == 20.0);
}

TEST_CASE("Sound Mixer: the overlay notes drawn sources and keeps the camera only while it is on, and hands a "
          "frame's sources over once (Q-SM10) [audio][mixer][editor]")
{
    using namespace Audio::Overlay;
    SetOn(false);
    DrawnSource source;
    source.sourceNumber = 37;
    NoteDrawn(source);
    CHECK(TakeDrawn().empty());
    const CameraState camera;
    KeepCamera(camera);
    CHECK(KeptCamera() == nullptr);
    SetOn(true);
    CHECK(IsOn());
    NoteDrawn(source);
    KeepCamera(camera);
    CHECK(KeptCamera() != nullptr);
    const std::vector<DrawnSource> drawn = TakeDrawn();
    REQUIRE(drawn.size() == 1);
    CHECK(drawn[0].sourceNumber == 37);
    CHECK(TakeDrawn().empty());
    // Off forgets the sources and the camera.
    NoteDrawn(source);
    SetOn(false);
    CHECK_FALSE(IsOn());
    CHECK(TakeDrawn().empty());
    CHECK(KeptCamera() == nullptr);
}

TEST_CASE("Sound Mixer: the overlay draws the sources a shown row names while a filter is set, and a monster's "
          "ring holds who hears it (Q-SM9, Q-SM10) [audio][mixer][editor]")
{
    using Audio::Overlay::DrawnSource;
    using namespace MuEditor::SoundOverlay;
    DrawnSource waterfall;
    waterfall.kind = SoundKind::MapObject;
    waterfall.sourceNumber = 37;
    DrawnSource rider;
    rider.kind = SoundKind::Monster;
    rider.name = "Death Rider";
    rider.sourceNumber = 306;
    rider.position = {1000.0f, 1000.0f, 0.0f};
    rider.inner = 500.0f;
    DrawnSource orc = rider;
    orc.name = "Forest Orc";
    orc.sourceNumber = 307;
    const std::vector<DrawnSource> drawn{waterfall, rider, orc};

    HeardRow riderRow = Row(1, "", "", 0);
    riderRow.sound.kind = SoundKind::Monster;
    riderRow.sound.name = "Death Rider";
    riderRow.sound.sourceNumber = 306;
    // An attack's sound names no set.
    HeardRow orcAttack = Row(2, "", "", 0);
    orcAttack.sound.kind = SoundKind::Monster;
    orcAttack.sound.sourceNumber = 307;
    HeardRow waterRow = Row(3, "", "", 0);
    waterRow.sound.kind = SoundKind::MapObject;
    waterRow.sound.sourceNumber = 37;
    CHECK(Names(riderRow, SoundKind::Monster, "Death Rider", 306));
    CHECK_FALSE(Names(riderRow, SoundKind::Monster, "Forest Orc", 306));
    CHECK_FALSE(Names(riderRow, SoundKind::Monster, "Death Rider", 307));
    CHECK_FALSE(Names(riderRow, SoundKind::MapObject, "", 306));
    CHECK(Names(orcAttack, SoundKind::Monster, "Forest Orc", 307));
    CHECK(Names(waterRow, SoundKind::MapObject, "", 37));

    const auto numbersOf = [](const std::vector<DrawnSource>& sources)
    {
        std::vector<int> numbers;
        for (const DrawnSource& source : sources)
        {
            numbers.push_back(source.sourceNumber);
        }
        return numbers;
    };
    // No filter: every source in view, whatever the rows.
    CHECK(numbersOf(Followed(drawn, {}, false)) == std::vector<int>{37, 306, 307});
    CHECK(numbersOf(Followed(drawn, {riderRow}, true)) == std::vector<int>{306});
    CHECK(numbersOf(Followed(drawn, {waterRow, orcAttack}, true)) == std::vector<int>{37, 307});
    CHECK(Followed(drawn, {}, true).empty());

    CHECK(IsInside(rider, 1300.0f, 1400.0f));
    CHECK_FALSE(IsInside(rider, 1400.0f, 1400.0f));
    // No ring: heard anywhere.
    DrawnSource anywhere = rider;
    anywhere.inner = 0.0f;
    CHECK(IsInside(anywhere, 90000.0f, 1400.0f));
}

TEST_CASE("Sound Mixer: the overlay draws the code's sounds of a place below the label of a set there (Q-SM11) "
          "[sounds][editor]")
{
    using MuEditor::SoundOverlay::LabelLine;
    using MuEditor::SoundOverlay::LabelTop;
    constexpr float gap = 2.0f;
    // A set's label: centred on its marker.
    CHECK(LabelTop(100.0f, 14.0f, LabelLine::Beside) == 93.0f);
    // The code's sounds: right below it, the backgrounds (a gap around the
    // text) touching but not overlapping.
    const float below = LabelTop(100.0f, 14.0f, LabelLine::Below);
    CHECK(below - gap == LabelTop(100.0f, 14.0f, LabelLine::Beside) + 14.0f + gap);
    CHECK(LabelTop(40.0f, 20.0f, LabelLine::Below) - gap == 30.0f + 20.0f + gap);
}

TEST_CASE("Sound Mixer: the overlay places a window pixel where the game window is, at its size (Q-SM9) "
          "[audio][mixer][editor]")
{
    using MuEditor::SoundOverlay::ToDisplay;
    const auto at = [](const ImVec2& point) { return std::pair{point.x, point.y}; };
    // The editor's viewports count from the desktop's corner: the window's place.
    CHECK(at(ToDisplay(100.0f, 50.0f, ImVec2(447.0f, 133.0f), ImVec2(1024.0f, 768.0f), 1024, 768)) ==
          std::pair{547.0f, 183.0f});
    // A viewport twice the window's pixels.
    CHECK(at(ToDisplay(100.0f, 50.0f, ImVec2(0.0f, 0.0f), ImVec2(2048.0f, 1536.0f), 1024, 768)) ==
          std::pair{200.0f, 100.0f});
    // No window size yet: as it is.
    CHECK(at(ToDisplay(100.0f, 50.0f, ImVec2(10.0f, 20.0f), ImVec2(800.0f, 600.0f), 0, 0)) == std::pair{110.0f, 70.0f});
}

TEST_CASE("Sound Mixer: a sound the game's own code plays gets its place's type and starts there, a source "
          "wins over a place, and the last start of each is marked while it plays (Q-SM11, Q-SM13) "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    const float wheel[3] = {100.0f, 200.0f, 30.0f};
    const float otherWheel[3] = {700.0f, 800.0f, 40.0f};
    const float rider[3] = {1000.0f, 1000.0f, 0.0f};
    const auto play = [&scope](int number, double now, bool starts, bool looped = false)
    {
        OnPlay(number, looped);
        if (starts)
        {
            ++scope.audio.started[number];
        }
        OnPlayed(number, now);
    };
    {
        const Audio::ScopedSoundPlace place(SoundKind::MapObject, 46, wheel);
        play(5, 10.0, true);
        play(6, 10.0, true, true);
        {
            // A monster drawn while the object is moved: the innermost place.
            const Audio::ScopedSoundPlace monster(SoundKind::Monster, 307, rider);
            play(9, 10.0, true);
        }
        // The object's place again.
        play(3, 10.0, true);
        {
            // The data's source wins over the place, with its position or none.
            const Audio::ScopedSoundSource source(SoundKind::Monster, "Death Rider", 306, rider);
            play(7, 10.0, true);
            const Audio::ScopedSoundSource unplaced(SoundKind::MapObject, {}, 37);
            play(8, 10.0, true);
        }
    }
    // No place left: nothing to mark.
    play(4, 10.0, true);

    const std::vector<HeardSound> heard = Heard();
    REQUIRE(FindHeard(heard, 5) != nullptr);
    CHECK(FindHeard(heard, 5)->kind == SoundKind::MapObject);
    CHECK(FindHeard(heard, 5)->sourceNumber == 46);
    CHECK(FindHeard(heard, 5)->name.empty());
    // A loop of a place is the place's, not map ambience.
    CHECK(FindHeard(heard, 6)->kind == SoundKind::MapObject);
    CHECK(FindHeard(heard, 9)->kind == SoundKind::Monster);
    CHECK(FindHeard(heard, 9)->sourceNumber == 307);
    CHECK(MuEditor::SoundMixer::SourceText(*FindHeard(heard, 9)) == "Monsters: 307");
    CHECK(FindHeard(heard, 7)->kind == SoundKind::Monster);
    CHECK(FindHeard(heard, 4)->kind == SoundKind::Other);

    const std::vector<PlacedStart> recent = RecentStarts(10.0, 1.0);
    REQUIRE(recent.size() == 5);
    CHECK(recent[0].number == 5);
    CHECK(recent[0].byCode);
    CHECK(recent[0].kind == SoundKind::MapObject);
    CHECK(recent[0].sourceNumber == 46);
    CHECK(recent[0].name.empty());
    CHECK(recent[0].position == std::array<float, 3>{100.0f, 200.0f, 30.0f});
    CHECK(recent[1].number == 6);
    CHECK(recent[2].number == 9);
    CHECK(recent[2].kind == SoundKind::Monster);
    CHECK(recent[2].sourceNumber == 307);
    CHECK(recent[3].number == 3);
    CHECK(recent[3].kind == SoundKind::MapObject);
    CHECK(recent[3].sourceNumber == 46);
    CHECK(recent[4].number == 7);
    CHECK_FALSE(recent[4].byCode);
    CHECK(recent[4].name == "Death Rider");

    const auto numbersOf = [](const std::vector<PlacedStart>& starts)
    {
        std::vector<int> numbers;
        for (const PlacedStart& start : starts)
        {
            numbers.push_back(start.number);
        }
        return numbers;
    };
    // A second after its start, also when it stopped.
    CHECK(numbersOf(CodeSounds(11.0)) == std::vector<int>{3, 5, 6, 9});
    // Later only while it plays.
    scope.audio.playing.insert(5);
    CHECK(numbersOf(CodeSounds(11.5)) == std::vector<int>{5});
    // Another place that starts it moves the mark; a call that starts nothing
    // does not.
    {
        const Audio::ScopedSoundPlace place(SoundKind::MapObject, 46, otherWheel);
        play(5, 12.0, true);
    }
    {
        const Audio::ScopedSoundPlace place(SoundKind::MapObject, 46, wheel);
        play(5, 12.5, false);
    }
    std::vector<PlacedStart> marked = CodeSounds(30.0);
    REQUIRE(marked.size() == 1);
    CHECK(marked[0].position == std::array<float, 3>{700.0f, 800.0f, 40.0f});
    CHECK(marked[0].time == 12.0);
    scope.audio.playing.clear();
    CHECK(CodeSounds(30.0).empty());
}

TEST_CASE("Sound Mixer: the overlay marks each place of the game's own code's sounds once, and a shown or hovered "
          "row of one of its sounds follows it (Q-SM11) [audio][mixer][editor]")
{
    using Audio::Mixer::PlacedStart;
    using namespace MuEditor::SoundOverlay;
    const auto codeStart = [](int number, SoundKind kind, int sourceNumber, std::array<float, 3> position)
    {
        PlacedStart start;
        start.number = number;
        start.kind = kind;
        start.sourceNumber = sourceNumber;
        start.position = position;
        start.byCode = true;
        return start;
    };
    const std::array<float, 3> here{100.0f, 200.0f, 30.0f};
    const std::array<float, 3> there{700.0f, 800.0f, 40.0f};
    const std::vector<CodeMarker> markers = GroupCodeSounds({
        codeStart(5, SoundKind::MapObject, 46, here),
        codeStart(9, SoundKind::MapObject, 46, here),
        codeStart(7, SoundKind::Monster, 46, here),
        codeStart(8, SoundKind::MapObject, 47, here),
        codeStart(6, SoundKind::MapObject, 46, there),
    });
    REQUIRE(markers.size() == 4);
    CHECK(markers[0].kind == SoundKind::MapObject);
    CHECK(markers[0].sourceNumber == 46);
    CHECK(markers[0].position == here);
    CHECK(markers[0].sounds == std::vector<int>{5, 9});
    CHECK(markers[1].kind == SoundKind::Monster);
    CHECK(markers[1].sounds == std::vector<int>{7});
    CHECK(markers[2].sourceNumber == 47);
    CHECK(markers[3].position == there);
    CHECK(markers[3].sounds == std::vector<int>{6});

    // A code start follows a row of its sound, whatever the row's kind.
    const PlacedStart wheel = codeStart(5, SoundKind::MapObject, 46, here);
    CHECK(Follows(wheel, {Row(6, "", "", 0), Row(5, "", "", 0)}));
    CHECK_FALSE(Follows(wheel, {Row(6, "", "", 0)}));
    CHECK_FALSE(Follows(wheel, {}));
    // A data start follows a row that names its source, not one of its sound.
    PlacedStart riderStart;
    riderStart.number = 5;
    riderStart.kind = SoundKind::Monster;
    riderStart.name = "Death Rider";
    riderStart.sourceNumber = 306;
    HeardRow riderRow = Row(11, "", "", 0);
    riderRow.sound.kind = SoundKind::Monster;
    riderRow.sound.name = "Death Rider";
    riderRow.sound.sourceNumber = 306;
    CHECK(Follows(riderStart, {riderRow}));
    CHECK_FALSE(Follows(riderStart, {Row(5, "", "", 0)}));

    CHECK(Highlights(Row(9, "", "", 0), markers[0]));
    CHECK(Highlights(Row(5, "", "", 0), markers[0]));
    CHECK_FALSE(Highlights(Row(7, "", "", 0), markers[0]));
}

TEST_CASE("Sound Mixer: without a source or place a sound gets the type and place of the object it plays at, and a "
          "start without a position is logged, music too (Q-SM13, Q-SM15) [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    const auto play = [&scope](int number, double now, bool looped = false, const Audio::SoundPlace* object = nullptr)
    {
        OnPlay(number, looped, object);
        ++scope.audio.started[number];
        OnPlayed(number, now);
    };
    const Audio::SoundPlace bird{SoundKind::Creature, 175, std::array<float, 3>{1.0f, 2.0f, 3.0f}};
    const float hero[3] = {50.0f, 60.0f, 0.0f};
    play(5, 10.0, false, &bird);
    {
        // A living place wins over the object.
        const Audio::ScopedSoundPlace place(SoundKind::Hero, -1, hero);
        play(6, 10.0, false, &bird);
    }
    {
        // A place without a position: logged, not marked.
        const Audio::ScopedSoundPlace ui(SoundKind::Interface, -1, nullptr);
        play(7, 10.1);
        // Its starts each get a line, also of the same sound.
        play(7, 10.3);
    }
    play(8, 10.4, true);
    play(9, 10.5);
    // A call that starts nothing logs nothing.
    OnPlay(10, false);
    OnPlayed(10, 10.6);
    OnMusicStarted("Data\\Music\\MuTheme.mp3", 10.7);

    const std::vector<HeardSound> heard = Heard();
    CHECK(FindHeard(heard, 5)->kind == SoundKind::Creature);
    CHECK(FindHeard(heard, 5)->sourceNumber == 175);
    CHECK(MuEditor::SoundMixer::SourceText(*FindHeard(heard, 5)) == "Creatures: 175");
    CHECK(FindHeard(heard, 6)->kind == SoundKind::Hero);
    CHECK(MuEditor::SoundMixer::SourceText(*FindHeard(heard, 6)) == "Your character");
    CHECK(FindHeard(heard, 7)->kind == SoundKind::Interface);
    CHECK(FindHeard(heard, 8)->kind == SoundKind::MapAmbience);
    CHECK(FindHeard(heard, 9)->kind == SoundKind::Other);

    const std::vector<PlacedStart> recent = RecentStarts(10.7, 1.0);
    REQUIRE(recent.size() == 2);
    CHECK(recent[0].number == 5);
    CHECK(recent[0].byCode);
    CHECK(recent[0].kind == SoundKind::Creature);
    CHECK(recent[0].position == std::array<float, 3>{1.0f, 2.0f, 3.0f});
    CHECK(recent[1].number == 6);
    CHECK(recent[1].kind == SoundKind::Hero);
    CHECK(recent[1].position == std::array<float, 3>{50.0f, 60.0f, 0.0f});

    const std::vector<PlacelessStart> logged = PlacelessStarts();
    REQUIRE(logged.size() == 5);
    CHECK(IsMusicNumber(logged[0].number));
    CHECK(MusicOf(logged[0].number) == "Data\\Music\\MuTheme.mp3");
    CHECK(logged[0].kind == SoundKind::Music);
    CHECK(logged[0].music == "Data\\Music\\MuTheme.mp3");
    CHECK(logged[1].number == 9);
    CHECK(logged[1].kind == SoundKind::Other);
    CHECK(logged[2].number == 8);
    CHECK(logged[2].kind == SoundKind::MapAmbience);
    CHECK(logged[2].looped);
    CHECK(logged[3].number == 7);
    CHECK(logged[3].time == 10.3);
    CHECK(logged[4].number == 7);
    CHECK(logged[4].kind == SoundKind::Interface);
    CHECK_FALSE(logged[4].looped);
    CHECK(logged[4].time == 10.1);

    // A few dozen are kept, the newest.
    for (int i = 0; i < 70; ++i)
    {
        play(9, 20.0 + i);
    }
    const std::vector<PlacelessStart> kept = PlacelessStarts();
    CHECK(kept.size() == 64);
    CHECK(kept.front().time == 89.0);
    CHECK(kept.back().time == 26.0);
}

TEST_CASE("Sound Mixer: PlayMp3 logs a track when the backend starts it (Q-SM15) [audio][mixer][editor]")
{
    MixerScope scope;
    PlayMp3("Data\\Music\\MuTheme.mp3", TRUE);
    // The track that plays starts nothing.
    PlayMp3("Data\\Music\\MuTheme.mp3", TRUE);
    PlayMp3("Data\\Music\\Devias.mp3", TRUE);
    const std::vector<Audio::Mixer::PlacelessStart> logged = Audio::Mixer::PlacelessStarts();
    REQUIRE(logged.size() == 2);
    CHECK(logged[0].music == "Data\\Music\\Devias.mp3");
    CHECK(logged[0].kind == SoundKind::Music);
    CHECK(logged[1].music == "Data\\Music\\MuTheme.mp3");
}

TEST_CASE("Sound Mixer: each music track heard gets a row, counted like a sound, and its controls mute the music "
          "while it plays (Q-SM17) [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    // The login scene asks for its track each frame: one start, one row.
    PlayMp3("Data\\Music\\login_theme.mp3", TRUE);
    PlayMp3("Data\\Music\\login_theme.mp3", TRUE);
    std::vector<HeardSound> heard = Heard();
    REQUIRE(heard.size() == 1);
    const int login = heard[0].number;
    CHECK(IsMusicNumber(login));
    CHECK_FALSE(IsMusicNumber(-1));
    CHECK_FALSE(IsMusicNumber(0));
    CHECK(MusicOf(login) == "Data\\Music\\login_theme.mp3");
    CHECK(MusicOf(5).empty());
    CHECK(heard[0].kind == SoundKind::Music);
    CHECK(heard[0].plays == 1);
    CHECK(heard[0].lastStarted >= 0.0);

    // The window's row: the file without its folder, the type, no length.
    const MuEditor::SoundMixer::HeardRow row = MuEditor::SoundMixer::MakeRow(heard[0]);
    CHECK(row.file == "login_theme.mp3");
    CHECK(row.source == "Music");
    CHECK(row.length == 0.0f);
    CHECK_FALSE(row.playing);

    // Muting the row mutes the music while that track plays.
    scope.audio.musicGains.clear();
    SoundControl(login).muted = true;
    Apply();
    REQUIRE_FALSE(scope.audio.musicGains.empty());
    CHECK(scope.audio.musicGains.back() == 0.0f);
    CHECK(MuEditor::SoundMixer::MakeRow(Heard()[0]).changed);
    // Another track plays at full gain, from before it starts.
    PlayMp3("Data\\Music\\MuTheme.mp3", TRUE);
    CHECK(scope.audio.musicGains.back() == 1.0f);
    REQUIRE(Heard().size() == 2);
    const int theme = Heard()[1].number;
    CHECK(theme != login);
    CHECK(IsMusicNumber(theme));
    // Back to the muted one: silent again, before the backend starts it.
    scope.audio.musicGains.clear();
    PlayMp3("Data\\Music\\login_theme.mp3", TRUE);
    REQUIRE_FALSE(scope.audio.musicGains.empty());
    CHECK(scope.audio.musicGains.front() == 0.0f);
    CHECK(FindHeard(Heard(), login)->plays == 2);

    // Solo of the track: the music plays, the sounds do not.
    SoundControl(login).muted = false;
    SoundControl(login).solo = true;
    CHECK(MusicGain() == 1.0f);
    CHECK(GainFor(7, SoundKind::Monster) == 0.0f);
    // Solo of a sound silences the music.
    SoundControl(login).solo = false;
    SoundControl(7).solo = true;
    CHECK(MusicGain() == 0.0f);
    SoundControl(7).solo = false;
    // The type's volume and the track's multiply.
    KindControl(SoundKind::Music).volume = 0.5f;
    SoundControl(login).volume = 0.5f;
    CHECK(MusicGain() == doctest::Approx(0.25f));

    // Clear all: the rows go; a track keeps its number for the session.
    ClearHeard();
    CHECK(Heard().empty());
    CHECK(MusicGain() == doctest::Approx(0.5f));
    PlayMp3("Data\\Music\\MuTheme.mp3", TRUE);
    REQUIRE(Heard().size() == 1);
    CHECK(Heard()[0].number == theme);
    CHECK(Heard()[0].plays == 1);
}

TEST_CASE("Sound Mixer: a character's sounds are typed by its kind, the hero's as your character, and an effect's "
          "by its maker (Q-SM13) [audio][mixer][editor]")
{
    using Audio::CharacterSoundKind;
    CHECK(CharacterSoundKind(KIND_PLAYER, true) == SoundKind::Hero);
    CHECK(CharacterSoundKind(KIND_PLAYER, false) == SoundKind::Player);
    CHECK(CharacterSoundKind(KIND_MONSTER, false) == SoundKind::Monster);
    CHECK(CharacterSoundKind(KIND_TRAP, false) == SoundKind::Monster);
    CHECK(CharacterSoundKind(KIND_NPC, false) == SoundKind::Npc);
    CHECK_FALSE(CharacterSoundKind(KIND_PET, false).has_value());

    // The game's characters for the scope.
    std::unique_ptr<CHARACTER[]> characters(new CHARACTER[MAX_CHARACTERS_CLIENT + 1]{});
    CHARACTER* const previousCharacters = CharactersClient;
    CHARACTER* const previousHero = Hero;
    CharactersClient = characters.get();
    Hero = &characters[3];
    characters[3].Object.Kind = KIND_PLAYER;
    Vector(100.0f, 200.0f, 0.0f, characters[3].Object.Position);
    characters[4].Object.Kind = KIND_PLAYER;
    characters[5].Object.Kind = KIND_NPC;
    characters[5].MonsterIndex = static_cast<EMonsterType>(229);
    Vector(300.0f, 400.0f, 10.0f, characters[5].Object.Position);
    characters[6].Object.Kind = KIND_PET;
    Audio::ForgetMakers();

    const auto placeOf = [](const CHARACTER& c) -> std::optional<Audio::SoundPlace>
    {
        std::optional<Audio::ScopedSoundPlace> place;
        Audio::PlaceCharacter(place, c);
        if (const Audio::SoundPlace* living = Audio::CurrentSoundPlace(); place && living != nullptr)
        {
            return *living;
        }
        return std::nullopt;
    };
    const std::optional<Audio::SoundPlace> hero = placeOf(characters[3]);
    REQUIRE(hero.has_value());
    CHECK(hero->kind == SoundKind::Hero);
    CHECK(hero->number == -1);
    CHECK(hero->position == std::array<float, 3>{100.0f, 200.0f, 0.0f});
    CHECK(placeOf(characters[4])->kind == SoundKind::Player);
    const std::optional<Audio::SoundPlace> npc = placeOf(characters[5]);
    REQUIRE(npc.has_value());
    CHECK(npc->kind == SoundKind::Npc);
    CHECK(npc->number == 229);
    HeardSound npcSound;
    npcSound.kind = SoundKind::Npc;
    npcSound.sourceNumber = 229;
    CHECK(MuEditor::SoundMixer::SourceText(npcSound) == "NPCs: 229");
    HeardSound playerSound;
    playerSound.kind = SoundKind::Player;
    CHECK(MuEditor::SoundMixer::SourceText(playerSound) == "Other players");
    CHECK(npc->position == std::array<float, 3>{300.0f, 400.0f, 10.0f});
    CHECK_FALSE(placeOf(characters[6]).has_value());
    CHECK(Audio::CurrentSoundPlace() == nullptr);

    // The objects PlayBuffer gets: a character, an item, a creature, an
    // effect, anything else.
    CHECK(Audio::PlaceOfObject(&characters[3].Object)->kind == SoundKind::Hero);
    CHECK(Audio::PlaceOfObject(&characters[5].Object)->number == 229);
    Items[4].Object.Type = 512;
    CHECK(Audio::PlaceOfObject(&Items[4].Object)->kind == SoundKind::Item);
    CHECK(Audio::PlaceOfObject(&Items[4].Object)->number == 512);
    Fishs[1].Type = 180;
    CHECK(Audio::PlaceOfObject(&Fishs[1])->kind == SoundKind::Creature);
    OBJECT loose;
    loose.Type = 77;
    Vector(1.0f, 2.0f, 3.0f, loose.Position);
    const std::optional<Audio::SoundPlace> other = Audio::PlaceOfObject(&loose);
    REQUIRE(other.has_value());
    CHECK(other->kind == SoundKind::Other);
    CHECK(other->number == 77);
    CHECK(other->position == std::array<float, 3>{1.0f, 2.0f, 3.0f});
    CHECK_FALSE(Audio::PlaceOfObject(nullptr).has_value());

    // An effect made while the hero's code runs counts as the hero's, where
    // the effect is; one made by an effect of the hero's too; one nobody
    // made is an effect of its type.
    const float effectAt[3] = {7.0f, 8.0f, 9.0f};
    {
        std::optional<Audio::ScopedSoundPlace> place;
        Audio::PlaceCharacter(place, characters[3]);
        Audio::RememberMaker(&Effects[10]);
    }
    {
        std::optional<Audio::ScopedSoundPlace> place;
        Audio::PlaceEffect(place, &Effects[10], 334, effectAt);
        Audio::RememberMaker(&Effects[11]);
    }
    Audio::RememberMaker(&Effects[12]);
    const Audio::SoundPlace fireball = Audio::EffectPlace(&Effects[10], 334, effectAt);
    CHECK(fireball.kind == SoundKind::Hero);
    CHECK(fireball.number == -1);
    CHECK(fireball.position == std::array<float, 3>{7.0f, 8.0f, 9.0f});
    CHECK(Audio::EffectPlace(&Effects[11], 335, effectAt).kind == SoundKind::Hero);
    const Audio::SoundPlace nobodys = Audio::EffectPlace(&Effects[12], 336, effectAt);
    CHECK(nobodys.kind == SoundKind::Effect);
    CHECK(nobodys.number == 336);
    // PlayBuffer's effect object: by its maker too.
    Effects[10].Type = 334;
    CHECK(Audio::PlaceOfObject(&Effects[10])->kind == SoundKind::Hero);
    // A slot made again by nobody forgets its maker.
    Audio::RememberMaker(&Effects[10]);
    CHECK(Audio::EffectPlace(&Effects[10], 334, effectAt).kind == SoundKind::Effect);

    Audio::ForgetMakers();
    Hero = previousHero;
    CharactersClient = previousCharacters;
}

TEST_CASE("Sound Mixer: a type's Hear box mutes it and its Show box hides it from the overlay and its log (Q-SM14) "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    TestImGuiContext context;
    ImGuiIO& io = ImGui::GetIO();
    Audio::Mixer::Control control;
    int clicks[2] = {};
    const auto frame = [&]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin("Boxes");
        clicks[0] += MuEditor::SoundMixer::RenderHearBox(control) ? 1 : 0;
        clicks[1] += MuEditor::SoundMixer::RenderShowBox(SoundKind::Hero) ? 1 : 0;
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](float y)
    {
        const float x = ImGui::GetStyle().WindowPadding.x + ImGui::GetFrameHeight() * 0.5f;
        io.AddMousePosEvent(x, y);
        frame();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        frame();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        frame();
    };
    frame();
    const float top = ImGui::GetStyle().WindowPadding.y + ImGui::GetFrameHeight();
    const float hearY = top + ImGui::GetFrameHeight() * 0.5f;
    const float showY = hearY + ImGui::GetFrameHeightWithSpacing();
    CHECK(MuEditor::SoundMixer::HearBoxTicked(control));
    click(hearY);
    CHECK(clicks[0] == 1);
    CHECK(control.muted);
    CHECK_FALSE(MuEditor::SoundMixer::HearBoxTicked(control));
    click(showY);
    CHECK(clicks[1] == 1);
    CHECK_FALSE(Audio::Overlay::IsShown(SoundKind::Hero));
    CHECK(Audio::Overlay::IsShown(SoundKind::Player));
    click(hearY);
    CHECK_FALSE(control.muted);

    // The overlay leaves out what is of a hidden type.
    using Audio::Overlay::DrawnSource;
    DrawnSource waterfall;
    waterfall.kind = SoundKind::MapObject;
    waterfall.sourceNumber = 37;
    DrawnSource rider;
    rider.kind = SoundKind::Monster;
    rider.sourceNumber = 306;
    Audio::Overlay::SetShown(SoundKind::Monster, false);
    const std::vector<DrawnSource> followed = MuEditor::SoundOverlay::Followed({waterfall, rider}, {}, false);
    REQUIRE(followed.size() == 1);
    CHECK(followed[0].sourceNumber == 37);
    Audio::Mixer::PlacelessStart click1{5, SoundKind::Interface, {}, false, 1.0};
    Audio::Mixer::PlacelessStart music{-1, SoundKind::Music, "Data\\Music\\MuTheme.mp3", true, 0.5};
    Audio::Overlay::SetShown(SoundKind::Interface, false);
    const std::vector<Audio::Mixer::PlacelessStart> logged = MuEditor::SoundOverlay::Logged({click1, music}, {}, false);
    REQUIRE(logged.size() == 1);
    CHECK(logged[0].kind == SoundKind::Music);
    // A code start's marker and flash: of a shown type, and while filtered of
    // a shown row's sound.
    Audio::Mixer::PlacedStart step;
    step.number = 8;
    step.kind = SoundKind::Hero;
    step.byCode = true;
    Audio::Mixer::PlacedStart swing = step;
    swing.number = 60;
    swing.kind = SoundKind::Player;
    CHECK_FALSE(MuEditor::SoundOverlay::Draws(step, {}, false));
    CHECK(MuEditor::SoundOverlay::Draws(swing, {}, false));
    CHECK(MuEditor::SoundOverlay::Draws(swing, {Row(60, "", "", 0)}, true));
    CHECK_FALSE(MuEditor::SoundOverlay::Draws(swing, {Row(61, "", "", 0)}, true));
    for (int kind = 0; kind < Audio::SoundKindCount; ++kind)
    {
        Audio::Overlay::SetShown(static_cast<SoundKind>(kind), true);
    }
}

TEST_CASE("Sound Mixer: the log shows the newest 10 starts without a place, how long ago, and while a filter is "
          "set those of the rows shown (Q-SM15) [audio][mixer][editor]")
{
    using Audio::Mixer::PlacelessStart;
    using namespace MuEditor::SoundOverlay;
    std::vector<PlacelessStart> starts;
    for (int i = 0; i < 12; ++i)
    {
        starts.push_back(PlacelessStart{100 + i, SoundKind::Interface, {}, false, 20.0 - i});
    }
    std::vector<PlacelessStart> logged = Logged(starts, {}, false);
    REQUIRE(logged.size() == LogLines);
    CHECK(logged.front().number == 100);
    CHECK(logged.back().number == 109);
    // While filtered: the starts of the rows shown; a track has no row.
    starts.insert(starts.begin(), PlacelessStart{-1, SoundKind::Music, "Data\\Music\\MuTheme.mp3", true, 21.0});
    logged = Logged(starts, {Row(111, "", "", 0), Row(103, "", "", 0)}, true);
    REQUIRE(logged.size() == 2);
    CHECK(logged[0].number == 103);
    CHECK(logged[1].number == 111);
    CHECK(Logged(starts, {}, true).empty());
    CHECK(Logged(starts, {}, false).front().kind == SoundKind::Music);

    CHECK(MusicText("Data\\Music\\MuTheme.mp3") == "MuTheme.mp3");
    CHECK(MusicText("data/music/Devias.mp3") == "Devias.mp3");
    CHECK(MusicText("Other\\Theme.mp3") == "Other/Theme.mp3");
    CHECK(LogText(PlacelessStart{-1, SoundKind::Music, "Data\\Music\\MuTheme.mp3", true, 10.0}, 13.0) ==
          "3.0 s  Music: MuTheme.mp3");
    CHECK(LogText(PlacelessStart{5, SoundKind::Interface, {}, false, 10.0}, 10.2) == "0.2 s  #5 (Interface)");
    CHECK(LogText(PlacelessStart{6, SoundKind::MapAmbience, {}, true, 10.0}, 51.0) ==
          "41.0 s  #6 (Map ambience, loop)");
}

TEST_CASE("Sound Mixer: the reminder counts what is muted, solo and lowered (Q-SM5) [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    CHECK_FALSE(CountChanges().Any());
    CHECK(MuEditor::SoundMixer::ChangesText(CountChanges()).empty());
    KindControl(SoundKind::Music).muted = true;
    SoundControl(4).muted = true;
    SoundControl(4).volume = 0.5f;
    SoundControl(5).solo = true;
    KindControl(SoundKind::Monster).volume = 0.7f;
    const Changes changes = CountChanges();
    CHECK(changes.muted == 2);
    CHECK(changes.solo == 1);
    // A muted sound that is also turned down counts as muted.
    CHECK(changes.lowered == 1);
    CHECK(MuEditor::SoundMixer::ChangesText(changes) == "2 muted, 1 solo, 1 lowered");
    // The overlay is named too.
    CHECK(MuEditor::SoundMixer::ReminderText(changes, true) == "2 muted, 1 solo, 1 lowered, overlay on");
    CHECK(MuEditor::SoundMixer::ReminderText({}, true) == "overlay on");
    CHECK(MuEditor::SoundMixer::ReminderText({}, false).empty());
    ResetAll();
    CHECK_FALSE(CountChanges().Any());
}

TEST_CASE("Sound Mixer: a starting sound and every change give the backend the gains, the music's too (Q-SM4) "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    KindControl(SoundKind::Other).muted = true;
    CHECK(OnPlay(12, false) == 0.0f);
    REQUIRE(scope.audio.gains.size() == 1);
    CHECK(scope.audio.gains[0] == std::pair{12, 0.0f});
    // The window's buttons apply at once: every sound heard and the music.
    scope.audio.gains.clear();
    MuEditor::SoundMixer::ToggleMute(KindControl(SoundKind::Other));
    REQUIRE(scope.audio.gains.size() == 1);
    CHECK(scope.audio.gains[0] == std::pair{12, 1.0f});
    REQUIRE(scope.audio.musicGains.size() == 1);
    CHECK(scope.audio.musicGains[0] == 1.0f);
    MuEditor::SoundMixer::SetVolume(SoundControl(12), 0.25f);
    CHECK(scope.audio.gains.back() == std::pair{12, 0.25f});
    MuEditor::SoundMixer::ToggleSolo(KindControl(SoundKind::Music));
    CHECK(scope.audio.gains.back() == std::pair{12, 0.0f});
    CHECK(scope.audio.musicGains.back() == 1.0f);
    ResetAll();
    CHECK(scope.audio.gains.back() == std::pair{12, 1.0f});
}

// A map object's sound is asked for each frame it is drawn: a play reads
// whether a control is on solo from the last Apply (every change of a control
// ends in one), instead of scanning the controls of the session.
TEST_CASE("Sound Mixer: a play takes the solo from the last Apply, while GainFor reads the controls as they are "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    SoundControl(5).solo = true;
    CHECK(GainFor(6, SoundKind::Other) == 0.0f);
    CHECK(OnPlay(6, false) == 1.0f);
    Apply();
    CHECK(OnPlay(6, false) == 0.0f);
    CHECK(OnPlay(5, false) == 1.0f);
    // The window's Solo button applies, so the next play follows it.
    MuEditor::SoundMixer::ToggleSolo(SoundControl(5));
    CHECK(OnPlay(6, false) == 1.0f);
    // Putting every control back applies too.
    SoundControl(5).solo = true;
    ResetAll();
    CHECK(OnPlay(6, false) == 1.0f);
}

TEST_CASE("Sound Mixer: a sound two types play starts in each type's group with that type's gain, and a change "
          "gives each group its own, so a muted type leaves the other type's copies alone (Q-SM4) "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    KindControl(SoundKind::Player).muted = true;
    {
        const Audio::ScopedSoundSource hero(SoundKind::Hero, "Hero");
        CHECK(OnPlay(12, false) == 1.0f);
    }
    {
        const Audio::ScopedSoundSource player(SoundKind::Player, "Another player");
        CHECK(OnPlay(12, false) == 0.0f);
    }
    REQUIRE(scope.audio.gains.size() == 2);
    const int heroGroup = scope.audio.gainGroups[0];
    const int playerGroup = scope.audio.gainGroups[1];
    CHECK(heroGroup != playerGroup);
    CHECK(scope.audio.gains[0] == std::pair{12, 1.0f});
    CHECK(scope.audio.gains[1] == std::pair{12, 0.0f});
    // Unmuting the players reaches their group; the hero's keeps its gain.
    scope.audio.gains.clear();
    scope.audio.gainGroups.clear();
    MuEditor::SoundMixer::ToggleMute(KindControl(SoundKind::Player));
    MuEditor::SoundMixer::ToggleMute(KindControl(SoundKind::Hero));
    std::map<int, float> byGroup;
    for (size_t index = 0; index < scope.audio.gains.size(); ++index)
    {
        CHECK(scope.audio.gains[index].first == 12);
        byGroup[scope.audio.gainGroups[index]] = scope.audio.gains[index].second;
    }
    CHECK(byGroup.size() == 2);
    CHECK(byGroup[heroGroup] == 0.0f);
    CHECK(byGroup[playerGroup] == 1.0f);
}

TEST_CASE("Sound Mixer: a sound on a free channel is a change of the game inside a draw, as PlayBuffer is, and "
          "one nothing loaded is noted for the Sound Editor [audio][mixer][editor]")
{
    MixerScope scope;
    ForgetLoadedSounds();
    Audio::SoundLoads::Clear();
    Diagnostics::DrawSpawnCheck::ResetViolations();
    const auto unloaded = static_cast<ESound>(SOUND_CLICK01 + 1);
    {
        const Diagnostics::DrawSpawnCheck::DrawScope drawing;
        PlayBuffer(SOUND_CLICK01);
        PlayBufferOnFreeChannel(unloaded, 0.5f);
    }
    CHECK(Diagnostics::DrawSpawnCheck::Violations() == 2);
    CHECK(Audio::SoundLoads::PlayedUnloaded() ==
          std::set<int>{static_cast<int>(SOUND_CLICK01), static_cast<int>(unloaded)});
    Diagnostics::DrawSpawnCheck::ResetViolations();
    Audio::SoundLoads::Clear();
}

TEST_CASE("Sound Mixer: PlayBuffer notes a loaded sound with its gain, and starts a muted one silently (Q-SM4) "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    ForgetLoadedSounds();
    LoadWaveFile(SOUND_CLICK01, L"Data\\Sound\\iButtonClick.wav", 1);
    Audio::Mixer::SoundControl(SOUND_CLICK01).muted = true;
    PlayBuffer(SOUND_CLICK01);
    // A sound that was never loaded is not listed.
    PlayBuffer(static_cast<ESound>(SOUND_CLICK01 + 1));
    PlayBufferOnFreeChannel(SOUND_CLICK01, 0.5f);
    // A call that starts nothing counts nothing.
    scope.audio.starts = false;
    PlayBuffer(SOUND_CLICK01, nullptr, TRUE);
    const std::vector<HeardSound> heard = Audio::Mixer::Heard();
    REQUIRE(heard.size() == 1);
    CHECK(heard[0].number == SOUND_CLICK01);
    CHECK(heard[0].plays == 2);
    CHECK(heard[0].looped);
    CHECK(heard[0].lastStarted >= 0.0);
    REQUIRE_FALSE(scope.audio.gains.empty());
    CHECK(scope.audio.gains.front() == std::pair{static_cast<int>(SOUND_CLICK01), 0.0f});
    // It still starts, silently, so that unmuting brings it back.
    CHECK_FALSE(scope.audio.plays.empty());
    CHECK(MuEditor::SoundMixer::SoundText(SOUND_CLICK01) == "ibuttonclick.wav");
    CHECK(MuEditor::SoundMixer::SoundText(SOUND_CLICK01 + 1) == "#" + std::to_string(SOUND_CLICK01 + 1));
    // The object it plays at gives a sound without a source or place its type
    // and place (Q-SM13).
    LoadWaveFile(SOUND_BIRD01, L"Data\\Sound\\aBird1.wav", 1);
    Boids[2].Type = 175;
    Vector(10.0f, 20.0f, 30.0f, Boids[2].Position);
    scope.audio.starts = true;
    PlayBuffer(SOUND_BIRD01, &Boids[2]);
    const std::vector<HeardSound> withBird = Audio::Mixer::Heard();
    const HeardSound* bird = FindHeard(withBird, SOUND_BIRD01);
    REQUIRE(bird != nullptr);
    CHECK(bird->kind == SoundKind::Creature);
    CHECK(bird->sourceNumber == 175);
    const std::vector<Audio::Mixer::PlacedStart> marked = Audio::Mixer::CodeSounds(Audio::Mixer::Now());
    REQUIRE(marked.size() == 1);
    CHECK(marked[0].position == std::array<float, 3>{10.0f, 20.0f, 30.0f});
    ForgetLoadedSounds();
}

namespace
{
std::string g_clipboard;
}

TEST_CASE("Sound Mixer: a click on a Sound cell copies its file, the tooltip says so [audio][mixer][editor]")
{
    using MuEditor::SoundMixer::HeardRow;
    TestImGuiContext context;
    g_clipboard.clear();
    ImGui::GetPlatformIO().Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) { g_clipboard = text; };
    ImGuiIO& io = ImGui::GetIO();
    HeardRow energy;
    energy.sound.number = 120;
    energy.file = "penergy.wav";
    // Another number of the same file.
    HeardRow again = energy;
    again.sound.number = 121;
    int copied = -1;
    int clicks[2] = {};
    ImVec2 secondCell;
    const auto frame = [&]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin("Cells");
        clicks[0] += MuEditor::SoundMixer::RenderSoundCell(energy, copied) ? 1 : 0;
        clicks[1] += MuEditor::SoundMixer::RenderSoundCell(again, copied) ? 1 : 0;
        secondCell = ImVec2((ImGui::GetItemRectMin().x + ImGui::GetItemRectMax().x) * 0.5f,
                            (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f);
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    io.AddMousePosEvent(secondCell.x, secondCell.y);
    frame();
    frame();
    CHECK(g_clipboard.empty());
    CHECK(MuEditor::SoundMixer::SoundCellTip(again, copied) == "#121, click to copy the name");
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    frame();
    // The second cell: its file, under its own number.
    CHECK(clicks[0] == 0);
    CHECK(clicks[1] == 1);
    CHECK(g_clipboard == "penergy.wav");
    CHECK(copied == 121);
    CHECK(MuEditor::SoundMixer::SoundCellTip(again, copied) == "Copied: penergy.wav");
    // The other number of the file was not copied.
    CHECK(MuEditor::SoundMixer::SoundCellTip(energy, copied) == "#120, click to copy the name");
    ImGui::GetPlatformIO().Platform_SetClipboardTextFn = nullptr;
}

TEST_CASE("Sound Mixer: a sound the Sound Editor named shows its name in its cell, the overlay and the log, is "
          "found and sorted by it and still copies its file (Q-SE1) [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace MuEditor::SoundMixer;
    const std::vector<Data::Sounds::SoundFile> previous = g_SoundFileDatabase.GetAll();
    Data::Sounds::SoundFile click;
    click.id = "SOUND_CLICK01";
    click.number = static_cast<int>(SOUND_CLICK01);
    click.name = "Button click";
    click.file = "Data/Sound/iButtonClick.wav";
    g_SoundFileDatabase.Build(std::vector<Data::Sounds::SoundFile>{click});
    ForgetLoadedSounds();
    LoadWaveFile(SOUND_CLICK01, L"Data\\Sound\\iButtonClick.wav", 1);
    HeardSound heard;
    heard.number = static_cast<int>(SOUND_CLICK01);
    const HeardRow row = MakeRow(heard);
    CHECK(row.name == "Button click");
    CHECK(row.file == "ibuttonclick.wav");
    CHECK(CellText(row) == "Button click");
    CHECK(SoundCellTip(row, -1) ==
          "#" + std::to_string(SOUND_CLICK01) + ", ibuttonclick.wav, click to copy the file name");
    CHECK(SoundCellTip(row, row.sound.number) == "Copied: ibuttonclick.wav");
    CHECK(ShownSoundText(SOUND_CLICK01) == "Button click");
    CHECK(ShownSoundText(SOUND_RAIN01) == SoundText(SOUND_RAIN01));
    HeardFilter filter;
    // In the name only, not in the file.
    filter.text = "N CL";
    CHECK(Passes(row, filter));
    filter.text = "ibuttonclick";
    CHECK(Passes(row, filter));
    Audio::Mixer::PlacelessStart start{static_cast<int>(SOUND_CLICK01), SoundKind::Interface, {}, false, 10.0};
    CHECK(MuEditor::SoundOverlay::LogText(start, 10.2) == "0.2 s  Button click (Interface)");
    // Sorted by what the cell shows.
    HeardRow other = Row(5, "aaa.wav", "", 0);
    other.name = "Zebra";
    std::vector<HeardRow> rows{other, row};
    SortRows(rows, HeardSort{HeardColumn::Sound, false}, 0.0);
    CHECK(rows[0].name == "Button click");
    ForgetLoadedSounds();
    g_SoundFileDatabase.Build(previous);
}

TEST_CASE("Sound Mixer: a code sound without a source or place takes the type the Sound Editor stored, and each "
          "sound keeps every type it was heard as (Q-SE2) [audio][mixer][editor]")
{
    MixerScope scope;
    using namespace Audio::Mixer;
    const std::vector<Data::Sounds::SoundFile> previous = g_SoundFileDatabase.GetAll();
    Data::Sounds::SoundFile levelUp;
    levelUp.id = "SOUND_LEVEL_UP";
    levelUp.number = static_cast<int>(SOUND_LEVEL_UP);
    levelUp.file = "Data/Sound/pLevelUp.wav";
    levelUp.type = SoundKind::Hero;
    Data::Sounds::SoundFile wind = levelUp;
    wind.id = "SOUND_WIND01";
    wind.number = static_cast<int>(SOUND_WIND01);
    wind.type.reset();
    g_SoundFileDatabase.Build(std::vector<Data::Sounds::SoundFile>{levelUp, wind});
    const int level = static_cast<int>(SOUND_LEVEL_UP);
    OnPlay(level, false);
    CHECK(FindHeard(Heard(), level)->kind == SoundKind::Hero);
    // Who plays it still decides first.
    const float at[3] = {1.0f, 2.0f, 3.0f};
    {
        const Audio::ScopedSoundPlace place(SoundKind::Player, -1, at);
        OnPlay(level, false);
    }
    CHECK(FindHeard(Heard(), level)->kind == SoundKind::Player);
    // Without a stored type: the loop's rule.
    OnPlay(static_cast<int>(SOUND_WIND01), true);
    CHECK(FindHeard(Heard(), static_cast<int>(SOUND_WIND01))->kind == SoundKind::MapAmbience);
    CHECK(KindsHeard(level) == std::vector<SoundKind>{SoundKind::Hero, SoundKind::Player});
    ClearHeard();
    CHECK(KindsHeard(level).size() == 2);
    CHECK(KindsHeard(static_cast<int>(SOUND_RAIN01)).empty());
    g_SoundFileDatabase.Build(previous);
}

TEST_CASE("Sound Mixer: a click on a Sound cell selects the sound for the Sound Editor too (Q-SE3) "
          "[audio][mixer][editor]")
{
    MuEditor::SoundSelection::Set(-1);
    const int version = MuEditor::SoundSelection::GetVersion();
    MuEditor::SoundSelection::Set(-1);
    CHECK(MuEditor::SoundSelection::GetVersion() == version);
    MuEditor::SoundSelection::Set(42);
    CHECK(MuEditor::SoundSelection::Get() == 42);
    CHECK(MuEditor::SoundSelection::GetVersion() == version + 1);
    TestImGuiContext context;
    ImGuiIO& io = ImGui::GetIO();
    HeardRow cell = Row(120, "penergy.wav", "", 0);
    int copied = -1;
    ImVec2 center;
    const auto frame = [&]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin("Cell");
        MuEditor::SoundMixer::RenderSoundCell(cell, copied);
        center = ImVec2((ImGui::GetItemRectMin().x + ImGui::GetItemRectMax().x) * 0.5f,
                        (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f);
        ImGui::End();
        ImGui::Render();
    };
    frame();
    io.AddMousePosEvent(center.x, center.y);
    frame();
    frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    frame();
    CHECK(MuEditor::SoundSelection::Get() == 120);
    MuEditor::SoundSelection::Set(-1);
}

TEST_CASE("Sound Mixer: the window draws its kinds and the sounds heard, the reminder opens it (Q-SM1, Q-SM5) "
          "[audio][mixer][editor]")
{
    MixerScope scope;
    Audio::Mixer::OnPlay(20, false);
    Audio::Mixer::OnPlay(21, true);
    scope.audio.playing.insert(21);
    TestImGuiContext context;
    MuEditor::ToolWindows windows;
    const auto draw = [&]
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuSoundMixerUI.Render(&open);
        g_MuSoundMixerUI.RenderOverlay();
        ImGui::Begin("Bar", nullptr, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            MuEditor::SoundMixer::RenderReminder(windows);
            ImGui::EndMenuBar();
        }
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    draw();
    CHECK(ImGui::FindWindowByName("Sound Mixer###SoundMixer") != nullptr);
    Audio::Mixer::SoundControl(20).muted = true;
    draw();
    CHECK_FALSE(windows.soundMixer);
    // The overlay draws its sources over the game, also while filtered.
    Audio::Overlay::SetOn(true);
    Audio::Overlay::KeepCamera(CameraState{});
    Audio::Overlay::DrawnSource rider;
    rider.kind = SoundKind::Monster;
    rider.name = "Death Rider";
    rider.inner = 500.0f;
    Audio::Overlay::NoteDrawn(rider);
    draw();
    CHECK(Audio::Overlay::TakeDrawn().empty());
    Audio::Overlay::SetOn(false);
}
#endif // _EDITOR
