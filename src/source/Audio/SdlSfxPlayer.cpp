///////////////////////////////////////////////////////////////////////////////
// SDL_mixer sound effects (non-Windows)
///////////////////////////////////////////////////////////////////////////////

// Implements the sound-effect API from DSPlaySound.h on top of SDL3_mixer for
// platforms without DirectSound (issue #462 follow-up).  It shares the mixer
// device that AudioPlayer creates for music and mirrors the observable
// behavior of the DirectSound backend: one decoded buffer per ESound with a
// small round-robin channel pool, and volumes in DirectSound's hundredths of
// a decibel.  3D spatialization is not implemented — the DirectSound backend
// never enables it either (its 3D flag has no callers), so parity is 2D.

#include "stdafx.h"

#ifndef _WIN32

#include <algorithm>
#include <array>
#include <cmath>
#include <mutex>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "Audio/AudioPlayer.h"
#include "Engine/Object/ZzzCharacter.h"  // OBJECT, used in the DSPlaySound.h API
#include "Audio/DSPlaySound.h"
#include "Core/Platform/PathResolve.h"
#include "Core/Platform/WinNls.h"

namespace
{
// DirectSound volume scale: hundredths of a decibel, 0 (full) .. -10000 (mute).
constexpr long kDsVolumeMax = 0L;
constexpr long kDsVolumeMin = -10000L;
// Hundredths of dB per factor-10 amplitude change (20 dB * 100).
constexpr float kDsCentibelsPerDecade = 2000.0f;

float DsVolumeToGain(long volume)
{
    const long clamped = std::clamp(volume, kDsVolumeMin, kDsVolumeMax);
    if (clamped <= kDsVolumeMin)
    {
        return 0.0f;
    }

    return std::pow(10.0f, static_cast<float>(clamped) / kDsCentibelsPerDecade);
}

struct SfxEntry
{
    MIX_Audio* audio = nullptr;
    std::array<MIX_Track*, MAX_CHANNEL> tracks {};
    int activeChannel = 0;
    int maxChannels = 0;
};

class SdlSfxManager
{
public:
    HRESULT Initialize();
    void Shutdown();

    void SetEnabled(bool enabled);
    bool IsEnabled() const noexcept;

    HRESULT LoadWaveFile(ESound bufferId, const wchar_t* filename, int maxChannel);
    HRESULT ReleaseBuffer(ESound bufferId);

    HRESULT PlayBuffer(ESound bufferId, bool looped);
    void StopBuffer(ESound bufferId);
    void StopAll();

    void SetVolume(ESound bufferId, long volume);
    void SetMasterVolume(long volume);

private:
    void ReleaseAllBuffers();
    void ResetEntry(SfxEntry& entry);
    void SetGainInternal(SfxEntry& entry, float gain);
    bool IsValidBufferIndex(int bufferId) const noexcept;

    mutable std::mutex mutex_;
    MIX_Mixer* mixer_ = nullptr;
    std::array<SfxEntry, MAX_BUFFER> entries_ {};
    bool enableSound_ = false;
    float masterGain_ = 1.0f;
};

SdlSfxManager& Manager()
{
    static SdlSfxManager instance;
    return instance;
}

HRESULT SdlSfxManager::Initialize()
{
    std::lock_guard lock(mutex_);

    // AudioPlayer::Initialize() runs first (see Winmain's CreateOpenGL); if it
    // failed there is no audio device, so sound effects stay disabled too.
    mixer_ = AudioPlayer::GetMixer();
    if (mixer_ == nullptr)
    {
        g_ErrorReport.Write(L"InitDirectSound - no SDL mixer available, sound effects disabled\r\n");
        return E_FAIL;
    }

    enableSound_ = true;
    return S_OK;
}

void SdlSfxManager::Shutdown()
{
    std::lock_guard lock(mutex_);

    ReleaseAllBuffers();
    mixer_ = nullptr;  // owned by AudioPlayer, which shuts down after us
    enableSound_ = false;
}

void SdlSfxManager::SetEnabled(bool enabled)
{
    enableSound_ = enabled;
}

bool SdlSfxManager::IsEnabled() const noexcept
{
    return enableSound_;
}

HRESULT SdlSfxManager::LoadWaveFile(ESound bufferId, const wchar_t* filename, int maxChannel)
{
    if (!IsEnabled())
    {
        return E_FAIL;
    }

    if (!IsValidBufferIndex(bufferId) || filename == nullptr)
    {
        return E_INVALIDARG;
    }

    const int clampedChannels = std::clamp(maxChannel, 1, MAX_CHANNEL);

    std::lock_guard lock(mutex_);
    auto& entry = entries_[bufferId];
    if (entry.maxChannels > 0)
    {
        return S_FALSE;
    }

    // Sound paths are Windows-spelled wide strings (backslashes, mixed case);
    // convert to UTF-8 and resolve against the case-sensitive filesystem.
    char path[4096] = { 0 };
    if (WideCharToMultiByte(CP_UTF8, 0, filename, -1, path, sizeof(path) - 1, nullptr, nullptr) == 0)
    {
        return E_INVALIDARG;
    }

    // Predecode: effects are short and played repeatedly, so trading a little
    // memory for zero decode latency at play time is the right call.
    MIX_Audio* audio = MIX_LoadAudio(mixer_, MuResolvePath(path).c_str(), /*predecode=*/true);
    if (audio == nullptr)
    {
        g_ErrorReport.Write(L"LoadWaveFile - failed to load %ls (%hs)\r\n", filename, SDL_GetError());
        return E_FAIL;
    }

    for (int channel = 0; channel < clampedChannels; ++channel)
    {
        MIX_Track* track = MIX_CreateTrack(mixer_);
        if (track == nullptr || !MIX_SetTrackAudio(track, audio))
        {
            g_ErrorReport.Write(L"LoadWaveFile - failed to create track for %ls (%hs)\r\n", filename, SDL_GetError());
            if (track != nullptr)
            {
                MIX_DestroyTrack(track);
            }
            entry.audio = audio;
            entry.maxChannels = channel;  // release only the tracks created so far
            ResetEntry(entry);
            return E_FAIL;
        }

        MIX_SetTrackGain(track, masterGain_);
        entry.tracks[channel] = track;
    }

    entry.audio = audio;
    entry.activeChannel = 0;
    entry.maxChannels = clampedChannels;
    return S_OK;
}

HRESULT SdlSfxManager::ReleaseBuffer(ESound bufferId)
{
    if (!IsValidBufferIndex(bufferId))
    {
        return E_INVALIDARG;
    }

    std::lock_guard lock(mutex_);
    ResetEntry(entries_[bufferId]);
    return S_OK;
}

void SdlSfxManager::ReleaseAllBuffers()
{
    for (auto& entry : entries_)
    {
        ResetEntry(entry);
    }
}

HRESULT SdlSfxManager::PlayBuffer(ESound bufferId, bool looped)
{
    if (!IsEnabled())
    {
        return E_FAIL;
    }

    if (!IsValidBufferIndex(bufferId))
    {
        return E_INVALIDARG;
    }

    std::lock_guard lock(mutex_);
    auto& entry = entries_[bufferId];
    if (entry.maxChannels == 0)
    {
        return E_FAIL;
    }

    const int currentChannel = entry.activeChannel % entry.maxChannels;
    entry.activeChannel = (entry.activeChannel + 1) % entry.maxChannels;

    MIX_Track* track = entry.tracks[currentChannel];
    if (track == nullptr)
    {
        return E_FAIL;
    }

    // Looping must be requested at play time: starting a stopped track resets
    // the loop count to the MIX_PROP_PLAY_LOOPS_NUMBER option (default 0), so
    // a prior MIX_SetTrackLoops would be overwritten here.
    bool played = false;
    if (looped)
    {
        const SDL_PropertiesID options = SDL_CreateProperties();
        SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
        played = MIX_PlayTrack(track, options);
        SDL_DestroyProperties(options);
    }
    else
    {
        played = MIX_PlayTrack(track, 0);
    }

    if (!played)
    {
        g_ConsoleDebug->Write(MCD_ERROR, L"SdlSfxManager::PlayBuffer failed for %d (%hs)", bufferId, SDL_GetError());
        return E_FAIL;
    }

    return S_OK;
}

void SdlSfxManager::StopBuffer(ESound bufferId)
{
    if (!IsValidBufferIndex(bufferId))
    {
        return;
    }

    std::lock_guard lock(mutex_);
    auto& entry = entries_[bufferId];
    for (int channel = 0; channel < entry.maxChannels; ++channel)
    {
        if (entry.tracks[channel] != nullptr)
        {
            MIX_StopTrack(entry.tracks[channel], 0);
        }
    }
}

void SdlSfxManager::StopAll()
{
    for (int i = 0; i < MAX_BUFFER; ++i)
    {
        StopBuffer(static_cast<ESound>(i));
    }
}

void SdlSfxManager::SetVolume(ESound bufferId, long volume)
{
    if (!IsValidBufferIndex(bufferId))
    {
        return;
    }

    std::lock_guard lock(mutex_);
    SetGainInternal(entries_[bufferId], DsVolumeToGain(volume));
}

void SdlSfxManager::SetMasterVolume(long volume)
{
    const float gain = DsVolumeToGain(volume);

    std::lock_guard lock(mutex_);
    masterGain_ = gain;
    for (auto& entry : entries_)
    {
        SetGainInternal(entry, gain);
    }
}

void SdlSfxManager::ResetEntry(SfxEntry& entry)
{
    // Tracks hold a pointer to the audio, so destroy them before it.
    for (int channel = 0; channel < entry.maxChannels; ++channel)
    {
        if (entry.tracks[channel] != nullptr)
        {
            MIX_DestroyTrack(entry.tracks[channel]);
            entry.tracks[channel] = nullptr;
        }
    }

    if (entry.audio != nullptr)
    {
        MIX_DestroyAudio(entry.audio);
        entry.audio = nullptr;
    }

    entry.activeChannel = 0;
    entry.maxChannels = 0;
}

void SdlSfxManager::SetGainInternal(SfxEntry& entry, float gain)
{
    // NOTE: Assumes caller holds the mutex.
    for (int channel = 0; channel < entry.maxChannels; ++channel)
    {
        if (entry.tracks[channel] != nullptr)
        {
            MIX_SetTrackGain(entry.tracks[channel], gain);
        }
    }
}

bool SdlSfxManager::IsValidBufferIndex(int bufferId) const noexcept
{
    return bufferId >= 0 && bufferId < MAX_BUFFER;
}
} // namespace

HRESULT InitDirectSound(HWND)
{
    return Manager().Initialize();
}

void SetEnableSound(bool enabled)
{
    Manager().SetEnabled(enabled);
    if (!enabled)
    {
        Manager().StopAll();
    }
}

void FreeDirectSound()
{
    Manager().Shutdown();
}

void LoadWaveFile(ESound bufferId, const wchar_t* filename, int maxChannel, bool /*enable3D*/)
{
    Manager().LoadWaveFile(bufferId, filename, maxChannel);
}

HRESULT PlayBuffer(ESound bufferId, OBJECT* /*object*/, BOOL looped)
{
    return Manager().PlayBuffer(bufferId, looped != FALSE);
}

void StopBuffer(ESound bufferId, BOOL /*resetPosition*/)
{
    // A stopped SDL_mixer track always restarts from the beginning on the
    // next play, so the reset-position flag has nothing to do here.
    Manager().StopBuffer(bufferId);
}

void AllStopSound()
{
    Manager().StopAll();
}

void Set3DSoundPosition()
{
    // No 3D spatialization — see the note at the top of this file.
}

HRESULT ReleaseBuffer(int bufferId)
{
    return Manager().ReleaseBuffer(static_cast<ESound>(bufferId));
}

HRESULT RestoreBuffers(int, int)
{
    // DirectSound's lost-buffer recovery has no SDL_mixer equivalent.
    return S_OK;
}

void SetVolume(int bufferId, long volume)
{
    Manager().SetVolume(static_cast<ESound>(bufferId), volume);
}

void SetMasterVolume(long volume)
{
    Manager().SetMasterVolume(volume);
}

#endif // !_WIN32
