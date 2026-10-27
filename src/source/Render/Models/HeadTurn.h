#pragma once

#include <cmath>

// How a character's head turn (OBJECT::HeadAngle) spreads into the neck and spine of a Biped skeleton (Bip01 Head on
// Neck, Spine1 and Spine), so the shoulders follow the head as a person's do. BMD::Animation turns the bones by it,
// and a player's wings, which hang from the skeleton's root, turn with the shoulders by it.
namespace Render::Models::HeadTurn
{
// The share of the head's turn the neck and spine take together; the head keeps the rest, so it ends up turned as far
// as the head alone used to.
constexpr float ShouldersShare = 0.5f;
// How that share splits between the neck and the two spine bones (the clavicles hang from the neck).
constexpr float NeckPart = 0.4f;
constexpr float Spine1Part = 0.35f;
constexpr float SpinePart = 0.25f;

// HeadAngle[0] as -180 to 180 degrees: it's kept as 0 to 360 (300 is 60 to the other side), which is the same
// rotation for the head alone but not when a share of it goes to each bone.
inline float SignedYaw(float headAngle)
{
    return headAngle > 180.f ? headAngle - 360.f : headAngle;
}

// The degrees the shoulders turn (the neck and spine together) for a head turn of `yaw` degrees (signed).
inline float ShouldersDegrees(float yaw)
{
    return yaw * ShouldersShare;
}

// Each bone's part of a head turn of `yaw` degrees (signed). A skeleton without a bone (`hasNeck`, ...) leaves its
// part to the head, so the head always ends up turned by the whole of `yaw`.
struct BoneTurns
{
    float head = 0.f;
    float neck = 0.f;
    float spine1 = 0.f;
    float spine = 0.f;
};
inline BoneTurns SplitTurn(float yaw, bool hasNeck, bool hasSpine1, bool hasSpine)
{
    const float shoulders = ShouldersDegrees(yaw);
    BoneTurns turns;
    turns.neck = hasNeck ? shoulders * NeckPart : 0.f;
    turns.spine1 = hasNeck && hasSpine1 ? shoulders * Spine1Part : 0.f;
    turns.spine = hasNeck && hasSpine1 && hasSpine ? shoulders * SpinePart : 0.f;
    turns.head = yaw - turns.neck - turns.spine1 - turns.spine;
    return turns;
}

// Turns a transform (3x4, x and y across, z up) by `radians` about the upright through (pivotX, pivotY): how a part
// hanging from the root (wings) follows a turn of the spine above it.
inline void TurnAboutUpright(float matrix[3][4], float radians, float pivotX, float pivotY)
{
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    for (int column = 0; column < 4; ++column)
    {
        const bool position = column == 3;
        const float x = matrix[0][column] - (position ? pivotX : 0.f);
        const float y = matrix[1][column] - (position ? pivotY : 0.f);
        matrix[0][column] = cosine * x - sine * y + (position ? pivotX : 0.f);
        matrix[1][column] = sine * x + cosine * y + (position ? pivotY : 0.f);
    }
}
} // namespace Render::Models::HeadTurn
