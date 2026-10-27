#include "stdafx.h"
#include "Engine/Object/Movement/Momentum.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cmath>

namespace Engine::Characters
{
namespace
{
// A walking character turning back by more than this slows to a stop over about this long, the way it was going, and
// speeds up from a stop (from rest too) over about this long. Short, as the server times the walk tile by tile.
constexpr float MomentumTurnBackDegrees = 90.f;
constexpr float MomentumStopSeconds = 0.12f;
constexpr float MomentumStartSeconds = 0.15f;
// A gap this long between steps (in world milliseconds) means the character stood still in between.
constexpr double MomentumRestMilliseconds = 100.0;
} // namespace

float StepMomentum(CHARACTER* c, float& heading)
{
    if (!EnhancedMovementEnabled())
    {
        // Full speed the way it's headed, as before; the next step with it on starts from rest.
        c->Motion.MomentumPace = 1.f;
        c->Motion.MomentumHeading = NAN;
        c->Motion.MomentumAt = -1.0;
        return 1.f;
    }
    const float seconds = FrameSeconds();
    if (c->Motion.MomentumAt < 0.0 || WorldTime - c->Motion.MomentumAt > MomentumRestMilliseconds ||
        std::isnan(c->Motion.MomentumHeading))
    {
        // From rest: the new way, from no speed.
        c->Motion.MomentumHeading = heading;
        c->Motion.MomentumPace = 0.f;
    }
    c->Motion.MomentumAt = WorldTime;
    const bool turningBack = std::fabs(FarAngle(c->Motion.MomentumHeading, heading, false)) > MomentumTurnBackDegrees;
    if (turningBack && c->Motion.MomentumPace > 0.f)
    {
        // Still going the old way, slowing down.
        c->Motion.MomentumPace = std::max(0.f, c->Motion.MomentumPace - seconds / MomentumStopSeconds);
    }
    else
    {
        c->Motion.MomentumHeading = heading;
        c->Motion.MomentumPace = std::min(1.f, c->Motion.MomentumPace + seconds / MomentumStartSeconds);
    }
    heading = c->Motion.MomentumHeading;
    // Eased: it slows into the stop and out of the start.
    return c->Motion.MomentumPace * c->Motion.MomentumPace * (3.f - 2.f * c->Motion.MomentumPace);
}
} // namespace Engine::Characters
