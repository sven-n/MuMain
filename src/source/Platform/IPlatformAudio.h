#pragma once

// Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
// Pure virtual interface for the platform audio backend.
// Mirrors the public API of DSPlaySound.h (LoadWaveFile/PlayBuffer/StopBuffer/etc.)
// and the wzAudio wrappers in Winmain.cpp (PlayMp3/StopMp3/IsEndMp3/GetMp3PlayPosition).
// Story 5.2.1 wired BGM via g_platformAudio (Winmain.cpp replaced wzAudio calls).
// Story 5.2.2 wired SFX (LoadSound/PlaySound/StopSound/AllStopSound) into g_platformAudio.
// (MEDIUM-4 fix, code-review-finalize 2026-03-20)

#include "DSPlaySound.h" // ESound enum, MAX_CHANNEL, MAX_BUFFER

namespace mu
{

class IPlatformAudio
{
public:
    virtual ~IPlatformAudio() = default;

    // Lifecycle
    [[nodiscard]] virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    // Sound effects — mirrors DSPlaySound.h public API exactly
    // (zero call-site changes when Stories 5.2.1/5.2.2 delegate to g_platformAudio)
    virtual void LoadSound(ESound buffer, const wchar_t* filename, int channels = MAX_CHANNEL,
                           bool enable3D = false) = 0;
    [[nodiscard]] virtual bool PlaySound(ESound buffer, const void* pObject = nullptr, bool looped = false) = 0;
    virtual void StopSound(ESound buffer, bool resetPosition) = 0;
    virtual void AllStopSound() = 0;
    virtual void Set3DSoundPosition() = 0;
    virtual void SetVolume(ESound buffer, long vol) = 0;
    virtual void SetMasterVolume(long vol) = 0;

    // Music — mirrors Winmain.cpp wzAudio wrapper signatures exactly
    virtual void PlayMusic(const char* name, bool enforce) = 0;
    virtual void StopMusic(const char* name, bool enforce) = 0;
    [[nodiscard]] virtual bool IsEndMusic() = 0;
    [[nodiscard]] virtual int GetMusicPosition() = 0;

    // Volume controls — linear 0.0 (mute) to 1.0 (full). Story 5.4.1.
    virtual void SetBGMVolume(float level) = 0;
    virtual void SetSFXVolume(float level) = 0;
    [[nodiscard]] virtual float GetBGMVolume() const = 0;
    [[nodiscard]] virtual float GetSFXVolume() const = 0;
};

} // namespace mu

// Global audio backend pointer — nullptr until game initializes audio (Story 5.2.1)
// TODO (LOW-NEW-1, deferred to future story): This singleton declaration belongs in
// Winmain.h alongside the other audio free-function declarations, not in the pure
// interface header. Moving it here couples every consumer of IPlatformAudio to the
// singleton pattern. Pre-existing from Story 5.1.1; no regression introduced in 5.2.1.
extern mu::IPlatformAudio* g_platformAudio;
