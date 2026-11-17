// The angle math of the engine, which is used by the AI and the directions (cinematics).
// It's declared in ZzzAI.h. It's in its own file without dependencies on the game state, so that it can be tested.

#include "stdafx.h"
#include "Engine/AI/ZzzAI.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
constexpr float FULL_ROTATION_DEGREES = 360.f;
constexpr float HALF_ROTATION_DEGREES = 180.f;
constexpr float RAD_TO_DEG = 180.f / static_cast<float>(Q_PI);

float NormalizeAngleDegrees(float angle)
{
    angle = std::fmod(angle, FULL_ROTATION_DEGREES);
    if (angle < 0.f)
    {
        angle += FULL_ROTATION_DEGREES;
    }

    return angle;
}

int NormalizeAngleInt(int angle)
{
    angle %= static_cast<int>(FULL_ROTATION_DEGREES);
    if (angle < 0)
    {
        angle += static_cast<int>(FULL_ROTATION_DEGREES);
    }
    return angle;
}

float SignedAngleDelta(float from, float to)
{
    float delta = NormalizeAngleDegrees(to) - NormalizeAngleDegrees(from);
    if (delta > HALF_ROTATION_DEGREES)
    {
        delta -= FULL_ROTATION_DEGREES;
    }
    else if (delta < -HALF_ROTATION_DEGREES)
    {
        delta += FULL_ROTATION_DEGREES;
    }
    return delta;
}

float StepTowardsAngle(float current, float target, float maxDelta)
{
    const float clampedDelta = std::clamp(SignedAngleDelta(current, target), -maxDelta, maxDelta);
    return NormalizeAngleDegrees(current + clampedDelta);
}
} // namespace

float CreateAngle2D(const vec3_t from, const vec2_t to)
{
    return CreateAngle(from[0], from[1], to[0], to[1]);
}

float CreateAngle(float x1, float y1, float x2, float y2)
{
    const float dx = x2 - x1;
    const float dy = y2 - y1;

    if (std::fabs(dx) < std::numeric_limits<float>::epsilon() && std::fabs(dy) < std::numeric_limits<float>::epsilon())
    {
        return 0.f;
    }

    const float angle = std::atan2(dx, -dy) * RAD_TO_DEG;
    return NormalizeAngleDegrees(angle);
}

int TurnAngle(int iTheta, int iHeading, int maxTURN)
{
    if (maxTURN <= 0)
    {
        return NormalizeAngleInt(iTheta);
    }

    const float updated =
        StepTowardsAngle(static_cast<float>(iTheta), static_cast<float>(iHeading), static_cast<float>(maxTURN));
    return NormalizeAngleInt(static_cast<int>(std::lround(updated)));
}

float TurnAngle2(float angle, float a, float d)
{
    if (d <= 0.f)
    {
        return NormalizeAngleDegrees(angle);
    }

    return StepTowardsAngle(angle, a, d);
}

float FarAngle(float angle1, float angle2, bool absolute)
{
    const float delta = SignedAngleDelta(angle1, angle2);
    return absolute ? std::fabs(delta) : delta;
}

float StepEasedTurn(float angle, float target, float& speed, float seconds, float maxSpeed, float acceleration)
{
    if (seconds <= 0.f)
    {
        return NormalizeAngleDegrees(angle);
    }
    const float gap = SignedAngleDelta(angle, target);
    if (std::fabs(gap) < 0.01f)
    {
        speed = 0.f;
        return NormalizeAngleDegrees(target);
    }
    // The fastest speed that still stops on the target when slowing down from here (v^2 = 2 a d), and no faster
    // than maxSpeed; the speed moves toward it by at most the acceleration.
    const float direction = gap > 0.f ? 1.f : -1.f;
    const float wanted = direction * std::min(maxSpeed, std::sqrt(2.f * acceleration * std::fabs(gap)));
    const float change = acceleration * seconds;
    speed += std::clamp(wanted - speed, -change, change);
    const float step = speed * seconds;
    // A step past the target (the last frame of a turn) lands on it.
    if ((step > 0.f) == (gap > 0.f) && std::fabs(step) >= std::fabs(gap))
    {
        speed = 0.f;
        return NormalizeAngleDegrees(target);
    }
    return NormalizeAngleDegrees(angle + step);
}

void StepSpring(float& value, float& velocity, float seconds, float frequency, float damping)
{
    // Semi-implicit Euler in short steps, so a slow frame doesn't make the spring blow up or swing differently.
    constexpr float MaxStepSeconds = 1.f / 240.f;
    const float omega = 2.f * static_cast<float>(Q_PI) * frequency;
    while (seconds > 0.f)
    {
        const float step = std::min(seconds, MaxStepSeconds);
        velocity += (-omega * omega * value - 2.f * damping * omega * velocity) * step;
        value += velocity * step;
        seconds -= step;
    }
}

int CalcAngle(float PositionX, float PositionY, float TargetX, float TargetY)
{
    const float targetAngle = CreateAngle(PositionX, PositionY, TargetX, TargetY);
    return NormalizeAngleInt(static_cast<int>(std::lround(targetAngle)));
}
