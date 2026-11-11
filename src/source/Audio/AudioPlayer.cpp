#include "stdafx.h"
#include "Audio/AudioPlayer.h"
#include "Core/Utilities/DrawSpawnCheck.h"

#include "App/Platform/Windows/Winmain.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Data/GameConfig/GameConfig.h"
#ifdef _EDITOR
#include "Audio/SoundMixer.h"
#endif

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
    Diagnostics::DrawSpawnCheck::Note("music");
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
    Diagnostics::DrawSpawnCheck::Note("music");
    if (Destroy || (!m_MusicOnOff && !enforce))
    {
        return;
    }
    if (g_platformAudio != nullptr)
    {
#ifdef _EDITOR
        // The track's row in the Sound Mixer may be muted (Q-SM17).
        Audio::Mixer::OnMusicPlay(name);
        const std::uint32_t before = g_platformAudio->GetMusicStarts();
#endif
        g_platformAudio->PlayMusic(name, enforce != FALSE);
#ifdef _EDITOR
        // The Sound Mixer's log of starts without a place (Q-SM15).
        if (g_platformAudio->GetMusicStarts() != before)
        {
            Audio::Mixer::OnMusicStarted(name, Audio::Mixer::Now());
        }
#endif
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
