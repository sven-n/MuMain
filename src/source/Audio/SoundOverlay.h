#pragma once

#ifdef _EDITOR

#include "Audio/SoundSource.h"

#include <array>
#include <string>
#include <vector>

class CameraState;

// The Sound Mixer's overlay (docs/superpowers/specs/2026-11-01-sound-mixer-design.md,
// Q-SM9, Q-SM10): while it is on, the game notes the sound sources it draws
// (in the update, for what the last frame drew) with the ranges the data gives
// them, and the editor draws them over the game once a frame.
namespace Audio::Overlay
{
// A sound source drawn this frame.
struct DrawnSource
{
    SoundKind kind = SoundKind::MapObject;
    // A monster's sound set; "" for a map object.
    std::string name;
    // The map object's type or the monster's number.
    int sourceNumber = -1;
    // The map object's sound; -1 for a monster.
    int soundNumber = -1;
    std::array<float, 3> position{};
    // A map object plays at full volume within `inner` and at its quietest
    // from `outer`; a monster is heard within `inner` (`outer` 0).
    float inner = 0.0f;
    float outer = 0.0f;
};

// The window's "Show in world", for the session.
bool IsOn();
void SetOn(bool on);

// A type's Show box (Q-SM14), for the session: whether the overlay draws the
// type's sources, markers, flashes and log lines. Every type at first.
bool IsShown(SoundKind kind);
void SetShown(SoundKind kind, bool shown);

// Notes a source while the overlay is on; nothing while it is off.
void NoteDrawn(const DrawnSource& source);

// The sources noted since the last call: the overlay's frame.
std::vector<DrawnSource> TakeDrawn();

// Keeps the world camera of this frame while the overlay is on
// (Scenes/MainScene.cpp, where the mouse ray is cast): later in the frame
// other cameras draw the item previews.
void KeepCamera(const CameraState& camera);

// The camera kept; nullptr before one was.
const CameraState* KeptCamera();
} // namespace Audio::Overlay

#endif // _EDITOR
