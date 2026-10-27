#include "stdafx.h"
#include "Engine/Object/Movement/TurnLag.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"

#include <cmath>

namespace Engine::Characters
{
namespace
{
// The wings and the ponytail trail behind a turn on a spring: they swing back this many times a second, damped like
// this (a little overshoot as they settle), never more than this many degrees behind.
constexpr float TurnLagFrequency = 2.5f;
constexpr float TurnLagDamping = 0.6f;
constexpr float TurnLagMaxDegrees = 12.f;
} // namespace

void UpdateTurnLag(CHARACTER* c)
{
    const OBJECT* o = &c->Object;
    if (o->Type != MODEL_PLAYER)
    {
        c->Motion.TurnLag = 0.f;
        c->Motion.TurnLagSpeed = 0.f;
        c->Motion.TurnLagFacing = NAN;
        return;
    }
    const float facing = o->Angle[2];
    if (std::isnan(c->Motion.TurnLagFacing))
    {
        c->Motion.TurnLagFacing = facing;
    }
    // The body turned by this much since the last frame: the wings don't, at first. Off, no new lag; the last settles.
    const float turned = EnhancedMovementEnabled() ? FarAngle(c->Motion.TurnLagFacing, facing, false) : 0.f;
    c->Motion.TurnLagFacing = facing;
    if (turned == 0.f && c->Motion.TurnLag == 0.f && c->Motion.TurnLagSpeed == 0.f)
    {
        return;
    }
    c->Motion.TurnLag -= turned;
    if (std::fabs(c->Motion.TurnLag) > TurnLagMaxDegrees)
    {
        c->Motion.TurnLag = std::copysign(TurnLagMaxDegrees, c->Motion.TurnLag);
        if ((c->Motion.TurnLagSpeed > 0.f) == (c->Motion.TurnLag > 0.f))
        {
            c->Motion.TurnLagSpeed = 0.f;
        }
    }
    const float seconds = FrameSeconds();
    StepSpring(c->Motion.TurnLag, c->Motion.TurnLagSpeed, seconds, TurnLagFrequency, TurnLagDamping);
    // Settled: nothing to do until the next turn.
    if (std::fabs(c->Motion.TurnLag) < 0.01f && std::fabs(c->Motion.TurnLagSpeed) < 0.1f)
    {
        c->Motion.TurnLag = 0.f;
        c->Motion.TurnLagSpeed = 0.f;
    }
}
} // namespace Engine::Characters
