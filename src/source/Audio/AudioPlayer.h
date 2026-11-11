#pragma once

// Music playback facade backed by the shared miniaudio platform backend.
// The legacy free functions (PlayMp3, StopMp3, StopMusic, IsEndMp3,
// GetMp3PlayPosition) are declared in Winmain.h to preserve existing call sites;
// their definitions live in AudioPlayer.cpp.

namespace AudioPlayer
{
    // Music volume levels are stored as a 0..MaxVolumeLevel scale in
    // the project's config; miniaudio expects a 0.0..1.0 gain.
    constexpr int MinVolumeLevel = 0;
    constexpr int MaxVolumeLevel = 10;
    constexpr int DefaultVolumeLevel = 5;

    // Apply the saved music volume after the platform audio backend starts.
    void Initialize();

    // Stop and release the active music stream before backend shutdown.
    void Shutdown();

    // Apply a music volume on the project's 0..10 scale.  Out-of-range
    // values fall back to DefaultVolumeLevel.
    void SetMusicVolume(int level);

    int ClampVolume(int level);

}
