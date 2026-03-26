#include "stdafx.h"

// Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
// Implementation of MiniAudioBackend — miniaudio-based platform audio backend.
// See IPlatformAudio.h for the interface contract.

#include "MiniAudioBackend.h"
#include "ErrorReport.h"
#include "PlatformCompat.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// S_OK, S_FALSE are now provided by PlatformTypes.h (via PCH)
#ifndef E_INVALIDARG
#define E_INVALIDARG 0x80070057L
#endif

// ---------------------------------------------------------------------------
// Global backend pointer — nullptr until the game initialises audio.
// Story 5.1.1: defined here (nullptr). Story 5.2.1: set to new mu::MiniAudioBackend()
// and Initialize() called during game startup (Winmain.cpp). Story 5.2.2 will
// extend SFX delegation via this pointer.
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
            // CRITICAL-1 fix: uninit only the channels actually initialised by LoadSound().
            for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
            {
                ma_sound_uninit(&m_sounds[buf][ch]);
            }
            m_soundLoaded[buf] = false;
            m_loadedChannels[buf] = 0;
        }
        // Story 5.2.2 (Task 1.3): Clear per-slot OBJECT* tracking to prevent dangling
        // pointers if the backend is restarted. Safe even if slot was never loaded.
        m_soundObjects[buf] = nullptr;
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
        for (int ch = 0; ch < m_loadedChannels[bufIdx]; ++ch)
        {
            ma_sound_uninit(&m_sounds[bufIdx][ch]);
        }
        m_soundLoaded[bufIdx] = false;
        // HIGH-2 fix: clear stale object pointer on reload so Set3DSoundPosition()
        // cannot dereference a pointer that belonged to the previous load lifetime.
        m_soundObjects[bufIdx] = nullptr;
    }

    // Convert wchar_t filename to UTF-8 for miniaudio (which uses narrow char*)
    std::string utf8Path = mu_wchar_to_utf8(filename);

    // Story 5.2.2 (Task 5.1): Normalize path separators for Linux/macOS.
    // SFX file paths use Windows backslashes (e.g., L"Data\\Sound\\nBlackSmith.wav").
    // mu_wchar_to_utf8() preserves them; replace before passing to ma_sound_init_from_file().
    // On Windows, forward slashes work identically to backslashes for file paths.
    // Mirrors the PlayMusic() normalization pattern from Story 5.2.1.
    std::replace(utf8Path.begin(), utf8Path.end(), '\\', '/');

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
    // CRITICAL-1 fix: record how many slots were actually initialised so that
    // PlaySound/StopSound/AllStopSound never touch uninitialized ma_sound handles.
    m_loadedChannels[bufIdx] = numChannels;
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

    // Round-robin channel selection — clamp to loaded channel count (CRITICAL-1 fix).
    // m_loadedChannels[bufIdx] >= 1 is guaranteed by the m_soundLoaded guard above.
    const int ch = m_activeChannel[bufIdx];
    m_activeChannel[bufIdx] = (ch + 1) % m_loadedChannels[bufIdx];

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

    // Story 5.2.2 (Task 1.2): Store the object pointer for per-frame position updates
    // in Set3DSoundPosition(). Only track for 3D-enabled slots — mono/stereo SFX do not
    // need per-frame position updates. pObject may be nullptr (checked in Set3DSoundPosition).
    if (m_sound3DEnabled[bufIdx])
    {
        m_soundObjects[bufIdx] = pObject;
    }

    // Story 5.4.1: Apply stored SFX volume to newly-started effect
    ma_sound_set_volume(pSound, m_sfxVolume);

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

    // CRITICAL-1 / MEDIUM-2 fix: iterate only the channels actually initialised.
    for (int ch = 0; ch < m_loadedChannels[bufIdx]; ++ch)
    {
        ma_sound_stop(&m_sounds[bufIdx][ch]);
        if (resetPosition)
        {
            ma_sound_seek_to_pcm_frame(&m_sounds[bufIdx][ch], 0);
        }
    }

    // LOW-2 fix: clear stale OBJECT* after stopping so Set3DSoundPosition() cannot
    // dereference a pointer to an object that has since been deleted (e.g. NPC despawn).
    // The slot remains loaded — m_soundLoaded stays true — so the next PlaySound()
    // call can reuse the slot without reloading. Per-frame position updates simply
    // skip this slot (m_soundObjects[bufIdx] == nullptr guard) until PlaySound() fires.
    m_soundObjects[bufIdx] = nullptr;
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
            // CRITICAL-1 fix: iterate only the channels actually initialised.
            for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
            {
                ma_sound_stop(&m_sounds[buf][ch]);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Set3DSoundPosition — update spatial positions for all 3D-enabled sounds
// Mirrors DSplaysound.cpp Set3DSoundPosition() behaviour.
// Called each frame from the game loop for sounds that have an attached OBJECT.
//
// Story 5.2.2 (Tasks 2.1/2.2): Full implementation using per-slot m_soundObjects[].
// PlaySound() stores the OBJECT* in m_soundObjects[bufIdx] for 3D-enabled slots.
// This loop reads that pointer each frame and calls ma_sound_set_position() for
// every actively-playing channel in the slot.
// Guard order: !m_soundLoaded → skip; !m_sound3DEnabled → skip;
//              m_soundObjects[buf] == nullptr → skip (safe — no object attached).
// OBJECT::Position is vec3_t (float[3]): [0]=X, [1]=Y, [2]=Z.
// Lifetime assumption: game engine guarantees the OBJECT outlives its attached sound
// during normal play (same contract as DirectSoundManager::attachedObjects).
// ---------------------------------------------------------------------------
void MiniAudioBackend::Set3DSoundPosition()
{
    if (!m_initialized)
    {
        return;
    }

    for (int buf = 0; buf < static_cast<int>(MAX_BUFFER); ++buf)
    {
        if (!m_soundLoaded[buf] || !m_sound3DEnabled[buf] || m_soundObjects[buf] == nullptr)
        {
            continue;
        }

        for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
        {
            if (!ma_sound_is_playing(&m_sounds[buf][ch]))
            {
                continue;
            }

            // Update 3D position from the stored OBJECT* set in PlaySound().
            // m_soundObjects[buf] is non-null (checked above); Position is vec3_t float[3].
            ma_sound_set_position(&m_sounds[buf][ch], m_soundObjects[buf]->Position[0],
                                  m_soundObjects[buf]->Position[1], m_soundObjects[buf]->Position[2]);
        }
    }
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
    for (int ch = 0; ch < m_loadedChannels[bufIdx]; ++ch)
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
//
// Story 5.2.1: Path normalization — MUSIC_* constants in mu_enum.h use Windows
// backslash separators (e.g., "data\\music\\Pub.mp3"). On Linux/macOS miniaudio
// requires forward slashes. Replace '\\' with '/' via std::replace before passing
// to ma_sound_init_from_file(). No new Win32 calls — pure std::string manipulation.
// m_currentMusicName stores the normalized path for the same-track guard.
// ---------------------------------------------------------------------------
void MiniAudioBackend::PlayMusic(const char* name, BOOL enforce)
{
    if (!m_initialized || name == nullptr)
    {
        return;
    }

    // Normalize path separators: MUSIC_* constants use Windows-style backslashes
    std::string normalizedName(name);
    std::replace(normalizedName.begin(), normalizedName.end(), '\\', '/');

    // If not enforced and same track is already playing, do nothing
    if (!enforce && !m_currentMusicName.empty() && m_currentMusicName == normalizedName)
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

    // Start new stream — MA_SOUND_FLAG_STREAM keeps it off the decode buffer.
    // MA_SOUND_FLAG_ASYNC is intentionally NOT used: async init causes ma_sound_is_playing()
    // to return MA_FALSE for several frames after ma_sound_start(), making IsEndMusic()
    // return true spuriously. BGM is triggered on scene load (infrequent), so synchronous
    // stream init cost is acceptable. (HIGH-1 fix, code-review-finalize 2026-03-19)
    // NOTE: Synchronous init may stall the game loop 10–100ms on HDDs or network shares
    // (common in server-hosted MU setups) due to file open + ID3 header parse + decoder init
    // on the calling thread. Acceptable for BGM at scene transitions on SSD; known limitation
    // for HDD/network installs. Deferred to 5.2.x if a non-blocking init path is needed.
    ma_result result = ma_sound_init_from_file(&m_engine, normalizedName.c_str(), MA_SOUND_FLAG_STREAM, nullptr,
                                               nullptr, &m_musicSound);

    if (result != MA_SUCCESS)
    {
        g_ErrorReport.Write(L"AUDIO: MiniAudioBackend::PlayMusic -- failed to init stream '%hs' (%d)\r\n", name,
                            static_cast<int>(result));
        return;
    }

    ma_sound_set_looping(&m_musicSound, MA_TRUE);
    // Story 5.4.1: Apply stored BGM volume to new track before starting
    ma_sound_set_volume(&m_musicSound, m_bgmVolume);
    ma_sound_start(&m_musicSound);

    m_musicLoaded = true;
    m_currentMusicName = normalizedName;
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
//
// KNOWN LIMITATION (LOW-NEW-2): After StopMusic(nullptr, FALSE) (soft pause),
// m_musicLoaded=true and m_currentMusicName is unchanged. The next PlayMusic() call
// with the same track name hits the same-track guard and returns early — the music
// stays paused with no way to resume from the current position. IPlatformAudio has
// no ResumeMusic() method. In practice, all game call sites use enforce=TRUE (hard
// stop via the StopMusic() free function in Winmain.cpp), so this dead-end path is
// not reachable from current gameplay. A ResumeMusic() API may be added in 5.2.2.
// ---------------------------------------------------------------------------
void MiniAudioBackend::StopMusic(const char* name, BOOL enforce)
{
    if (!m_initialized || !m_musicLoaded)
    {
        return;
    }

    // If not enforced, only stop if the name matches the current track.
    // nullptr name means "stop current track regardless of name" (unconditional soft stop).
    // Normalize the name for comparison — m_currentMusicName stores normalized paths.
    if (!enforce && name != nullptr && !m_currentMusicName.empty())
    {
        std::string normalizedName(name);
        std::replace(normalizedName.begin(), normalizedName.end(), '\\', '/');
        if (m_currentMusicName != normalizedName)
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

// ---------------------------------------------------------------------------
// SetBGMVolume — set BGM volume (linear 0.0 to 1.0), clamped. Story 5.4.1.
// Stores the value unconditionally; applies to the active music sound only
// when the engine is initialized and music is loaded.
// ---------------------------------------------------------------------------
void MiniAudioBackend::SetBGMVolume(float level)
{
    m_bgmVolume = std::clamp(level, 0.0f, 1.0f);
    if (m_initialized && m_musicLoaded)
    {
        ma_sound_set_volume(&m_musicSound, m_bgmVolume);
    }
}

// ---------------------------------------------------------------------------
// SetSFXVolume — set SFX volume (linear 0.0 to 1.0), clamped. Story 5.4.1.
// Applies per-slot ma_sound_set_volume to all loaded SFX channels.
// Does NOT use ma_engine_set_volume (would also affect BGM).
// ---------------------------------------------------------------------------
void MiniAudioBackend::SetSFXVolume(float level)
{
    m_sfxVolume = std::clamp(level, 0.0f, 1.0f);
    if (!m_initialized)
    {
        return;
    }
    // Update all currently-loaded SFX slots
    for (int buf = 0; buf < static_cast<int>(MAX_BUFFER); ++buf)
    {
        if (!m_soundLoaded[buf])
        {
            continue;
        }
        for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
        {
            ma_sound_set_volume(&m_sounds[buf][ch], m_sfxVolume);
        }
    }
}

// ---------------------------------------------------------------------------
// GetBGMVolume / GetSFXVolume — return stored volume level. Story 5.4.1.
// ---------------------------------------------------------------------------
float MiniAudioBackend::GetBGMVolume() const
{
    return m_bgmVolume;
}

float MiniAudioBackend::GetSFXVolume() const
{
    return m_sfxVolume;
}

// ---------------------------------------------------------------------------
// GetAudioDeviceNames — enumerate playback devices for error reporting.
// [Story 7-6-7: AC-6] Isolated here to keep miniaudio.h out of ErrorReport.cpp.
// ---------------------------------------------------------------------------
std::vector<std::string> GetAudioDeviceNames()
{
    ma_context ctx;
    if (ma_context_init(nullptr, 0, nullptr, &ctx) != MA_SUCCESS)
    {
        return {};
    }

    ma_device_info* pPlayback = nullptr;
    ma_uint32 playbackCount = 0;
    if (ma_context_get_devices(&ctx, &pPlayback, &playbackCount, nullptr, nullptr) != MA_SUCCESS)
    {
        ma_context_uninit(&ctx);
        return {};
    }

    std::vector<std::string> names;
    for (ma_uint32 i = 0; i < playbackCount; ++i)
    {
        names.emplace_back(pPlayback[i].name);
    }

    ma_context_uninit(&ctx);
    return names;
}

} // namespace mu
