#include "stdafx.h"

#ifdef _EDITOR

#include "PreviewModelPose.h"

#include <algorithm>
#include <cfloat>

extern float BoneScale;

namespace MuEditor::Effects
{
void PoseLikeRenderObject(OBJECT& o, BMD& b)
{
    b.BodyHeight = 0.0f;
    b.ContrastEnable = o.ContrastEnable;
    b.LightEnable = o.LightEnable;
    VectorCopy(o.Light, b.BodyLight);
    b.BodyScale = o.Scale;
    b.CurrentAction = o.CurrentAction;
    VectorCopy(o.Position, b.BodyOrigin);
    b.Animation(BoneTransform, o.AnimationFrame, o.PriorAnimationFrame, o.PriorAction, o.Angle, o.HeadAngle, false,
                true);
    BoneScale = 1.0f;
}

bool MeasureModel(OBJECT& o, PreviewVector& min, PreviewVector& max)
{
    BMD& b = Models[o.Type];
    if (b.NumActions <= 0)
        return false;
    const ScopedModelState saved(b);
    const float savedBoneScale = BoneScale;
    PoseLikeRenderObject(o, b);
    min = {FLT_MAX, FLT_MAX, FLT_MAX};
    max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    for (int mesh = 0; mesh < b.NumMeshs; ++mesh)
    {
        if (mesh == o.HiddenMesh)
            continue;
        for (int vertex = 0; vertex < b.Meshs[mesh].NumVertices; ++vertex)
        {
            vec3_t p;
            b.SkinVertex(mesh, vertex, BoneTransform, false, 0.0f, p);
            for (int axis = 0; axis < 3; ++axis)
            {
                min[axis] = std::min(min[axis], p[axis]);
                max[axis] = std::max(max[axis], p[axis]);
            }
        }
    }
    BoneScale = savedBoneScale;
    return min[0] <= max[0];
}
} // namespace MuEditor::Effects

#endif // _EDITOR
