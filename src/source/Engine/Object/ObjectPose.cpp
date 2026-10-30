#include "stdafx.h"

#include "Engine/Object/ObjectPose.h"

#include "Render/Models/ZzzBMD.h"

namespace
{
bool Equal(const vec3_t a, const vec3_t b)
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}
} // namespace

namespace Engine::Object::Pose
{
void Build(OBJECT* o, BMD* model)
{
    model->Animation(o->BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle,
                     o->HeadAngle, false, false, nullptr, o->CurrentAction);
    Record(o);
}

void Record(OBJECT* o)
{
    PoseInputs& pose = o->Pose;
    pose.Type = o->Type;
    pose.CurrentAction = o->CurrentAction;
    pose.PriorAction = o->PriorAction;
    pose.AnimationFrame = o->AnimationFrame;
    pose.PriorAnimationFrame = o->PriorAnimationFrame;
    VectorCopy(o->Angle, pose.Angle);
    VectorCopy(o->HeadAngle, pose.HeadAngle);
}

bool IsCurrent(const OBJECT* o)
{
    const PoseInputs& pose = o->Pose;
    return o->BoneTransform != nullptr && pose.Type == o->Type && pose.CurrentAction == o->CurrentAction &&
           pose.PriorAction == o->PriorAction && pose.AnimationFrame == o->AnimationFrame &&
           pose.PriorAnimationFrame == o->PriorAnimationFrame && Equal(pose.Angle, o->Angle) &&
           Equal(pose.HeadAngle, o->HeadAngle);
}

void Invalidate(OBJECT* o)
{
    o->Pose.Type = -1;
}
} // namespace Engine::Object::Pose
