#include "stdafx.h"

// Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
// Implementation of MiniAudioBackend — miniaudio-based platform audio backend.
// See IPlatformAudio.h for the interface contract.

#include "MiniAudioBackend.h"
#include "ErrorReport.h"
#include "PlatformCompat.h"

#include <cmath>
#include <cstring>

// ---------------------------------------------------------------------------
// Global backend pointer — nullptr until the game initialises audio.
// Story 5.2.1 will set g_platformAudio = new mu::MiniAudioBackend() and call
// Initialize() during game startup. This story only creates the abstraction.
// ---------------------------------------------------------------------------
mu::IPlatformAudio* g_platformAudio = nullptr;

namespace mu
{

// ---------------------------------------------------------------------------
// Destructor — ensure resources are released even if Shutdown() was not called
// ---------------------------------------------------------------------------
MiniAudioBackend::~MiniAudioBackend()
{
    if (m_initialized)
    {
        Shutdown();
    }
}

// ---------------------------------------------------------------------------
// Initialize — start the miniaudio engine
// Returns false (and logs) if ma_engine_init fails (e.g., no audio device on CI)
// ---------------------------------------------------------------------------
[[nodiscard]] bool MiniAudioBackend::Initialize()
{
    if (m_initialized)
    {
        return true;
    }

    ma_result result = ma_engine_init(nullptr, &m_engine);
    if (result != MA_SUCCESS)
    {
        g_ErrorReport.Write(L"AUDIO: MiniAudioBackend::Initialize -- ma_engine_init failed (%d)\r\n",
                            static_cast<int>(result));
        return false;
    }

    m_initialized = true;
    return true;
}

// ---------------------------------------------------------------------------
// Shutdown — release all loaded sounds and uninit the engine
// ---------------------------------------------------------------------------
void MiniAudioBackend::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    // Release all loaded sound effect slots
    for (int buf = 0; buf < static_cast<int>(MAX_BUFFER); ++buf)
    {
        if (m_soundLoaded[buf])
        {
            for (int ch = 0; ch < MAX_CHANNEL; ++ch)
            {
                ma_sound_uninit(&m_sounds[buf][ch]);
            }
            m_soundLoaded[buf] = false;
        }
    }

    // Release music stream
    if (m_musicLoaded)
    {
        ma_sound_uninit(&m_musicSound);
        m_musicLoaded = false;
        m_currentMusicName.clear();
    }

    ma_engine_uninit(&m_engine);
    m_initialized = false;
}

// ---------------------------------------------------------------------------
// LoadSound — load a WAV/OGG sound file into polyphonic slots
// Each ESound slot gets MAX_CHANNEL (4) duplicate ma_sound instances for polyphony.
// Sound effects use MA_SOUND_FLAG_DECODE (pre-decode to PCM for low-latency).
// 3D sounds also enable spatialization per slot.
// ---------------------------------------------------------------------------
void MiniAudioBackend::LoadSound(ESound buffer, const wchar_t* filename, int channels, bool enable3D)
{
    if (!m_initialized)
    {
        return;
    }

    const int bufIdx = static_cast<int>(buffer);
    if (bufIdx < 0 || bufIdx >= static_cast<int>(MAX_BUFFER))
    {
        return;
    }

    // Unload previously loaded sound at this slot
    if (m_soundLoaded[bufIdx])
    {
        for (int ch = 0; ch < MAX_CHANNEL; ++ch)
        {
            ma_sound_uninit(&m_sounds[bufIdx][ch]);
        }
        m_soundLoaded[bufIdx] = false;
    }

    // Convert wchar_t filename to UTF-8 for miniaudio (which uses narrow char*)
    std::string utf8Path = mu_wchar_to_utf8(filename);

    // Clamp channels to the array dimension
    const int numChannels = (channels > 0 && channels <= MAX_CHANNEL) ? channels : MAX_CHANNEL;

    // Load each polyphonic slot
    for (int ch = 0; ch < numChannels; ++ch)
    {
        ma_result result = ma_sound_init_from_file(&m_engine, utf8Path.c_str(),
                                                   MA_SOUND_FLAG_DECODE, // pre-decode for low-latency sound effects
                                                   nullptr, nullptr, &m_sounds[bufIdx][ch]);

        if (result != MA_SUCCESS)
        {
            g_ErrorReport.Write(L"AUDIO: MiniAudioBackend::LoadSound -- ma_sound_init_from_file failed for '%ls' "
                                L"channel %d (%d)\r\n",
                                filename, ch, static_cast<int>(result));
            // Uninit any channels already loaded for this slot
            for (int prev = 0; prev < ch; ++prev)
            {
                ma_sound_uninit(&m_sounds[bufIdx][prev]);
            }
            return;
        }

        if (enable3D)
        {
            ma_sound_set_spatialization_enabled(&m_sounds[bufIdx][ch], MA_TRUE);
        }
    }

    m_soundLoaded[bufIdx] = true;
    m_sound3DEnabled[bufIdx] = enable3D;
    m_activeChannel[bufIdx] = 0;
}

// ---------------------------------------------------------------------------
// PlaySound — start playback of a sound effect using round-robin channel selection
// Mirrors DSPlaySound.h PlayBuffer() signature exactly.
// ---------------------------------------------------------------------------
HRESULT MiniAudioBackend::PlaySound(ESound buffer, OBJECT* pObject, BOOL looped)
{
    if (!m_initialized)
    {
        return S_OK;
    }

    const int bufIdx = static_cast<int>(buffer);
    if (bufIdx < 0 || bufIdx >= static_cast<int>(MAX_BUFFER))
    {
        return E_INVALIDARG;
    }

    if (!m_soundLoaded[bufIdx])
    {
        return S_FALSE;
    }

    // Round-robin channel selection
    const int ch = m_activeChannel[bufIdx];
    m_activeChannel[bufIdx] = (ch + 1) % MAX_CHANNEL;

    ma_sound* pSound = &m_sounds[bufIdx][ch];

    // Seek to start before playing to handle overlapping sounds cleanly
    ma_sound_seek_to_pcm_frame(pSound, 0);
    ma_sound_set_looping(pSound, looped ? MA_TRUE : MA_FALSE);
    ma_sound_start(pSound);

    return S_OK;
}

// ---------------------------------------------------------------------------
// StopSound — stop all channels for the given sound slot
// ---------------------------------------------------------------------------
void MiniAudioBackend::StopSound(ESound buffer, BOOL resetPosition)
{
    if (!m_initialized)
    {
        return;
    }

    const int bufIdx = static_cast<int>(buffer);
    if (bufIdx < 0 || bufIdx >= static_cast<int>(MAX_BUFFER))
    {
        return;
    }

    if (!m_soundLoaded[bufIdx])
    {
        return;
    }

    for (int ch = 0; ch < MAX_CHANNEL; ++ch)
    {
        ma_sound_stop(&m_sounds[bufIdx][ch]);
        if (resetPosition)
        {
            ma_sound_seek_to_pcm_frame(&m_sounds[bufIdx][ch], 0);
        }
    }
}

// ---------------------------------------------------------------------------
// AllStopSound — stop all sound effects across all slots
// ---------------------------------------------------------------------------
void MiniAudioBackend::AllStopSound()
{
    if (!m_initialized)
    {
        return;
    }

    for (int buf = 0; buf < static_cast<int>(MAX_BUFFER); ++buf)
    {
        if (m_soundLoaded[buf])
        {
            for (int ch = 0; ch < MAX_CHANNEL; ++ch)
            {
                ma_sound_stop(&m_sounds[buf][ch]);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Set3DSoundPosition — update spatial positions for all 3D-enabled sounds
// Mirrors DSplaysound.cpp Set3DSoundPosition() behaviour.
// Called each frame for sounds that have an attached OBJECT.
// ---------------------------------------------------------------------------
void MiniAudioBackend::Set3DSoundPosition()
{
    // NOTE: 3D position update requires OBJECT world positions.
    // OBJECT includes World position members; full implementation in Story 5.2.1
    // when call sites are migrated. This stub satisfies the interface contract.
}

// ---------------------------------------------------------------------------
// SetVolume — set the volume of a sound effect slot
// DirectSound vol: -10000 (silent) to 0 (full) in 1/100ths of a dB.
// Converts to miniaudio linear 0.0..1.0 via 10^(vol/2000).
// ---------------------------------------------------------------------------
void MiniAudioBackend::SetVolume(ESound buffer, long vol)
{
    if (!m_initialized)
    {
        return;
    }

    const int bufIdx = static_cast<int>(buffer);
    if (bufIdx < 0 || bufIdx >= static_cast<int>(MAX_BUFFER))
    {
        return;
    }

    if (!m_soundLoaded[bufIdx])
    {
        return;
    }

    const float linear = DbToLinear(vol);
    for (int ch = 0; ch < MAX_CHANNEL; ++ch)
    {
        ma_sound_set_volume(&m_sounds[bufIdx][ch], linear);
    }
}

// ---------------------------------------------------------------------------
// SetMasterVolume — set the master engine volume
// ---------------------------------------------------------------------------
void MiniAudioBackend::SetMasterVolume(long vol)
{
    if (!m_initialized)
    {
        return;
    }

    ma_engine_set_volume(&m_engine, DbToLinear(vol));
}

// ---------------------------------------------------------------------------
// PlayMusic — start streaming a music track
// If enforce=false and the same track is already playing, returns early (no restart).
// Matches wzAudioPlay() + Mp3FileName guard logic from Winmain.cpp.
// ---------------------------------------------------------------------------
void MiniAudioBackend::PlayMusic(const char* name, BOOL enforce)
{
    if (!m_initialized || name == nullptr)
    {
        return;
    }

    // If not enforced and same track is already playing, do nothing
    if (!enforce && !m_currentMusicName.empty() && m_currentMusicName == name)
    {
        return;
    }

    // Stop and release previous music stream
    if (m_musicLoaded)
    {
        ma_sound_stop(&m_musicSound);
        ma_sound_uninit(&m_musicSound);
        m_musicLoaded = false;
        m_currentMusicName.clear();
    }

    // Start new stream — MA_SOUND_FLAG_STREAM keeps it off the decode buffer
    ma_result result = ma_sound_init_from_file(&m_engine, name, MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_ASYNC, nullptr,
                                               nullptr, &m_musicSound);

    if (result != MA_SUCCESS)
    {
        g_ErrorReport.Write(L"AUDIO: MiniAudioBackend::PlayMusic -- failed to init stream '%hs' (%d)\r\n", name,
                            static_cast<int>(result));
        return;
    }

    ma_sound_set_looping(&m_musicSound, MA_TRUE);
    ma_sound_start(&m_musicSound);

    m_musicLoaded = true;
    m_currentMusicName = name;
}

// ---------------------------------------------------------------------------
// StopMusic — stop the current music stream
// If enforce=true, also uninit the stream and clear the current track name.
// Mirrors wzAudioStop() + Mp3FileName guard logic from Winmain.cpp.
// ---------------------------------------------------------------------------
void MiniAudioBackend::StopMusic(const char* name, BOOL enforce)
{
    if (!m_initialized || !m_musicLoaded)
    {
        return;
    }

    // If not enforced, only stop if the name matches the current track
    if (!enforce && name != nullptr && !m_currentMusicName.empty())
    {
        if (m_currentMusicName != name)
        {
            return;
        }
    }

    ma_sound_stop(&m_musicSound);

    if (enforce)
    {
        ma_sound_uninit(&m_musicSound);
        m_musicLoaded = false;
        m_currentMusicName.clear();
    }
}

// ---------------------------------------------------------------------------
// IsEndMusic — returns true when no music is playing
// Mirrors wzAudioGetStreamOffsetRange() == 100 semantics from Winmain.cpp.
// Returns true when not initialized (safe default state for tests and pre-init).
// ---------------------------------------------------------------------------
[[nodiscard]] bool MiniAudioBackend::IsEndMusic()
{
    if (!m_initialized || !m_musicLoaded)
    {
        return true;
    }

    return !ma_sound_is_playing(&m_musicSound);
}

// ---------------------------------------------------------------------------
// GetMusicPosition — returns music playback position as 0..100 percentage
// Mirrors wzAudioGetStreamOffsetRange() return value from Winmain.cpp.
// Returns 0 when not initialized or no music loaded.
// ---------------------------------------------------------------------------
[[nodiscard]] int MiniAudioBackend::GetMusicPosition()
{
    if (!m_initialized || !m_musicLoaded)
    {
        return 0;
    }

    // Get cursor in PCM frames and convert to seconds
    ma_uint64 cursorFrames = 0;
    ma_sound_get_cursor_in_pcm_frames(&m_musicSound, &cursorFrames);

    const ma_uint32 sampleRate = ma_engine_get_sample_rate(&m_engine);
    if (sampleRate == 0)
    {
        return 0;
    }

    // Get total length in frames for percentage calculation
    ma_uint64 totalFrames = 0;
    ma_sound_get_length_in_pcm_frames(&m_musicSound, &totalFrames);

    if (totalFrames == 0)
    {
        return 0;
    }

    // Normalise to 0..100 to match wzAudioGetStreamOffsetRange() semantics
    const int position =
        static_cast<int>((static_cast<float>(cursorFrames) / static_cast<float>(totalFrames)) * 100.0f);
    return position;
}

// ---------------------------------------------------------------------------
// DbToLinear — convert DirectSound dB*100 scale to linear 0.0..1.0
// DirectSound volume: -10000 (DSBVOLUME_MIN, silent) to 0 (full volume)
// Each unit = 1/100th of a dB, so vol / 2000.0f = dB / 20.0f (standard dB-to-linear)
// ---------------------------------------------------------------------------
float MiniAudioBackend::DbToLinear(long dsVol)
{
    return std::pow(10.0f, static_cast<float>(dsVol) / 2000.0f);
}

} // namespace mu
