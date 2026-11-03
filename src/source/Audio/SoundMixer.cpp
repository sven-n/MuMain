#include "stdafx.h"

#ifdef _EDITOR

#include "Audio/SoundMixer.h"

#include "Core/Platform/IPlatformAudio.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>

namespace Audio
{
namespace
{
const SoundSource* g_currentSource = nullptr;
const SoundPlace* g_currentPlace = nullptr;
} // namespace

ScopedSoundSource::ScopedSoundSource(SoundKind kind, std::string_view name, int number, const float* position)
    : m_source{kind, std::string(name), number, std::nullopt}, m_outer(g_currentSource)
{
    if (position != nullptr)
    {
        m_source.position = std::array<float, 3>{position[0], position[1], position[2]};
    }
    g_currentSource = &m_source;
}

ScopedSoundSource::~ScopedSoundSource()
{
    g_currentSource = m_outer;
}

const SoundSource* CurrentSoundSource()
{
    return g_currentSource;
}

ScopedSoundPlace::ScopedSoundPlace(SoundKind kind, int number, const float* position)
    : m_place{kind, number, std::nullopt}, m_outer(g_currentPlace)
{
    if (position != nullptr)
    {
        m_place.position = std::array<float, 3>{position[0], position[1], position[2]};
    }
    g_currentPlace = &m_place;
}

ScopedSoundPlace::ScopedSoundPlace(const SoundPlace& place) : m_place(place), m_outer(g_currentPlace)
{
    g_currentPlace = &m_place;
}

ScopedSoundPlace::~ScopedSoundPlace()
{
    g_currentPlace = m_outer;
}

const SoundPlace* CurrentSoundPlace()
{
    return g_currentPlace;
}
} // namespace Audio

namespace Audio::Mixer
{
namespace
{
// The placed starts kept for the overlay: a few seconds, at most so many.
constexpr double KeptStartSeconds = 5.0;
constexpr size_t KeptStarts = 256;
// A sound of the game's own code is marked so long after its start, also when
// it played for less.
constexpr double CodeSoundHoldSeconds = 1.0;
// The starts without a place kept for the overlay's log (Q-SM15).
constexpr size_t KeptPlacelessStarts = 64;

// Where a starting sound comes from: its type and source, and its position
// if it has one.
struct Origin
{
    SoundKind kind = SoundKind::Other;
    std::string name;
    int sourceNumber = -1;
    std::optional<std::array<float, 3>> position;
    // Named by a place (the game's own code), not by a source (data).
    bool byCode = false;
};

struct Listed
{
    HeardSound sound;
    // Its place in the list: the order of first hearing.
    int order = 0;
};

struct State
{
    std::array<Control, SoundKindCount> kinds;
    std::map<int, Control> sounds;
    // The rows since the list was cleared.
    std::map<int, Listed> listed;
    int nextOrder = 0;
    // The kind of every sound heard this session, for Apply: a cleared sound
    // may still play.
    std::map<int, SoundKind> kindOf;
    // The backend's starts of OnPlay's sound before its play, and where it
    // comes from.
    int playing = -1;
    std::uint32_t startsBefore = 0;
    Origin origin;
    // The latest placed starts, oldest first.
    std::deque<PlacedStart> starts;
    // The last start of each sound of the game's own code, by sound number.
    std::map<int, PlacedStart> codeStarts;
    // The latest starts without a place, newest first.
    std::deque<PlacelessStart> placeless;
    // Every type each sound was heard as this session (the Sound Editor's
    // "heard as").
    std::map<int, std::set<SoundKind>> kindsHeard;
};

State& Mixer()
{
    static State state;
    return state;
}

size_t IndexOf(SoundKind kind)
{
    return static_cast<size_t>(kind);
}

// The living source, else the innermost living place, else the object's
// place, else the type the Sound Editor stored for the sound (Q-SE2), else a
// loop is map ambience and the rest everything else (Q-SM13).
Origin Resolve(int number, bool looped, const SoundPlace* objectPlace)
{
    if (const SoundSource* source = CurrentSoundSource())
    {
        return Origin{source->kind, source->name, source->number, source->position, false};
    }
    const SoundPlace* place = CurrentSoundPlace();
    if (place == nullptr)
    {
        place = objectPlace;
    }
    if (place != nullptr)
    {
        return Origin{place->kind, {}, place->number, place->position, true};
    }
    if (const Data::Sounds::SoundFile* sound = g_SoundFileDatabase.Find(number); sound != nullptr && sound->type)
    {
        return Origin{*sound->type, {}, -1, std::nullopt, true};
    }
    return Origin{looped ? SoundKind::MapAmbience : SoundKind::Other, {}, -1, std::nullopt, true};
}

void KeepPlaceless(State& state, PlacelessStart start)
{
    state.placeless.push_front(std::move(start));
    if (state.placeless.size() > KeptPlacelessStarts)
    {
        state.placeless.pop_back();
    }
}

bool AnySolo(const State& state)
{
    return std::any_of(state.kinds.begin(), state.kinds.end(), [](const Control& control) { return control.solo; }) ||
           std::any_of(state.sounds.begin(), state.sounds.end(), [](const auto& sound) { return sound.second.solo; });
}

float Level(float volume)
{
    return std::clamp(volume, 0.0f, 1.0f);
}

std::uint32_t StartsOf(int number)
{
    return g_platformAudio != nullptr ? g_platformAudio->GetSoundStarts(static_cast<ESound>(number)) : 0;
}

bool IsChanged(const State& state, int number)
{
    const auto control = state.sounds.find(number);
    return control != state.sounds.end() && control->second.IsChanged();
}

void Count(const Control& control, Changes& changes)
{
    if (control.muted)
    {
        ++changes.muted;
    }
    else if (control.volume < 1.0f)
    {
        ++changes.lowered;
    }
    if (control.solo)
    {
        ++changes.solo;
    }
}
} // namespace

bool Control::IsChanged() const
{
    return muted || solo || volume < 1.0f;
}

Control& KindControl(SoundKind kind)
{
    return Mixer().kinds[IndexOf(kind)];
}

Control& SoundControl(int number)
{
    return Mixer().sounds[number];
}

float GainFor(int number, SoundKind kind)
{
    const State& state = Mixer();
    const Control& ofKind = state.kinds[IndexOf(kind)];
    const auto found = state.sounds.find(number);
    const Control ofSound = found != state.sounds.end() ? found->second : Control{};
    if (ofKind.muted || ofSound.muted)
    {
        return 0.0f;
    }
    if (AnySolo(state) && !ofKind.solo && !ofSound.solo)
    {
        return 0.0f;
    }
    return Level(ofKind.volume) * Level(ofSound.volume);
}

float MusicGain()
{
    const State& state = Mixer();
    const Control& music = state.kinds[IndexOf(SoundKind::Music)];
    if (music.muted || (AnySolo(state) && !music.solo))
    {
        return 0.0f;
    }
    return Level(music.volume);
}

float OnPlay(int number, bool looped, const SoundPlace* objectPlace)
{
    State& state = Mixer();
    const auto [row, added] = state.listed.try_emplace(number);
    if (added)
    {
        row->second.order = state.nextOrder++;
    }
    HeardSound& heard = row->second.sound;
    heard.number = number;
    heard.looped = looped;
    state.origin = Resolve(number, looped, objectPlace);
    heard.kind = state.origin.kind;
    state.kindsHeard[number].insert(heard.kind);
    heard.name = state.origin.name;
    heard.sourceNumber = state.origin.sourceNumber;
    state.kindOf[number] = heard.kind;
    state.playing = number;
    state.startsBefore = StartsOf(number);
    const float gain = GainFor(number, heard.kind);
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->SetSoundMixGain(static_cast<ESound>(number), gain);
    }
    return gain;
}

void OnPlayed(int number, double now)
{
    State& state = Mixer();
    if (state.playing != number)
    {
        return;
    }
    state.playing = -1;
    const std::uint32_t started = StartsOf(number) - state.startsBefore;
    const auto row = state.listed.find(number);
    if (started == 0 || row == state.listed.end())
    {
        return;
    }
    row->second.sound.plays += static_cast<int>(started);
    row->second.sound.lastStarted = now;
    const Origin& origin = state.origin;
    if (!origin.position)
    {
        KeepPlaceless(state, PlacelessStart{number, origin.kind, {}, row->second.sound.looped, now});
        return;
    }
    PlacedStart start{number, origin.kind, origin.name, origin.sourceNumber, *origin.position, now, origin.byCode};
    if (origin.byCode)
    {
        state.codeStarts[number] = start;
    }
    state.starts.push_back(std::move(start));
    while (state.starts.size() > KeptStarts || now - state.starts.front().time > KeptStartSeconds)
    {
        state.starts.pop_front();
    }
}

std::vector<PlacedStart> RecentStarts(double now, double seconds)
{
    std::vector<PlacedStart> recent;
    for (const PlacedStart& start : Mixer().starts)
    {
        if (now - start.time <= seconds)
        {
            recent.push_back(start);
        }
    }
    return recent;
}

std::vector<PlacedStart> CodeSounds(double now)
{
    State& state = Mixer();
    std::vector<PlacedStart> sounds;
    for (auto start = state.codeStarts.begin(); start != state.codeStarts.end();)
    {
        const bool playing =
            g_platformAudio != nullptr && g_platformAudio->IsSoundPlaying(static_cast<ESound>(start->first));
        if (playing || now - start->second.time <= CodeSoundHoldSeconds)
        {
            sounds.push_back(start->second);
            ++start;
        }
        else
        {
            start = state.codeStarts.erase(start);
        }
    }
    return sounds;
}

std::vector<SoundKind> KindsHeard(int number)
{
    const State& state = Mixer();
    const auto found = state.kindsHeard.find(number);
    if (found == state.kindsHeard.end())
    {
        return {};
    }
    return {found->second.begin(), found->second.end()};
}

void OnMusicStarted(const std::string& music, double now)
{
    KeepPlaceless(Mixer(), PlacelessStart{-1, SoundKind::Music, music, true, now});
}

std::vector<PlacelessStart> PlacelessStarts()
{
    const State& state = Mixer();
    return {state.placeless.begin(), state.placeless.end()};
}

std::vector<HeardSound> Heard()
{
    const State& state = Mixer();
    std::vector<const Listed*> rows;
    rows.reserve(state.listed.size());
    for (const auto& row : state.listed)
    {
        rows.push_back(&row.second);
    }
    std::sort(rows.begin(), rows.end(),
              [](const Listed* left, const Listed* right) { return left->order < right->order; });
    std::vector<HeardSound> heard;
    heard.reserve(rows.size());
    for (const Listed* row : rows)
    {
        heard.push_back(row->sound);
    }
    return heard;
}

void ClearHeard()
{
    State& state = Mixer();
    for (auto row = state.listed.begin(); row != state.listed.end();)
    {
        if (IsChanged(state, row->first))
        {
            row->second.sound.plays = 0;
            row->second.sound.lastStarted = -1.0;
            ++row;
        }
        else
        {
            row = state.listed.erase(row);
        }
    }
}

Changes CountChanges()
{
    const State& state = Mixer();
    Changes changes;
    for (const Control& control : state.kinds)
    {
        Count(control, changes);
    }
    for (const auto& sound : state.sounds)
    {
        Count(sound.second, changes);
    }
    return changes;
}

void Apply()
{
    if (g_platformAudio == nullptr)
    {
        return;
    }
    for (const auto& [number, kind] : Mixer().kindOf)
    {
        g_platformAudio->SetSoundMixGain(static_cast<ESound>(number), GainFor(number, kind));
    }
    g_platformAudio->SetMusicMixGain(MusicGain());
}

void ResetAll()
{
    State& state = Mixer();
    state.kinds = {};
    state.sounds.clear();
    Apply();
}

double Now()
{
    using Seconds = std::chrono::duration<double>;
    return std::chrono::duration_cast<Seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void Clear()
{
    Mixer() = State{};
}
} // namespace Audio::Mixer

#endif // _EDITOR
