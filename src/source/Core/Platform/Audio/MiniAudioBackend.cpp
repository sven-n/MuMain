#include "stdafx.h"

// MuAudio Abstraction Layer
// Implementation of MiniAudioBackend — miniaudio-based platform audio backend.
// See IPlatformAudio.h for the interface contract.

#include "MiniAudioBackend.h"
#include "Core/Platform/PathResolve.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Platform/WinCompat.h"
#include "Core/Text/Utf8Path.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <system_error>
#include <utility>

// S_OK, S_FALSE are now provided by PlatformTypes.h (via PCH)
#ifndef E_INVALIDARG
#define E_INVALIDARG 0x80070057L
#endif

namespace
{
// Sound effects are pre-decoded and always played in 2D. The original client hard-coded
// g_Enable3DSound = false, so nothing was ever spatialized. Placing sounds at absolute
// world positions while the listener stays at the origin makes them inaudible.
constexpr ma_uint32 kSoundEffectFlags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;

std::string AudioPathToUtf8(const wchar_t* text)
{
#ifdef _WIN32
    if (text == nullptr)
    {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1)
    {
        return {};
    }

    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), size, nullptr, nullptr);
    result.pop_back();
    return result;
#else
    return mu_wchar_to_utf8(text);
#endif
}

// Asset paths (SFX filenames, MUSIC_* constants) use Windows-style backslashes.
void NormalizeSeparators(std::string& path)
{
    std::replace(path.begin(), path.end(), '\\', '/');
}

// Converts a sound effect path to UTF-8 and resolves it for the current platform.
// SFX paths use Windows backslashes (e.g. L"Data\\Sound\\nBlackSmith.wav"); forward
// slashes work on Windows too, and other platforms also resolve the file name case.
std::string ResolveSoundEffectPath(const wchar_t* filename)
{
    std::string utf8Path = AudioPathToUtf8(filename);
    NormalizeSeparators(utf8Path);
#ifndef _WIN32
    utf8Path = MuResolvePath(utf8Path.c_str());
#endif
    return utf8Path;
}

// Same-track guard shared by PlayMusic()/StopMusic(). m_currentMusicName holds
// the separators-normalized track name, NOT the filesystem-resolved path: the
// guard runs every frame for many tracks from ManageBackgroundMusic(), so it
// compares against the raw name with the same normalization applied on the fly,
// without allocating or touching the filesystem.
bool IsSameMusicTrack(const std::string& identity, const char* name)
{
    size_t i = 0;
    for (; name[i] != '\0'; ++i)
    {
        const char c = (name[i] == '\\') ? '/' : name[i];
        if (i >= identity.size() || identity[i] != c)
        {
            return false;
        }
    }
    return i == identity.size();
}
} // namespace

// ---------------------------------------------------------------------------
// Global backend pointer — nullptr until the game initialises audio, then set to
// new mu::MiniAudioBackend() and Initialize() called during game startup (MuMain.cpp).
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
        // Qualified call: dynamic dispatch does not apply in a destructor.
        MiniAudioBackend::Shutdown();
    }
}

bool MiniAudioBackend::HasSlot(int buffer) const
{
    return buffer >= 0 && buffer < GetSlotCount();
}

int MiniAudioBackend::GetSlotCount() const
{
    return static_cast<int>(m_soundLoaded.size());
}

// A number after the SOUND_* list gets its slot (and the ones before it) when
// its sound is loaded.
bool MiniAudioBackend::AddSlotsUpTo(int buffer)
{
    if (buffer < static_cast<int>(MAX_BUFFER) || buffer >= static_cast<int>(MAX_BUFFER) + MAX_FILE_SOUNDS)
    {
        return false;
    }
    const size_t count = static_cast<size_t>(buffer) + 1;
    m_sounds.resize(count);
    m_soundLoaded.resize(count);
    m_activeChannel.resize(count);
    m_loadedChannels.resize(count);
    m_mixGain.resize(count, 1.0f);
    m_mixGroup.resize(count);
    m_channelVolume.resize(count);
    m_channelLevel.resize(count);
    m_channelGain.resize(count);
    m_channelGroup.resize(count);
    m_channelMixed.resize(count);
    m_starts.resize(count);
    return true;
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
        mu::log::Get("audio")->error("AUDIO: MiniAudioBackend::Initialize -- ma_engine_init failed ({})",
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
    for (int buf = 0; buf < GetSlotCount(); ++buf)
    {
        if (m_soundLoaded[buf])
        {
            // Uninit only the channels actually initialised by LoadSound().
            for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
            {
                ma_sound_uninit(&m_sounds[buf][ch]);
            }
            m_soundLoaded[buf] = false;
            m_loadedChannels[buf] = 0;
        }
    }

    ReleaseMusicStream();
    m_failedMusicNames.clear();

    ma_engine_uninit(&m_engine);
    m_initialized = false;
}

// ---------------------------------------------------------------------------
// LoadSound — load a WAV/OGG sound file into polyphonic slots
// Each ESound slot gets MAX_CHANNEL (4) duplicate ma_sound instances for polyphony.
// Sound effects use MA_SOUND_FLAG_DECODE (pre-decode to PCM for low-latency).
// The enable3D argument is ignored on purpose: sound effects always play in 2D,
// as in the original client (see kSoundEffectFlags).
// ---------------------------------------------------------------------------
void MiniAudioBackend::LoadSound(ESound buffer, const wchar_t* filename, int channels, bool /*enable3D*/)
{
    if (!m_initialized)
    {
        return;
    }

    const int bufIdx = static_cast<int>(buffer);
    if (!HasSlot(bufIdx) && !AddSlotsUpTo(bufIdx))
    {
        return;
    }

    // Reset slot state unconditionally before the unload block so that a failed load of the same
    // slot (partial channels) leaves the slot in a consistent clean state rather than holding
    // stale values.
    m_activeChannel[bufIdx] = 0;

    // Unload previously loaded sound at this slot
    if (m_soundLoaded[bufIdx])
    {
        for (int ch = 0; ch < m_loadedChannels[bufIdx]; ++ch)
        {
            ma_sound_uninit(&m_sounds[bufIdx][ch]);
        }
        m_soundLoaded[bufIdx] = false;
    }

    const std::string utf8Path = ResolveSoundEffectPath(filename);

    // Clamp channels to the array dimension
    const int numChannels = (channels > 0 && channels <= MAX_CHANNEL) ? channels : MAX_CHANNEL;

    // Load each polyphonic slot
    for (int ch = 0; ch < numChannels; ++ch)
    {
        ma_result result = ma_sound_init_from_file(&m_engine, utf8Path.c_str(),
                                                   kSoundEffectFlags, // pre-decoded, 2D
                                                   nullptr, nullptr, &m_sounds[bufIdx][ch]);

        if (result != MA_SUCCESS)
        {
            mu::log::Get("audio")->error(
                "AUDIO: MiniAudioBackend::LoadSound -- ma_sound_init_from_file failed for '{}' "
                "channel {} ({})",
                AudioPathToUtf8(filename), ch, static_cast<int>(result));
            // Uninit any channels already loaded for this slot
            for (int prev = 0; prev < ch; ++prev)
            {
                ma_sound_uninit(&m_sounds[bufIdx][prev]);
            }
            return;
        }
    }

    m_soundLoaded[bufIdx] = true;
    m_activeChannel[bufIdx] = 0;
    // Record how many slots were actually initialised so that
    // PlaySound/StopSound/AllStopSound never touch uninitialized ma_sound handles.
    m_loadedChannels[bufIdx] = numChannels;
}

// ---------------------------------------------------------------------------
// PlaySound — start playback of a sound effect using round-robin channel selection
// Mirrors DSPlaySound.h PlayBuffer() signature exactly.
// Returns S_FALSE (not S_OK) when !m_initialized to signal a no-op to callers, matching
// !m_soundLoaded behaviour. pObject only marks the call as object-anchored for the dedupe
// below; sound effects play in 2D, so its position is not used. Configures all properties
// before ma_sound_start(), because the audio thread starts mixing immediately. Also stops
// the channel with ma_sound_stop()
// before ma_sound_seek_to_pcm_frame() to prevent seeking a currently-playing channel (round-robin
// reuse of looping SFX slots), avoiding a pop/click artifact from seeking mid-mix.
// ---------------------------------------------------------------------------
bool MiniAudioBackend::PlaySound(ESound buffer, const void* pObject, bool looped)
{
    return StartSound(buffer, pObject, looped, 1.0f, true);
}

bool MiniAudioBackend::PlaySoundAtLevel(ESound buffer, float level)
{
    return StartSound(buffer, nullptr, false, std::clamp(level, 0.0f, 1.0f), false);
}

bool MiniAudioBackend::PlaySoundOnFreeChannel(ESound buffer, float level)
{
    if (!m_initialized)
    {
        return false;
    }
    const int bufIdx = static_cast<int>(buffer);
    if (!HasSlot(bufIdx) || !m_soundLoaded[bufIdx])
    {
        return false;
    }

    // Round the channels as DirectSound did; a channel that still plays keeps
    // playing (DirectSound's Play on a playing buffer did nothing).
    const int ch = m_activeChannel[bufIdx];
    m_activeChannel[bufIdx] = (ch + 1) % m_loadedChannels[bufIdx];
    ma_sound* pSound = &m_sounds[bufIdx][ch];
    if (ma_sound_is_playing(pSound))
    {
        return true;
    }

    // The channel has ended or never played: from the start, once, at its
    // level of the effect volume.
    ma_sound_seek_to_pcm_frame(pSound, 0);
    ma_sound_set_looping(pSound, MA_FALSE);
    StartChannelVolume(bufIdx, ch, std::clamp(level, 0.0f, 1.0f), true);
    ma_sound_start(pSound);
    ++m_starts[bufIdx];
    return true;
}

void MiniAudioBackend::StartChannelVolume(int buffer, int channel, float level, bool mixed)
{
    m_channelLevel[buffer][channel] = level;
    m_channelGain[buffer][channel] = m_mixGain[buffer];
    m_channelGroup[buffer][channel] = m_mixGroup[buffer];
    SetChannelVolume(buffer, channel, mixed ? m_sfxVolume * level : level, mixed);
}

void MiniAudioBackend::SetChannelVolume(int buffer, int channel, float volume, bool mixed)
{
    m_channelVolume[buffer][channel] = volume;
    m_channelMixed[buffer][channel] = mixed;
    ma_sound_set_volume(&m_sounds[buffer][channel], mixed ? volume * m_channelGain[buffer][channel] : volume);
}

bool MiniAudioBackend::StartSound(ESound buffer, const void* pObject, bool looped, float level, bool mixed)
{
    if (!m_initialized)
    {
        return false;
    }

    const int bufIdx = static_cast<int>(buffer);
    if (!HasSlot(bufIdx))
    {
        return false;
    }

    if (!m_soundLoaded[bufIdx])
    {
        return false;
    }

    // Dedupe ambient/positional SFX: when a caller passes an object-anchored
    // pObject (the standard pattern for per-frame ambient triggers like
    // MapProcess::MoveObject -> BaseMap::PlayObjectSound -> PlayBuffer
    // (SOUND_X, pObject, false)), ignore the call if any channel is
    // already playing this sound. Without this guard, uncapped SDL3 GPU
    // frame rate produces 60-200 stop-seek-restart cycles per second on
    // each ambient SFX (canonical reproducer: Lorencia anvil sounding
    // "hundreds of times" instead of once per hammer cycle).
    //
    // Single-channel buffers also rely on DirectSound's already-playing
    // no-op behavior. Multi-channel event SFX continue to overlap through
    // their round-robin slots.
    if (m_loadedChannels[bufIdx] == 1 || pObject != nullptr || looped)
    {
        for (int existingCh = 0; existingCh < m_loadedChannels[bufIdx]; ++existingCh)
        {
            if (ma_sound_is_playing(&m_sounds[bufIdx][existingCh]))
            {
                return true;
            }
        }
    }

    // Round-robin channel selection — clamp to loaded channel count.
    // m_loadedChannels[bufIdx] >= 1 is guaranteed by the m_soundLoaded guard above.
    const int ch = m_activeChannel[bufIdx];
    m_activeChannel[bufIdx] = (ch + 1) % m_loadedChannels[bufIdx];

    ma_sound* pSound = &m_sounds[bufIdx][ch];

    // Stop the channel before seeking to avoid a seek-during-play race.
    // When all MAX_CHANNEL slots are busy (e.g. looping SFX), the oldest channel
    // is reused. Stopping first prevents a pop/click from seeking mid-mix.
    ma_sound_stop(pSound);

    // Seek to start so reused channels always play from the beginning.
    ma_sound_seek_to_pcm_frame(pSound, 0);

    // Configure all sound properties BEFORE starting — miniaudio audio thread
    // begins mixing immediately on ma_sound_start(), so any property set after
    // start() is subject to a data race on the first mix tick.
    ma_sound_set_looping(pSound, looped ? MA_TRUE : MA_FALSE);

    // Its level of the effect volume, or a preview's own level.
    StartChannelVolume(bufIdx, ch, level, mixed);

    ma_sound_start(pSound);
    if (mixed)
    {
        ++m_starts[bufIdx];
    }

    return true;
}

// ---------------------------------------------------------------------------
// StopSound — stop all channels for the given sound slot
// ---------------------------------------------------------------------------
void MiniAudioBackend::StopSound(ESound buffer, bool resetPosition)
{
    if (!m_initialized)
    {
        return;
    }

    const int bufIdx = static_cast<int>(buffer);
    if (!HasSlot(bufIdx))
    {
        return;
    }

    if (!m_soundLoaded[bufIdx])
    {
        return;
    }

    // Iterate only the channels actually initialised.
    for (int ch = 0; ch < m_loadedChannels[bufIdx]; ++ch)
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

    for (int buf = 0; buf < GetSlotCount(); ++buf)
    {
        if (m_soundLoaded[buf])
        {
            // Iterate only the channels actually initialised.
            for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
            {
                ma_sound_stop(&m_sounds[buf][ch]);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// ReleaseSound — unload a previously loaded sound from all its polyphonic slots.
// Called by game code (e.g., ZzzOpenData.cpp) when switching maps to free NPC/monster sounds.
// Added to IPlatformAudio to replace legacy DirectSoundManager::ReleaseBuffer().
// ---------------------------------------------------------------------------
void MiniAudioBackend::ReleaseSound(ESound buffer)
{
    const int buf = static_cast<int>(buffer);
    if (!HasSlot(buf) || !m_initialized)
    {
        return;
    }

    if (!m_soundLoaded[buf])
    {
        return;
    }

    for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
    {
        ma_sound_uninit(&m_sounds[buf][ch]);
    }

    m_soundLoaded[buf] = false;
    m_activeChannel[buf] = 0;
    m_loadedChannels[buf] = 0;
}

// ---------------------------------------------------------------------------
// Set3DSoundPosition — intentionally empty: sound effects always play in 2D,
// as in the original client, so there are no positions to update.
// ---------------------------------------------------------------------------
void MiniAudioBackend::Set3DSoundPosition()
{
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
    if (!HasSlot(bufIdx))
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
        SetChannelVolume(bufIdx, ch, linear, m_channelMixed[bufIdx][ch]);
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
// PlayMusic -- start streaming a music track.
// Per-frame scene requests do no file I/O after success or failure. Enforced playback retries
// failed tracks; stopping music does not clear failures because scenes also stop tracks each frame.
// ---------------------------------------------------------------------------
void MiniAudioBackend::PlayMusic(const char* name, bool enforce)
{
    if (!m_initialized || name == nullptr || name[0] == '\0')
    {
        return;
    }
    if (!enforce && IsSameMusicTrack(m_currentMusicName, name))
    {
        return;
    }

    ReleaseMusicStream();
    if (!enforce && HasFailedMusicTrack(name))
    {
        return;
    }
    std::string identity(name);
    NormalizeSeparators(identity);
    if (!StartMusicStream(identity))
    {
        if (!HasFailedMusicTrack(identity.c_str()))
        {
            m_failedMusicNames.push_back(std::move(identity));
        }
        return;
    }

    std::erase(m_failedMusicNames, identity);
    m_currentMusicName = std::move(identity);
    ++m_musicStarts;
}

bool MiniAudioBackend::HasFailedMusicTrack(const char* name) const
{
    return std::any_of(m_failedMusicNames.begin(), m_failedMusicNames.end(),
        [name](const std::string& track) { return IsSameMusicTrack(track, name); });
}

bool MiniAudioBackend::StartMusicStream(const std::string& identity)
{
#ifdef _WIN32
    const std::string& filePath = identity;
#else
    const std::string filePath = MuResolvePath(identity.c_str());
#endif
    std::error_code error;
    if (!std::filesystem::is_regular_file(Core::Text::PathFromUtf8(filePath), error))
    {
        mu::log::Get("audio")->error("AUDIO: MiniAudioBackend::PlayMusic -- unavailable music file '{}' ({})",
            filePath, error ? error.message() : "not a regular file");
        return false;
    }

    // Synchronous init keeps IsEndMusic() accurate while starting. Missing files never reach
    // the resource-manager job queue; decoder failures are remembered by PlayMusic(). The pinned
    // miniaudio is patched so a failed stream load cannot stall the next one
    // (cmake/patches/miniaudio-0.11.25-stream-load-signal-order.patch).
    ma_result result = ma_sound_init_from_file(&m_engine, filePath.c_str(), MA_SOUND_FLAG_STREAM,
        nullptr, nullptr, &m_musicSound);
    if (result != MA_SUCCESS)
    {
        mu::log::Get("audio")->error("AUDIO: MiniAudioBackend::PlayMusic -- failed to init stream '{}' ({})",
            filePath, static_cast<int>(result));
        return false;
    }

    ma_sound_set_looping(&m_musicSound, MA_TRUE);
    ma_sound_set_volume(&m_musicSound, m_bgmVolume * m_musicMixGain);
    result = ma_sound_start(&m_musicSound);
    if (result != MA_SUCCESS)
    {
        ma_sound_uninit(&m_musicSound);
        mu::log::Get("audio")->error("AUDIO: MiniAudioBackend::PlayMusic -- failed to start stream '{}' ({})",
            filePath, static_cast<int>(result));
        return false;
    }
    m_musicLoaded = true;
    return true;
}

void MiniAudioBackend::ReleaseMusicStream()
{
    if (m_musicLoaded)
    {
        ma_sound_stop(&m_musicSound);
        ma_sound_uninit(&m_musicSound);
        m_musicLoaded = false;
    }
    m_currentMusicName.clear();
}

// ---------------------------------------------------------------------------
// StopMusic — stop and release the current music stream
//
// Every stop releases the stream (file handle + decoder) and clears the current
// track name, like the old StopMp3() cleared Mp3FileName. Keeping the name after
// a stop would make the next PlayMusic() of the same track hit the same-track
// guard and stay silent (e.g. Lorencia -> Arena -> Lorencia).
//
// name semantics:
//   Non-null name + enforce=false: only stop if the named track is the current one.
//   nullptr       + enforce=false: stop the current track regardless of name.
//   name is ignored when enforce=true (always stops regardless).
// ---------------------------------------------------------------------------
void MiniAudioBackend::StopMusic(const char* name, bool enforce)
{
    if (!m_initialized || !m_musicLoaded)
    {
        return;
    }

    // If not enforced, only stop if the name matches the current track.
    // Same comparison as PlayMusic()'s guard — see IsSameMusicTrack().
    if (!enforce && name != nullptr && !m_currentMusicName.empty())
    {
        if (!IsSameMusicTrack(m_currentMusicName, name))
        {
            return;
        }
    }

    ReleaseMusicStream();
}

// ---------------------------------------------------------------------------
// IsEndMusic — returns true when no music is playing
// Mirrors wzAudioGetStreamOffsetRange() == 100 semantics from MuMain.cpp.
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
// Mirrors wzAudioGetStreamOffsetRange() return value from MuMain.cpp.
// Returns 0 when not initialized or no music loaded.
//
// Music is streamed with MA_SOUND_FLAG_STREAM and loaded synchronously, so the
// stream's length is known once PlayMusic() has started it. Cursor and length
// are read in seconds. A length of 0 (an empty file or a format that cannot
// report one) gives 0.
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

    // Get total track length in seconds
    float totalSeconds = 0.0f;
    ma_sound_get_length_in_seconds(&m_musicSound, &totalSeconds);

    if (totalSeconds <= 0.0f)
    {
        // No length to divide by. Callers checking for the end of the track
        // should use IsEndMusic() instead.
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
// Clamp dsVol to <= 0 before
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
// SetBGMVolume — set BGM volume (linear 0.0 to 1.0), clamped.
// Stores the value unconditionally; applies to the active music sound only
// when the engine is initialized and music is loaded.
// ---------------------------------------------------------------------------
void MiniAudioBackend::SetBGMVolume(float level)
{
    m_bgmVolume = std::clamp(level, 0.0f, 1.0f);
    if (m_initialized && m_musicLoaded)
    {
        ma_sound_set_volume(&m_musicSound, m_bgmVolume * m_musicMixGain);
    }
}

void MiniAudioBackend::SetMusicMixGain(float gain)
{
    m_musicMixGain = std::clamp(gain, 0.0f, 1.0f);
    if (m_initialized && m_musicLoaded)
    {
        ma_sound_set_volume(&m_musicSound, m_bgmVolume * m_musicMixGain);
    }
}

void MiniAudioBackend::SetSoundMixGain(ESound buffer, int group, float gain)
{
    const int bufIdx = static_cast<int>(buffer);
    if (!HasSlot(bufIdx))
    {
        return;
    }
    m_mixGain[bufIdx] = std::clamp(gain, 0.0f, 1.0f);
    m_mixGroup[bufIdx] = group;
    if (!m_initialized || !m_soundLoaded[bufIdx])
    {
        return;
    }
    // The group's playing copies follow at once (a muted loop keeps running
    // silently); copies another group started keep their gain.
    for (int ch = 0; ch < m_loadedChannels[bufIdx]; ++ch)
    {
        if (m_channelMixed[bufIdx][ch] && m_channelGroup[bufIdx][ch] == group)
        {
            m_channelGain[bufIdx][ch] = m_mixGain[bufIdx];
            ma_sound_set_volume(&m_sounds[bufIdx][ch], m_channelVolume[bufIdx][ch] * m_mixGain[bufIdx]);
        }
    }
}

// ---------------------------------------------------------------------------
// SetSFXVolume — set SFX volume (linear 0.0 to 1.0), clamped.
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
    // Each game copy that is loaded keeps its level of the new volume (a map
    // object copy its distance's); a preview keeps its own level.
    for (int buf = 0; buf < GetSlotCount(); ++buf)
    {
        if (!m_soundLoaded[buf])
        {
            continue;
        }
        for (int ch = 0; ch < m_loadedChannels[buf]; ++ch)
        {
            if (m_channelMixed[buf][ch])
            {
                SetChannelVolume(buf, ch, m_sfxVolume * m_channelLevel[buf][ch], true);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GetBGMVolume / GetSFXVolume — return stored volume level.
// ---------------------------------------------------------------------------
float MiniAudioBackend::GetBGMVolume() const
{
    return m_bgmVolume;
}

float MiniAudioBackend::GetSFXVolume() const
{
    return m_sfxVolume;
}

std::uint32_t MiniAudioBackend::GetSoundStarts(ESound buffer) const
{
    const int bufIdx = static_cast<int>(buffer);
    return HasSlot(bufIdx) ? m_starts[bufIdx] : 0;
}

std::uint32_t MiniAudioBackend::GetMusicStarts() const
{
    return m_musicStarts;
}

bool MiniAudioBackend::IsSoundPlaying(ESound buffer) const
{
    const int bufIdx = static_cast<int>(buffer);
    if (!m_initialized || !HasSlot(bufIdx) || !m_soundLoaded[bufIdx])
    {
        return false;
    }
    for (int ch = 0; ch < m_loadedChannels[bufIdx]; ++ch)
    {
        if (m_channelMixed[bufIdx][ch] && ma_sound_is_playing(&m_sounds[bufIdx][ch]))
        {
            return true;
        }
    }
    return false;
}

float MiniAudioBackend::GetSoundLength(ESound buffer) const
{
    const int bufIdx = static_cast<int>(buffer);
    if (!m_initialized || !HasSlot(bufIdx) || !m_soundLoaded[bufIdx])
    {
        return 0.0f;
    }
    float seconds = 0.0f;
    return ma_sound_get_length_in_seconds(&m_sounds[bufIdx][0], &seconds) == MA_SUCCESS ? seconds : 0.0f;
}

// ---------------------------------------------------------------------------
// GetAudioDeviceNames — enumerate playback devices for error reporting.
// Isolated here to keep miniaudio.h out of ErrorReport.cpp.
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
