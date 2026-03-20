#pragma once

// Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
// Concrete miniaudio backend implementing IPlatformAudio.
// Uses ma_engine for mixing and ma_sound for individual sound/music playback.
//
// INCLUDE ISOLATION NOTE (MEDIUM-2 fix, code-review-finalize 2026-03-19):
// miniaudio.h (95k lines) is included directly in this header because ma_engine
// and ma_sound require full type definitions for stack-allocated private members.
// A true PIMPL pattern (std::unique_ptr<Impl>) would isolate the include but
// requires heap allocation. That refactor is deferred to Story 5.2.x (future).
// Current workaround: MiniAudioImpl.cpp (Story 5.1.1) isolates MINIAUDIO_IMPLEMENTATION
// via SKIP_PRECOMPILE_HEADERS ON so the 95k-line implementation block is not
// included by the PCH. The header include below is intentional and known.
// Do NOT add #include "miniaudio.h" to callers of MiniAudioBackend.h.

#include "IPlatformAudio.h"

// miniaudio.h required for ma_engine and ma_sound full type definitions.
// See isolation note above — this include is the known limitation tracked for 5.2.x.
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

    // Per-slot OBJECT* for per-frame 3D position updates (Story 5.2.2).
    // Stores the most recent pObject passed to PlaySound() for each 3D-enabled buffer slot.
    // Lifetime: game engine guarantees object outlives the attached sound (same assumption
    // as DirectSoundManager::attachedObjects in DSplaysound.cpp). Raw pointer is intentional —
    // adding lifetime management here would require an engine-wide refactor.
    // nullptr = no object attached to this slot (safe — Set3DSoundPosition() checks before deref).
    std::array<const OBJECT*, MAX_BUFFER> m_soundObjects{};

    ma_sound m_musicSound{};
    bool m_musicLoaded = false;
    std::string m_currentMusicName;
    bool m_initialized = false;
};

} // namespace mu
