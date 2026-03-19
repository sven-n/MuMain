#pragma once

// Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
// Concrete miniaudio backend implementing IPlatformAudio.
// Uses ma_engine for mixing and ma_sound for individual sound/music playback.
//
// INCLUDE ISOLATION NOTE (HIGH-3 code-review-finalize 2026-03-19):
// miniaudio.h (95k lines) is intentionally NOT included here to avoid polluting
// all translation units that include this header. Full type definitions are
// required for stack-allocated ma_engine and ma_sound members, so a true PIMPL
// pattern would require heap allocation (std::unique_ptr<Impl>). That refactor
// is deferred to Story 5.2.1. In the current architecture MiniAudioBackend.cpp
// sets SKIP_PRECOMPILE_HEADERS ON to isolate the include from the PCH.
// Do NOT add #include "miniaudio.h" to callers of this header.

#include "IPlatformAudio.h"

// miniaudio.h is included here because ma_engine and ma_sound require full
// type definitions for stack allocation in the private members below.
// See isolation note above — this is a known limitation tracked for 5.2.1.
#include "miniaudio.h"

#include <array>
#include <string>

namespace mu
{

class MiniAudioBackend : public IPlatformAudio
{
public:
    MiniAudioBackend() = default;
    ~MiniAudioBackend() override;

    // Lifecycle
    [[nodiscard]] bool Initialize() override;
    void Shutdown() override;

    // Sound effects
    void LoadSound(ESound buffer, const wchar_t* filename, int channels = MAX_CHANNEL, bool enable3D = false) override;
    HRESULT PlaySound(ESound buffer, OBJECT* pObject = nullptr, BOOL looped = false) override;
    void StopSound(ESound buffer, BOOL resetPosition) override;
    void AllStopSound() override;
    void Set3DSoundPosition() override;
    void SetVolume(ESound buffer, long vol) override;
    void SetMasterVolume(long vol) override;

    // Music
    void PlayMusic(const char* name, BOOL enforce) override;
    void StopMusic(const char* name, BOOL enforce) override;
    [[nodiscard]] bool IsEndMusic() override;
    [[nodiscard]] int GetMusicPosition() override;

private:
    // Convert DirectSound volume (dB * 100, range -10000..0) to linear 0.0..1.0
    static float DbToLinear(long dsVol);

    ma_engine m_engine{};

    // Per-sound polyphonic slots: [ESound index][channel index]
    // MAX_BUFFER is the last ESound enum entry — used as the array size.
    std::array<std::array<ma_sound, MAX_CHANNEL>, MAX_BUFFER> m_sounds{};
    std::array<bool, MAX_BUFFER> m_soundLoaded{};
    std::array<int, MAX_BUFFER> m_activeChannel{};
    std::array<bool, MAX_BUFFER> m_sound3DEnabled{};

    ma_sound m_musicSound{};
    bool m_musicLoaded = false;
    std::string m_currentMusicName;
    bool m_initialized = false;
};

} // namespace mu
