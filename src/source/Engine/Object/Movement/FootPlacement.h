#pragma once

// Foot placement: a player's feet stand on the ground under them.
namespace Engine::Characters
{
// Puts a player's feet on the ground under them (a slope, a step), in its bones as just built in model space: the body
// lowered as far as the lower foot needs, each leg bent to reach (two bones, the knee kept pointing as it did).
// Players on foot only.
void PlaceFeet(const CHARACTER* c, float (*bones)[3][4]);
} // namespace Engine::Characters
