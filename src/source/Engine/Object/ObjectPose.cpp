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
    Record(o, model, false);
}

void Record(OBJECT* o, const BMD* model, bool world)
{
    PoseInputs& pose = o->Pose;
    pose.Type = o->Type;
    pose.CurrentAction = o->CurrentAction;
    pose.PriorAction = o->PriorAction;
    pose.AnimationFrame = o->AnimationFrame;
    pose.PriorAnimationFrame = o->PriorAnimationFrame;
    VectorCopy(o->Angle, pose.Angle);
    VectorCopy(o->HeadAngle, pose.HeadAngle);
    pose.World = world;
    VectorCopy(model->BodyOrigin, pose.Origin);
    pose.Scale = model->BodyScale;
    pose.Height = model->BodyHeight;
}

bool IsCurrent(const OBJECT* o, const BMD* model, bool world)
{
    const PoseInputs& pose = o->Pose;
    if (o->BoneTransform == nullptr || pose.Type != o->Type || pose.World != world || pose.Height != model->BodyHeight)
        return false;
    if (world && (!Equal(pose.Origin, model->BodyOrigin) || pose.Scale != model->BodyScale))
        return false;
    return pose.CurrentAction == o->CurrentAction && pose.PriorAction == o->PriorAction &&
           pose.AnimationFrame == o->AnimationFrame && pose.PriorAnimationFrame == o->PriorAnimationFrame &&
           Equal(pose.Angle, o->Angle) && Equal(pose.HeadAngle, o->HeadAngle);
}

void Invalidate(OBJECT* o)
{
    o->Pose.Type = -1;
}

float (*World(OBJECT* o, BMD* model))[3][4]
{
    if (o->BoneTransform == nullptr)
    {
        // shortcut: MAX_BONES per object whose effects read its pose; size it to the model's bones once skinning
        // palettes carry the array's length instead of assuming MAX_BONES.
        o->BoneTransform = new vec34_t[MAX_BONES];
        o->OwnsBoneTransform = true;
        Invalidate(o);
    }
    o->EnableBoneMatrix = true;
    if (!IsCurrent(o, model, true))
    {
        model->Animation(o->BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle,
                         o->HeadAngle, false, true, nullptr, o->CurrentAction);
        Record(o, model, true);
    }
    return o->BoneTransform;
}

void Release(OBJECT* o)
{
    if (!o->OwnsBoneTransform)
        return;
    delete[] o->BoneTransform;
    o->BoneTransform = nullptr;
    o->OwnsBoneTransform = false;
    o->EnableBoneMatrix = false;
}
} // namespace Engine::Object::Pose
