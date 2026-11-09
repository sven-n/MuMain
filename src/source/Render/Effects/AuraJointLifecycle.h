#pragma once

class OBJECT;

namespace Render::Effects
{

// The swirling MODEL_SPEARSKILL aura (joint subtypes 4 and 9) belongs to Greater Defense and to the
// Elf Soldier buff; Greater Damage shows only its red weapon sparkles.
[[nodiscard]] constexpr bool ShouldKeepAuraJointAlive(bool hasDefense, bool hasHelpNpc)
{
    return hasDefense || hasHelpNpc;
}

// Whether the object's buffs give it the defense aura: where the aura is created and where it is kept alive.
[[nodiscard]] bool HasDefenseAura(OBJECT* o);

} // namespace Render::Effects
