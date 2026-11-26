#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"

namespace Engine::Object
{
// A teleporting object fades out (TELEPORT_BEGIN), stays hidden (TELEPORT)
// and fades back in (TELEPORT_END). Below this alpha it still counts as
// mid-teleport, so the fade back in also blocks a new move or skill.
constexpr float TeleportVisibleAlpha = 0.7f;

// True from the moment an object starts to vanish until it is visible again
// after landing. Pass OBJECT::Teleport and OBJECT::Alpha. The hero may not
// walk, attack or cast in this time: the server still has the old position.
inline bool IsMidTeleport(int teleportState, float alpha)
{
    return teleportState == TELEPORT_BEGIN || teleportState == TELEPORT || alpha < TeleportVisibleAlpha;
}
} // namespace Engine::Object
