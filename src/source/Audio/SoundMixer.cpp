#include "stdafx.h"

#ifdef _EDITOR

#include "Audio/SoundMixer.h"

#include "Core/Platform/IPlatformAudio.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <map>

namespace Audio
{
namespace
{
const SoundSource* g_currentSource = nullptr;
} // namespace

ScopedSoundSource::ScopedSoundSource(SoundKind kind, std::string_view name, int number)
    : m_source{kind, std::string(name), number}, m_outer(g_currentSource)
{
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
} // namespace Audio

namespace Audio::Mixer
{
namespace
{
struct State
{
    std::array<Control, SoundKindCount> kinds;
    std::map<int, Control> sounds;
    std::map<int, HeardSound> heard;
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

bool AnySolo(const State& state)
{
    return std::any_of(state.kinds.begin(), state.kinds.end(), [](const Control& control) { return control.solo; }) ||
           std::any_of(state.sounds.begin(), state.sounds.end(), [](const auto& sound) { return sound.second.solo; });
}

float Level(float volume)
{
    return std::clamp(volume, 0.0f, 1.0f);
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

float OnPlay(int number, bool looped, double now)
{
    HeardSound& heard = Mixer().heard[number];
    heard.number = number;
    if (const SoundSource* source = CurrentSoundSource())
    {
        heard.kind = source->kind;
        heard.name = source->name;
        heard.sourceNumber = source->number;
    }
    else
    {
        heard.kind = looped ? SoundKind::MapAmbience : SoundKind::Other;
        heard.name.clear();
        heard.sourceNumber = -1;
    }
    heard.lastHeard = now;
    ++heard.plays;
    const float gain = GainFor(number, heard.kind);
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->SetSoundMixGain(static_cast<ESound>(number), gain);
    }
    return gain;
}

std::vector<HeardSound> Heard(double now)
{
    const State& state = Mixer();
    std::vector<HeardSound> heard;
    for (const auto& [number, sound] : state.heard)
    {
        const auto control = state.sounds.find(number);
        const bool changed = control != state.sounds.end() && control->second.IsChanged();
        if (changed || now - sound.lastHeard <= HeardSeconds)
        {
            heard.push_back(sound);
        }
    }
    std::stable_sort(heard.begin(), heard.end(),
                     [](const HeardSound& left, const HeardSound& right) { return left.lastHeard > right.lastHeard; });
    return heard;
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
    for (const auto& [number, sound] : Mixer().heard)
    {
        g_platformAudio->SetSoundMixGain(static_cast<ESound>(number), GainFor(number, sound.kind));
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
