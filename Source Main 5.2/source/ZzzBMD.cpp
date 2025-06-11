///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h" 
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "zzzlodterrain.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "SMD.h"
#include "zzzEffect.h"

#include "UIMng.h"
#include "CameraMove.h"
#include "PhysicsManager.h"
#include "NewUISystem.h"

BMD* Models;
BMD* ModelsDump;

vec4_t BoneQuaternion[MAX_BONES];
short  BoundingVertices[MAX_BONES];
vec3_t BoundingMin[MAX_BONES];
vec3_t BoundingMax[MAX_BONES];

float  BoneTransform[MAX_BONES][3][4];

vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
vec3_t NormalTransform[MAX_MESH][MAX_VERTICES];
float  IntensityTransform[MAX_MESH][MAX_VERTICES];
vec3_t LightTransform[MAX_MESH][MAX_VERTICES];

vec3_t RenderArrayVertices[MAX_VERTICES * 3];
vec4_t RenderArrayColors[MAX_VERTICES * 3];
vec2_t RenderArrayTexCoords[MAX_VERTICES * 3];

bool  StopMotion = false;
float ParentMatrix[3][4];

static vec3_t LightVector = { 0.f, -0.1f, -0.8f };
static vec3_t LightVector2 = { 0.f, -0.5f, -0.8f };

void BMD::Animation(float(*BoneMatrix)[3][4], float AnimationFrame, float PriorFrame, unsigned short PriorAction, vec3_t Angle, vec3_t HeadAngle, bool Parent, bool Translate)
{
    if (NumActions <= 0) return;

    if (PriorAction >= NumActions) PriorAction = 0;
    if (CurrentAction >= NumActions)CurrentAction = 0;
    VectorCopy(Angle, BodyAngle);

    CurrentAnimation = AnimationFrame;
    CurrentAnimationFrame = (int)AnimationFrame;
    float s1 = (CurrentAnimation - CurrentAnimationFrame);
    float s2 = 1.f - s1;
    auto PriorAnimationFrame = (int)PriorFrame;
    if (NumActions > 0)
    {
        if (PriorAnimationFrame < 0)
            PriorAnimationFrame = 0;
        if (CurrentAnimationFrame < 0)
            CurrentAnimationFrame = 0;
        if (PriorAnimationFrame >= Actions[PriorAction].NumAnimationKeys)
            PriorAnimationFrame = 0;
        if (CurrentAnimationFrame >= Actions[CurrentAction].NumAnimationKeys)
            CurrentAnimationFrame = 0;
    }

    // bones
    for (int i = 0; i < NumBones; i++)
    {
        Bone_t* b = &Bones[i];
        if (b->Dummy)
        {
            continue;
        }
        BoneMatrix_t* bm1 = &b->BoneMatrixes[PriorAction];
        BoneMatrix_t* bm2 = &b->BoneMatrixes[CurrentAction];
        vec4_t q1, q2;

        if (i == BoneHead)
        {
            vec3_t Angle1, Angle2;
            VectorCopy(bm1->Rotation[PriorAnimationFrame], Angle1);
            VectorCopy(bm2->Rotation[CurrentAnimationFrame], Angle2);

            float HeadAngleX = HeadAngle[0] / (180.f / Q_PI);
            float HeadAngleY = HeadAngle[1] / (180.f / Q_PI);
            Angle1[0] -= HeadAngleX;
            Angle2[0] -= HeadAngleX;
            Angle1[2] -= HeadAngleY;
            Angle2[2] -= HeadAngleY;
            AngleQuaternion(Angle1, q1);
            AngleQuaternion(Angle2, q2);
        }
        else
        {
            QuaternionCopy(bm1->Quaternion[PriorAnimationFrame], q1);
            QuaternionCopy(bm2->Quaternion[CurrentAnimationFrame], q2);
        }
        if (!QuaternionCompare(q1, q2))
        {
            QuaternionSlerp(q1, q2, s1, BoneQuaternion[i]);
        }
        else
        {
            QuaternionCopy(q1, BoneQuaternion[i]);
        }

        float Matrix[3][4];
        QuaternionMatrix(BoneQuaternion[i], Matrix);
        float* Position1 = bm1->Position[PriorAnimationFrame];
        float* Position2 = bm2->Position[CurrentAnimationFrame];

        if (i == 0 && (Actions[PriorAction].LockPositions || Actions[CurrentAction].LockPositions))
        {
            Matrix[0][3] = bm2->Position[0][0];
            Matrix[1][3] = bm2->Position[0][1];
            Matrix[2][3] = Position1[2] * s2 + Position2[2] * s1 + BodyHeight;
        }
        else
        {
            Matrix[0][3] = Position1[0] * s2 + Position2[0] * s1;
            Matrix[1][3] = Position1[1] * s2 + Position2[1] * s1;
            Matrix[2][3] = Position1[2] * s2 + Position2[2] * s1;
        }

        if (b->Parent == -1)
        {
            if (!Parent)
            {
                AngleMatrix(BodyAngle, ParentMatrix);
                if (Translate)
                {
                    for (auto & y : ParentMatrix)
                    {
                        for (int x = 0; x < 3; ++x)
                        {
                            y[x] *= BodyScale;
                        }
                    }

                    ParentMatrix[0][3] = BodyOrigin[0];
                    ParentMatrix[1][3] = BodyOrigin[1];
                    ParentMatrix[2][3] = BodyOrigin[2];
                }
            }
            R_ConcatTransforms(ParentMatrix, Matrix, BoneMatrix[i]);
        }
        else
        {
            R_ConcatTransforms(BoneMatrix[b->Parent], Matrix, BoneMatrix[i]);
        }
    }
}

extern EGameScene SceneFlag;
extern int EditFlag;

bool HighLight = true;
float BoneScale = 1.f;

void BMD::Transform(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate, float _Scale)
{
    vec3_t LightPosition;

    if (LightEnable)
    {
        vec3_t Position;

        float Matrix[3][4];
        if (HighLight)
        {
            Vector(1.3f, 0.f, 2.f, Position);
        }
        else if (gMapManager.InBattleCastle())
        {
            Vector(0.5f, -1.f, 1.f, Position);
            Vector(0.f, 0.f, -45.f, ShadowAngle);
        }
        else
        {
            Vector(0.f, -1.5f, 0.f, Position);
        }

        AngleMatrix(ShadowAngle, Matrix);
        VectorIRotate(Position, Matrix, LightPosition);
    }
    vec3_t BoundingMin;
    vec3_t BoundingMax;
#ifdef _DEBUG
#else
    if (EditFlag == 2)
#endif
    {
        Vector(999999.f, 999999.f, 999999.f, BoundingMin);
        Vector(-999999.f, -999999.f, -999999.f, BoundingMax);
    }
    for (int i = 0; i < NumMeshs; i++)
    {
        Mesh_t* m = &Meshs[i];
        for (int j = 0; j < m->NumVertices; j++)
        {
            Vertex_t* v = &m->Vertices[j];
            float* vp = VertexTransform[i][j];

            if (BoneScale == 1.f)
            {
                if (_Scale)
                {
                    vec3_t Position;
                    VectorCopy(v->Position, Position);
                    VectorScale(Position, _Scale, Position);
                    VectorTransform(Position, BoneMatrix[v->Node], vp);
                }
                else
                    VectorTransform(v->Position, BoneMatrix[v->Node], vp);
                if (Translate)
                    VectorScale(vp, BodyScale, vp);
            }
            else
            {
                VectorRotate(v->Position, BoneMatrix[v->Node], vp);
                vp[0] = vp[0] * BoneScale + BoneMatrix[v->Node][0][3];
                vp[1] = vp[1] * BoneScale + BoneMatrix[v->Node][1][3];
                vp[2] = vp[2] * BoneScale + BoneMatrix[v->Node][2][3];
                if (Translate)
                    VectorScale(vp, BodyScale, vp);
            }
#ifdef _DEBUG
#else
            if (EditFlag == 2)
#endif
            {
                for (int k = 0; k < 3; k++)
                {
                    if (vp[k] < BoundingMin[k]) BoundingMin[k] = vp[k];
                    if (vp[k] > BoundingMax[k]) BoundingMax[k] = vp[k];
                }
            }
            if (Translate)
                VectorAdd(vp, BodyOrigin, vp);
        }

        for (int j = 0; j < m->NumNormals; j++)
        {
            Normal_t* sn = &m->Normals[j];
            float* tn = NormalTransform[i][j];
            VectorRotate(sn->Normal, BoneMatrix[sn->Node], tn);
            if (LightEnable)
            {
                float Luminosity;
                Luminosity = DotProduct(tn, LightPosition) * 0.8f + 0.4f;

                if (Luminosity < 0.2f) Luminosity = 0.2f;
                IntensityTransform[i][j] = Luminosity;
            }
        }
    }
    if (EditFlag == 2)
    {
        VectorCopy(BoundingMin, OBB->StartPos);
        OBB->XAxis[0] = (BoundingMax[0] - BoundingMin[0]);
        OBB->YAxis[1] = (BoundingMax[1] - BoundingMin[1]);
        OBB->ZAxis[2] = (BoundingMax[2] - BoundingMin[2]);
    }
    else
    {
        VectorCopy(BoundingBoxMin, OBB->StartPos);
        OBB->XAxis[0] = (BoundingBoxMax[0] - BoundingBoxMin[0]);
        OBB->YAxis[1] = (BoundingBoxMax[1] - BoundingBoxMin[1]);
        OBB->ZAxis[2] = (BoundingBoxMax[2] - BoundingBoxMin[2]);
    }
    fTransformedSize = max(max(BoundingMax[0] - BoundingMin[0], BoundingMax[1] - BoundingMin[1]),
        BoundingMax[2] - BoundingMin[2]);
    VectorAdd(OBB->StartPos, BodyOrigin, OBB->StartPos);
    OBB->XAxis[1] = 0.f;
    OBB->XAxis[2] = 0.f;
    OBB->YAxis[0] = 0.f;
    OBB->YAxis[2] = 0.f;
    OBB->ZAxis[0] = 0.f;
    OBB->ZAxis[1] = 0.f;
}

void BMD::TransformByObjectBone(vec3_t vResultPosition, OBJECT* pObject, int iBoneNumber, vec3_t vRelativePosition)
{
    if (iBoneNumber < 0 || iBoneNumber >= NumBones)
    {
        assert(!"Bone number error");
        return;
    }
    if (pObject == nullptr)
    {
        assert(!"Empty Bone");
        return;
    }

    float(*TransformMatrix)[4];
    if (pObject->BoneTransform != nullptr)
    {
        TransformMatrix = pObject->BoneTransform[iBoneNumber];
    }
    else
    {
        TransformMatrix = BoneTransform[iBoneNumber];
    }

    vec3_t vTemp;
    if (vRelativePosition == nullptr)
    {
        vTemp[0] = TransformMatrix[0][3];
        vTemp[1] = TransformMatrix[1][3];
        vTemp[2] = TransformMatrix[2][3];
    }
    else
    {
        VectorTransform(vRelativePosition, TransformMatrix, vTemp);
    }
    VectorScale(vTemp, BodyScale, vTemp);
    VectorAdd(vTemp, pObject->Position, vResultPosition);
}

void BMD::TransformByBoneMatrix(vec3_t vResultPosition, float(*BoneMatrix)[4], vec3_t vWorldPosition, vec3_t vRelativePosition)
{
    if (BoneMatrix == nullptr)
    {
        assert(!"Empty Matrix");
        return;
    }

    vec3_t vTemp;
    if (vRelativePosition == nullptr)
    {
        vTemp[0] = BoneMatrix[0][3];
        vTemp[1] = BoneMatrix[1][3];
        vTemp[2] = BoneMatrix[2][3];
    }
    else
    {
        VectorTransform(vRelativePosition, BoneMatrix, vTemp);
    }
    if (vWorldPosition != nullptr)
    {
        VectorScale(vTemp, BodyScale, vTemp);
        VectorAdd(vTemp, vWorldPosition, vResultPosition);
    }
    else
    {
        VectorScale(vTemp, BodyScale, vResultPosition);
    }
}

void BMD::TransformPosition(float(*Matrix)[4], vec3_t Position, vec3_t WorldPosition, bool Translate)
{
    if (Translate)
    {
        vec3_t p;
        VectorTransform(Position, Matrix, p);
        VectorScale(p, BodyScale, p);
        VectorAdd(p, BodyOrigin, WorldPosition);
    }
    else
        VectorTransform(Position, Matrix, WorldPosition);
}

void BMD::RotationPosition(float(*Matrix)[4], vec3_t Position, vec3_t WorldPosition)
{
    vec3_t p;
    VectorRotate(Position, Matrix, p);
    VectorScale(p, BodyScale, WorldPosition);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ParentMatrix[i][j] = Matrix[i][j];
        }
    }
}

bool BMD::PlayAnimation(float* AnimationFrame, float* PriorAnimationFrame, unsigned short* PriorAction, float Speed, vec3_t Origin, vec3_t Angle)
{
    bool Loop = true;

    if (AnimationFrame == nullptr || PriorAnimationFrame == nullptr || PriorAction == nullptr || (NumActions > 0 && CurrentAction >= NumActions))
    {
        return Loop;
    }

    if (NumActions == 0 || Actions[CurrentAction].NumAnimationKeys <= 1)
    {
        return Loop;
    }

    const auto priorAnimationFrame = (int)*AnimationFrame;
    *AnimationFrame += Speed * FPS_ANIMATION_FACTOR;
    if (priorAnimationFrame != (int)*AnimationFrame)
    {
        *PriorAction = CurrentAction;
        *PriorAnimationFrame = (float)priorAnimationFrame;
    }
    if (*AnimationFrame <= 0.f)
    {
        *AnimationFrame += (float)Actions[CurrentAction].NumAnimationKeys - 1.f;
    }

    if (Actions[CurrentAction].Loop)
    {
        if (*AnimationFrame >= (float)Actions[CurrentAction].NumAnimationKeys)
        {
            *AnimationFrame = (float)Actions[CurrentAction].NumAnimationKeys - 0.01f;
            Loop = false;
        }
    }
    else
    {
        int Key;
        if (Actions[CurrentAction].LockPositions)
            Key = Actions[CurrentAction].NumAnimationKeys - 1;
        else
            Key = Actions[CurrentAction].NumAnimationKeys;

        float fTemp;

        if (SceneFlag == CHARACTER_SCENE)
        {
            fTemp = *AnimationFrame + 2;
        }
        else if (gMapManager.WorldActive == WD_39KANTURU_3RD && CurrentAction == MONSTER01_APEAR)
        {
            fTemp = *AnimationFrame + 1;
        }
        else
        {
            fTemp = *AnimationFrame;
        }

        if (fTemp >= (int)Key)
        {
            auto Frame = (int)*AnimationFrame;
            *AnimationFrame = (float)(Frame % (Key)) + (*AnimationFrame - (float)Frame);
            Loop = false;
        }
    }
    CurrentAnimation = *AnimationFrame;
    CurrentAnimationFrame = (int)maxf(0, CurrentAnimation);

    return Loop;
}
void BMD::AnimationTransformWithAttachHighModel_usingGlobalTM(OBJECT* oHighHierarchyModel, BMD* bmdHighHierarchyModel, int iBoneNumberHighHierarchyModel, vec3_t& vOutPosHighHiearachyModelBone, vec3_t* arrOutSetfAllBonePositions, bool bApplyTMtoVertices)
{
    if (NumBones < 1) return;
    if (NumBones > MAX_BONES) return;

    vec34_t* arrBonesTMLocal;

    vec34_t		tmBoneHierarchicalObject;

    vec3_t		Temp, v3Position;
    OBB_t		OBB;

    arrBonesTMLocal = new vec34_t[NumBones];
    Vector(0.0f, 0.0f, 0.0f, Temp);

    memset(arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones);
    memset(tmBoneHierarchicalObject, 0, sizeof(vec34_t));

    memcpy(tmBoneHierarchicalObject, oHighHierarchyModel->BoneTransform[iBoneNumberHighHierarchyModel], sizeof(vec34_t));
    BodyScale = oHighHierarchyModel->Scale;

    tmBoneHierarchicalObject[0][3] = tmBoneHierarchicalObject[0][3] * BodyScale;
    tmBoneHierarchicalObject[1][3] = tmBoneHierarchicalObject[1][3] * BodyScale;
    tmBoneHierarchicalObject[2][3] = tmBoneHierarchicalObject[2][3] * BodyScale;

    if (nullptr != vOutPosHighHiearachyModelBone)
    {
        Vector(tmBoneHierarchicalObject[0][3], tmBoneHierarchicalObject[1][3], tmBoneHierarchicalObject[2][3],
            vOutPosHighHiearachyModelBone);
    }

    VectorCopy(oHighHierarchyModel->Position, v3Position);

    Animation(arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false);

    for (int i_ = 0; i_ < NumBones; ++i_)
    {
        R_ConcatTransforms(tmBoneHierarchicalObject, arrBonesTMLocal[i_], BoneTransform[i_]);
        BoneTransform[i_][0][3] = BoneTransform[i_][0][3] + v3Position[0];
        BoneTransform[i_][1][3] = BoneTransform[i_][1][3] + v3Position[1];
        BoneTransform[i_][2][3] = BoneTransform[i_][2][3] + v3Position[2];

        Vector(BoneTransform[i_][0][3],
            BoneTransform[i_][1][3],
            BoneTransform[i_][2][3],
            arrOutSetfAllBonePositions[i_]);
    }

    if (true == bApplyTMtoVertices)
    {
        Transform(BoneTransform, Temp, Temp, &OBB, false);
    }

    delete[] arrBonesTMLocal;
}

void BMD::AnimationTransformWithAttachHighModel(OBJECT* oHighHierarchyModel, BMD* bmdHighHierarchyModel, int iBoneNumberHighHierarchyModel, vec3_t& vOutPosHighHiearachyModelBone, vec3_t* arrOutSetfAllBonePositions)
{
    if (NumBones < 1) return;
    if (NumBones > MAX_BONES) return;

    vec34_t* arrBonesTMLocal;
    vec34_t* arrBonesTMLocalResult;
    vec34_t		tmBoneHierarchicalObject;
    vec3_t		Temp, v3Position;

    arrBonesTMLocal = new vec34_t[NumBones];
    Vector(0.0f, 0.0f, 0.0f, Temp);

    arrBonesTMLocalResult = new vec34_t[NumBones];

    memset(arrBonesTMLocalResult, 0, sizeof(vec34_t) * NumBones);
    memset(arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones);

    memset(tmBoneHierarchicalObject, 0, sizeof(vec34_t));

    memcpy(tmBoneHierarchicalObject, oHighHierarchyModel->BoneTransform[iBoneNumberHighHierarchyModel], sizeof(vec34_t));

    BodyScale = oHighHierarchyModel->Scale;

    tmBoneHierarchicalObject[0][3] = tmBoneHierarchicalObject[0][3] * BodyScale;
    tmBoneHierarchicalObject[1][3] = tmBoneHierarchicalObject[1][3] * BodyScale;
    tmBoneHierarchicalObject[2][3] = tmBoneHierarchicalObject[2][3] * BodyScale;

    if (nullptr != vOutPosHighHiearachyModelBone)
    {
        Vector(tmBoneHierarchicalObject[0][3], tmBoneHierarchicalObject[1][3], tmBoneHierarchicalObject[2][3],
            vOutPosHighHiearachyModelBone);
    }

    VectorCopy(oHighHierarchyModel->Position, v3Position);

    Animation(arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false);
    for (int i_ = 0; i_ < NumBones; ++i_)
    {
        R_ConcatTransforms(tmBoneHierarchicalObject, arrBonesTMLocal[i_], arrBonesTMLocalResult[i_]);
        arrBonesTMLocalResult[i_][0][3] = arrBonesTMLocalResult[i_][0][3] + v3Position[0];
        arrBonesTMLocalResult[i_][1][3] = arrBonesTMLocalResult[i_][1][3] + v3Position[1];
        arrBonesTMLocalResult[i_][2][3] = arrBonesTMLocalResult[i_][2][3] + v3Position[2];

        Vector(arrBonesTMLocalResult[i_][0][3], arrBonesTMLocalResult[i_][1][3], arrBonesTMLocalResult[i_][2][3], arrOutSetfAllBonePositions[i_]);
    }

    delete[] arrBonesTMLocalResult;
    delete[] arrBonesTMLocal;
}

void BMD::AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions, const OBJECT* oSelf)
{
    if (NumBones < 1) return;
    if (NumBones > MAX_BONES) return;

    vec34_t* arrBonesTMLocal;

    vec3_t		Temp;

    arrBonesTMLocal = new vec34_t[NumBones];
    Vector(0.0f, 0.0f, 0.0f, Temp);

    memset(arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones);

    Animation(arrBonesTMLocal, oSelf->AnimationFrame, oSelf->PriorAnimationFrame, oSelf->PriorAction, (const_cast<OBJECT*>(oSelf))->Angle, Temp, false, true);

    for (int i_ = 0; i_ < NumBones; ++i_)
    {
        Vector(arrBonesTMLocal[i_][0][3], arrBonesTMLocal[i_][1][3], arrBonesTMLocal[i_][2][3], arrOutSetfAllBonePositions[i_]);
    }
    delete[] arrBonesTMLocal;
}

void BMD::AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions,
    const vec3_t& v3Angle,
    const vec3_t& v3Position,
    const float& fScale,
    OBJECT* oRefAnimation,
    const float fFrameArea,
    const float fWeight)
{
    if (NumBones < 1) return;
    if (NumBones > MAX_BONES) return;

    vec34_t* arrBonesTMLocal;
    vec3_t		v3RootAngle, v3RootPosition;
    float		fRootScale;
    vec3_t		Temp;

    fRootScale = const_cast<float&>(fScale);

    v3RootAngle[0] = v3Angle[0];
    v3RootAngle[1] = v3Angle[1];
    v3RootAngle[2] = v3Angle[2];

    v3RootPosition[0] = v3Position[0];
    v3RootPosition[1] = v3Position[1];
    v3RootPosition[2] = v3Position[2];

    arrBonesTMLocal = new vec34_t[NumBones];
    Vector(0.0f, 0.0f, 0.0f, Temp);

    memset(arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones);

    if (nullptr == oRefAnimation)
    {
        Animation(arrBonesTMLocal, 0, 0, 0, v3RootAngle, Temp, false, true);
    }
    else
    {
        float			fAnimationFrame = oRefAnimation->AnimationFrame,
            fPiriorAnimationFrame = oRefAnimation->PriorAnimationFrame;
        unsigned short	iPiriorAction = oRefAnimation->PriorAction;

        if (fWeight >= 0.0f && fFrameArea > 0.0f)
        {
            float fAnimationFrameStart = fAnimationFrame - fFrameArea;
            float fAnimationFrameEnd = fAnimationFrame;
            LInterpolationF(fAnimationFrame, fAnimationFrameStart, fAnimationFrameEnd, fWeight);
        }

        Animation(arrBonesTMLocal,
            fAnimationFrame,
            fPiriorAnimationFrame,
            iPiriorAction,
            v3RootAngle, Temp, false, true);
    }

    vec3_t	v3RelatePos;
    Vector(1.0f, 1.0f, 1.0f, v3RelatePos);
    for (int i_ = 0; i_ < NumBones; ++i_)
    {
        Vector(arrBonesTMLocal[i_][0][3],
            arrBonesTMLocal[i_][1][3],
            arrBonesTMLocal[i_][2][3],
            arrOutSetfAllBonePositions[i_]);
    }

    delete[] arrBonesTMLocal;
}

vec3_t		g_vright;		// needs to be set to viewer's right in order for chrome to work
int			g_smodels_total = 1;				// cookie
float		g_chrome[MAX_VERTICES][2];	// texture coords for surface normals
int			g_chromeage[MAX_BONES];	// last time chrome vectors were updated
vec3_t		g_chromeup[MAX_BONES];		// chrome vector "up" in bone reference frames
vec3_t		g_chromeright[MAX_BONES];	// chrome vector "right" in bone reference frames

void BMD::Chrome(float* pchrome, int bone, vec3_t normal)
{
    Vector(0.f, 0.f, 1.f, g_vright);

    float n;

    {
        vec3_t chromeupvec;		
        vec3_t chromerightvec;
        vec3_t tmp;			
        VectorScale(BodyOrigin, -1, tmp);
        VectorNormalize(tmp);
        CrossProduct(tmp, g_vright, chromeupvec);
        VectorNormalize(chromeupvec);
        CrossProduct(tmp, chromeupvec, chromerightvec);
        VectorNormalize(chromerightvec);

        g_chromeage[bone] = g_smodels_total;
    }

    n = DotProduct(normal, g_chromeright[bone]);
    pchrome[0] = (n + 1.f); // FIX: make this a float

    n = DotProduct(normal, g_chromeup[bone]);
    pchrome[1] = (n + 1.f); // FIX: make this a float
}

void BMD::Lighting(float* pLight, Light_t* lp, vec3_t Position, vec3_t Normal)
{
    vec3_t Light;
    VectorSubtract(lp->Position, Position, Light);
    float Length = sqrtf(Light[0] * Light[0] + Light[1] * Light[1] + Light[2] * Light[2]);

    float LightCos = (DotProduct(Normal, Light) / Length) * 0.8f + 0.3f;
    if (Length > lp->Range) LightCos -= (Length - lp->Range) * 0.01f;
    if (LightCos < 0.f) LightCos = 0.f;
    pLight[0] += LightCos * lp->Color[0];
    pLight[1] += LightCos * lp->Color[1];
    pLight[2] += LightCos * lp->Color[2];
}

///////////////////////////////////////////////////////////////////////////////
// light map
///////////////////////////////////////////////////////////////////////////////

#define AXIS_X  0
#define AXIS_Y  1
#define AXIS_Z  2

float SubPixel = 16.f;

void SmoothBitmap(int Width, int Height, unsigned char* Buffer)
{
    int RowStride = Width * 3;
    for (int i = 1; i < Height - 1; i++)
    {
        for (int j = 1; j < Width - 1; j++)
        {
            int Index = (i * Width + j) * 3;
            for (int k = 0; k < 3; k++)
            {
                Buffer[Index] = (Buffer[Index - RowStride - 3] + Buffer[Index - RowStride] + Buffer[Index - RowStride + 3] +
                    Buffer[Index - 3] + Buffer[Index + 3] +
                    Buffer[Index + RowStride - 3] + Buffer[Index + RowStride] + Buffer[Index + RowStride + 3]) / 8;
                Index++;
            }
        }
    }
}

bool BMD::CollisionDetectLineToMesh(vec3_t Position, vec3_t Target, bool Collision, int Mesh, int Triangle)
{
    int i, j;
    for (i = 0; i < NumMeshs; i++)
    {
        Mesh_t* m = &Meshs[i];

        for (j = 0; j < m->NumTriangles; j++)
        {
            if (i == Mesh && j == Triangle) continue;
            Triangle_t* tp = &m->Triangles[j];
            float* vp1 = VertexTransform[i][tp->VertexIndex[0]];
            float* vp2 = VertexTransform[i][tp->VertexIndex[1]];
            float* vp3 = VertexTransform[i][tp->VertexIndex[2]];
            float* vp4 = VertexTransform[i][tp->VertexIndex[3]];

            vec3_t Normal;
            FaceNormalize(vp1, vp2, vp3, Normal);
            bool success = CollisionDetectLineToFace(Position, Target, tp->Polygon, vp1, vp2, vp3, vp4, Normal, Collision);
            if (success == true) return true;
        }
    }
    return false;
}

void BMD::CreateLightMapSurface(Light_t* lp, Mesh_t* m, int i, int j, int MapWidth, int MapHeight, int MapWidthMax, int MapHeightMax, vec3_t BoundingMin, vec3_t BoundingMax, int Axis)
{
    int k, l;
    Triangle_t* tp = &m->Triangles[j];
    float* np = NormalTransform[i][tp->NormalIndex[0]];
    float* vp = VertexTransform[i][tp->VertexIndex[0]];
    float d = -DotProduct(vp, np);

    Bitmap_t* lmp = &LightMaps[NumLightMaps];
    if (lmp->Buffer == nullptr)
    {
        lmp->Width = MapWidthMax;
        lmp->Height = MapHeightMax;
        int BufferBytes = lmp->Width * lmp->Height * 3;
        lmp->Buffer = new unsigned char[BufferBytes];
        memset(lmp->Buffer, 0, BufferBytes);
    }

    for (k = 0; k < MapHeight; k++)
    {
        for (l = 0; l < MapWidth; l++)
        {
            vec3_t p;
            Vector(0.f, 0.f, 0.f, p);
            switch (Axis)
            {
            case AXIS_Z:
                p[0] = BoundingMin[0] + l * SubPixel;
                p[1] = BoundingMin[1] + k * SubPixel;
                if (p[0] >= BoundingMax[0]) p[0] = BoundingMax[0];
                if (p[1] >= BoundingMax[1]) p[1] = BoundingMax[1];
                p[2] = (np[0] * p[0] + np[1] * p[1] + d) / -np[2];
                break;
            case AXIS_Y:
                p[0] = BoundingMin[0] + (float)l * SubPixel;
                p[2] = BoundingMin[2] + (float)k * SubPixel;
                if (p[0] >= BoundingMax[0]) p[0] = BoundingMax[0];
                if (p[2] >= BoundingMax[2]) p[2] = BoundingMax[2];
                p[1] = (np[0] * p[0] + np[2] * p[2] + d) / -np[1];
                break;
            case AXIS_X:
                p[2] = BoundingMin[2] + l * SubPixel;
                p[1] = BoundingMin[1] + k * SubPixel;
                if (p[2] >= BoundingMax[2]) p[2] = BoundingMax[2];
                if (p[1] >= BoundingMax[1]) p[1] = BoundingMax[1];
                p[0] = (np[2] * p[2] + np[1] * p[1] + d) / -np[0];
                break;
            }
            vec3_t Direction;
            VectorSubtract(p, lp->Position, Direction);
            VectorNormalize(Direction);
            VectorSubtract(p, Direction, p);
            bool success = CollisionDetectLineToMesh(lp->Position, p, true, i, j);

            if (success == false)
            {
                unsigned char* Bitmap = &lmp->Buffer[(k * MapWidthMax + l) * 3];
                vec3_t Light;
                Vector(0.f, 0.f, 0.f, Light);
                Lighting(Light, lp, p, np);
                for (int c = 0; c < 3; c++)
                {
                    int color = Bitmap[c];
                    color += (unsigned char)(Light[c] * 255.f);
                    if (color > 255) color = 255;
                    Bitmap[c] = color;
                }
            }
        }
    }
}

void BMD::CreateLightMaps()
{
}

void BMD::BindLightMaps()
{
    if (LightMapEnable == true) return;

    for (int i = 0; i < NumLightMaps; i++)
    {
        Bitmap_t* lmp = &LightMaps[i];
        if (lmp->Buffer != nullptr)
        {
            SmoothBitmap(lmp->Width, lmp->Height, lmp->Buffer);
            SmoothBitmap(lmp->Width, lmp->Height, lmp->Buffer);

            glBindTexture(GL_TEXTURE_2D, i + IndexLightMap);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lmp->Width, lmp->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, lmp->Buffer);
        }
    }
    LightMapEnable = true;
}

void BMD::ReleaseLightMaps()
{
    if (LightMapEnable == false) return;
    for (int i = 0; i < NumLightMaps; i++)
    {
        Bitmap_t* lmp = &LightMaps[i];
        if (lmp->Buffer != nullptr)
        {
            delete lmp->Buffer;
            lmp->Buffer = nullptr;
        }
    }
    LightMapEnable = false;
}

void BMD::BeginRender(float Alpha)
{
    glPushMatrix();
}

void BMD::EndRender()
{
    glPopMatrix();
}

extern double WorldTime;
extern int WaterTextureNumber;

void BMD::BeginRenderCoinHeap()
{
    constexpr int meshIndex = 0;
    Mesh_t* m = &Meshs[meshIndex];
    const auto textureIndex = IndexTexture[m->Texture];

    BindTexture(textureIndex);
    DisableAlphaBlend();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

int BMD::AddToCoinHeap(int coinIndex, int target_vertex_index)
{
    const auto vertices = RenderArrayVertices;
    const auto colors = RenderArrayColors;
    const auto texCoords = RenderArrayTexCoords;

    constexpr auto alpha = 1.0f;
    constexpr int meshIndex = 0;

    Mesh_t* m = &Meshs[meshIndex];

    for (int j = 0; j < m->NumTriangles; j++)
    {
        const auto triangle = &m->Triangles[j];
        for (int k = 0; k < triangle->Polygon; k++)
        {
            const int source_vertex_index = triangle->VertexIndex[k];
            target_vertex_index++;

            VectorCopy(VertexTransform[meshIndex][source_vertex_index], vertices[target_vertex_index]);

            Vector4(BodyLight[0], BodyLight[1], BodyLight[2], alpha, colors[target_vertex_index]);

            auto texco = m->TexCoords[triangle->TexCoordIndex[k]];
            texCoords[target_vertex_index][0] = texco.TexCoordU;
            texCoords[target_vertex_index][1] = texco.TexCoordV;
        }
    }

    return target_vertex_index;
}

void BMD::EndRenderCoinHeap(int coinCount)
{
    const auto vertices = RenderArrayVertices;
    const auto colors = RenderArrayColors;
    const auto texCoords = RenderArrayTexCoords;

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, colors);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    constexpr int meshIndex = 0;
    Mesh_t* m = &Meshs[meshIndex];
    glDrawArrays(GL_TRIANGLES, 0, m->NumTriangles * 3 * coinCount);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void BMD::RenderMesh(int meshIndex, int renderFlags, float alpha, int blendMeshIndex, float blendMeshAlpha, float blendMeshTextureCoordU, float blendMeshTextureCoordV, int explicitTextureIndex)
{
    if (meshIndex >= NumMeshs || meshIndex < 0) return;

    Mesh_t* m = &Meshs[meshIndex];
    if (m->NumTriangles == 0) return;

    float wave = static_cast<long>(WorldTime) % 10000 * 0.0001f;

    int textureIndex = IndexTexture[m->Texture];
    if (textureIndex == BITMAP_HIDE)
    {
        return;
    }

    if (textureIndex == BITMAP_WATER)
    {
        textureIndex = BITMAP_WATER + WaterTextureNumber;
    }

    if (explicitTextureIndex != -1)
    {
        textureIndex = explicitTextureIndex;
    }

    const auto texture = Bitmaps.GetTexture(textureIndex);
    if (texture->IsSkin && HideSkin)
    {
        return;
    }

    if (texture->IsHair && HideSkin)
    {
        return;
    }

    bool EnableWave = false;
    int streamMesh = static_cast<u_char>(this->StreamMesh);
    if (m->m_csTScript != nullptr)
    {
        if (m->m_csTScript->getStreamMesh())
        {
            streamMesh = meshIndex;
        }
    }

    if ((meshIndex == blendMeshIndex || meshIndex == streamMesh)
        && (blendMeshTextureCoordU != 0.f || blendMeshTextureCoordV != 0.f))
    {
        EnableWave = true;
    }

    bool enableLight = LightEnable;
    if (meshIndex == StreamMesh)
    {
        glColor3fv(BodyLight);
        enableLight = false;
    }
    else if (enableLight)
    {
        for (int j = 0; j < m->NumNormals; j++)
        {
            VectorScale(BodyLight, IntensityTransform[meshIndex][j], LightTransform[meshIndex][j]);
        }
    }

    int finalRenderFlags = renderFlags;
    if ((renderFlags & RENDER_COLOR) == RENDER_COLOR)
    {
        finalRenderFlags = RENDER_COLOR;
        if ((renderFlags & RENDER_BRIGHT) == RENDER_BRIGHT)
        {
            EnableAlphaBlend();
        }
        else if ((renderFlags & RENDER_DARK) == RENDER_DARK)
        {
            EnableAlphaBlendMinus();
        }
        else
        {
            DisableAlphaBlend();
        }

        if ((renderFlags & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }

        DisableTexture();
        if (alpha >= 0.99f)
        {
            glColor3fv(BodyLight);
        }
        else
        {
            EnableAlphaTest();
            glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], alpha);
        }
    }
    else if ((renderFlags & RENDER_CHROME) == RENDER_CHROME ||
        (renderFlags & RENDER_CHROME2) == RENDER_CHROME2 ||
        (renderFlags & RENDER_CHROME3) == RENDER_CHROME3 ||
        (renderFlags & RENDER_CHROME4) == RENDER_CHROME4 ||
        (renderFlags & RENDER_CHROME5) == RENDER_CHROME5 ||
        (renderFlags & RENDER_CHROME6) == RENDER_CHROME6 ||
        (renderFlags & RENDER_CHROME7) == RENDER_CHROME7 ||
        (renderFlags & RENDER_METAL) == RENDER_METAL ||
        (renderFlags & RENDER_OIL) == RENDER_OIL
        )
    {
        if (m->m_csTScript != nullptr)
        {
            if (m->m_csTScript->getNoneBlendMesh()) return;
        }

        if (m->NoneBlendMesh)
            return;

        finalRenderFlags = RENDER_CHROME;
        if ((renderFlags & RENDER_CHROME4) == RENDER_CHROME4)
        {
            finalRenderFlags = RENDER_CHROME4;
        }
        if ((renderFlags & RENDER_OIL) == RENDER_OIL)
        {
            finalRenderFlags = RENDER_OIL;
        }

        float Wave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;

        vec3_t L = { (float)(cos(WorldTime * 0.001f)), (float)(sin(WorldTime * 0.002f)), 1.f };
        for (int j = 0; j < m->NumNormals; j++)
        {
            if (j > MAX_VERTICES) break;
            const auto normal = NormalTransform[meshIndex][j];

            if ((renderFlags & RENDER_CHROME2) == RENDER_CHROME2)
            {
                g_chrome[j][0] = (normal[2] + normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (normal[1] + normal[0]) * 1.0f + Wave2 * 3.f;
            }
            else if ((renderFlags & RENDER_CHROME3) == RENDER_CHROME3)
            {
                g_chrome[j][0] = DotProduct(normal, LightVector);
                g_chrome[j][1] = 1.f - DotProduct(normal, LightVector);
            }
            else if ((renderFlags & RENDER_CHROME4) == RENDER_CHROME4)
            {
                g_chrome[j][0] = DotProduct(normal, L);
                g_chrome[j][1] = 1.f - DotProduct(normal, L);
                g_chrome[j][1] -= normal[2] * 0.5f + wave * 3.f;
                g_chrome[j][0] += normal[1] * 0.5f + L[1] * 3.f;
            }
            else if ((renderFlags & RENDER_CHROME5) == RENDER_CHROME5)
            {
                g_chrome[j][0] = DotProduct(normal, L);
                g_chrome[j][1] = 1.f - DotProduct(normal, L);
                g_chrome[j][1] -= normal[2] * 2.5f + wave * 1.f;
                g_chrome[j][0] += normal[1] * 3.f + L[1] * 5.f;
            }
            else if ((renderFlags & RENDER_CHROME6) == RENDER_CHROME6)
            {
                g_chrome[j][0] = (normal[2] + normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (normal[2] + normal[0]) * 0.8f + Wave2 * 2.f;
            }
            else if ((renderFlags & RENDER_CHROME7) == RENDER_CHROME7)
            {
                g_chrome[j][0] = (normal[2] + normal[0]) * 0.8f + static_cast<float>(WorldTime) * 0.00006f;
                g_chrome[j][1] = (normal[2] + normal[0]) * 0.8f + static_cast<float>(WorldTime) * 0.00006f;
            }
            else if ((renderFlags & RENDER_OIL) == RENDER_OIL)
            {
                g_chrome[j][0] = normal[0];
                g_chrome[j][1] = normal[1];
            }
            else if ((renderFlags & RENDER_CHROME) == RENDER_CHROME)
            {
                g_chrome[j][0] = normal[2] * 0.5f + wave;
                g_chrome[j][1] = normal[1] * 0.5f + wave * 2.f;
            }
            else
            {
                g_chrome[j][0] = normal[2] * 0.5f + 0.2f;
                g_chrome[j][1] = normal[1] * 0.5f + 0.5f;
            }
        }

        if ((renderFlags & RENDER_CHROME3) == RENDER_CHROME3
            || (renderFlags & RENDER_CHROME4) == RENDER_CHROME4
            || (renderFlags & RENDER_CHROME5) == RENDER_CHROME5
            || (renderFlags & RENDER_CHROME7) == RENDER_CHROME7
            || (renderFlags & RENDER_BRIGHT) == RENDER_BRIGHT
            )
        {
            if (alpha < 0.99f)
            {
                BodyLight[0] *= alpha;
                BodyLight[1] *= alpha;
                BodyLight[2] *= alpha;
            }

            EnableAlphaBlend();
        }
        else if ((renderFlags & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else if ((renderFlags & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
        else if (alpha >= 0.99f)
        {
            DisableAlphaBlend();
        }
        else
        {
            EnableAlphaTest();
        }

        if ((renderFlags & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }

        if (explicitTextureIndex == -1)
        {
            if ((renderFlags & RENDER_CHROME2) == RENDER_CHROME2)
            {
                BindTexture(BITMAP_CHROME2);
            }
            else if ((renderFlags & RENDER_CHROME3) == RENDER_CHROME3)
            {
                BindTexture(BITMAP_CHROME2);
            }
            else if ((renderFlags & RENDER_CHROME4) == RENDER_CHROME4)
            {
                BindTexture(BITMAP_CHROME2);
            }
            else if ((renderFlags & RENDER_CHROME6) == RENDER_CHROME6)
            {
                BindTexture(BITMAP_CHROME6);
            }
            else if ((renderFlags & RENDER_CHROME) == RENDER_CHROME)
            {
                BindTexture(BITMAP_CHROME);
            }
            else if ((renderFlags & RENDER_METAL) == RENDER_METAL)
            {
                BindTexture(BITMAP_SHINY);
            }
        }
        else
        {
            BindTexture(textureIndex);
        }
    }
    else if (blendMeshIndex <= -2 || m->Texture == blendMeshIndex)
    {
        finalRenderFlags = RENDER_TEXTURE;
        BindTexture(textureIndex);
        if ((renderFlags & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else
            EnableAlphaBlend();

        if ((renderFlags & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }

        glColor3f(BodyLight[0] * blendMeshAlpha, 
            BodyLight[1] * blendMeshAlpha,
            BodyLight[2] * blendMeshAlpha);
        //glColor3f(BlendMeshLight,BlendMeshLight,BlendMeshLight);
        enableLight = false;
    }
    else if ((renderFlags & RENDER_TEXTURE) == RENDER_TEXTURE)
    {
        finalRenderFlags = RENDER_TEXTURE;
        BindTexture(textureIndex);
        if ((renderFlags & RENDER_BRIGHT) == RENDER_BRIGHT)
        {
            EnableAlphaBlend();
        }
        else if ((renderFlags & RENDER_DARK) == RENDER_DARK)
        {
            EnableAlphaBlendMinus();
        }
        else if (alpha < 0.99f || texture->Components == 4)
        {
            EnableAlphaTest();
        }
        else
        {
            DisableAlphaBlend();
        }

        if ((renderFlags & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }
    }
    else if ((renderFlags & RENDER_BRIGHT) == RENDER_BRIGHT)
    {
        if (texture->Components == 4 || m->Texture == blendMeshIndex)
        {
            return;
        }

        finalRenderFlags = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();

        if ((renderFlags & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }
    }
    else
    {
        finalRenderFlags = RENDER_TEXTURE;
    }
    if (renderFlags & RENDER_DOPPELGANGER)
    {
        if (texture->Components != 4)
        {
            EnableCullFace();
            EnableDepthMask();
        }
    }

    bool enableColor = (enableLight && finalRenderFlags == RENDER_TEXTURE)
        || finalRenderFlags == RENDER_CHROME
        || finalRenderFlags == RENDER_CHROME4
        || finalRenderFlags == RENDER_OIL;

    glEnableClientState(GL_VERTEX_ARRAY);
    if (enableColor) glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    auto vertices = RenderArrayVertices;
    auto colors = RenderArrayColors;
    auto texCoords = RenderArrayTexCoords;

    int target_vertex_index = -1;
    for (int j = 0; j < m->NumTriangles; j++)
    {
        const auto triangle = &m->Triangles[j];
        for (int k = 0; k < triangle->Polygon; k++)
        {
            const int source_vertex_index = triangle->VertexIndex[k];
            target_vertex_index++;

            VectorCopy(VertexTransform[meshIndex][source_vertex_index], vertices[target_vertex_index]);

            Vector4(BodyLight[0], BodyLight[1], BodyLight[2], alpha, colors[target_vertex_index]);

            auto texco = m->TexCoords[triangle->TexCoordIndex[k]];
            texCoords[target_vertex_index][0] = texco.TexCoordU;
            texCoords[target_vertex_index][1] = texco.TexCoordV;

            int normalIndex = triangle->NormalIndex[k];
            switch (finalRenderFlags)
            {
                case RENDER_TEXTURE:
                {
                    if (EnableWave)
                    {
                        texCoords[target_vertex_index][0] += blendMeshTextureCoordU;
                        texCoords[target_vertex_index][1] += blendMeshTextureCoordV;
                    }

                    if (enableLight)
                    {
                        auto light = LightTransform[meshIndex][normalIndex];
                        Vector4(light[0], light[1], light[2], alpha, colors[target_vertex_index]);
                    }

                    break;
                }
                case RENDER_CHROME:
                {
                    texCoords[target_vertex_index][0] = g_chrome[normalIndex][0];
                    texCoords[target_vertex_index][1] = g_chrome[normalIndex][1];
                    break;
                }
                case RENDER_CHROME4:
                {
                    texCoords[target_vertex_index][0] = g_chrome[normalIndex][0] + blendMeshTextureCoordU;
                    texCoords[target_vertex_index][1] = g_chrome[normalIndex][1] + blendMeshTextureCoordV;
                    break;
                }
                case RENDER_OIL:
                {
                    texCoords[target_vertex_index][0] = g_chrome[normalIndex][0] * texCoords[target_vertex_index][0] + blendMeshTextureCoordU;
                    texCoords[target_vertex_index][1] = g_chrome[normalIndex][1] * texCoords[target_vertex_index][1] + blendMeshTextureCoordV;
                    break;
                }
            }

            if ((renderFlags & RENDER_SHADOWMAP) == RENDER_SHADOWMAP)
            {
                vec3_t pos;
                VectorSubtract(vertices[target_vertex_index], BodyOrigin, pos);

                pos[0] += pos[2] * (pos[0] + 2000.f) / (pos[2] - 4000.f);
                pos[2] = 5.f;

                VectorAdd(pos, BodyOrigin, pos);
            }
            else if ((renderFlags & RENDER_WAVE) == RENDER_WAVE)
            {
                float time_sin = sinf((float)((int)WorldTime + source_vertex_index * 931) * 0.007f) * 28.0f;
                float* normal = NormalTransform[meshIndex][normalIndex];
                for (int iCoord = 0; iCoord < 3; ++iCoord)
                {
                    vertices[target_vertex_index][iCoord] += normal[iCoord] * time_sin;
                }
            }
        }
    }

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    if (enableColor) glColorPointer(4, GL_FLOAT, 0, colors);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    glDrawArrays(GL_TRIANGLES, 0, m->NumTriangles * 3);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    if (enableColor) glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void BMD::RenderMeshAlternative(int iRndExtFlag, int iParam, int i, int RenderFlag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int MeshTexture)
{
    if (i >= NumMeshs || i < 0) return;

    Mesh_t* m = &Meshs[i];
    if (m->NumTriangles == 0) return;
    float Wave = (int)WorldTime % 10000 * 0.0001f;

    int Texture = IndexTexture[m->Texture];
    if (Texture == BITMAP_HIDE)
        return;
    if (MeshTexture != -1)
        Texture = MeshTexture;

    BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

    bool EnableWave = false;
    int streamMesh = StreamMesh;
    if (m->m_csTScript != nullptr)
    {
        if (m->m_csTScript->getStreamMesh())
        {
            streamMesh = i;
        }
    }
    if ((i == BlendMesh || i == streamMesh) && (BlendMeshTexCoordU != 0.f || BlendMeshTexCoordV != 0.f))
        EnableWave = true;

    bool EnableLight = LightEnable;
    if (i == StreamMesh)
    {
        //vec3_t Light;
        //Vector(1.f,1.f,1.f,Light);
        glColor3fv(BodyLight);
        EnableLight = false;
    }
    else if (EnableLight)
    {
        for (int j = 0; j < m->NumNormals; j++)
        {
            VectorScale(BodyLight, IntensityTransform[i][j], LightTransform[i][j]);
        }
    }

    int Render = RenderFlag;
    if ((RenderFlag & RENDER_COLOR) == RENDER_COLOR)
    {
        Render = RENDER_COLOR;
        if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
            EnableAlphaBlend();
        else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else
            DisableAlphaBlend();

        if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }

        DisableTexture();
        if (Alpha >= 0.99f)
        {
            glColor3fv(BodyLight);
        }
        else
        {
            EnableAlphaTest();
            glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
        }
    }
    else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME ||
        (RenderFlag & RENDER_CHROME2) == RENDER_CHROME2 ||
        (RenderFlag & RENDER_CHROME3) == RENDER_CHROME3 ||
        (RenderFlag & RENDER_CHROME4) == RENDER_CHROME4 ||
        (RenderFlag & RENDER_CHROME5) == RENDER_CHROME5 ||
        (RenderFlag & RENDER_CHROME7) == RENDER_CHROME7 ||
        (RenderFlag & RENDER_METAL) == RENDER_METAL ||
        (RenderFlag & RENDER_OIL) == RENDER_OIL
        )
    {
        if (m->m_csTScript != nullptr)
        {
            if (m->m_csTScript->getNoneBlendMesh()) return;
        }
        if (m->NoneBlendMesh)
            return;
        Render = RENDER_CHROME;
        if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4)
        {
            Render = RENDER_CHROME4;
        }
        float Wave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;

        vec3_t L = { (float)(cos(WorldTime * 0.001f)), (float)(sin(WorldTime * 0.002f)), 1.f };
        for (int j = 0; j < m->NumNormals; j++)
        {
            if (j > MAX_VERTICES) break;
            float* Normal = NormalTransform[i][j];

            if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2)
            {
                g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
            }
            else if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3)
            {
                g_chrome[j][0] = DotProduct(Normal, LightVector);
                g_chrome[j][1] = 1.f - DotProduct(Normal, LightVector);
            }
            else if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4)
            {
                g_chrome[j][0] = DotProduct(Normal, L);
                g_chrome[j][1] = 1.f - DotProduct(Normal, L);
                g_chrome[j][1] -= Normal[2] * 0.5f + Wave * 3.f;
                g_chrome[j][0] += Normal[1] * 0.5f + L[1] * 3.f;
            }
            else if ((RenderFlag & RENDER_CHROME5) == RENDER_CHROME5)
            {
                Vector(0.1f, -0.23f, 0.22f, LightVector2);

                g_chrome[j][0] = (DotProduct(Normal, LightVector2) /*+ Normal[1] + LightVector2[1]*3.f */) / 1.08f;
                g_chrome[j][1] = (1.f - DotProduct(Normal, LightVector2) /*- Normal[2]*0.5f + 3.f */) / 1.08f;
            }
            else if ((RenderFlag & RENDER_CHROME6) == RENDER_CHROME6)
            {
                g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
            }
            else if ((RenderFlag & RENDER_CHROME7) == RENDER_CHROME7)
            {
                Vector(0.1f, -0.23f, 0.22f, LightVector2);

                g_chrome[j][0] = (DotProduct(Normal, LightVector2)) / 1.08f;
                g_chrome[j][1] = (1.f - DotProduct(Normal, LightVector2)) / 1.08f;
            }
            else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME)
            {
                g_chrome[j][0] = Normal[2] * 0.5f + Wave;
                g_chrome[j][1] = Normal[1] * 0.5f + Wave * 2.f;
            }
            else
            {
                g_chrome[j][0] = Normal[2] * 0.5f + 0.2f;
                g_chrome[j][1] = Normal[1] * 0.5f + 0.5f;
            }
        }

        if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3
            || (RenderFlag & RENDER_CHROME4) == RENDER_CHROME4
            || (RenderFlag & RENDER_CHROME5) == RENDER_CHROME5
            || (RenderFlag & RENDER_CHROME7) == RENDER_CHROME7
            )
        {
            if (Alpha < 0.99f)
            {
                BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
            }
            EnableAlphaBlend();
        }
        else if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
        {
            if (Alpha < 0.99f)
            {
                BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
            }
            EnableAlphaBlend();
        }
        else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else if ((RenderFlag & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
        else if (Alpha >= 0.99f)
        {
            DisableAlphaBlend();
        }
        else
        {
            EnableAlphaTest();
        }

        if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }

        if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2 && MeshTexture == -1)
        {
            BindTexture(BITMAP_CHROME2);
        }
        else if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3 && MeshTexture == -1)
        {
            BindTexture(BITMAP_CHROME2);
        }
        else if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4 && MeshTexture == -1)
        {
            BindTexture(BITMAP_CHROME2);
        }
        else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME && MeshTexture == -1)
            BindTexture(BITMAP_CHROME);
        else if ((RenderFlag & RENDER_METAL) == RENDER_METAL && MeshTexture == -1)
            BindTexture(BITMAP_SHINY);
        else
            BindTexture(Texture);
    }
    else if (BlendMesh <= -2 || m->Texture == BlendMesh)
    {
        Render = RENDER_TEXTURE;
        BindTexture(Texture);
        if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else
            EnableAlphaBlend();

        if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }

        glColor3f(BodyLight[0] * BlendMeshLight, BodyLight[1] * BlendMeshLight, BodyLight[2] * BlendMeshLight);
        //glColor3f(BlendMeshLight,BlendMeshLight,BlendMeshLight);
        EnableLight = false;
    }
    else if ((RenderFlag & RENDER_TEXTURE) == RENDER_TEXTURE)
    {
        Render = RENDER_TEXTURE;
        BindTexture(Texture);
        if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
        {
            EnableAlphaBlend();
        }
        else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
        {
            EnableAlphaBlendMinus();
        }
        else if (Alpha < 0.99f || pBitmap->Components == 4)
        {
            EnableAlphaTest();
        }
        else
        {
            DisableAlphaBlend();
        }

        if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }
    }
    else if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
    {
        if (pBitmap->Components == 4 || m->Texture == BlendMesh)
        {
            return;
        }
        Render = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();

        if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
        {
            DisableDepthTest();
        }
    }
    else
    {
        Render = RENDER_TEXTURE;
    }

    // ver 1.0 (triangle)
    glBegin(GL_TRIANGLES);
    for (int j = 0; j < m->NumTriangles; j++)
    {
        Triangle_t* tp = &m->Triangles[j];
        for (int k = 0; k < tp->Polygon; k++)
        {
            int vi = tp->VertexIndex[k];
            switch (Render)
            {
            case RENDER_TEXTURE:
            {
                TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
                if (EnableWave)
                    glTexCoord2f(texp->TexCoordU + BlendMeshTexCoordU, texp->TexCoordV + BlendMeshTexCoordV);
                else
                    glTexCoord2f(texp->TexCoordU, texp->TexCoordV);
                if (EnableLight)
                {
                    int ni = tp->NormalIndex[k];
                    if (Alpha >= 0.99f)
                    {
                        glColor3fv(LightTransform[i][ni]);
                    }
                    else
                    {
                        float* Light = LightTransform[i][ni];
                        glColor4f(Light[0], Light[1], Light[2], Alpha);
                    }
                }
                break;
            }
            case RENDER_CHROME:
            {
                if (Alpha >= 0.99f)
                    glColor3fv(BodyLight);
                else
                    glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
                int ni = tp->NormalIndex[k];
                glTexCoord2f(g_chrome[ni][0], g_chrome[ni][1]);
                break;
            }
            }
            if ((iRndExtFlag & RNDEXT_WAVE))
            {
                float vPos[3];
                float fParam = (float)((int)WorldTime + vi * 931) * 0.007f;
                float fSin = sinf(fParam);
                int ni = tp->NormalIndex[k];
                float* Normal = NormalTransform[i][ni];
                for (int iCoord = 0; iCoord < 3; ++iCoord)
                {
                    vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord] * fSin * 28.0f;
                }
                glVertex3fv(vPos);
            }
            else
            {
                glVertex3fv(VertexTransform[i][vi]);
            }
        }
    }
    glEnd();
}

void BMD::RenderMeshEffect(int i, int iType, int iSubType, vec3_t Angle, VOID* obj)
{
    if (i >= NumMeshs || i < 0) return;

    Mesh_t* m = &Meshs[i];
    if (m->NumTriangles <= 0) return;

    vec3_t angle, Light;
    int iEffectCount = 0;

    Vector(0.f, 0.f, 0.f, angle);
    Vector(1.f, 1.f, 1.f, Light);
    for (int j = 0; j < m->NumTriangles; j++)
    {
        Triangle_t* tp = &m->Triangles[j];
        for (int k = 0; k < tp->Polygon; k++)
        {
            int vi = tp->VertexIndex[k];

            switch (iType)
            {
            case MODEL_STONE_COFFIN:
                if (iSubType == 0)
                {
                    if (rand_fps_check(2))
                    {
                        CreateEffect(MODEL_STONE_COFFIN + 1, VertexTransform[i][vi], angle, Light);
                    }
                    if (rand_fps_check(10))
                    {
                        CreateEffect(MODEL_STONE_COFFIN, VertexTransform[i][vi], angle, Light);
                    }
                }
                else if (iSubType == 1)
                {
                    CreateEffect(MODEL_STONE_COFFIN + 1, VertexTransform[i][vi], angle, Light, 2);
                }
                else if (iSubType == 2)
                {
                    CreateEffect(MODEL_STONE_COFFIN + 1, VertexTransform[i][vi], angle, Light, 3);
                }
                else if (iSubType == 3)
                {
                    CreateEffect(MODEL_STONE_COFFIN + rand() % 2, VertexTransform[i][vi], angle, Light, 4);
                }
                break;
            case MODEL_GATE:
                if (iSubType == 1)
                {
                    Vector(0.2f, 0.2f, 0.2f, Light);
                    if (rand_fps_check(5))
                    {
                        CreateEffect(MODEL_GATE + 1, VertexTransform[i][vi], angle, Light, 2);
                    }
                    if (rand_fps_check(10))
                    {
                        CreateEffect(MODEL_GATE, VertexTransform[i][vi], angle, Light, 2);
                    }
                }
                else if (iSubType == 0)
                {
                    Vector(0.2f, 0.2f, 0.2f, Light);
                    if (rand_fps_check(12))
                    {
                        CreateEffect(MODEL_GATE + 1, VertexTransform[i][vi], angle, Light);
                    }
                    if (rand_fps_check(50))
                    {
                        CreateEffect(MODEL_GATE, VertexTransform[i][vi], angle, Light);
                    }
                }
                break;
            case MODEL_BIG_STONE_PART1:
                if (rand_fps_check(3))
                {
                    CreateEffect(MODEL_BIG_STONE_PART1 + rand() % 2, VertexTransform[i][vi], angle, Light, 1);
                }
                break;

            case MODEL_BIG_STONE_PART2:
                if (rand_fps_check(3))
                {
                    CreateEffect(MODEL_BIG_STONE_PART1 + rand() % 2, VertexTransform[i][vi], angle, Light);
                }
                break;

            case MODEL_WALL_PART1:
                if (rand_fps_check(3))
                {
                    CreateEffect(MODEL_WALL_PART1 + rand() % 2, VertexTransform[i][vi], angle, Light);
                }
                break;

            case MODEL_GATE_PART1:
                Vector(0.2f, 0.2f, 0.2f, Light);
                if (rand_fps_check(12))
                {
                    CreateEffect(MODEL_GATE_PART1 + 1, VertexTransform[i][vi], angle, Light);
                }
                if (rand_fps_check(40))
                {
                    CreateEffect(MODEL_GATE_PART1, VertexTransform[i][vi], angle, Light);
                }
                if (rand_fps_check(40))
                {
                    CreateEffect(MODEL_GATE_PART1 + 2, VertexTransform[i][vi], angle, Light);
                }
                break;
            case MODEL_GOLEM_STONE:
                if (rand_fps_check(45) && iEffectCount < 20)
                {
                    if (iSubType == 0) {	//. 불골렘
                        CreateEffect(MODEL_GOLEM_STONE, VertexTransform[i][vi], angle, Light);
                    }
                    else if (iSubType == 1) {	//. 독골렘
                        CreateEffect(MODEL_BIG_STONE_PART1, VertexTransform[i][vi], angle, Light, 2);
                        CreateEffect(MODEL_BIG_STONE_PART2, VertexTransform[i][vi], angle, Light, 2);
                    }
                    iEffectCount++;
                }
                break;
            case MODEL_SKIN_SHELL:
                if (rand_fps_check(8))
                {
                    CreateEffect(MODEL_SKIN_SHELL, VertexTransform[i][vi], angle, Light, iSubType);
                }
                break;
            case BITMAP_LIGHT:
                Vector(0.08f, 0.08f, 0.08f, Light);
                if (iSubType == 0)
                {
                    CreateSprite(BITMAP_LIGHT, VertexTransform[i][vi], BodyScale, Light, nullptr);
                }
                else if (iSubType == 1)
                {
                    Vector(1.f, 0.8f, 0.2f, Light);
                    if ((j % 22) == 0)
                    {
                        auto* o = (OBJECT*)obj;

                        angle[0] = -(float)(rand() % 90);
                        angle[1] = 0.f;
                        angle[2] = Angle[2] + (float)(rand() % 120 - 60);
                        CreateJoint(BITMAP_JOINT_SPIRIT, VertexTransform[i][vi], o->Position, angle, 13, o, 20.f, 0, 0);
                    }
                }
                break;
            case BITMAP_BUBBLE:
                Vector(1.f, 1.f, 1.f, Light);
                if (rand_fps_check(30))
                {
                    CreateParticle(BITMAP_BUBBLE, VertexTransform[i][vi], angle, Light, 2);
                }
                break;
            }
        }
    }
}

void BMD::RenderBody(int Flag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int HiddenMesh, int Texture)
{
    if (NumMeshs == 0) return;

    int iBlendMesh = BlendMesh;
    BeginRender(Alpha);
    if (!LightEnable)
    {
        if (Alpha >= 0.99f)
            glColor3fv(BodyLight);
        else
            glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
    }
    for (int i = 0; i < NumMeshs; i++)
    {
        iBlendMesh = BlendMesh;

        Mesh_t* m = &Meshs[i];
        if (m->m_csTScript != nullptr)
        {
            if (m->m_csTScript->getHiddenMesh() == false && i != HiddenMesh)
            {
                if (m->m_csTScript->getBright())
                {
                    iBlendMesh = i;
                }
                RenderMesh(i, Flag, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);

                BYTE shadowType = m->m_csTScript->getShadowMesh();
                if (shadowType == SHADOW_RENDER_COLOR)
                {
                    DisableAlphaBlend();
                    if (Alpha >= 0.99f)
                        glColor3f(0.f, 0.f, 0.f);
                    else
                        glColor4f(0.f, 0.f, 0.f, Alpha);

                    RenderMesh(i, RENDER_COLOR | RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV);
                    glColor3f(1.f, 1.f, 1.f);
                }
                else if (shadowType == SHADOW_RENDER_TEXTURE)
                {
                    DisableAlphaBlend();
                    if (Alpha >= 0.99f)
                        glColor3f(0.f, 0.f, 0.f);
                    else
                        glColor4f(0.f, 0.f, 0.f, Alpha);

                    RenderMesh(i, RENDER_TEXTURE | RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV);
                    glColor3f(1.f, 1.f, 1.f);
                }
            }
        }
        else
        {
            if (i != HiddenMesh)
            {
                RenderMesh(i, Flag, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);
            }
        }
    }
    EndRender();
}

void BMD::RenderBodyAlternative(int iRndExtFlag, int iParam, int Flag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int HiddenMesh, int Texture)
{
    if (NumMeshs == 0) return;

    BeginRender(Alpha);
    if (!LightEnable)
    {
        if (Alpha >= 0.99f)
            glColor3fv(BodyLight);
        else
            glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
    }
    for (int i = 0; i < NumMeshs; i++)
    {
        if (i != HiddenMesh)
        {
            RenderMeshAlternative(iRndExtFlag, iParam, i, Flag, Alpha, BlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);
        }
    }
    EndRender();
}

void BMD::RenderMeshTranslate(int i, int RenderFlag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int MeshTexture)
{
    if (i >= NumMeshs || i < 0) return;

    Mesh_t* m = &Meshs[i];
    if (m->NumTriangles == 0) return;
    float Wave = (int)WorldTime % 10000 * 0.0001f;

    int Texture = IndexTexture[m->Texture];
    if (Texture == BITMAP_HIDE)
        return;
    if (Texture == BITMAP_SKIN)
    {
        if (HideSkin) return;
        Texture = BITMAP_SKIN + Skin;
    }
    else if (Texture == BITMAP_WATER)
    {
        Texture = BITMAP_WATER + WaterTextureNumber;
    }
    if (MeshTexture != -1)
        Texture = MeshTexture;

    BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

    bool EnableWave = false;
    int streamMesh = StreamMesh;
    if (m->m_csTScript != nullptr)
    {
        if (m->m_csTScript->getStreamMesh())
        {
            streamMesh = i;
        }
    }
    if ((i == BlendMesh || i == streamMesh) && (BlendMeshTexCoordU != 0.f || BlendMeshTexCoordV != 0.f))
        EnableWave = true;

    bool EnableLight = LightEnable;
    if (i == StreamMesh)
    {
        //vec3_t Light;
        //Vector(1.f,1.f,1.f,Light);
        glColor3fv(BodyLight);
        EnableLight = false;
    }
    else if (EnableLight)
    {
        for (int j = 0; j < m->NumNormals; j++)
        {
            VectorScale(BodyLight, IntensityTransform[i][j], LightTransform[i][j]);
        }
    }

    int Render = RenderFlag;
    if ((RenderFlag & RENDER_COLOR) == RENDER_COLOR)
    {
        Render = RENDER_COLOR;
        if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
            EnableAlphaBlend();
        else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else
            DisableAlphaBlend();
        DisableTexture();
        glColor3fv(BodyLight);
    }
    else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME
        || (RenderFlag & RENDER_METAL) == RENDER_METAL
        || (RenderFlag & RENDER_CHROME2) == RENDER_CHROME2
        || (RenderFlag & RENDER_CHROME6) == RENDER_CHROME6
        )
    {
        if (m->m_csTScript != nullptr)
        {
            if (m->m_csTScript->getNoneBlendMesh()) return;
        }
        if (m->NoneBlendMesh)
            return;
        Render = RENDER_CHROME;

        float Wave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;

        for (int j = 0; j < m->NumNormals; j++)
        {
            //			Normal_t *np = &m->Normals[j];
            if (j > MAX_VERTICES) break;
            float* Normal = NormalTransform[i][j];

            if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2)
            {
                g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
            }
            else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME)
            {
                g_chrome[j][0] = Normal[2] * 0.5f + Wave;
                g_chrome[j][1] = Normal[1] * 0.5f + Wave * 2.f;
            }
            else if ((RenderFlag & RENDER_CHROME6) == RENDER_CHROME6)
            {
                g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
            }
            else
            {
                g_chrome[j][0] = Normal[2] * 0.5f + 0.2f;
                g_chrome[j][1] = Normal[1] * 0.5f + 0.5f;
            }
        }

        if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
            EnableAlphaBlend();
        else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else if ((RenderFlag & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
        else
            DisableAlphaBlend();

        if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2 && MeshTexture == -1)
        {
            BindTexture(BITMAP_CHROME2);
        }
        else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME && MeshTexture == -1)
            BindTexture(BITMAP_CHROME);
        else if ((RenderFlag & RENDER_METAL) == RENDER_METAL && MeshTexture == -1)
            BindTexture(BITMAP_SHINY);
        else
            BindTexture(Texture);
    }
    else if (BlendMesh <= -2 || m->Texture == BlendMesh)
    {
        Render = RENDER_TEXTURE;
        BindTexture(Texture);
        if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
            EnableAlphaBlendMinus();
        else
            EnableAlphaBlend();
        glColor3f(BodyLight[0] * BlendMeshLight, BodyLight[1] * BlendMeshLight, BodyLight[2] * BlendMeshLight);
        //glColor3f(BlendMeshLight,BlendMeshLight,BlendMeshLight);
        EnableLight = false;
    }
    else if ((RenderFlag & RENDER_TEXTURE) == RENDER_TEXTURE)
    {
        Render = RENDER_TEXTURE;
        BindTexture(Texture);
        if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
        {
            EnableAlphaBlend();
        }
        else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
        {
            EnableAlphaBlendMinus();
        }
        else if (Alpha < 0.99f || pBitmap->Components == 4)
        {
            EnableAlphaTest();
        }
        else
        {
            DisableAlphaBlend();
        }
    }
    else if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
    {
        if (pBitmap->Components == 4 || m->Texture == BlendMesh)
        {
            return;
        }
        Render = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();
    }
    else
    {
        Render = RENDER_TEXTURE;
    }

    glBegin(GL_TRIANGLES);
    for (int j = 0; j < m->NumTriangles; j++)
    {
        vec3_t  pos;
        Triangle_t* tp = &m->Triangles[j];
        for (int k = 0; k < tp->Polygon; k++)
        {
            int vi = tp->VertexIndex[k];
            switch (Render)
            {
            case RENDER_TEXTURE:
            {
                TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
                if (EnableWave)
                    glTexCoord2f(texp->TexCoordU + BlendMeshTexCoordU, texp->TexCoordV + BlendMeshTexCoordV);
                else
                    glTexCoord2f(texp->TexCoordU, texp->TexCoordV);
                if (EnableLight)
                {
                    int ni = tp->NormalIndex[k];
                    if (Alpha >= 0.99f)
                    {
                        glColor3fv(LightTransform[i][ni]);
                    }
                    else
                    {
                        float* Light = LightTransform[i][ni];
                        glColor4f(Light[0], Light[1], Light[2], Alpha);
                    }
                }
                break;
            }
            case RENDER_CHROME:
            {
                if (Alpha >= 0.99f)
                    glColor3fv(BodyLight);
                else
                    glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
                int ni = tp->NormalIndex[k];
                glTexCoord2f(g_chrome[ni][0], g_chrome[ni][1]);
                break;
            }
            }
            {
                VectorAdd(VertexTransform[i][vi], BodyOrigin, pos);
                glVertex3fv(pos);
            }
        }
    }
    glEnd();
}

void BMD::RenderBodyTranslate(int Flag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int HiddenMesh, int Texture)
{
    if (NumMeshs == 0) return;

    BeginRender(Alpha);
    if (!LightEnable)
    {
        if (Alpha >= 0.99f)
            glColor3fv(BodyLight);
        else
            glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
    }
    for (int i = 0; i < NumMeshs; i++)
    {
        if (i != HiddenMesh)
        {
            RenderMeshTranslate(i, Flag, Alpha, BlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);
        }
    }
    EndRender();
}

__forceinline void CalcShadowPosition(vec3_t* position, const vec3_t origin, const float sx, const float sy)
{
    vec3_t result;
    VectorCopy(*position, result);

    // Subtract the origin (position of the character) from the current position of the vertex
    // The result is the relative coordinate of the vertex to the origin.
    VectorSubtract(result, origin, result)

    // scale the shadow in the x direction
    result[0] += result[2] * (result[0] + sx) / (result[2] - sy);

    // Add the origin again, to get the absolute coordinate of the vertex again
    VectorAdd(result, origin, result);

    // put it on the ground by adding 5 to the actual ground coordinate.
    result[2] = RequestTerrainHeight(result[0], result[1]) + 5.f;

    // copy to result
    VectorCopy(result, *position);
}

__forceinline void GetClothShadowPosition(vec3_t* target, CPhysicsCloth* pCloth, const int index, const vec3_t origin, const float sx, const float sy)
{
    pCloth->GetPosition(index, target);
    CalcShadowPosition(target, origin, sx, sy);
}

void BMD::AddClothesShadowTriangles(void* pClothes, const int clothesCount, const float sx, const float sy) const
{
    auto vertices = RenderArrayVertices;
    int target_vertex_index = -1;
    
    for (int i = 0; i < clothesCount; i++)
    {
        auto* const pCloth = &static_cast<CPhysicsCloth*>(pClothes)[i];
        auto const columns = pCloth->GetVerticalCount();
        auto const rows = pCloth->GetHorizontalCount();
        
        for (int col = 0; col < columns - 1; ++col)
        {
            for (int row = 0; row < rows - 1; ++row)
            {
                // first we take each point for an square from which we derive
                // a A-Triangle and the V-Triangle.
                int a = rows * col + row;
                int b = rows * (col + 1) + row;
                int c = rows * col + row + 1;
                int d = rows * (col + 1) + row + 1;

                vec3_t posA, posB, posC, posD;

                GetClothShadowPosition(&posA, pCloth, a, BodyOrigin, sx, sy);
                GetClothShadowPosition(&posB, pCloth, b, BodyOrigin, sx, sy);
                GetClothShadowPosition(&posC, pCloth, c, BodyOrigin, sx, sy);
                GetClothShadowPosition(&posD, pCloth, d, BodyOrigin, sx, sy);

                // A-Triangle:
                target_vertex_index++;
                VectorCopy(posA, vertices[target_vertex_index]);
                target_vertex_index++;
                VectorCopy(posB, vertices[target_vertex_index]);
                target_vertex_index++;
                VectorCopy(posC, vertices[target_vertex_index]);

                // V-Triangle:
                target_vertex_index++;
                VectorCopy(posD, vertices[target_vertex_index]);
                target_vertex_index++;
                VectorCopy(posB, vertices[target_vertex_index]);
                target_vertex_index++;
                VectorCopy(posC, vertices[target_vertex_index]);
            }
        }
    }

    if (target_vertex_index < 0)
    {
        return;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, target_vertex_index + 1);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void BMD::AddMeshShadowTriangles(const int blendMesh, const int hiddenMesh, const int startMesh, const int endMesh, const float sx, const float sy) const
{
    auto vertices = RenderArrayVertices;
    int target_vertex_index = -1;

    for (int i = startMesh; i < endMesh; i++)
    {
        if (i == hiddenMesh)
        {
            continue;
        }

        const Mesh_t* mesh = &Meshs[i];
        if (mesh->NumTriangles <= 0 || mesh->Texture == blendMesh)
        {
            continue;
        }

        for (int j = 0; j < mesh->NumTriangles; j++)
        {
            const auto* tp = &mesh->Triangles[j];
            for (int k = 0; k < tp->Polygon; k++)
            {
                const int source_vertex_index = tp->VertexIndex[k];
                target_vertex_index++;

                VectorCopy(VertexTransform[i][source_vertex_index], vertices[target_vertex_index]);
                
                CalcShadowPosition(&vertices[target_vertex_index], BodyOrigin, sx, sy);
            }
        }
    }

    if (target_vertex_index < 0)
    {
        return;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, target_vertex_index + 1);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void BMD::RenderBodyShadow(const int blendMesh, const int hiddenMesh, const int startMeshNumber, const int endMeshNumber, void* pClothes, const int clothesCount)
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    if (NumMeshs == 0 && clothesCount == 0)
    {
        return;
    }

    EnableAlphaTest(false);

    glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // 50% opacity for shadows

    DisableTexture();
    DisableDepthMask();
    BeginRender(1.f);

    // enable stencil and continue draw
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    int startMesh = 0;
    int endMesh = NumMeshs;

    if (startMeshNumber != -1)
    {
        startMesh = startMeshNumber;
    }

    if (endMeshNumber != -1)
    {
        endMesh = endMeshNumber;
    }

    const float sx = gMapManager.InBattleCastle() ? 2500.f : 2000.f;
    const float sy = 4000.f;

    if (clothesCount == 0)
    {
        AddMeshShadowTriangles(blendMesh, hiddenMesh, startMesh, endMesh, sx, sy);
    }
    else
    {
        AddClothesShadowTriangles(pClothes, clothesCount, sx, sy);
    }

    EndRender();
    EnableDepthMask();

    glDisable(GL_STENCIL_TEST);
}

void BMD::RenderObjectBoundingBox()
{
    DisableTexture();
    glPushMatrix();
    glTranslatef(BodyOrigin[0], BodyOrigin[1], BodyOrigin[2]);
    glScalef(BodyScale, BodyScale, BodyScale);
    for (int i = 0; i < NumBones; i++)
    {
        Bone_t* b = &Bones[i];
        if (b->BoundingBox)
        {
            vec3_t BoundingVertices[8];
            for (int j = 0; j < 8; j++)
            {
                VectorTransform(b->BoundingVertices[j], BoneTransform[i], BoundingVertices[j]);
            }

            glBegin(GL_QUADS);
            glColor3f(0.2f, 0.2f, 0.2f);
            glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
            glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
            glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);
            glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[5]);

            glColor3f(0.2f, 0.2f, 0.2f);
            glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
            glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[2]);
            glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
            glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);

            glColor3f(0.6f, 0.6f, 0.6f);
            glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
            glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
            glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);
            glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[6]);

            glColor3f(0.6f, 0.6f, 0.6f);
            glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
            glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[1]);
            glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
            glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);

            glColor3f(0.4f, 0.4f, 0.4f);
            glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
            glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
            glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);
            glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[3]);

            glColor3f(0.4f, 0.4f, 0.4f);
            glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
            glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[4]);
            glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
            glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);
            glEnd();
        }
    }
    glPopMatrix();
    DisableAlphaBlend();
}

void BMD::RenderBone(float(*BoneMatrix)[3][4])
{
    DisableTexture();
    glDepthFunc(GL_ALWAYS);
    glColor3f(0.8f, 0.8f, 0.2f);
    for (int i = 0; i < NumBones; i++)
    {
        Bone_t* b = &Bones[i];
        if (!b->Dummy)
        {
            BoneMatrix_t* bm = &b->BoneMatrixes[CurrentAction];
            int Parent = b->Parent;
            if (Parent > 0)
            {
                float Scale = 1.f;
                float dx = bm->Position[CurrentAnimationFrame][0];
                float dy = bm->Position[CurrentAnimationFrame][1];
                float dz = bm->Position[CurrentAnimationFrame][2];
                Scale = sqrtf(dx * dx + dy * dy + dz * dz) * 0.05f;
                vec3_t Position[3];
                Vector(0.f, 0.f, -Scale, Position[0]);
                Vector(0.f, 0.f, Scale, Position[1]);
                Vector(0.f, 0.f, 0.f, Position[2]);
                vec3_t BoneVertices[3];
                VectorTransform(Position[0], BoneMatrix[Parent], BoneVertices[0]);
                VectorTransform(Position[1], BoneMatrix[Parent], BoneVertices[1]);
                VectorTransform(Position[2], BoneMatrix[i], BoneVertices[2]);
                for (auto & BoneVertice : BoneVertices)
                {
                    VectorMA(BodyOrigin, BodyScale, BoneVertice, BoneVertice);
                }
                glBegin(GL_LINES);
                glVertex3fv(BoneVertices[0]);
                glVertex3fv(BoneVertices[1]); 
                glVertex3fv(BoneVertices[1]);
                glVertex3fv(BoneVertices[2]);
                glVertex3fv(BoneVertices[2]);
                glVertex3fv(BoneVertices[0]);
                glEnd();
            }
        }
    }
    glDepthFunc(GL_LEQUAL);
}

void BMD::Release()
{
    if (Bones)
    {
        for (int i = 0; i < NumBones; ++i)
        {
            Bone_t* b = &Bones[i];

            if (!b->Dummy && b->BoneMatrixes)
            {
                for (int j = 0; j < NumActions; ++j)
                {
                    BoneMatrix_t* bm = &b->BoneMatrixes[j];
                    if (bm)
                    {
                        if (bm->Position) {
							delete[] bm->Position;
							bm->Position = nullptr;
                        }
                        if (bm->Rotation) {
							delete[] bm->Rotation;
							bm->Rotation = nullptr;
                        }
						if (bm->Quaternion) {
							delete[] bm->Quaternion;
							bm->Quaternion = nullptr;

                        }
                    }
                }
                if (b->BoneMatrixes) { delete[] b->BoneMatrixes; b->BoneMatrixes = nullptr; }
            }
        }
    }

    if (Actions)
    {
        for (int i = 0; i < NumActions; ++i)
        {
            Action_t* a = &Actions[i];
            if (a && a->LockPositions && a->Positions)
            {
                delete[] a->Positions;
                a->Positions = nullptr;
            }
        }
    }

    if (Meshs)
    {
        for (int i = 0; i < NumMeshs; ++i)
        {
            Mesh_t* m = &Meshs[i];

            if (m->Vertices) { delete[] m->Vertices; m->Vertices = nullptr; }
            if (m->Normals) { delete[] m->Normals; m->Normals = nullptr; }
            if (m->TexCoords) { delete[] m->TexCoords; m->TexCoords = nullptr; }
            if (m->Triangles) { delete[] m->Triangles; m->Triangles = nullptr; }

            if (m->m_csTScript)
            {
                delete m->m_csTScript;
                m->m_csTScript = nullptr;
            }

            if (IndexTexture && m->Texture >= 0)
            {
                auto textureIndex = IndexTexture[m->Texture];
                if (textureIndex >= BITMAP_SKIN_BEGIN && textureIndex <= BITMAP_SKIN_END)
                    continue;

                DeleteBitmap(textureIndex);
            }
        }
    }

    if (Meshs) { delete[] Meshs; Meshs = nullptr; }
    if (Bones) { delete[] Bones; Bones = nullptr; }
    if (Actions) { delete[] Actions; Actions = nullptr; }
    if (Textures) { delete[] Textures; Textures = nullptr; }
    if (IndexTexture) { delete[] IndexTexture; IndexTexture = nullptr; }

    NumBones = 0;
    NumActions = 0;
    NumMeshs = 0;

#ifdef LDS_FIX_SETNULLALLOCVALUE_WHEN_BMDRELEASE
    m_bCompletedAlloc = false;
#endif
}

void BMD::FindNearTriangle()
{
    for (int iMesh = 0; iMesh < NumMeshs; iMesh++)
    {
        Mesh_t* m = &Meshs[iMesh];

        Triangle_t* pTriangle = m->Triangles;
        int iNumTriangles = m->NumTriangles;
        for (int iTri = 0; iTri < iNumTriangles; ++iTri)
        {
            for (int i = 0; i < 3; ++i)
            {
                pTriangle[iTri].EdgeTriangleIndex[i] = -1;
            }
        }
        for (int iTri = 0; iTri < iNumTriangles; ++iTri)
        {
            FindTriangleForEdge(iMesh, iTri, 0);
            FindTriangleForEdge(iMesh, iTri, 1);
            FindTriangleForEdge(iMesh, iTri, 2);
        }
    }
}

void BMD::FindTriangleForEdge(int iMesh, int iTri1, int iIndex11)
{
    if (iMesh >= NumMeshs || iMesh < 0) return;

    Mesh_t* m = &Meshs[iMesh];
    Triangle_t* pTriangle = m->Triangles;

    Triangle_t* pTri1 = &pTriangle[iTri1];
    if (pTri1->EdgeTriangleIndex[iIndex11] != -1)
    {
        return;
    }

    int iNumTriangles = m->NumTriangles;
    for (int iTri2 = 0; iTri2 < iNumTriangles; ++iTri2)
    {
        if (iTri1 == iTri2)
        {
            continue;
        }

        Triangle_t* pTri2 = &pTriangle[iTri2];
        int iIndex12 = (iIndex11 + 1) % 3;
        for (int iIndex21 = 0; iIndex21 < 3; ++iIndex21)
        {
            int iIndex22 = (iIndex21 + 1) % 3;
            if (pTri2->EdgeTriangleIndex[iIndex21] == -1 &&
                pTri1->VertexIndex[iIndex11] == pTri2->VertexIndex[iIndex22] &&
                pTri1->VertexIndex[iIndex12] == pTri2->VertexIndex[iIndex21])
            {
                pTri1->EdgeTriangleIndex[iIndex11] = iTri2;
                pTri2->EdgeTriangleIndex[iIndex21] = iTri1;
                return;
            }
        }
    }
}
//#endif //USE_SHADOWVOLUME


class BMDReader {
public:
    BMDReader(unsigned char* data, size_t size) : data(data), size(size), ptr(0) {}

    void Skip(size_t bytes) { ptr += bytes; }

    template <typename T>
    T Read() {
        T value;
        memcpy(&value, data + ptr, sizeof(T));
        ptr += sizeof(T);
        return value;
    }

    void ReadBytes(void* dst, size_t count) {
        memcpy(dst, data + ptr, count);
        ptr += count;
    }

    size_t Tell() const { return ptr; }
    unsigned char* GetPointer() const { return data + ptr; }

private:
    unsigned char* data;
    size_t size;
    size_t ptr;
};


bool BMD::Open2(wchar_t* DirName, wchar_t* ModelFileName, bool bReAlloc)
{
    if (m_bCompletedAlloc)
    {
        if (!bReAlloc)
            return true;
        Release();
    }

    wchar_t ModelPath[260] = {};
    _snwprintf(ModelPath, std::size(ModelPath), L"%s%s", DirName, ModelFileName);

    FILE* fp = _wfopen(ModelPath, L"rb");
    if (!fp)
    {
        //// wprintf(L"[Open2] ERROR: Unable to open file: %s\n", ModelPath);
        m_bCompletedAlloc = false;
        return false;
    }

    fseek(fp, 0, SEEK_END);
    int dataSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> fileData(new(std::nothrow) unsigned char[dataSize]);
    if (!fileData)
    {
        fclose(fp);
        m_bCompletedAlloc = false;
        return false;
    }

    fread(fileData.get(), 1, dataSize, fp);
    fclose(fp);

    // *** Check the "BMD" header ***
    if (!(fileData[0] == 'B' && fileData[1] == 'M' && fileData[2] == 'D'))
    {
        wprintf(L"[Open2] ERROR: Invalid file header (expected 'BMD') in file %.64s\n", ModelPath);
        m_bCompletedAlloc = false;
        return false;
    }

    int ptr = 3;
    Version = fileData[ptr++];
    

    std::unique_ptr<unsigned char[]> decryptedData;
    if (Version == 0xC)
    {
        //// wprintf(L"[Open2] Version: %d\n", Version);
        long encSize = *(long*)(fileData.get() + ptr); ptr += sizeof(long);
        unsigned char* encData = fileData.get() + ptr;
        //// wprintf(L"[Open2] Encrypted Size: %ld\n", encSize);

        long decSize = MapFileDecrypt(nullptr, encData, encSize);
        //// wprintf(L"[Open2] Decrypted Size: %ld\n", decSize);

        decryptedData.reset(new(std::nothrow) unsigned char[decSize]);
        if (!decryptedData)
        {
            m_bCompletedAlloc = false;
            return false;
        }

        MapFileDecrypt(decryptedData.get(), encData, encSize);
        ptr = 0;
    }
    else if (Version == 0xE)
    {
        wprintf(L"[Open2] Version: %d\n, not yet supported. File: %.64s\n", Version, ModelPath);
        // FIXME FOR NEW MAPS 
        // DECRYPT KEY: webzen#@!01webzen#@!01webzen#@!0
    }
    else if (Version == 0xA)
    {
        // wprintf(L"[Open2] Version: %d\n", Version);
        ptr = 4;
    }
    else
    {
        wprintf(L"[Open2] Unknown BMD version: %ld\n in %.64s\n", Version, ModelPath);
        m_bCompletedAlloc = false;
        return false;
    }


    unsigned char* data = decryptedData ? decryptedData.get() : fileData.get();

    memcpy(Name, data + ptr, 32); ptr += 32;

    const char* ext = strrchr(Name, '.');
    if (!ext || (_stricmp(ext, ".smd") != 0))
    {
        // wprintf(L"[Open2] WARNING: Invalid file extension: %.64hs in %.64s\n", Name, ModelPath);
    }

    NumMeshs = *(short*)(data + ptr); ptr += sizeof(short);
    NumBones = *(short*)(data + ptr); ptr += sizeof(short);
    NumActions = *(short*)(data + ptr); ptr += sizeof(short);

    assert(NumBones <= MAX_BONES && "Bones 200");
    //// wprintf(L"[Open2] Model: %.32hs | Meshes: %d | Bones: %d | Actions: %d\n", Name, NumMeshs, NumBones, NumActions);

    const int meshCount = NumMeshs > 0 ? NumMeshs : 1;
    const int boneCount = NumBones > 0 ? NumBones : 1;
    const int actionCount = NumActions > 0 ? NumActions : 1;

    Meshs = new(std::nothrow) Mesh_t[meshCount]();
    Bones = new(std::nothrow) Bone_t[boneCount]();
    Actions = new(std::nothrow) Action_t[actionCount]();
    Textures = new(std::nothrow) Texture_t[meshCount]();
    IndexTexture = new(std::nothrow) GLuint[meshCount]();

    if (!Meshs || !Bones || !Actions || !Textures || !IndexTexture)
    {
        Release();
        m_bCompletedAlloc = false;
        return false;
    }

    for (int i = 0; i < NumMeshs; ++i)
    {
        Mesh_t& m = Meshs[i];
        m.NumVertices = *(short*)(data + ptr); ptr += sizeof(short);
        m.NumNormals = *(short*)(data + ptr); ptr += sizeof(short);
        m.NumTexCoords = *(short*)(data + ptr); ptr += sizeof(short);
        m.NumTriangles = *(short*)(data + ptr); ptr += sizeof(short);
        m.Texture = *(short*)(data + ptr); ptr += sizeof(short);
        m.NoneBlendMesh = false;

        //// wprintf(L"[Open2] Mesh[%d] V:%d N:%d T:%d Tri:%d Tex:%d\n", i, m.NumVertices, m.NumNormals, m.NumTexCoords, m.NumTriangles, m.Texture);

        m.Vertices = new Vertex_t[m.NumVertices];
        m.Normals = new Normal_t[m.NumNormals];
        m.TexCoords = new TexCoord_t[m.NumTexCoords];
        m.Triangles = new Triangle_t[m.NumTriangles];

        memcpy(m.Vertices, data + ptr, m.NumVertices * sizeof(Vertex_t));  ptr += m.NumVertices * sizeof(Vertex_t);
        memcpy(m.Normals, data + ptr, m.NumNormals * sizeof(Normal_t));   ptr += m.NumNormals * sizeof(Normal_t);
        memcpy(m.TexCoords, data + ptr, m.NumTexCoords * sizeof(TexCoord_t)); ptr += m.NumTexCoords * sizeof(TexCoord_t);

        for (int j = 0; j < m.NumTriangles; ++j)
        {
            memcpy(&m.Triangles[j], data + ptr, sizeof(Triangle_t));
            ptr += sizeof(Triangle_t2);
        }

        memcpy(Textures[i].FileName, data + ptr, 32); ptr += 32;

        TextureScriptParsing script;
        if (script.parsingTScriptA(Textures[i].FileName))
        {
            m.m_csTScript = new TextureScript;
            m.m_csTScript->setScript(script);
        }
        else
        {
            m.m_csTScript = nullptr;
        }
    }

    for (int i = 0; i < NumActions; ++i)
    {
        Action_t& a = Actions[i];
        a.Loop = false;
        a.NumAnimationKeys = *(short*)(data + ptr); ptr += sizeof(short);
        a.LockPositions = *(bool*)(data + ptr);  ptr += sizeof(bool);

        //// wprintf(L"[Open2] Action[%d] Keys: %d Lock: %d\n", i, a.NumAnimationKeys, a.LockPositions);

        if (a.LockPositions && a.NumAnimationKeys > 0)
        {
            a.Positions = new vec3_t[a.NumAnimationKeys];
            memcpy(a.Positions, data + ptr, sizeof(vec3_t) * a.NumAnimationKeys);
            ptr += sizeof(vec3_t) * a.NumAnimationKeys;
        }
        else
        {
            a.Positions = nullptr;
        }
    }

    for (int i = 0; i < NumBones; ++i)
    {
        Bone_t& b = Bones[i];
        b.Dummy = *(char*)(data + ptr); ptr += sizeof(char);

        if (!b.Dummy)
        {
            memcpy(b.Name, data + ptr, 32); ptr += 32;
            b.Parent = *(short*)(data + ptr); ptr += sizeof(short);

            //// wprintf(L"[Open2] Bone[%d] Name: %.32hs Parent: %d\n", i, b.Name, b.Parent);

            b.BoneMatrixes = new BoneMatrix_t[NumActions]();

            for (int j = 0; j < NumActions; ++j)
            {
                BoneMatrix_t& bm = b.BoneMatrixes[j];
                int numKeys = Actions[j].NumAnimationKeys;

                if (numKeys > 0)
                {
                    bm.Position = new vec3_t[numKeys];
                    bm.Rotation = new vec3_t[numKeys];
                    bm.Quaternion = new vec4_t[numKeys];

                    memcpy(bm.Position, data + ptr, sizeof(vec3_t) * numKeys); ptr += sizeof(vec3_t) * numKeys;
                    memcpy(bm.Rotation, data + ptr, sizeof(vec3_t) * numKeys); ptr += sizeof(vec3_t) * numKeys;

                    for (int k = 0; k < numKeys; ++k)
                        AngleQuaternion(bm.Rotation[k], bm.Quaternion[k]);

                }
                else
                {
                    bm.Position = nullptr;
                    bm.Rotation = nullptr;
                    bm.Quaternion = nullptr;
                }
            }
        }
    }

    Init(false);
    m_bCompletedAlloc = true;
    return true;
}


bool BMD::Save2(wchar_t* DirName, wchar_t* ModelFileName)
{
    wchar_t ModelName[64];
    wcscpy(ModelName, DirName);
    wcscat(ModelName, ModelFileName);
    FILE* fp = _wfopen(ModelName, L"wb");
    if (fp == nullptr) return false;
    putc('B', fp);
    putc('M', fp);
    putc('D', fp);
    Version = 12;
    fwrite(&Version, 1, 1, fp);

    auto* pbyBuffer = new BYTE[1024 * 1024];
    BYTE* pbyCur = pbyBuffer;
    memcpy(pbyCur, Name, 32); pbyCur += 32;
    memcpy(pbyCur, &NumMeshs, 2); pbyCur += 2;
    memcpy(pbyCur, &NumBones, 2); pbyCur += 2;
    memcpy(pbyCur, &NumActions, 2); pbyCur += 2;

    int i;
    for (i = 0; i < NumMeshs; i++)
    {
        Mesh_t* m = &Meshs[i];
        memcpy(pbyCur, &m->NumVertices, 2); pbyCur += 2;
        memcpy(pbyCur, &m->NumNormals, 2); pbyCur += 2;
        memcpy(pbyCur, &m->NumTexCoords, 2); pbyCur += 2;
        memcpy(pbyCur, &m->NumTriangles, 2); pbyCur += 2;
        memcpy(pbyCur, &m->Texture, 2); pbyCur += 2;
        memcpy(pbyCur, m->Vertices, m->NumVertices * sizeof(Vertex_t)); pbyCur += m->NumVertices * sizeof(Vertex_t);
        memcpy(pbyCur, m->Normals, m->NumNormals * sizeof(Normal_t)); pbyCur += m->NumNormals * sizeof(Normal_t);
        memcpy(pbyCur, m->TexCoords, m->NumTexCoords * sizeof(TexCoord_t)); pbyCur += m->NumTexCoords * sizeof(TexCoord_t);
        for (int j = 0; j < m->NumTriangles; j++)
        {
            memcpy(pbyCur, &m->Triangles[j], sizeof(Triangle_t2)); pbyCur += sizeof(Triangle_t2);
        }
        memcpy(pbyCur, Textures[i].FileName, 32); pbyCur += 32;
    }
    for (i = 0; i < NumActions; i++)
    {
        Action_t* a = &Actions[i];
        memcpy(pbyCur, &a->NumAnimationKeys, 2); pbyCur += 2;
        memcpy(pbyCur, &a->LockPositions, 1); pbyCur += 1;
        if (a->LockPositions)
        {
            memcpy(pbyCur, a->Positions, a->NumAnimationKeys * sizeof(vec3_t)); pbyCur += a->NumAnimationKeys * sizeof(vec3_t);
        }
    }
    for (i = 0; i < NumBones; i++)
    {
        Bone_t* b = &Bones[i];
        memcpy(pbyCur, &b->Dummy, 1); pbyCur += 1;
        if (!b->Dummy)
        {
            memcpy(pbyCur, b->Name, 32); pbyCur += 32;
            memcpy(pbyCur, &b->Parent, 2); pbyCur += 2;
            for (int j = 0; j < NumActions; j++)
            {
                BoneMatrix_t* bm = &b->BoneMatrixes[j];
                memcpy(pbyCur, bm->Position, Actions[j].NumAnimationKeys * sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys * sizeof(vec3_t);
                memcpy(pbyCur, bm->Rotation, Actions[j].NumAnimationKeys * sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys * sizeof(vec3_t);
            }
        }
    }
    auto lSize = (long)(pbyCur - pbyBuffer);
    long lEncSize = MapFileEncrypt(nullptr, pbyBuffer, lSize);
    auto* pbyEnc = new BYTE[lEncSize];
    MapFileEncrypt(pbyEnc, pbyBuffer, lSize);
    fwrite(&lEncSize, sizeof(long), 1, fp);
    fwrite(pbyEnc, lEncSize, 1, fp);
    fclose(fp);
    delete[] pbyBuffer;
    delete[] pbyEnc;
    return true;
}

void BMD::Init(bool Dummy)
{
    if (Dummy)
    {
        int i;
        for (i = 0; i < NumBones; i++)
        {
            Bone_t* b = &Bones[i];
            if (b->Name[0] == 'D' && b->Name[1] == 'u')
                b->Dummy = true;
            else
                b->Dummy = false;
        }
    }
    renderCount = 0;
    BoneHead = -1;
    StreamMesh = -1;
    CreateBoundingBox();
}

void BMD::CreateBoundingBox()
{
    for (int i = 0; i < NumBones; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            BoundingMin[i][j] = 9999.0;
            BoundingMax[i][j] = -9999.0;
        }
        BoundingVertices[i] = 0;
    }

    for (int i = 0; i < NumMeshs; i++)
    {
        Mesh_t* m = &Meshs[i];
        for (int j = 0; j < m->NumVertices; j++)
        {
            Vertex_t* v = &m->Vertices[j];
            for (int k = 0; k < 3; k++)
            {
                if (v->Position[k] < BoundingMin[v->Node][k]) BoundingMin[v->Node][k] = v->Position[k];
                if (v->Position[k] > BoundingMax[v->Node][k]) BoundingMax[v->Node][k] = v->Position[k];
            }
            BoundingVertices[v->Node]++;
        }
    }
    for (int i = 0; i < NumBones; i++)
    {
        Bone_t* b = &Bones[i];
        if (BoundingVertices[i])
            b->BoundingBox = true;
        else
            b->BoundingBox = false;
        Vector(BoundingMax[i][0], BoundingMax[i][1], BoundingMax[i][2], b->BoundingVertices[0]);
        Vector(BoundingMax[i][0], BoundingMax[i][1], BoundingMin[i][2], b->BoundingVertices[1]);
        Vector(BoundingMax[i][0], BoundingMin[i][1], BoundingMax[i][2], b->BoundingVertices[2]);
        Vector(BoundingMax[i][0], BoundingMin[i][1], BoundingMin[i][2], b->BoundingVertices[3]);
        Vector(BoundingMin[i][0], BoundingMax[i][1], BoundingMax[i][2], b->BoundingVertices[4]);
        Vector(BoundingMin[i][0], BoundingMax[i][1], BoundingMin[i][2], b->BoundingVertices[5]);
        Vector(BoundingMin[i][0], BoundingMin[i][1], BoundingMax[i][2], b->BoundingVertices[6]);
        Vector(BoundingMin[i][0], BoundingMin[i][1], BoundingMin[i][2], b->BoundingVertices[7]);
    }
}

BMD::~BMD()
{
    Release();
}

void BMD::InterpolationTrans(float(*Mat1)[4], float(*TransMat2)[4], float _Scale)
{
    TransMat2[0][3] = TransMat2[0][3] - (TransMat2[0][3] - Mat1[0][3]) * (1 - _Scale);
    TransMat2[1][3] = TransMat2[1][3] - (TransMat2[1][3] - Mat1[1][3]) * (1 - _Scale);
    TransMat2[2][3] = TransMat2[2][3] - (TransMat2[2][3] - Mat1[2][3]) * (1 - _Scale);
}