#include "App/stdafx.h"

// The editor's Sound Mixer (docs/superpowers/specs/2026-11-01-sound-mixer-design.md):
// its rules, the kinds of the sounds' sources, the heard list, the reminder's
// counts, the gains it gives the backend and the PlayBuffer hook.

#include "doctest.h"

#ifdef _EDITOR
#include "Audio/DSPlaySound.h"
#include "Audio/SoundMixer.h"
#include "Audio/SoundOverlay.h"
#include "Audio/SoundSource.h"
#include "Camera/CameraState.h"
#include "Core/Platform/IPlatformAudio.h"
#include "TestImGuiContext.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/SoundMixer/MuSoundMixerUI.h"
#include "UI/SoundMixer/MuSoundOverlay.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstdint>
#include <map>
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
    void SetSoundMixGain(ESound buffer, float gain) override
    {
        gains.emplace_back(static_cast<int>(buffer), gain);
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
    std::vector<float> musicGains;
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

TEST_CASE("Sound Mixer: a sound's kind is its source's, a loop's map ambience, else everything else (Q-SM2) "
          "[audio][mixer][editor]")
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
    CHECK(MuEditor::SoundMixer::SourceText(*FindHeard(heard, 7)) == "Monster sounds: Death Rider (306)");
    CHECK(MuEditor::SoundMixer::SourceText(*FindHeard(heard, 8)) == "Map object sounds: object 37");
}

TEST_CASE("Sound Mixer: the heard list keeps each sound where it was first heard and counts the backend's starts; "
          "Clear all keeps the changed sounds [audio][mixer][editor]")
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

    // Clear all: the lowered sound stays, with no plays; the other goes.
    SoundControl(2).volume = 0.4f;
    ClearHeard();
    heard = Heard();
    REQUIRE(heard.size() == 1);
    CHECK(heard[0].number == 2);
    CHECK(heard[0].plays == 0);
    CHECK(heard[0].lastStarted < 0.0);
    // The sounds heard after it come after it, a cleared one as new.
    play(3, 9.0, true);
    play(1, 10.0, true);
    heard = Heard();
    REQUIRE(heard.size() == 3);
    CHECK(heard[0].number == 2);
    CHECK(heard[1].number == 3);
    CHECK(heard[2].number == 1);
    CHECK(heard[2].plays == 1);

    // A cleared sound still gets its gain: it may play on.
    ClearHeard();
    REQUIRE(Heard().size() == 1);
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
    ForgetLoadedSounds();
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
