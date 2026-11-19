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

#include "Core/Platform/IPlatformAudio.h"

// miniaudio.h required for ma_engine and ma_sound full type definitions.
// See isolation note above — this include is the known limitation tracked for 5.2.x.
#include "miniaudio.h"

#include <array>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace mu
{

// [Story 7-6-7: AC-6] Enumerate playback device names via miniaudio.
// Isolated here to keep miniaudio.h out of ErrorReport.cpp.
std::vector<std::string> GetAudioDeviceNames();

// The file system the engine's resource manager reads sounds and music through: the game's content.
ma_vfs* ContentAudioFiles();

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
    [[nodiscard]] bool PlaySound(ESound buffer, const void* pObject = nullptr, bool looped = false) override;
    [[nodiscard]] bool PlaySoundAtLevel(ESound buffer, float level) override;
    [[nodiscard]] bool PlaySoundOnFreeChannel(ESound buffer, float level) override;
    void StopSound(ESound buffer, bool resetPosition) override;
    void AllStopSound() override;
    void ReleaseSound(ESound buffer) override;
    void Set3DSoundPosition() override;
    void SetVolume(ESound buffer, long vol) override;
    void SetMasterVolume(long vol) override;

    // Music
    void PlayMusic(const char* name, bool enforce) override;
    void StopMusic(const char* name, bool enforce) override;
    [[nodiscard]] bool IsEndMusic() override;
    [[nodiscard]] int GetMusicPosition() override;

    // Volume controls (Story 5.4.1)
    void SetBGMVolume(float level) override;
    void SetSFXVolume(float level) override;
    void SetSoundMixGain(ESound buffer, int group, float gain) override;
    void SetMusicMixGain(float gain) override;
    [[nodiscard]] std::uint32_t GetSoundStarts(ESound buffer) const override;
    [[nodiscard]] std::uint32_t GetMusicStarts() const override;
    [[nodiscard]] bool IsSoundPlaying(ESound buffer) const override;
    [[nodiscard]] float GetSoundLength(ESound buffer) const override;
    [[nodiscard]] float GetBGMVolume() const override;
    [[nodiscard]] float GetSFXVolume() const override;

private:
    // PlaySound at `level`: of the effect volume and times the sound's mix
    // gain when `mixed` (the game's plays), a preview's own level otherwise.
    bool StartSound(ESound buffer, const void* pObject, bool looped, float level, bool mixed);

    // A channel that starts takes its sound's mix gain and group for the next
    // start and keeps its level, then SetChannelVolume.
    void StartChannelVolume(int buffer, int channel, float level, bool mixed);
    // Sets the channel to `volume`, times the mix gain it started with when
    // `mixed`, and keeps both for a later change of the gain.
    void SetChannelVolume(int buffer, int channel, float volume, bool mixed);

    // Convert DirectSound volume (dB * 100, range -10000..0) to linear 0.0..1.0
    static float DbToLinear(long dsVol);

    bool HasFailedMusicTrack(const char* name) const;
    bool StartMusicStream(const std::string& identity);
    void ReleaseMusicStream();

    ma_engine m_engine{};

    // Whether `buffer` has a slot; LoadSound adds the slots up to a number
    // after the SOUND_* list.
    bool HasSlot(int buffer) const;
    bool AddSlotsUpTo(int buffer);
    int GetSlotCount() const;

    // Per-sound polyphonic slots: [sound number][channel index]. One for each
    // SOUND_* id (MAX_BUFFER) from the start; LoadSound adds the slots of the
    // numbers that sounds without an id get (Audio/SoundNumbers.h). Deques,
    // because miniaudio keeps pointers to the sounds: adding moves none.
    std::deque<std::array<ma_sound, MAX_CHANNEL>> m_sounds = std::deque<std::array<ma_sound, MAX_CHANNEL>>(MAX_BUFFER);
    std::deque<bool> m_soundLoaded = std::deque<bool>(MAX_BUFFER);
    std::deque<int> m_activeChannel = std::deque<int>(MAX_BUFFER);
    // CRITICAL-1 fix: tracks how many polyphonic slots were actually initialised by
    // LoadSound(). PlaySound/StopSound/AllStopSound must iterate only these slots —
    // iterating all MAX_CHANNEL slots when numChannels < MAX_CHANNEL calls miniaudio
    // APIs on uninitialized ma_sound handles (UB).
    std::deque<int> m_loadedChannels = std::deque<int>(MAX_BUFFER);
    // The Sound Mixer's gain and group for a sound number's next start (1 and
    // 0 unless the editor sets them); each channel's volume without the gain,
    // the level it started at (a mixed channel's share of the effect volume:
    // 1, or a map object copy's distance level), the gain and group it started
    // with, and whether the gain and the effect volume apply to the channel
    // (not to a preview's PlaySoundAtLevel).
    std::deque<float> m_mixGain = std::deque<float>(MAX_BUFFER, 1.0f);
    std::deque<int> m_mixGroup = std::deque<int>(MAX_BUFFER);
    std::deque<std::array<float, MAX_CHANNEL>> m_channelVolume = std::deque<std::array<float, MAX_CHANNEL>>(MAX_BUFFER);
    std::deque<std::array<float, MAX_CHANNEL>> m_channelLevel = std::deque<std::array<float, MAX_CHANNEL>>(MAX_BUFFER);
    std::deque<std::array<float, MAX_CHANNEL>> m_channelGain = std::deque<std::array<float, MAX_CHANNEL>>(MAX_BUFFER);
    std::deque<std::array<int, MAX_CHANNEL>> m_channelGroup = std::deque<std::array<int, MAX_CHANNEL>>(MAX_BUFFER);
    std::deque<std::array<bool, MAX_CHANNEL>> m_channelMixed = std::deque<std::array<bool, MAX_CHANNEL>>(MAX_BUFFER);
    // How often each sound started on a mixed channel (GetSoundStarts).
    std::deque<std::uint32_t> m_starts = std::deque<std::uint32_t>(MAX_BUFFER);
    float m_musicMixGain = 1.0f;
    // How many tracks started (GetMusicStarts).
    std::uint32_t m_musicStarts = 0;

    float m_bgmVolume = 1.0f; // BGM volume: 0.0 (mute) to 1.0 (full) — Story 5.4.1
    float m_sfxVolume = 1.0f; // SFX volume: 0.0 (mute) to 1.0 (full) — Story 5.4.1

    ma_sound m_musicSound{};
    bool m_musicLoaded = false;
    // Separators-normalized track name, not the resolved path — see IsSameMusicTrack().
    std::string m_currentMusicName;
    // Session failures survive stop calls and track changes; enforced playback retries them.
    std::vector<std::string> m_failedMusicNames;
    bool m_initialized = false;
};

} // namespace mu
