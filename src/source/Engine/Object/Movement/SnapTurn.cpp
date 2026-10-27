#include "stdafx.h"
#include "Engine/Object/Movement/SnapTurn.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"

#include <algorithm>
#include <cmath>

namespace Engine::Characters
{
namespace
{
// A facing turned this far in one frame is a sudden turn (walking and standing turns ease and never go this fast); it's
// drawn catching up as fast as this, eased, which turns half around in about this long.
constexpr float SnapTurnDegrees = 30.f;
constexpr float SnapTurnSeconds = 0.15f;
// A half turn (180 degrees) eased in and out in SnapTurnSeconds: speeding up for half the time and slowing down for
// the other half, d = a t^2 / 4.
constexpr float SnapTurnAcceleration = 4.f * 180.f / (SnapTurnSeconds * SnapTurnSeconds);
constexpr float SnapTurnMaxSpeed = SnapTurnAcceleration * SnapTurnSeconds / 2.f;
} // namespace

void UpdateSnapTurn(CHARACTER* c)
{
    const OBJECT* o = &c->Object;
    const float facing = o->Angle[2];
    if (std::isnan(c->Motion.SnapTurnFacing))
    {
        c->Motion.SnapTurnFacing = facing;
    }
    const float turned = FarAngle(c->Motion.SnapTurnFacing, facing, false);
    c->Motion.SnapTurnFacing = facing;
    // Off, no sudden turn is drawn eased; one under way still finishes.
    if (EnhancedMovementEnabled() && std::fabs(turned) >= SnapTurnDegrees)
    {
        // The drawn body stays where it was, and catches up from there.
        c->Motion.SnapTurnOffset = std::clamp(c->Motion.SnapTurnOffset - turned, -180.f, 180.f);
    }
    if (c->Motion.SnapTurnOffset == 0.f && c->Motion.SnapTurnSpeed == 0.f)
    {
        return;
    }
    const float seconds = FrameSeconds();
    // StepEasedTurn on the offset, toward 0: it lands there with no speed left. Kept as -180 to 180.
    const float stepped = StepEasedTurn(c->Motion.SnapTurnOffset, 0.f, c->Motion.SnapTurnSpeed, seconds,
                                        SnapTurnMaxSpeed, SnapTurnAcceleration);
    c->Motion.SnapTurnOffset = stepped > 180.f ? stepped - 360.f : stepped;
}
} // namespace Engine::Characters
