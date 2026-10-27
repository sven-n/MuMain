#include "stdafx.h"
#include "Engine/Object/Movement/FlightFeel.h"

#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cmath>

namespace Engine::Characters
{
namespace
{
// How a winged player flies (the fly animations, moving or hovering, outside the safe zones): it banks into a turn, up
// to this many degrees at the turn's top speed, and drifts slowly up and down (above its usual height) by this much,
// this many times a second, fading in and out over about this long.
constexpr float FlightMaxBankDegrees = 8.f;
constexpr float FlightHoverHeight = 30.f;
constexpr float FlightHoverCyclesPerSecond = 0.15f;
constexpr float FlightEaseSeconds = 0.25f;
// The turning speed that banks fully: a walk's turns are short and rarely reach WalkTurnDegreesPerSecond.
constexpr float FlightFullBankTurnSpeed = 300.f;
// The bank pivots this high above the feet (about the waist), as a flying body banks about its middle.
constexpr float FlightBankPivotHeight = 65.f;

// A mount leans into a turn up to this many degrees, and its rider this share of that, to stay balanced on it.
constexpr float MountMaxLeanDegrees = 10.f;
constexpr float MountRiderLeanShare = 0.5f;
} // namespace

bool IsRiding(const CHARACTER* c)
{
    // In a safe zone a mount is put away and its rider walks.
    return c->Object.Type == MODEL_PLAYER && !c->SafeZone &&
           (c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT ||
            c->Helper.Type == MODEL_DARK_HORSE_ITEM || c->Helper.Type == MODEL_HORN_OF_FENRIR);
}

bool IsFlying(const OBJECT* o)
{
    // Flying on, or hovering in place.
    return o->Type == MODEL_PLAYER &&
           (o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW ||
            o->CurrentAction == PLAYER_STOP_FLY || o->CurrentAction == PLAYER_STOP_FLY_CROSSBOW);
}

float MountLeanFromRider(float riderLean)
{
    return riderLean / MountRiderLeanShare;
}

void UpdateFlightFeel(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    // Off, the bank and the drift fade out as when landing.
    const bool enhancedMovement = EnhancedMovementEnabled();
    const bool flying = enhancedMovement && IsFlying(o);
    const bool riding = enhancedMovement && !flying && IsRiding(c);
    if (!flying && !riding && c->Motion.FlightBank == 0.f && c->Motion.FlightHover == 0.f &&
        c->Motion.FlightHoverApplied == 0.f && c->Motion.FlightPivotShift[0] == 0.f &&
        c->Motion.FlightPivotShift[1] == 0.f)
    {
        return;
    }
    const float seconds = FrameSeconds();
    // Eased toward its target over about FlightEaseSeconds, the same at any frame rate.
    const float ease = EaseStep(seconds, FlightEaseSeconds);

    // Bank: rolled into the turn by how fast it turns (TurnSpeed), the wings with it (they hang from the root).
    // Riding, the rider leans a share of the mount's lean (MoveMount gives the mount all of it).
    // A mount leans only on the move: standing, it straightens up (TurnSpeed can be left over from the last step).
    const float maxBank = flying                  ? FlightMaxBankDegrees
                          : riding && c->Movement ? MountMaxLeanDegrees * MountRiderLeanShare
                                                  : 0.f;
    const float wantedBank = std::clamp(c->TurnSpeed / FlightFullBankTurnSpeed, -1.f, 1.f) * maxBank;
    const float bank = std::fabs(wantedBank - c->Motion.FlightBank) < 0.01f && wantedBank == 0.f
                           ? 0.f
                           : c->Motion.FlightBank + (wantedBank - c->Motion.FlightBank) * ease;
    o->Angle[1] += bank - c->Motion.FlightBank;
    c->Motion.FlightBank = bank;
    // The model rolls about its feet; moving it back by how far the roll carried the waist makes it pivot there.
    // (A mount and its rider lean about where they stand, as a horse banks at the hooves.)
    float shift[2] = {0.f, 0.f};
    if (bank != 0.f && flying)
    {
        const vec3_t waist = {0.f, 0.f, FlightBankPivotHeight};
        const vec3_t rolled = {o->Angle[0], o->Angle[1], o->Angle[2]};
        const vec3_t upright = {o->Angle[0], o->Angle[1] - bank, o->Angle[2]};
        float matrix[3][4];
        vec3_t withBank, withoutBank;
        AngleMatrix(rolled, matrix);
        VectorRotate(waist, matrix, withBank);
        AngleMatrix(upright, matrix);
        VectorRotate(waist, matrix, withoutBank);
        shift[0] = withoutBank[0] - withBank[0];
        shift[1] = withoutBank[1] - withBank[1];
    }
    o->Position[0] += shift[0] - c->Motion.FlightPivotShift[0];
    o->Position[1] += shift[1] - c->Motion.FlightPivotShift[1];
    c->Motion.FlightPivotShift[0] = shift[0];
    c->Motion.FlightPivotShift[1] = shift[1];

    // Hover: faded in while it flies.
    const float wantedHover = flying ? 1.f : 0.f;
    c->Motion.FlightHover = std::fabs(wantedHover - c->Motion.FlightHover) < 0.01f
                                ? wantedHover
                                : c->Motion.FlightHover + (wantedHover - c->Motion.FlightHover) * ease;
    // Up and back down once a cycle, each character on its own beat (its key), easing in and out at the top and the
    // bottom (smootherstep, which lingers there longer than a sine), and only upward from its usual height, which near
    // the ground is just above it.
    const float phase = static_cast<float>(c->Key) * 0.37f;
    const float cycle = static_cast<float>(WorldTime) * 0.001f * FlightHoverCyclesPerSecond + phase;
    const float rise = 1.f - std::fabs(2.f * (cycle - std::floor(cycle)) - 1.f); // 0 to 1 and back, linearly
    const float eased = rise * rise * rise * (rise * (rise * 6.f - 15.f) + 10.f);
    const float height = c->Motion.FlightHover * FlightHoverHeight * eased;
    o->Position[2] += height - c->Motion.FlightHoverApplied;
    c->Motion.FlightHoverApplied = height;
}
} // namespace Engine::Characters
