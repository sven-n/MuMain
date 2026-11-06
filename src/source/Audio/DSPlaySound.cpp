///////////////////////////////////////////////////////////////////////////////
// Audio free-function layer
///////////////////////////////////////////////////////////////////////////////
//
// Story 7-9-4: Kill DirectSound — Miniaudio-Only Audio Layer
//
// All audio free functions delegate exclusively to g_platformAudio (IPlatformAudio).
// The legacy DirectSoundManager class and all #ifdef _WIN32 fallback paths have been
// deleted. miniaudio (via MiniAudioBackend) is the sole audio backend on all platforms.
//
// Original file: DirectX Sound sample (Microsoft Corp, 1999) — rewritten for
// cross-platform audio via IPlatformAudio abstraction (Stories 5-2-1, 5-2-2, 7-9-4).

#include "stdafx.h"

#include "Core/Utilities/DrawSpawnCheck.h"
#include "DSPlaySound.h"
#include "Audio/SoundIds.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#ifdef _EDITOR
#include "Audio/EditorSoundMute.h"
#include "Audio/SoundLoads.h"
#include "Audio/SoundMixer.h"
#include "Audio/SoundPlaces.h"
#endif

#include <cwctype>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace
{
// What LoadWaveFile loaded into a sound.
struct LoadedSound
{
    std::wstring file;
    int channels = 0;
    bool enable3D = false;

    bool operator==(const LoadedSound&) const = default;
};

std::unordered_map<int, LoadedSound>& LoadedSounds()
{
    static std::unordered_map<int, LoadedSound> sounds;
    return sounds;
}

LoadedSound Describe(const wchar_t* filename, int channels, bool enable3D)
{
    std::wstring file = filename != nullptr ? filename : L"";
    for (wchar_t& c : file)
    {
        c = static_cast<wchar_t>(std::towlower(c));
    }
    return {std::move(file), channels, enable3D};
}
} // namespace

void LoadWaveFile(ESound bufferId, const wchar_t* filename, int maxChannel, bool enable3D)
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->LoadSound(bufferId, filename, maxChannel, enable3D);
        LoadedSounds()[static_cast<int>(bufferId)] = Describe(filename, maxChannel, enable3D);
    }
}

void LoadGameSound(ESound bufferId)
{
    const Data::Sounds::SoundFile* sound = g_SoundFileDatabase.Find(static_cast<int>(bufferId));
    if (sound == nullptr)
    {
        static std::unordered_set<int> reported;
        if (reported.insert(static_cast<int>(bufferId)).second)
        {
            MU_LOG_ERROR(mu::log::Get("audio"), "{} ({}) is not in Data/Audio/SoundFiles.json: it is silent",
                         ::Audio::SoundIds::NameOf(bufferId), static_cast<int>(bufferId));
        }
        return;
    }
    // "Data/Sound/a.wav" -> "Data\Sound\a.wav", as the code named it.
    const std::wstring file = Data::Paths::ToLoaderPath(sound->file);
    LoadWaveFile(bufferId, file.c_str(), sound->channels, sound->spatial);
#ifdef _EDITOR
    Audio::SoundLoads::NoteLoad(static_cast<int>(bufferId));
#endif
}

bool IsSoundLoaded(ESound bufferId, const wchar_t* filename, int maxChannel, bool enable3D)
{
    const auto sound = LoadedSounds().find(static_cast<int>(bufferId));
    return sound != LoadedSounds().end() && sound->second == Describe(filename, maxChannel, enable3D);
}

void ForgetLoadedSounds()
{
    LoadedSounds().clear();
}

HRESULT ReleaseBuffer(int bufferId)
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->ReleaseSound(static_cast<ESound>(bufferId));
    }
    LoadedSounds().erase(bufferId);
    return S_OK;
}

std::wstring GetLoadedSoundFile(ESound bufferId)
{
    const auto sound = LoadedSounds().find(static_cast<int>(bufferId));
    return sound != LoadedSounds().end() ? sound->second.file : std::wstring();
}

#ifdef _EDITOR
namespace
{
// The Sound Mixer lists a loaded sound and gives it its gain before it plays,
// and counts it after when it started. `object`: where it plays, or nullptr;
// its place counts when no source or place of the code lives (Q-SM13).
void NoteForMixer(ESound bufferId, bool looped, const OBJECT* object = nullptr)
{
    if (LoadedSounds().contains(static_cast<int>(bufferId)))
    {
        const std::optional<Audio::SoundPlace> place = Audio::PlaceOfObject(object);
        Audio::Mixer::OnPlay(static_cast<int>(bufferId), looped, place ? &*place : nullptr);
    }
}

void CountForMixer(ESound bufferId)
{
    Audio::Mixer::OnPlayed(static_cast<int>(bufferId), Audio::Mixer::Now());
}
} // namespace
#endif

HRESULT PlayBuffer(ESound bufferId, OBJECT* object, BOOL looped)
{
    Diagnostics::DrawSpawnCheck::Note("sound");
#ifdef _EDITOR
    // Nothing started; no caller reads the result.
    if (Audio::EditorMute::IsMuted() && looped == FALSE)
        return S_FALSE;
    // A sound nothing loaded is silent: the Sound Editor lists it.
    if (!LoadedSounds().contains(static_cast<int>(bufferId)))
    {
        Audio::SoundLoads::NotePlayedUnloaded(static_cast<int>(bufferId));
    }
    NoteForMixer(bufferId, looped != FALSE, object);
#endif
    HRESULT result = E_FAIL;
    if (g_platformAudio != nullptr)
    {
        result = g_platformAudio->PlaySound(bufferId, object, looped != FALSE) ? S_OK : E_FAIL;
    }
#ifdef _EDITOR
    CountForMixer(bufferId);
#endif
    return result;
}

HRESULT PlayBufferAtLevel(ESound bufferId, float level)
{
    Diagnostics::DrawSpawnCheck::Note("sound");
#ifdef _EDITOR
    if (Audio::EditorMute::IsMuted())
        return S_FALSE;
#endif
    if (g_platformAudio != nullptr)
    {
        return g_platformAudio->PlaySoundAtLevel(bufferId, level) ? S_OK : E_FAIL;
    }
    return E_FAIL;
}

HRESULT PlayBufferOnFreeChannel(ESound bufferId, float level)
{
    Diagnostics::DrawSpawnCheck::Note("sound");
#ifdef _EDITOR
    if (Audio::EditorMute::IsMuted())
        return S_FALSE;
    // A sound nothing loaded is silent: the Sound Editor lists it.
    if (!LoadedSounds().contains(static_cast<int>(bufferId)))
    {
        Audio::SoundLoads::NotePlayedUnloaded(static_cast<int>(bufferId));
    }
    NoteForMixer(bufferId, false);
#endif
    HRESULT result = E_FAIL;
    if (g_platformAudio != nullptr)
    {
        result = g_platformAudio->PlaySoundOnFreeChannel(bufferId, level) ? S_OK : E_FAIL;
    }
#ifdef _EDITOR
    CountForMixer(bufferId);
#endif
    return result;
}

VOID StopBuffer(ESound bufferId, BOOL resetPosition)
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->StopSound(bufferId, resetPosition != FALSE);
    }
}

void AllStopSound()
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->AllStopSound();
    }
}

void SetVolume(int bufferId, long volume)
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->SetVolume(static_cast<ESound>(bufferId), volume);
    }
}

void SetMasterVolume(long volume)
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->SetMasterVolume(volume);
    }
}

void Set3DSoundPosition()
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->Set3DSoundPosition();
    }
}
