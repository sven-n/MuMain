#pragma once

#include "Core/Math/ZzzMathLib.h"

class BMD;
class OBJECT;

namespace Engine::Object
{
// The animation state a pose in OBJECT::BoneTransform was built from. A world-space pose also bakes in the model's
// BodyOrigin and BodyScale; both kinds bake in its BodyHeight.
struct PoseInputs
{
    int Type = -1;
    unsigned short CurrentAction = 0;
    unsigned short PriorAction = 0;
    float AnimationFrame = 0.f;
    float PriorAnimationFrame = 0.f;
    vec3_t Angle{};
    vec3_t HeadAngle{};
    bool World = false;
    vec3_t Origin{};
    float Scale = 0.f;
    float Height = 0.f;
};
} // namespace Engine::Object

// An object's own pose: built once in the update, reused by every draw while its animation state is unchanged.
namespace Engine::Object::Pose
{
// Animates o's bones in model space and records the state they show.
void Build(OBJECT* o, BMD* model);
// Records that o's bones show its current state in that space; for a caller that animated them itself.
void Record(OBJECT* o, const BMD* model, bool world);
// Whether o's bones still show its current animation state, in that space and with the model's current placement.
bool IsCurrent(const OBJECT* o, const BMD* model, bool world);
// Forgets the recorded state, for bones that were replaced or no longer show it.
void Invalidate(OBJECT* o);

// A map object's world-space pose, placed by the model's BodyOrigin and BodyScale. The object gets its own bones
// the first time, and its draws reuse them.
float (*World(OBJECT* o, BMD* model))[3][4];
// Frees the bones World gave a map object; DeleteObject calls it.
void Release(OBJECT* o);
} // namespace Engine::Object::Pose
