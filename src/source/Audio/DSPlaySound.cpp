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

#include "DSPlaySound.h"
#include "Core/Platform/IPlatformAudio.h"
#ifdef _EDITOR
#include "Audio/EditorSoundMute.h"
#endif

#include <cwctype>
#include <string>
#include <unordered_map>

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

HRESULT PlayBuffer(ESound bufferId, OBJECT* object, BOOL looped)
{
#ifdef _EDITOR
    // Nothing started; no caller reads the result.
    if (Audio::EditorMute::IsMuted() && looped == FALSE)
        return S_FALSE;
#endif
    if (g_platformAudio != nullptr)
    {
        return g_platformAudio->PlaySound(bufferId, object, looped != FALSE) ? S_OK : E_FAIL;
    }
    return E_FAIL;
}

HRESULT PlayBufferAtLevel(ESound bufferId, float level)
{
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
