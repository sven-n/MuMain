#pragma once

class OBJECT;

namespace Render::Effects
{

// The aura's joints and their scale, wherever the aura is created.
inline constexpr int DefenseAuraJointCount = 5;
inline constexpr float DefenseAuraScale = 20.0f;

// Whether the object's buffs give it the defense aura: where the aura is created and where it is kept alive. The
// swirling MODEL_SPEARSKILL aura (joint subtypes 4 and 9) belongs to Greater Defense and to the Elf Soldier buff;
// Greater Damage shows only its red weapon sparkles. MoveJoint ends the aura the frame after the last of them leaves.
[[nodiscard]] bool HasDefenseAura(OBJECT* o);

} // namespace Render::Effects
