#pragma once

#ifdef _EDITOR

#include "Audio/SoundSource.h"

#include <array>
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

// A start of a sound whose source has a position: the overlay flashes the
// source (Q-SM9). A sound the game's own code plays starts at its place
// (Q-SM11): `byCode`, with the place's kind and number and no name.
struct PlacedStart
{
    int number = 0;
    SoundKind kind = SoundKind::Other;
    std::string name;
    int sourceNumber = -1;
    std::array<float, 3> position{};
    // The mixer's clock (Now).
    double time = 0.0;
    bool byCode = false;
};

// A start with no place in the world (Q-SM15): the interface, ambience, a
// music track, some of the server's messages. The overlay logs them.
struct PlacelessStart
{
    // A music track's row number (IsMusicNumber) for a track.
    int number = -1;
    SoundKind kind = SoundKind::Other;
    // The music's file; "" for a sound.
    std::string music;
    bool looped = false;
    // The mixer's clock (Now).
    double time = 0.0;
};

Control& KindControl(SoundKind kind);
Control& SoundControl(int number);

// The gain of a sound of the kind, and of the music: the type's controls
// and the controls of the track that plays now.
float GainFor(int number, SoundKind kind);
float MusicGain();

// A music track's row (Q-SM17): each track heard this session gets a number
// below -1 of its own, kept until the game ends (also after Clear all), so
// its controls work like a sound's.
bool IsMusicNumber(int number);
// The track's file; "" for any other number.
std::string MusicOf(int number);
// Whether the track is the music asked for last and still plays.
bool IsMusicPlaying(int number);

// PlayMp3 is about to ask the backend for a track: it is the music now, and
// the backend gets its gain when it changes (its row may be muted).
void OnMusicPlay(const std::string& music);

// A sound is about to play (PlayBuffer, PlayBufferOnFreeChannel): its type is
// the living source's, else the innermost living place's, else the place of
// the object it plays at (`objectPlace`, or nullptr), else map ambience for a
// loop, else everything else (Q-SM13); before the loop's rule, a code sound
// takes the type Data/Audio/SoundFiles.json stores (Q-SE2). Lists it, gives
// the backend its gain before it starts and returns the gain. Whether a
// control is on solo is what Apply found: a play scans no controls.
float OnPlay(int number, bool looped, const SoundPlace* objectPlace = nullptr);

// The play OnPlay announced is done: counts a start at `now` when the backend
// started the sound, and keeps where it started: at the position of what
// OnPlay found (a source, a place of the game's own code), else as a start
// without a place.
void OnPlayed(int number, double now);

// Every type the sound was heard as this session, in the types' order; also
// after Clear all.
std::vector<SoundKind> KindsHeard(int number);

// A music track started (PlayMp3): a start without a place.
// The backend started a track: its row counts a start, the log a line.
void OnMusicStarted(const std::string& music, double now);

// The latest starts without a place, newest first; a few dozen are kept.
std::vector<PlacelessStart> PlacelessStarts();

// The sounds heard since the list was last cleared, in the order they were
// first heard, so that a row stays where it is.
std::vector<HeardSound> Heard();

// The starts with a position in the last `seconds` before `now`, oldest
// first. A few seconds of them are kept.
std::vector<PlacedStart> RecentStarts(double now, double seconds);

// The sounds of the game's own code to mark (Q-SM11): the last start of each,
// while it plays or for a second after it started, by sound number.
std::vector<PlacedStart> CodeSounds(double now);

// The window's "Clear all": empties the list, starts the counts again and puts
// every sound's own controls back (unmuted, no solo, full volume); the types'
// controls stay.
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
// cleared, for each type it was heard as (its copies of that type follow),
// and of the music; after a control changes. Finds once whether a control
// is on solo, for these gains and the plays until the next Apply.
void Apply();

// Puts every control back to normal, and applies it.
void ResetAll();

// Seconds on a steady clock.
double Now();

// Forgets the controls and the sounds heard (tests).
void Clear();
} // namespace Audio::Mixer

#endif // _EDITOR
