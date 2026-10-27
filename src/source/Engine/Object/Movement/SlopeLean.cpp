#include "stdafx.h"
#include "Engine/Object/Movement/SlopeLean.h"

#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FlightFeel.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include <algorithm>
#include <cmath>

namespace Engine::Characters
{
namespace
{
// The slope is the terrain's rise between this far ahead of the character and this far behind it, along its facing.
// A mount follows it this much (a little more than the ground itself, so the game's gentle slopes show), up to this
// many degrees; a player walking leans this share of it into a climb, up to this many. Both ease in and out over
// about this long.
constexpr float SlopeProbeDistance = 60.f;
constexpr float SlopeMountShare = 1.5f;
constexpr float SlopeMountMaxDegrees = 25.f;
constexpr float SlopeWalkShare = 0.5f;
constexpr float SlopeWalkMaxDegrees = 10.f;
constexpr float SlopeEaseSeconds = 0.25f;
} // namespace

void UpdateSlopeLean(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    const bool riding = IsRiding(c);
    const bool walking = c->Movement && !IsFlying(o);
    float wanted = 0.f;
    // Off, a lean under way eases back upright.
    if (EnhancedMovementEnabled() && o->Type == MODEL_PLAYER && (riding || walking))
    {
        // Ahead is where it faces: a model faces -Y before its facing turns it.
        const float facing = o->Angle[2] * (Q_PI / 180.f);
        const float aheadX = std::sin(facing) * SlopeProbeDistance;
        const float aheadY = -std::cos(facing) * SlopeProbeDistance;
        const float rise = RequestTerrainHeight(o->Position[0] + aheadX, o->Position[1] + aheadY) -
                           RequestTerrainHeight(o->Position[0] - aheadX, o->Position[1] - aheadY);
        const float slope = std::atan2(rise, 2.f * SlopeProbeDistance) * (180.f / Q_PI);
        // A positive pitch tips the nose down: a mount climbing tips it up (follows the slope), a walker climbing
        // leans forward into it.
        wanted = riding ? std::clamp(-slope * SlopeMountShare, -SlopeMountMaxDegrees, SlopeMountMaxDegrees)
                        : std::clamp(slope * SlopeWalkShare, -SlopeWalkMaxDegrees, SlopeWalkMaxDegrees);
    }
    if (wanted == 0.f && c->Motion.SlopePitch == 0.f)
    {
        return;
    }
    const float seconds = FrameSeconds();
    const float ease = EaseStep(seconds, SlopeEaseSeconds);
    const float pitch = std::fabs(wanted - c->Motion.SlopePitch) < 0.01f
                            ? wanted
                            : c->Motion.SlopePitch + (wanted - c->Motion.SlopePitch) * ease;
    o->Angle[0] += pitch - c->Motion.SlopePitch;
    c->Motion.SlopePitch = pitch;
}
} // namespace Engine::Characters
