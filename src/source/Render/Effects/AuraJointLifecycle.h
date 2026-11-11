#pragma once

namespace Render::Effects
{

[[nodiscard]] constexpr bool ShouldKeepAuraJointAlive(bool hasAttack, bool hasDefense, bool hasHelpNpc)
{
    return hasAttack || hasDefense || hasHelpNpc;
}

} // namespace Render::Effects
