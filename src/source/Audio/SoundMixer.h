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

// A row of the list of sounds heard: a sound as it last played, and how often
// it started since the list was cleared.
struct HeardSound
{
    int number = 0;
    SoundKind kind = SoundKind::Other;
    std::string name;
    int sourceNumber = -1;
    // Whether it last played as a loop.
    bool looped = false;
    // The starts since the list was cleared: a call while it plays and is not
    // restarted (a loop, a map object's sound each frame) is none.
    int plays = 0;
    // The mixer's clock (Now) at its last start; -1 for none since the clear.
    double lastStarted = -1.0;
};

Control& KindControl(SoundKind kind);
Control& SoundControl(int number);

// The gain of a sound of the kind, and of the music.
float GainFor(int number, SoundKind kind);
float MusicGain();

// A sound is about to play (PlayBuffer, PlayBufferOnFreeChannel): its kind is
// the living source's, else map ambience for a loop, else everything else.
// Lists it, gives the backend its gain before it starts and returns the gain.
float OnPlay(int number, bool looped);

// The play OnPlay announced is done: counts a start at `now` when the backend
// started the sound.
void OnPlayed(int number, double now);

// The sounds heard since the list was last cleared, in the order they were
// first heard, so that a row stays where it is.
std::vector<HeardSound> Heard();

// The window's "Clear all": empties the list and starts the counts again. The
// sounds whose controls are changed stay listed, with no plays, so that what
// is muted can be found again.
void ClearHeard();

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

// Gives the backend the gains of every sound heard this session, listed or
// cleared, and of the music; after a control changes.
void Apply();

// Puts every control back to normal, and applies it.
void ResetAll();

// Seconds on a steady clock.
double Now();

// Forgets the controls and the sounds heard (tests).
void Clear();
} // namespace Audio::Mixer

#endif // _EDITOR
