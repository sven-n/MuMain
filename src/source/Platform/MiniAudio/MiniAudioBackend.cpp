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
bool MiniAudioBackend::Initialize()
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

    // MEDIUM-4 (code-review-finalize 2026-03-19): Reset slot state unconditionally
    // before the unload block so that a failed load of the same slot (partial channels)
    // leaves the slot in a consistent clean state rather than holding stale values.
    m_activeChannel[bufIdx] = 0;
    m_sound3DEnabled[bufIdx] = false;

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
// MEDIUM-2 (code-review-finalize 2026-03-19): Returns S_FALSE (not S_OK) when
// !m_initialized to signal a no-op to callers, matching !m_soundLoaded behaviour.
// HIGH-1 (code-review-finalize 2026-03-19): Apply the 3D world position from
// pObject BEFORE calling ma_sound_start() so the first mix tick on the audio
// thread already sees the correct spatial position (not silently at origin).
// NEW-HIGH-1 (code-review-finalize 2026-03-19): Ordering fix — set_position must
// come before start() to avoid the audio-thread race where the first mix tick
// renders at (0,0,0). Correct miniaudio pattern: configure all properties first,
// then start.
// NEW-LOW-1 (code-review-finalize 2026-03-19): Call ma_sound_stop() before
// ma_sound_seek_to_pcm_frame() to prevent seeking a currently-playing channel
// (round-robin reuse of looping SFX slots). Avoids the pop/click artifact caused
// by seeking while the audio thread is actively mixing the channel.
// ---------------------------------------------------------------------------
HRESULT MiniAudioBackend::PlaySound(ESound buffer, OBJECT* pObject, BOOL looped)
{
    if (!m_initialized)
    {
        return S_FALSE;
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

    // NEW-LOW-1: Stop the channel before seeking to avoid a seek-during-play race.
    // When all MAX_CHANNEL slots are busy (e.g. looping SFX), the oldest channel
    // is reused. Stopping first prevents a pop/click from seeking mid-mix.
    ma_sound_stop(pSound);

    // Seek to start so reused channels always play from the beginning.
    ma_sound_seek_to_pcm_frame(pSound, 0);

    // Configure all sound properties BEFORE starting — miniaudio audio thread
    // begins mixing immediately on ma_sound_start(), so any property set after
    // start() is subject to a data race on the first mix tick.
    ma_sound_set_looping(pSound, looped ? MA_TRUE : MA_FALSE);

    // NEW-HIGH-1 / HIGH-1: Set 3D position BEFORE start() so the audio thread
    // never renders at the default origin (0,0,0) for even one mix tick.
    // OBJECT::Position is vec3_t (float[3]): [0]=X, [1]=Y, [2]=Z.
    if (m_sound3DEnabled[bufIdx] && pObject != nullptr)
    {
        ma_sound_set_position(pSound, pObject->Position[0], pObject->Position[1], pObject->Position[2]);
    }

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
// MEDIUM-3 (code-review-finalize 2026-03-19): Documents pause vs stop semantics.
// LOW-3 (code-review-finalize 2026-03-19): Documents nullptr name behaviour.
//
// enforce=true:  Hard stop — calls ma_sound_uninit(), releases file handle and
//                decoder. Current track name is cleared. Matches wzAudioStop().
// enforce=false: Soft pause — calls ma_sound_stop() only. Stream/decoder remain
//                open. Use this only when you intend to resume later. To avoid
//                resource leaks, always call with enforce=true when done with a track.
//
// name semantics:
//   Non-null name + enforce=false: only stop if the named track is currently playing.
//   nullptr       + enforce=false: stop the current track regardless of name.
//   name is ignored when enforce=true (always stops regardless).
// ---------------------------------------------------------------------------
void MiniAudioBackend::StopMusic(const char* name, BOOL enforce)
{
    if (!m_initialized || !m_musicLoaded)
    {
        return;
    }

    // If not enforced, only stop if the name matches the current track.
    // nullptr name means "stop current track regardless of name" (unconditional soft stop).
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
        // Hard stop: release stream resources to avoid file handle / decoder leaks.
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
bool MiniAudioBackend::IsEndMusic()
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
//
// HIGH-2 (code-review-finalize 2026-03-19): Music is loaded with
// MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_ASYNC. For streaming sounds,
// ma_sound_get_length_in_pcm_frames() returns MA_RESULT indicating the length
// is unavailable while the stream is still opening — totalFrames stays 0.
// Fix: use ma_sound_get_cursor_in_seconds() for the cursor and
// ma_sound_get_length_in_seconds() for the total length. The seconds API uses
// the data-source length which streams can report even while decoding.
// If length is still unavailable (streaming not yet started), fall back to 0.
// ---------------------------------------------------------------------------
int MiniAudioBackend::GetMusicPosition()
{
    if (!m_initialized || !m_musicLoaded)
    {
        return 0;
    }

    // Get current playback position in seconds
    float cursorSeconds = 0.0f;
    ma_sound_get_cursor_in_seconds(&m_musicSound, &cursorSeconds);

    // Get total track length in seconds — works for streaming sounds unlike
    // ma_sound_get_length_in_pcm_frames() which returns 0 until fully decoded.
    float totalSeconds = 0.0f;
    ma_sound_get_length_in_seconds(&m_musicSound, &totalSeconds);

    if (totalSeconds <= 0.0f)
    {
        // Length not yet available (stream still opening) — return 0.
        // Callers checking for "IsEndMusic()" should use IsEndMusic() instead.
        return 0;
    }

    // Normalise to 0..100 to match wzAudioGetStreamOffsetRange() semantics
    const float ratio = cursorSeconds / totalSeconds;
    const int position = static_cast<int>(ratio * 100.0f);
    return (position < 0) ? 0 : (position > 100) ? 100 : position;
}

// ---------------------------------------------------------------------------
// DbToLinear — convert DirectSound dB*100 scale to linear 0.0..1.0
// DirectSound volume: -10000 (DSBVOLUME_MIN, silent) to 0 (full volume)
// Each unit = 1/100th of a dB, so vol / 2000.0f = dB / 20.0f (standard dB-to-linear)
//
// NEW-MEDIUM-1 (code-review-finalize 2026-03-19): Clamp dsVol to <= 0 before
// conversion. The valid DirectSound range is DSBVOLUME_MIN (-10000) to 0.
// A positive dsVol would produce gain > 1.0, causing over-amplified distorted
// audio because miniaudio does not hard-clamp the volume internally.
// Clamping to 0 ensures full volume (1.0) is the maximum output, matching the
// DirectSound contract. Callers passing positive values get clamped silently.
// ---------------------------------------------------------------------------
float MiniAudioBackend::DbToLinear(long dsVol)
{
    // Clamp to valid DirectSound range: DSBVOLUME_MIN (-10000) to 0
    const float clampedVol = (dsVol > 0L) ? 0.0f : static_cast<float>(dsVol);
    return std::pow(10.0f, clampedVol / 2000.0f);
}

} // namespace mu
