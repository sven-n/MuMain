#pragma once

#include "Core/Globals/_enum.h"

class OBJECT;

// The lights a stat-down debuff (eDeBuff_AttackDown from Weakness, eDeBuff_DefenseDown from Innovation) shows on its
// character: one BITMAP_SHINY + 6 and one BITMAP_PIN_LIGHT, subtype 1 for attack down and 2 for defense down.
namespace Render::Effects::StatDownDebuffLights
{
// The light colors, also used by the Weakness and Innovation casts that apply the debuffs.
inline constexpr float AttackDownColor[3] = {1.4f, 0.2f, 0.2f};
inline constexpr float DefenseDownColor[3] = {0.25f, 1.0f, 0.7f};

// Creates a stat-down debuff's lights on `o` unless they are there already. Other debuffs are ignored.
void Create(OBJECT* o, eBuffState debuff);
// Deletes a stat-down debuff's lights from `o`, if there. Other debuffs are ignored.
void Delete(OBJECT* o, eBuffState debuff);
// Creates the lights of each stat-down debuff `o` has and deletes those of the ones it doesn't.
void Update(OBJECT* o);
} // namespace Render::Effects::StatDownDebuffLights
