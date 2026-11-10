#pragma once

// The hero's own head: what it looks at before the cursor (someone who nodded at it, an enemy in sight, an NPC while
// the mouse rests), and idle glances around when the mouse rests for a few seconds.
namespace Engine::Characters
{
// One frame of the hero's head (MoveHero), before the cursor's: `mayLook` is whether it may follow the cursor now
// (not stunned, not auto-attacking). Whether it turned the head; if not, the cursor (or nothing) does.
bool UpdateHeroLook(CHARACTER* hero, bool mayLook);
// Whether the hero is glancing around idly now (its head on no one, and not on the cursor).
bool HeroIsGlancing();
} // namespace Engine::Characters
