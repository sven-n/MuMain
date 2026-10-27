#pragma once

// Greetings, from the hero's point of view: a player or NPC that has looked at the hero for a moment greets it once
// (a nod, a player's Hello, an NPC's idle variation), and the hero turns its head to it and nods back.
namespace Engine::Characters
{
// RemoveNod takes last frame's nod off the head before the head eases to its target; UpdateNod moves the nods and
// greetings on and puts this frame's back (each frame, everyone).
void RemoveNod(CHARACTER* c);
void UpdateNod(CHARACTER* c);
// Who the hero is turning its head to, to nod back (MoveHero's head), or nullptr.
const CHARACTER* HeroNodAttention();
} // namespace Engine::Characters
