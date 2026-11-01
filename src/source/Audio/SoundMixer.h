#pragma once

#ifdef _EDITOR

#include "Audio/SoundSource.h"

#include <string>
#include <vector>

// The editor's Sound Mixer (docs/superpowers/specs/2026-11-01-sound-mixer-design.md):
// mute, solo and a volume for each kind of sound and each sound heard, for the
// session. A sound plays at gain 0 when it or its kind is muted, or when
// something is solo and neither it nor its kind is; else at its kind's volume
// times its own. Changes reach playing sounds at once (Q-SM4).
namespace Audio::Mixer
{
struct Control
{
    bool muted = false;
    bool solo = false;
    // 0 to 1.
    float volume = 1.0f;

    // Muted, solo or below full volume.
    bool IsChanged() const;
};

// The sounds of the last ten seconds are listed (Q-SM2).
constexpr double HeardSeconds = 10.0;

// A sound heard this session, as it last played.
struct HeardSound
{
    int number = 0;
    SoundKind kind = SoundKind::Other;
    std::string name;
    int sourceNumber = -1;
    // The mixer's clock (Now) when it last started.
    double lastHeard = 0.0;
    int plays = 0;
};

Control& KindControl(SoundKind kind);
Control& SoundControl(int number);

// The gain of a sound of the kind, and of the music.
float GainFor(int number, SoundKind kind);
float MusicGain();

// A sound starts (PlayBuffer, PlayBufferOnFreeChannel): its kind is the
// living source's, else map ambience for a loop, else everything else. Notes
// it as heard at `now`, gives the backend its gain and returns the gain.
float OnPlay(int number, bool looped, double now);

// The sounds heard in the last HeardSeconds before `now` and those whose
// controls are changed, newest first.
std::vector<HeardSound> Heard(double now);

struct Changes
{
    int muted = 0;
    int solo = 0;
    // Below full volume, not muted.
    int lowered = 0;

    bool Any() const
    {
        return muted + solo + lowered > 0;
    }
};

// What is changed, for the menu bar's reminder (Q-SM5).
Changes CountChanges();

// Gives the backend the gains of every sound heard this session and of the
// music; after a control changes.
void Apply();

// Puts every control back to normal, and applies it.
void ResetAll();

// Seconds on a steady clock.
double Now();

// Forgets the controls and the sounds heard (tests).
void Clear();
} // namespace Audio::Mixer

#endif // _EDITOR
