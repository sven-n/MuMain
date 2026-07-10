#include "stdafx.h"
#include "Audio/AudioPlayer.h"

#include "App/Platform/Windows/Winmain.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Data/GameConfig/GameConfig.h"

extern bool Destroy;

namespace AudioPlayer
{
    int ClampVolume(int level)
    {
        if (level < MinVolumeLevel || level > MaxVolumeLevel)
        {
            return DefaultVolumeLevel;
        }
        return level;
    }

    void Initialize()
    {
        SetMusicVolume(GameConfig::GetInstance().GetMusicVolume());
    }

    void Shutdown()
    {
        if (g_platformAudio != nullptr)
        {
            g_platformAudio->StopMusic(nullptr, true);
        }
    }

    void SetMusicVolume(int level)
    {
        if (g_platformAudio == nullptr)
        {
            return;
        }

        const float gain = static_cast<float>(ClampVolume(level)) / static_cast<float>(MaxVolumeLevel);
        g_platformAudio->SetBGMVolume(gain);
    }
}

void StopMusic()
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->StopMusic(nullptr, true);
    }
}

void StopMp3(const char* name, BOOL enforce)
{
    if (!m_MusicOnOff && !enforce)
    {
        return;
    }
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->StopMusic(name, enforce != FALSE);
    }
}

void PlayMp3(const char* name, BOOL enforce)
{
    if (Destroy || (!m_MusicOnOff && !enforce))
    {
        return;
    }
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->PlayMusic(name, enforce != FALSE);
    }
}

bool IsEndMp3()
{
    return g_platformAudio == nullptr || g_platformAudio->IsEndMusic();
}

int GetMp3PlayPosition()
{
    return (g_platformAudio != nullptr) ? g_platformAudio->GetMusicPosition() : 0;
}
