#pragma once

#include "Core/Math/ZzzMathLib.h"

class BMD;
class OBJECT;

namespace Engine::Object
{
// The animation state a model-space pose in OBJECT::BoneTransform was built from.
struct PoseInputs
{
    int Type = -1;
    unsigned short CurrentAction = 0;
    unsigned short PriorAction = 0;
    float AnimationFrame = 0.f;
    float PriorAnimationFrame = 0.f;
    vec3_t Angle{};
    vec3_t HeadAngle{};
};
} // namespace Engine::Object

// An object's own pose: built once in the update, reused by every draw while its animation state is unchanged.
// The model's BodyHeight must be 0, as drawing animates objects with own bones.
namespace Engine::Object::Pose
{
// Animates o's bones in model space and records the state they show.
void Build(OBJECT* o, BMD* model);
// Records that o's bones show its current state; for a caller that animated them itself.
void Record(OBJECT* o);
// Whether o's bones still show its current animation state.
bool IsCurrent(const OBJECT* o);
// Forgets the recorded state, for bones that were replaced or no longer show it.
void Invalidate(OBJECT* o);
} // namespace Engine::Object::Pose
