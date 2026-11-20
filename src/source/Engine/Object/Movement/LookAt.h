#pragma once

// Look-at: heads (and shoulders) turn toward someone near: what a character attacks, an enemy in sight, an NPC,
// another player passing by, or the hero passing an NPC.
namespace Engine::Characters
{
// A head looks at someone within this many degrees of where its character faces, and turns at most that far.
constexpr float LookViewDegrees = 90.f;
// The hero's head turns at most this far to a side, toward someone (an enemy, an NPC, someone who nodded at it) as
// toward the cursor; it answers a nod only from someone within this far of where it faces.
constexpr float HeroLookDegrees = 60.f;
// A head pitches toward what it looks at from about the eyes, this high above the feet, by this many degrees per unit
// of height between the eyes and the other's feet.
constexpr float LookEyeHeight = 50.f;
constexpr float LookPitchPerUnit = 0.2f;

// Who the character's head looks at, if anyone (each frame): a player looks at what it attacks or the nearest enemy in
// sight in a combat zone, else the nearest NPC, else another player passing by; a monster at its target; an NPC at the
// hero passing by.
const CHARACTER* UpdateLookAt(CHARACTER* c);
// Turns the head toward `target`, at most `maxDegrees` to either side.
void LookTowards(OBJECT* o, const CHARACTER* target, float maxDegrees);
} // namespace Engine::Characters
