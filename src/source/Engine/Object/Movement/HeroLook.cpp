#include "stdafx.h"
#include "Engine/Object/Movement/HeroLook.h"

#include "Core/Utilities/Random.h"
#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Engine/Object/Movement/Greetings.h"
#include "Engine/Object/Movement/LookAt.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>

namespace Engine::Characters
{
namespace
{
// The hero's head looks at an enemy in sight (UpdateLookAt) rather than the cursor. Near an NPC, it follows the cursor
// only while the mouse is in use: moved for this long without a pause longer than this (a twitch doesn't count), then
// for this long after it stops. It turns at most HeroLookDegrees to the side, as far as the cursor turns it.
constexpr float CursorLookMoveSeconds = 0.5f;
constexpr float CursorLookPauseSeconds = 0.15f;
constexpr float CursorLookHoldSeconds = 2.f;
int s_lastMouseX = 0;
int s_lastMouseY = 0;
float s_mouseMovingSeconds = 0.f;
float s_mouseStillSeconds = 0.f;
float s_cursorLookSeconds = 0.f;
// With nothing to look at and the mouse resting this long, the hero glances around now and then: this far to a side,
// for this long, then back for this long until the next glance (each picked at random between the two).
constexpr float IdleGlanceAfterSeconds = 4.f;
constexpr float IdleGlanceDegrees[2] = {25.f, 55.f};
constexpr float IdleGlanceHoldSeconds[2] = {1.f, 1.8f};
constexpr float IdleGlanceGapSeconds[2] = {2.5f, 5.f};
float s_glanceYaw = 0.f;
float s_glanceLeftSeconds = 0.f;
float s_nextGlanceSeconds = 0.f;

// One frame of the idle glances: the head's yaw while it glances (0 to 360), or a negative number when it doesn't.
float UpdateIdleGlance(float seconds)
{
    if (s_mouseStillSeconds < IdleGlanceAfterSeconds)
    {
        // The mouse moved: any glance ends at once, and the first after the next rest comes soon.
        s_glanceLeftSeconds = 0.f;
        s_nextGlanceSeconds = Random::RangeFloat(0.5f, 1.5f);
        return -1.f;
    }
    if (s_glanceLeftSeconds > 0.f)
    {
        s_glanceLeftSeconds -= seconds;
        return s_glanceYaw;
    }
    s_nextGlanceSeconds -= seconds;
    if (s_nextGlanceSeconds <= 0.f)
    {
        const float side = Random::RangeFloat(0.f, 1.f) < 0.5f ? -1.f : 1.f;
        const float yaw = side * Random::RangeFloat(IdleGlanceDegrees[0], IdleGlanceDegrees[1]);
        s_glanceYaw = yaw < 0.f ? yaw + 360.f : yaw;
        s_glanceLeftSeconds = Random::RangeFloat(IdleGlanceHoldSeconds[0], IdleGlanceHoldSeconds[1]);
        s_nextGlanceSeconds =
            s_glanceLeftSeconds + Random::RangeFloat(IdleGlanceGapSeconds[0], IdleGlanceGapSeconds[1]);
        return s_glanceYaw;
    }
    return -1.f;
}

// One frame of the mouse: whether the head follows the cursor now.
bool UpdateCursorLook(float seconds)
{
    if (MouseX != s_lastMouseX || MouseY != s_lastMouseY)
    {
        s_lastMouseX = MouseX;
        s_lastMouseY = MouseY;
        s_mouseMovingSeconds += seconds;
        s_mouseStillSeconds = 0.f;
    }
    else
    {
        s_mouseStillSeconds += seconds;
        if (s_mouseStillSeconds > CursorLookPauseSeconds)
        {
            s_mouseMovingSeconds = 0.f;
        }
    }
    s_cursorLookSeconds = s_mouseMovingSeconds >= CursorLookMoveSeconds ? CursorLookHoldSeconds
                                                                        : std::max(0.f, s_cursorLookSeconds - seconds);
    return s_cursorLookSeconds > 0.f;
}
} // namespace

bool UpdateHeroLook(CHARACTER* hero, bool mayLook)
{
    OBJECT* o = &hero->Object;
    const float frameSeconds = FrameSeconds();
    const bool followCursor = UpdateCursorLook(frameSeconds);
    // Without Enhanced Movement the head follows only the cursor, as before.
    const bool enhancedMovement = EnhancedMovementEnabled();
    if (!enhancedMovement)
    {
        hero->Motion.LookTarget = -1;
    }
    const CHARACTER* lookAt = enhancedMovement && mayLook ? UpdateLookAt(hero) : nullptr;
    const float glanceYaw = enhancedMovement && mayLook && lookAt == nullptr ? UpdateIdleGlance(frameSeconds) : -1.f;
    // Someone who just nodded at the hero gets its head for a moment (it nods back, UpdateNod); an enemy in sight keeps
    // the head whatever the mouse does; an NPC gives it to the cursor while the mouse is in use.
    if (const CHARACTER* nodder = mayLook ? HeroNodAttention() : nullptr)
    {
        LookTowards(o, nodder, HeroLookDegrees);
        return true;
    }
    if (lookAt != nullptr && (lookAt->Object.Kind == KIND_MONSTER || !followCursor))
    {
        LookTowards(o, lookAt, HeroLookDegrees);
        return true;
    }
    if (glanceYaw >= 0.f)
    {
        o->HeadTargetAngle[0] = glanceYaw;
        o->HeadTargetAngle[1] = 0.f;
        return true;
    }
    return false;
}

bool HeroIsGlancing()
{
    return s_glanceLeftSeconds > 0.f && s_mouseStillSeconds >= IdleGlanceAfterSeconds;
}
} // namespace Engine::Characters
