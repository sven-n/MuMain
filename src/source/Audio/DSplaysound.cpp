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
#include "IPlatformAudio.h"

void LoadWaveFile(ESound bufferId, const wchar_t* filename, int maxChannel, bool enable3D)
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->LoadSound(bufferId, filename, maxChannel, enable3D);
    }
}

HRESULT ReleaseBuffer(int bufferId)
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->ReleaseSound(static_cast<ESound>(bufferId));
    }
    return S_OK;
}

HRESULT PlayBuffer(ESound bufferId, OBJECT* object, BOOL looped)
{
    if (g_platformAudio != nullptr)
    {
        return g_platformAudio->PlaySound(bufferId, object, looped != FALSE) ? S_OK : E_FAIL;
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
