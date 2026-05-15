#include "stdafx.h"
#include "Audio/AudioPlayer.h"

#include "Data/GameConfig/GameConfig.h"
#include "Platform/Windows/Winmain.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <cstring>
#include <string>

extern bool Destroy;

namespace
{
    MIX_Mixer*  g_Mixer        = nullptr;
    MIX_Track*  g_MusicTrack   = nullptr;
    MIX_Audio*  g_CurrentAudio = nullptr;
    std::string g_CurrentPath;

    bool IsReady()
    {
        return g_Mixer != nullptr && g_MusicTrack != nullptr;
    }

    void ReleaseCurrentAudio()
    {
        if (g_CurrentAudio)
        {
            MIX_DestroyAudio(g_CurrentAudio);
            g_CurrentAudio = nullptr;
        }
        g_CurrentPath.clear();
    }

    bool LoadAndStartMusic(const char* path)
    {
        MIX_Audio* audio = MIX_LoadAudio(g_Mixer, path, /*predecode=*/false);
        if (!audio)
            return false;

        if (!MIX_SetTrackAudio(g_MusicTrack, audio))
        {
            MIX_DestroyAudio(audio);
            return false;
        }

        ReleaseCurrentAudio();
        g_CurrentAudio = audio;
        g_CurrentPath  = path;

        // Match the legacy wzAudioPlay(name, 1) semantics: play once.
        // On failure, clear g_CurrentPath so a subsequent PlayMp3 with the
        // same name doesn't early-return on the "already playing" check
        // and is allowed to retry.
        if (!MIX_PlayTrack(g_MusicTrack, 0))
        {
            g_CurrentPath.clear();
            return false;
        }
        return true;
    }
}

namespace AudioPlayer
{
    int ClampVolume(int level)
    {
        if (level < MinVolumeLevel || level > MaxVolumeLevel)
            return DefaultVolumeLevel;
        return level;
    }

    void Initialize()
    {
        if (IsReady())
            return;

        if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
            return;

        if (!MIX_Init())
        {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return;
        }

        g_Mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (!g_Mixer)
        {
            MIX_Quit();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return;
        }

        g_MusicTrack = MIX_CreateTrack(g_Mixer);
        if (!g_MusicTrack)
        {
            MIX_DestroyMixer(g_Mixer);
            g_Mixer = nullptr;
            MIX_Quit();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return;
        }

        SetMusicVolume(GameConfig::GetInstance().GetMusicVolume());
    }

    void Shutdown()
    {
        // Order matters: the track holds a pointer to g_CurrentAudio, so the
        // track must be destroyed before its audio (otherwise we leave the
        // track briefly pointed at freed memory).  The mixer goes last since
        // both the track and the audio were created against it.
        if (g_MusicTrack)
        {
            MIX_DestroyTrack(g_MusicTrack);
            g_MusicTrack = nullptr;
        }

        ReleaseCurrentAudio();

        if (g_Mixer)
        {
            MIX_DestroyMixer(g_Mixer);
            g_Mixer = nullptr;
        }

        MIX_Quit();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    void SetMusicVolume(int level)
    {
        if (!IsReady())
            return;
        const float gain = static_cast<float>(ClampVolume(level)) / static_cast<float>(MaxVolumeLevel);
        MIX_SetTrackGain(g_MusicTrack, gain);
    }
}

// ---------------------------------------------------------------------------
// Legacy free-function wrappers (declared in Winmain.h).
// Preserved verbatim semantics — gating on m_MusicOnOff and Destroy, plus
// the "no-op if same track is already loaded" behavior.
// ---------------------------------------------------------------------------

void StopMusic()
{
    if (!m_MusicOnOff) return;
    if (!IsReady()) return;
    MIX_StopTrack(g_MusicTrack, 0);
}

void StopMp3(const char* Name, BOOL bEnforce)
{
    if (!m_MusicOnOff && !bEnforce) return;
    if (!IsReady()) return;
    if (g_CurrentPath.empty()) return;

    if (std::strcmp(Name, g_CurrentPath.c_str()) == 0)
    {
        MIX_StopTrack(g_MusicTrack, 0);
        // Unbind before releasing: the track keeps a pointer to the audio
        // even when stopped, and ReleaseCurrentAudio destroys it.
        MIX_SetTrackAudio(g_MusicTrack, nullptr);
        ReleaseCurrentAudio();
    }
}

void PlayMp3(const char* Name, BOOL bEnforce)
{
    if (Destroy) return;
    if (!m_MusicOnOff && !bEnforce) return;
    if (!IsReady()) return;

    if (g_CurrentPath == Name)
        return;

    // Stop the prior track before swapping (matches the original
    // wzAudio WZAOPT_STOPBEFOREPLAY behavior).
    MIX_StopTrack(g_MusicTrack, 0);

    LoadAndStartMusic(Name);
}

bool IsEndMp3()
{
    if (!IsReady()) return false;
    if (!g_CurrentAudio) return false;
    return !MIX_TrackPlaying(g_MusicTrack);
}

int GetMp3PlayPosition()
{
    if (!IsReady() || !g_CurrentAudio) return 0;

    const Sint64 duration = MIX_GetAudioDuration(g_CurrentAudio);
    if (duration <= 0) return 0;

    const Sint64 position = MIX_GetTrackPlaybackPosition(g_MusicTrack);
    return static_cast<int>((position * 100) / duration);
}
