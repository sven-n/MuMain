///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <cstdint>
#include <cmath>
#include <cassert>
#include <set>
#include <utility>
#include "Render/Textures/ZzzOpenglUtil.h"

#include "Engine/Object/ZzzInfomation.h"
#include "ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "SMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Shaders/ItemSpecularShader.h"
#include "Render/Shaders/PlanarShadowShader.h"
#include "Render/Shaders/BMDMeshShader.h"
#include "Render/Core/BoneUBO.h"
#include "Render/Core/GlobalUBO.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "Camera/CameraState.h"

#include "UI/Legacy/UIMng.h"
#include "Camera/CameraMove.h"
#include "Engine/Physics/PhysicsManager.h"
#include "UI/NewUI/NewUISystem.h"
#include "Render/Core/RenderConfig.h"
#include "Render/Core/ImmediateRenderer.h"
#include "Render/Core/BindState.h"
#include "Render/Shaders/PassthroughShader.h"
#include "Core/Utilities/FrameProfiler.h"
#include <SDL3/SDL.h>

#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (APIENTRY* PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY* PFNGLVERTEXATTRIBIPOINTERPROC)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);

static PFNGLGENVERTEXARRAYSPROC          fn_glGenVertexArrays          = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC       fn_glDeleteVertexArrays       = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC      fn_glVertexAttribPointer      = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC  fn_glEnableVertexAttribArray  = nullptr;
static PFNGLGENBUFFERSPROC               fn_glGenBuffers               = nullptr;
static PFNGLBINDBUFFERPROC               fn_glBindBuffer               = nullptr;
static PFNGLBUFFERDATAPROC               fn_glBufferData               = nullptr;
static PFNGLBUFFERSUBDATAPROC            fn_glBufferSubData            = nullptr;
static PFNGLDELETEBUFFERSPROC            fn_glDeleteBuffers            = nullptr;
static PFNGLVERTEXATTRIBIPOINTERPROC     fn_glVertexAttribIPointer     = nullptr;

static bool LoadBMDGLFunctions()
{
    static bool loaded = false;
    if (loaded) return true;

    fn_glGenVertexArrays          = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    fn_glDeleteVertexArrays       = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    fn_glVertexAttribPointer      = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    fn_glEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    fn_glGenBuffers               = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    fn_glBindBuffer               = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    fn_glBufferData               = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    fn_glBufferSubData            = (PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData");
    fn_glDeleteBuffers            = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
    fn_glVertexAttribIPointer     = (PFNGLVERTEXATTRIBIPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribIPointer");

    loaded = (fn_glGenVertexArrays != nullptr &&
              fn_glDeleteVertexArrays != nullptr &&
              fn_glVertexAttribPointer != nullptr &&
              fn_glEnableVertexAttribArray != nullptr &&
              fn_glGenBuffers != nullptr &&
              fn_glBindBuffer != nullptr &&
              fn_glBufferData != nullptr &&
              fn_glBufferSubData != nullptr &&
              fn_glDeleteBuffers != nullptr);
    return loaded;
}

BMD* Models;
BMD* ModelsDump;

vec4_t BoneQuaternion[MAX_BONES];
short  BoundingVertices[MAX_BONES];
vec3_t BoundingMin[MAX_BONES];
vec3_t BoundingMax[MAX_BONES];

float  BoneTransform[MAX_BONES][3][4];
const float (*g_pActiveBoneTransform)[3][4] = nullptr;
unsigned int g_BoneTransformVersion = 0;

void SetActiveBoneTransform(const float (*ptr)[3][4])
{
    g_pActiveBoneTransform = ptr;
    ++g_BoneTransformVersion;
}

vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
vec3_t NormalTransform[MAX_MESH][MAX_VERTICES];
float  IntensityTransform[MAX_MESH][MAX_VERTICES];
vec3_t LightTransform[MAX_MESH][MAX_VERTICES];

// DXP-20 increment 4: lazy CPU-skin materialization. Bumped by every TransformCheap() call;
// a BMD's m_SkinStamp matching this value means it currently owns VertexTransform/NormalTransform/
// IntensityTransform (last-writer-wins global scratch, same sharing model as before this increment).
// g_LazyCpuSkin is a kill switch -- false reproduces pre-increment-4 eager behavior exactly.
static uint32_t g_SkinStampCounter = 0;
static bool     g_LazyCpuSkin = true; // DXP-20 inc4 Step D: gate flipped on -- see DXP-20-inc4-plan.md

vec3_t RenderArrayVertices[MAX_VERTICES * 3];
vec4_t RenderArrayColors[MAX_VERTICES * 3];
vec2_t RenderArrayTexCoords[MAX_VERTICES * 3];

bool  StopMotion = false;
thread_local float ParentMatrix[3][4];

static vec3_t LightVector = { 0.f, -0.1f, -0.8f };
static vec3_t LightVector2 = { 0.f, -0.5f, -0.8f };

void BMD::Animation(float(*BoneMatrix)[3][4], float AnimationFrame, float PriorFrame, unsigned short PriorAction, vec3_t Angle, vec3_t HeadAngle, bool Parent, bool Translate, const float (*ExtParentMatrix)[4], short CurrentActionArg, int BoneMatrixCapacity)
{
    if (NumActions <= 0) return;

    unsigned short currentAction = (CurrentActionArg < 0) ? CurrentAction : (unsigned short)CurrentActionArg;
    if (PriorAction >= NumActions) PriorAction = 0;
    if (currentAction >= NumActions) currentAction = 0;

    vec4_t boneQuaternion[MAX_BONES] = {};

    float currentAnimation = AnimationFrame;
    int currentAnimationFrame = (int)AnimationFrame;
    float s1 = (currentAnimation - currentAnimationFrame);
    float s2 = 1.f - s1;
    auto PriorAnimationFrame = (int)PriorFrame;
    if (NumActions > 0)
    {
        if (PriorAnimationFrame < 0)
            PriorAnimationFrame = 0;
        if (currentAnimationFrame < 0)
            currentAnimationFrame = 0;
        if (PriorAnimationFrame >= Actions[PriorAction].NumAnimationKeys)
            PriorAnimationFrame = 0;
        if (currentAnimationFrame >= Actions[currentAction].NumAnimationKeys)
            currentAnimationFrame = 0;
    }

    // Pre-calculate localParentMatrix ONCE outside bone loop for thread-safe root transforms
    float localParentMatrix[3][4];
    if (!Parent)
    {
        AngleMatrix(Angle, localParentMatrix);
        if (Translate)
        {
            for (auto & y : localParentMatrix)
            {
                for (int x = 0; x < 3; ++x)
                {
                    y[x] *= BodyScale;
                }
            }

            localParentMatrix[0][3] = BodyOrigin[0];
            localParentMatrix[1][3] = BodyOrigin[1];
            localParentMatrix[2][3] = BodyOrigin[2];
        }
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                ParentMatrix[r][c] = localParentMatrix[r][c];
            }
        }
    }

    // Pre-calculate Head bone quaternions ONCE outside loop
    vec4_t headQ1, headQ2;
    const bool hasHeadBone = (BoneHead >= 0 && BoneHead < NumBones && !Bones[BoneHead].Dummy);
    if (hasHeadBone)
    {
        const Bone_t* hb = &Bones[BoneHead];
        const BoneMatrix_t* hbm1 = &hb->BoneMatrixes[PriorAction];
        const BoneMatrix_t* hbm2 = &hb->BoneMatrixes[currentAction];

        vec3_t Angle1, Angle2;
        VectorCopy(hbm1->Rotation[PriorAnimationFrame], Angle1);
        VectorCopy(hbm2->Rotation[currentAnimationFrame], Angle2);

        constexpr float radFactor = 1.0f / (180.f / Q_PI);
        const float HeadAngleX = HeadAngle[0] * radFactor;
        const float HeadAngleY = HeadAngle[1] * radFactor;

        Angle1[0] -= HeadAngleX;
        Angle2[0] -= HeadAngleX;
        Angle1[2] -= HeadAngleY;
        Angle2[2] -= HeadAngleY;

        AngleQuaternion(Angle1, headQ1);
        AngleQuaternion(Angle2, headQ2);
    }

    const bool bLockPositions = (Actions[PriorAction].LockPositions || Actions[currentAction].LockPositions);

    // bones loop
    for (int i = 0; i < NumBones; i++)
    {
        const Bone_t* b = &Bones[i];
        if (b->Dummy)
        {
            // DXP-24 fix (part 2): Dummy bones carry no name/parent/animation data (see Open2's
            // loader -- the !Dummy branch is the only one that reads anything), so this slot was
            // previously left holding whatever the LAST model to animate into this shared buffer
            // wrote there. If that was a differently-positioned character (character-select roster),
            // any vertex or child bone reading this slot picks up a real matrix anchored thousands
            // of units away -- which PlanarShadowShader's skew division then amplifies into the
            // "infinity shadow". The tail fill below only covers slots >= NumBones; Dummy slots
            // live INSIDE [0, NumBones) and need the same identity treatment.
            BoneMatrix[i][0][0] = 1.f; BoneMatrix[i][0][1] = 0.f; BoneMatrix[i][0][2] = 0.f; BoneMatrix[i][0][3] = 0.f;
            BoneMatrix[i][1][0] = 0.f; BoneMatrix[i][1][1] = 1.f; BoneMatrix[i][1][2] = 0.f; BoneMatrix[i][1][3] = 0.f;
            BoneMatrix[i][2][0] = 0.f; BoneMatrix[i][2][1] = 0.f; BoneMatrix[i][2][2] = 1.f; BoneMatrix[i][2][3] = 0.f;
            continue;
        }
        const BoneMatrix_t* bm1 = &b->BoneMatrixes[PriorAction];
        const BoneMatrix_t* bm2 = &b->BoneMatrixes[currentAction];

        const float* q1;
        const float* q2;

        if (i == BoneHead)
        {
            q1 = headQ1;
            q2 = headQ2;
        }
        else
        {
            q1 = bm1->Quaternion[PriorAnimationFrame];
            q2 = bm2->Quaternion[currentAnimationFrame];
        }

        if (!QuaternionCompare(q1, q2))
        {
            QuaternionNLERP(q1, q2, s1, boneQuaternion[i]);
        }
        else
        {
            QuaternionCopy(q1, boneQuaternion[i]);
        }

        float Matrix[3][4];
        QuaternionMatrix(boneQuaternion[i], Matrix);
        const float* Position1 = bm1->Position[PriorAnimationFrame];
        const float* Position2 = bm2->Position[currentAnimationFrame];

        if (i == 0 && bLockPositions)
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
            if (Parent && ExtParentMatrix)
            {
                R_ConcatTransforms(ExtParentMatrix, Matrix, BoneMatrix[i]);
            }
            else if (!Parent)
            {
                R_ConcatTransforms(localParentMatrix, Matrix, BoneMatrix[i]);
            }
            else
            {
                R_ConcatTransforms(ParentMatrix, Matrix, BoneMatrix[i]);
            }
        }
        else
        {
            R_ConcatTransforms(BoneMatrix[b->Parent], Matrix, BoneMatrix[i]);
        }
    }

    // DXP-24 fix: this model's own skeleton may have fewer than MAX_BONES real bones, and the loop
    // above only ever writes BoneMatrix[0..NumBones). BoneMatrix is caller-supplied and shared/reused
    // across different models' Animation() calls (not cleared between them), so slots >= NumBones
    // would otherwise keep holding a PREVIOUS, differently-boned model's real (not garbage) transform
    // matrix. BoneUBO::UploadBones() always uploads the full MAX_BONES-slot buffer regardless, so any
    // mesh vertex whose a_BoneIndex lands in that untouched tail would read a foreign character's
    // matrix -- confirmed via RenderDoc to be the root cause of a shadow-rendering vertex explosion
    // (PlanarShadowShader's ground-skew division amplifies a stale-but-plausible foreign matrix into
    // an astronomical position when it lands near the division's singularity; see
    // .ai-os/memory/tasks/dxp/DXP-24-shadow-bone-matrix-corruption.md). Identity-fill the unused tail
    // so any out-of-range bone index reads a safe no-op transform instead. Bounded by the caller's
    // declared buffer capacity -- NOT MAX_BONES -- because several internal callers pass
    // NumBones-sized heap scratch buffers (AnimationTransformWithAttachHighModel* /
    // AnimationTransformOnlySelf), which a MAX_BONES-bound loop would overflow.
    for (int i = NumBones; i < BoneMatrixCapacity; i++)
    {
        BoneMatrix[i][0][0] = 1.f; BoneMatrix[i][0][1] = 0.f; BoneMatrix[i][0][2] = 0.f; BoneMatrix[i][0][3] = 0.f;
        BoneMatrix[i][1][0] = 0.f; BoneMatrix[i][1][1] = 1.f; BoneMatrix[i][1][2] = 0.f; BoneMatrix[i][1][3] = 0.f;
        BoneMatrix[i][2][0] = 0.f; BoneMatrix[i][2][1] = 0.f; BoneMatrix[i][2][2] = 1.f; BoneMatrix[i][2][3] = 0.f;
    }
}

extern EGameScene SceneFlag;
extern int EditFlag;

bool HighLight = true;
float BoneScale = 1.f;

void BMD::ClaimSkinStamp() const
{
    if (m_SkinStamp == g_SkinStampCounter) return;
#ifdef _DEBUG
    // Reaching here means some OTHER BMD's TransformCheap() ran since this BMD's own last one --
    // this BMD's slice of the shared scratch arrays was evicted, and we're about to re-derive it
    // from our own stashed skin request (self-heal). This is a pre-existing sharing model (last
    // Transform() wins), not new to DXP-20 inc4 -- but a consumer reaching this branch means it
    // read/wrote the arrays OUTSIDE the Calc/Draw (or equivalent) bracket that owns this BMD's
    // data, which is worth knowing about if the soak turns up anything odd.
    g_ErrorReport.Write(L"[DXP-20-inc4] ClaimSkinStamp: stale skin stamp (this=%p, stamp=%u vs current=%u) -- self-healing\r\n",
        (void*)this, m_SkinStamp, g_SkinStampCounter);
#endif
    m_SkinStamp = ++g_SkinStampCounter;
    for (int i = 0; i < MAX_MESH; i++)
    {
        m_CpuVertsReady[i] = false;
        m_CpuNormalsReady[i] = false;
    }
}

void BMD::TransformCheap(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate, float _Scale)
{
    m_pCurrentBoneTransform = BoneMatrix;
    SetActiveBoneTransform(BoneMatrix);
    m_LastTranslate = Translate; // persist for RenderMesh GPU skinning path
    m_LastSkinScale = _Scale;    // DXP-20 inc4: stashed for EnsureCpuVertices()
    m_LastBoneScale = BoneScale; // DXP-20 inc4: snapshot of the global -- callers mutate it right
                                  // after Transform() returns (e.g. monster edge-scale resets),
                                  // so a deferred read of the live global would skin wrong.
    m_SkinStamp = ++g_SkinStampCounter; // this BMD now owns the shared scratch arrays
    for (int i = 0; i < MAX_MESH; i++)
    {
        m_CpuVertsReady[i] = false;
        m_CpuNormalsReady[i] = false;
    }

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
        VectorIRotate(Position, Matrix, m_LastLightPosition); // DXP-20: for RenderMesh's GPU-skinned in-shader lighting
    }

    // Release/gameplay OBB: from the caller-supplied bounding box args, not a vertex-loop-derived
    // one -- this is Transform()'s own "else" branch (EditFlag != 2), which is why TransformCheap()
    // is only a valid substitute for Transform() in that same case (see header comment).
    VectorCopy(BoundingBoxMin, OBB->StartPos);
    OBB->XAxis[0] = (BoundingBoxMax[0] - BoundingBoxMin[0]);
    OBB->YAxis[1] = (BoundingBoxMax[1] - BoundingBoxMin[1]);
    OBB->ZAxis[2] = (BoundingBoxMax[2] - BoundingBoxMin[2]);
    VectorAdd(OBB->StartPos, BodyOrigin, OBB->StartPos);
    OBB->XAxis[1] = 0.f;
    OBB->XAxis[2] = 0.f;
    OBB->YAxis[0] = 0.f;
    OBB->YAxis[2] = 0.f;
    OBB->ZAxis[0] = 0.f;
    OBB->ZAxis[1] = 0.f;
    // NOTE: does not update fTransformedSize (left at its last value from a real Transform() call).
    // Its only consumer (ZzzCharacter.cpp's character-select pick-box fallback height) already
    // floors the result, and Models[] is shared-by-type mutable state already, so a stale value
    // here is no worse than the existing sharing model.
}

void BMD::SkinVertex(int mesh, int vertexIndex, float(*BoneMatrix)[3][4], bool Translate, float _Scale, vec3_t out) const
{
    const Vertex_t* v = &Meshs[mesh].Vertices[vertexIndex];

    // DXP-20 inc4: reads the BoneScale snapshotted at TransformCheap() time, not the live global --
    // this makes SkinVertex()/SkinVertices() safe to call from a deferred EnsureCpuVertices(), where
    // the global may already have been reset/reused by a later object. Behavior-identical for the
    // pre-inc4 callers (coin heap, skin-shell effect), which always run immediately after
    // TransformCheap() -- the stash and the global agree at that point.
    if (m_LastBoneScale == 1.f)
    {
        if (_Scale)
        {
            vec3_t Position;
            VectorCopy(v->Position, Position);
            VectorScale(Position, _Scale, Position);
            VectorTransform(Position, BoneMatrix[v->Node], out);
        }
        else
            VectorTransform(v->Position, BoneMatrix[v->Node], out);
        if (Translate)
            VectorScale(out, BodyScale, out);
    }
    else
    {
        VectorRotate(v->Position, BoneMatrix[v->Node], out);
        out[0] = out[0] * m_LastBoneScale + BoneMatrix[v->Node][0][3];
        out[1] = out[1] * m_LastBoneScale + BoneMatrix[v->Node][1][3];
        out[2] = out[2] * m_LastBoneScale + BoneMatrix[v->Node][2][3];
        if (Translate)
            VectorScale(out, BodyScale, out);
    }
    if (Translate)
        VectorAdd(out, BodyOrigin, out);
}

void BMD::SkinVertices(int mesh, float(*BoneMatrix)[3][4], bool Translate, float _Scale) const
{
    const Mesh_t* m = &Meshs[mesh];
    for (int j = 0; j < m->NumVertices; j++)
        SkinVertex(mesh, j, BoneMatrix, Translate, _Scale, VertexTransform[mesh][j]);
}

void BMD::EnsureCpuVertices(int mesh) const
{
    ClaimSkinStamp();

    if (mesh < 0)
    {
        for (int i = 0; i < NumMeshs; i++)
            EnsureCpuVertices(i);
        return;
    }

    if (mesh >= MAX_MESH || mesh >= NumMeshs) return;
    if (m_CpuVertsReady[mesh]) return;

    SkinVertices(mesh, m_pCurrentBoneTransform, m_LastTranslate, m_LastSkinScale);
    m_CpuVertsReady[mesh] = true;
}

void BMD::EnsureCpuNormals(int mesh) const
{
    ClaimSkinStamp();

    if (mesh < 0)
    {
        for (int i = 0; i < NumMeshs; i++)
            EnsureCpuNormals(i);
        return;
    }

    if (mesh >= MAX_MESH || mesh >= NumMeshs) return;
    if (m_CpuNormalsReady[mesh]) return;

    const Mesh_t* m = &Meshs[mesh];
    for (int j = 0; j < m->NumNormals; j++)
    {
        const Normal_t* sn = &m->Normals[j];
        float* tn = NormalTransform[mesh][j];
        VectorRotate(sn->Normal, m_pCurrentBoneTransform[sn->Node], tn);
        if (LightEnable)
        {
            float Luminosity = DotProduct(tn, m_LastLightPosition) * 0.8f + 0.4f;
            if (Luminosity < 0.2f) Luminosity = 0.2f;
            IntensityTransform[mesh][j] = Luminosity;
        }
    }
    m_CpuNormalsReady[mesh] = true;
}

void BMD::MarkCpuVerticesExternallyWritten(int mesh) const
{
    if (mesh < 0 || mesh >= MAX_MESH) return;
    ClaimSkinStamp();
    m_CpuVertsReady[mesh] = true;
}

void BMD::Transform(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate, float _Scale)
{
    FRAME_PROFILE(Skinning); // DXP-20 increment 1 baseline measurement
    TransformCheap(BoneMatrix, BoundingBoxMin, BoundingBoxMax, OBB, Translate, _Scale);

    // DXP-20 increment 4: with the lazy-skin gate on, ordinary (EditFlag != 2) bodies defer the
    // vertex/normal loops below to EnsureCpuVertices()/EnsureCpuNormals() at each consumer site
    // instead of running them here unconditionally -- TransformCheap() already stashed everything
    // those need. EditFlag == 2 (map editor) always takes the eager path below: it needs the
    // vertex-loop-derived OBB override further down, which EnsureCpu*() never computes (see
    // TransformCheap()'s header comment -- same restriction that already applied to it in
    // increment 2). fTransformedSize is intentionally left stale on the lazy path in both Debug
    // and Release builds (see DXP-20-inc4-plan.md) -- its only consumer already floors the result.
    if (g_LazyCpuSkin && EditFlag != 2)
        return;

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
                Luminosity = DotProduct(tn, m_LastLightPosition) * 0.8f + 0.4f;

                if (Luminosity < 0.2f) Luminosity = 0.2f;
                IntensityTransform[i][j] = Luminosity;
            }
        }
    }
    if (EditFlag == 2)
    {
        // Overrides TransformCheap()'s args-based OBB with the vertex-loop-derived bounds --
        // the map-editor-only accurate path.
        VectorCopy(BoundingMin, OBB->StartPos);
        OBB->XAxis[0] = (BoundingMax[0] - BoundingMin[0]);
        OBB->YAxis[1] = (BoundingMax[1] - BoundingMin[1]);
        OBB->ZAxis[2] = (BoundingMax[2] - BoundingMin[2]);
        VectorAdd(OBB->StartPos, BodyOrigin, OBB->StartPos);
        OBB->XAxis[1] = 0.f;
        OBB->XAxis[2] = 0.f;
        OBB->YAxis[0] = 0.f;
        OBB->YAxis[2] = 0.f;
        OBB->ZAxis[0] = 0.f;
        OBB->ZAxis[1] = 0.f;
    }
    fTransformedSize = std::max<float>(std::max<float>(BoundingMax[0] - BoundingMin[0], BoundingMax[1] - BoundingMin[1]),
        BoundingMax[2] - BoundingMin[2]);
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

    Animation(arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false, nullptr, -1, NumBones);

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

    Animation(arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false, nullptr, -1, NumBones);
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

    Animation(arrBonesTMLocal, oSelf->AnimationFrame, oSelf->PriorAnimationFrame, oSelf->PriorAction, (const_cast<OBJECT*>(oSelf))->Angle, Temp, false, true, nullptr, -1, NumBones);

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
        Animation(arrBonesTMLocal, 0, 0, 0, v3RootAngle, Temp, false, true, nullptr, -1, NumBones);
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
            v3RootAngle, Temp, false, true, nullptr, -1, NumBones);
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
    EnsureCpuVertices(-1); // DXP-20 inc4: mouse-picking/lightmap-bake reader, not in the original spec's consumer list
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
    EnsureCpuVertices(i); // DXP-20 inc4: lightmap bake reader, not in the original spec's consumer list
    EnsureCpuNormals(i);
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

            BindTexture2D(0, i + IndexLightMap);
            // DXP-08a: same GL_TEXTURE_ENV no-op as GlobalBitmap.cpp's OpenTga — FFP-only
            // texture-combiner state the shader path never reads, guarded rather than deleted.
            if (!g_CoreProfile) glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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

// TASK-27: per-body MVP cache. Both VBO paths in RenderMesh() need Proj*ModelView, but
// querying it via glGetFloatv per mesh is redundant driver work — the GL matrices provably
// cannot change between meshes of one BeginRender()/EndRender() bracket (RenderMesh contains
// no matrix ops, and no bracket in the codebase mutates matrices between RenderMesh calls;
// verified 2026-07-31 across ZzzObject.cpp + all GameMap callers). The cache is scoped
// strictly to the bracket: direct RenderMesh() calls outside a bracket always recompute,
// and nested brackets (BeginRender → RenderBody → EndRender) fall back to recomputing
// after the inner EndRender — conservative, never stale. Render-thread only.
static GLfloat s_BodyMVP[16];
static bool    s_InBodyRender = false;
static bool    s_BodyMVPValid = false;

// GlobalUBO is the CPU source of truth for MVP (fed by BeginOpengl()/BeginBitmap() per DXP-07a/b);
// GetCurrentMVP() just applies the per-body cache on top of it.
static void GetCurrentMVP(GLfloat outMVP[16])
{
    if (s_InBodyRender && s_BodyMVPValid)
    {
        memcpy(outMVP, s_BodyMVP, sizeof(s_BodyMVP));
        return;
    }

    memcpy(outMVP, GlobalUBO::Instance().GetMVP(), sizeof(GLfloat) * 16);

    if (s_InBodyRender)
    {
        memcpy(s_BodyMVP, outMVP, sizeof(s_BodyMVP));
        s_BodyMVPValid = true;
    }
}

void BMD::BeginRender(float Alpha)
{
    s_InBodyRender = true;
    s_BodyMVPValid = false;
}

void BMD::EndRender()
{
    s_InBodyRender = false;
    s_BodyMVPValid = false;
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

    constexpr int meshIndex = 0;
    Mesh_t* m = &Meshs[meshIndex];
    const int vertexCount = m->NumTriangles * 3 * coinCount;

    PassthroughShader::Instance().SetUseTexture(true);
    IR::Begin(GL_TRIANGLES);
    for (int i = 0; i < vertexCount; i++)
    {
        IR::Color4f(colors[i][0], colors[i][1], colors[i][2], colors[i][3]);
        IR::TexCoord2f(texCoords[i][0], texCoords[i][1]);
        IR::Vertex3fv(vertices[i]);
    }
    IR::End();
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
    // FIX-549: modulates the bodyLight value fed to BMDMeshShader::Bind() below for the
    // blendMeshIndex<=-2 branch (Force Wave / MODEL_WAVES-style fade effects). Under Core
    // Profile, glColor3f() (used further down for this same branch) is intercepted into a
    // no-op -- the shader never sees the fade, so it must be carried through explicitly here.
    float blendTintAlpha = 1.0f;
    if (meshIndex == StreamMesh)
    {
        glColor3fv(BodyLight);
        enableLight = false;
    }
    // DXP-20 inc4 Step C: the LightTransform-materializing loop that used to run right here
    // unconditionally (even for meshes that end up on the GPU-skinned draw path, which computes
    // lighting in-shader and never reads LightTransform) has moved into the
    // materializeCpuLightingAndChrome() lambda below, called only from the CPU-fallback sub-paths
    // that actually read it.

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

        // DXP-20 inc4 Step C: the g_chrome-writing loop that used to run right here unconditionally
        // (even for the plain-RENDER_CHROME case, which IS GPU-eligible and never reads g_chrome on
        // that path) has moved into the materializeCpuLightingAndChrome() lambda below, called only
        // from the CPU-fallback sub-paths that actually read it.

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
        blendTintAlpha = blendMeshAlpha;
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

    bool bShaderActive = CItemSpecularShader::Instance().GetActiveVariant() != SHADER_VARIANT_NONE;

    const bool useChrome = (renderFlags & RENDER_CHROME)  || (renderFlags & RENDER_CHROME2) ||
                           (renderFlags & RENDER_CHROME3)  || (renderFlags & RENDER_CHROME4) ||
                           (renderFlags & RENDER_CHROME5)  || (renderFlags & RENDER_CHROME6) ||
                           (renderFlags & RENDER_CHROME7)  || (renderFlags & RENDER_OIL)     ||
                           (renderFlags & RENDER_METAL);

    // TASK-28 increment 2 / DXP-02 increments 1-3: item specular tiers 1-4 (+7/+8, +9/+10, +11/+12,
    // +13/+15 gear) — renderFlags here carries no chrome bits at all (the multi-texture blend is
    // purely a function of the bound CItemSpecularShader variant, not renderFlags), so `useChrome`
    // is false for this call; only the blanket `!bShaderActive` below excludes it from the GPU path.
    const EShaderVariant activeSpecularVariant = CItemSpecularShader::Instance().GetActiveVariant();
    const bool itemSpecularGpuEligible = bShaderActive &&
        (activeSpecularVariant == SHADER_VARIANT_CHROME_1 || activeSpecularVariant == SHADER_VARIANT_CHROME_METAL ||
         activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V1 || activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V2);

    // DXP-20 inc4 Step C: materializes NormalTransform/IntensityTransform plus the two CPU-fallback-
    // only derived products (LightTransform, g_chrome) that used to be computed unconditionally
    // earlier in this function -- even for meshes that end up on the GPU-skinned draw path below,
    // which computes lighting and chrome UVs in-shader and never reads either array. Called from
    // both CPU-fallback sub-paths (UploadDynamicBuffers and the legacy client-array loop) right
    // before each needs the result; a no-op for meshes that take the true GPU-skinned return path,
    // since neither call site is reached in that case. Bodies are unchanged from their original
    // locations (were: the `else if (enableLight)` block above, and the chrome dispatch's
    // Wave2/L/g_chrome loop above).
    auto materializeCpuLightingAndChrome = [&]()
    {
        EnsureCpuNormals(meshIndex);

        if (enableLight)
        {
            for (int j = 0; j < m->NumNormals; j++)
            {
                VectorScale(BodyLight, IntensityTransform[meshIndex][j], LightTransform[meshIndex][j]);
            }
        }

        if (useChrome)
        {
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
        }
    };

    if ((!bShaderActive || itemSpecularGpuEligible) && !(renderFlags & (RENDER_SHADOWMAP | RENDER_WAVE)))
    {
        // Only attempt upload if not yet tried. m_MeshIndexCount is allocated at the TOP
        // of UploadStaticVBOs() before the threshold check, so non-null means already attempted.
        // Without this sentinel, models below the threshold call UploadStaticVBOs every frame.
        if (m_VAO_Static == 0 && m_MeshIndexCount == nullptr)
        {
            UploadStaticVBOs();
        }

        // GPU Skinning path for static meshes (when chrome UV is not needed, or is the plain
        // RENDER_CHROME variant — TASK-28 increment 1: the chrome UV is generated in-shader from
        // the GPU-skinned normal instead of requiring the CPU dynamic-VBO rebuild below. Other
        // chrome variants (CHROME2..7/OIL/METAL) are unchanged and still fall through to that path.
        const bool chromeGpuEligible = (finalRenderFlags == RENDER_CHROME) && !bShaderActive;
        const bool staticGpuBufferReady = (m_VAO_StaticGPU != 0);
        if (m_HasStaticGPUVBO && staticGpuBufferReady && (!useChrome || chromeGpuEligible) &&
            m_MeshIndexCount && m_MeshIndexOffset && m_MeshIndexCount[meshIndex] > 0)
        {
            int shaderRenderMode = 0;
            if (finalRenderFlags == RENDER_BRIGHT) shaderRenderMode = 2;
            else if (chromeGpuEligible) shaderRenderMode = 3;
            else if (itemSpecularGpuEligible)
            {
                if (activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V2) shaderRenderMode = 7;
                else if (activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V1) shaderRenderMode = 6;
                else if (activeSpecularVariant == SHADER_VARIANT_CHROME_METAL) shaderRenderMode = 5;
                else shaderRenderMode = 4;
            }

            // TASK-27: MVP fetched via per-body cache — computed once per BeginRender/EndRender
            // bracket instead of re-querying the driver for every mesh.
            GLfloat mvp[16];
            GetCurrentMVP(mvp);

            // Upload active object's bone transformation palette to BoneUBO (Slot 2).
            // Always upload all MAX_BONES (200) matrices because armor items reference character bones > NumBones.
            // UploadBones() dedups against (pointer, g_BoneTransformVersion) — body + every
            // equipped armor piece of one character share the same palette and version, so
            // only the first RenderMesh() call of that group actually re-uploads.
            const void* activeBones = g_pActiveBoneTransform ? (const void*)g_pActiveBoneTransform : (m_pCurrentBoneTransform ? (const void*)m_pCurrentBoneTransform : (const void*)BoneTransform);
            BoneUBO::Instance().UploadBones(activeBones, MAX_BONES, g_BoneTransformVersion);

            GLuint activeTexID = texture ? texture->TextureNumber : 0;
            // GPU skinning world-space placement:
            //   Translate=true  → bone matrices are in local skeleton space; BodyOrigin+BodyScale shift to world.
            //   Translate=false → bone matrices already encode world position (world objects, effects);
            //                     pass identity (origin=0, scale=1) so the shader adds nothing.
            const float* gpuBodyOrigin = m_LastTranslate ? BodyOrigin : nullptr; // nullptr → BMDMeshShader uses (0,0,0)
            float        gpuBodyScale  = m_LastTranslate ? BodyScale  : 1.0f;
            // `wave` (function-top local, computed at line ~1118) matches the CPU g_chrome[] formula's
            // wave term exactly — consumed by the shader when shaderRenderMode is 3 (chromeGpuEligible)
            // or 4 (itemSpecularGpuEligible).
            GLuint chromeTex1ID = 0;
            GLuint metalTexID = 0;
            GLuint chromeTex2ID = 0;
            float chromeWave2 = 0.0f;
            float chromeLightVecX = 0.0f, chromeLightVecY = 0.0f;
            const float* specularTint = nullptr;
            int chromeVariant = 0;
            float chromeTimeTerm = 0.0f;
            if (itemSpecularGpuEligible)
            {
                chromeTex1ID = Bitmaps.GetTexture(BITMAP_CHROME)->TextureNumber;
                specularTint = CItemSpecularShader::Instance().GetSpecularTint();
                if (activeSpecularVariant == SHADER_VARIANT_CHROME_METAL || activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V1 ||
                    activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V2)
                    metalTexID = Bitmaps.GetTexture(BITMAP_SHINY)->TextureNumber;
                if (activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V1 || activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V2)
                    chromeTex2ID = Bitmaps.GetTexture(BITMAP_CHROME2)->TextureNumber;
                if (activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V1)
                {
                    // Matches the CPU g_chrome[] RENDER_CHROME2 prepass's Wave2s exactly (ZzzBMD.cpp ~1631).
                    chromeWave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;
                }
                else if (activeSpecularVariant == SHADER_VARIANT_FULL_SPECULAR_V2)
                {
                    // Matches the CPU g_chrome[] RENDER_CHROME4 prepass's Lp vector exactly (ZzzBMD.cpp ~1633).
                    chromeLightVecX = (float)cos(WorldTime * 0.001f);
                    chromeLightVecY = (float)sin(WorldTime * 0.002f);
                }
            }
            else if (chromeGpuEligible)
            {
                // DXP-20 inc5: finalRenderFlags collapses CHROME2/3/5/6/7/METAL down to RENDER_CHROME
                // (see the `finalRenderFlags = RENDER_CHROME;` default a few hundred lines up), so this
                // GPU-skinned path was already silently taking these variants before this uniform
                // existed -- always with the plain-chrome formula. Select the CPU-verified per-variant
                // formula (ZzzBMD.cpp's g_chrome[] loop, same bit-check order) via u_ChromeVariant.
                // CHROME4 and OIL get their own distinct finalRenderFlags and never reach this branch --
                // they correctly stay on the CPU dynamic-VBO path (mode 1) with the right formula already.
                if ((renderFlags & RENDER_CHROME2) == RENDER_CHROME2)
                {
                    chromeVariant = 1;
                    chromeWave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;
                }
                else if ((renderFlags & RENDER_CHROME3) == RENDER_CHROME3)
                {
                    chromeVariant = 2;
                }
                else if ((renderFlags & RENDER_CHROME5) == RENDER_CHROME5)
                {
                    chromeVariant = 3;
                    chromeLightVecX = (float)cos(WorldTime * 0.001f);
                    chromeLightVecY = (float)sin(WorldTime * 0.002f);
                }
                else if ((renderFlags & RENDER_CHROME6) == RENDER_CHROME6)
                {
                    chromeVariant = 4;
                    chromeWave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;
                }
                else if ((renderFlags & RENDER_CHROME7) == RENDER_CHROME7)
                {
                    chromeVariant = 5;
                    chromeTimeTerm = static_cast<float>(WorldTime) * 0.00006f;
                }
                else if ((renderFlags & RENDER_METAL) == RENDER_METAL)
                {
                    chromeVariant = 6;
                }
                // else: plain RENDER_CHROME -> chromeVariant stays 0 (existing formula, untouched).
            }
            // DXP-20 increment 1: lighting moves in-shader (BMDMeshShader now computes it from the
            // GPU-skinned normal + u_LightDir/u_BodyLight/u_LightEnable) — the per-corner
            // LightTransform color VBO upload that used to run here every draw is gone; only alpha
            // and the light state need passing, as uniforms. `useLight` mirrors BMD::Transform's
            // IntensityTransform gating (RENDER_BRIGHT / !LightEnable / StreamMesh keep flat BodyLight).
            bool isBright = (renderFlags & RENDER_BRIGHT) != 0;
            bool useLight = !isBright && (LightEnable && meshIndex != StreamMesh);

            // FIX-549: blendTintAlpha is 1.0 outside the blendMeshIndex<=-2 branch (no-op here),
            // and blendMeshAlpha inside it -- carries that branch's fade into the shader uniform
            // since glColor3f() above is a no-op under Core Profile.
            const float shaderBodyLight[3] = {
                BodyLight[0] * blendTintAlpha,
                BodyLight[1] * blendTintAlpha,
                BodyLight[2] * blendTintAlpha
            };

            BMDMeshShader::Instance().Bind(shaderRenderMode, EnableWave ? blendMeshTextureCoordU : 0.0f, EnableWave ? blendMeshTextureCoordV : 0.0f, activeTexID, mvp, 1, gpuBodyOrigin, gpuBodyScale, wave, chromeTex1ID, specularTint, metalTexID, chromeTex2ID, chromeWave2, chromeLightVecX, chromeLightVecY, m_LastLightPosition, shaderBodyLight, useLight ? 1 : 0, alpha, chromeVariant, chromeTimeTerm);

            const int flatCount  = m_MeshIndexCount[meshIndex];
            const int baseCorner = m_MeshIndexOffset[meshIndex];

            BindVAO(m_VAO_StaticGPU);

            glDrawArrays(GL_TRIANGLES, baseCorner, flatCount);

            // DXP-22 step 4: no reflexive BindVAO(0)/Unbind() here -- the next draw's own Bind()/
            // BindVAO() call (any of the 4 shader classes, or this same one) self-corrects via the
            // cache regardless of what's left bound. The only case that needs program forced back to
            // 0 is immediately before raw fixed-function glBegin/glEnd draws (ZzzLodTerrain.cpp's
            // grass-pass sites), which now call UnbindAllShaders() explicitly at pass level instead
            // of relying on this per-mesh reset. See DXP-22-bind-state-monopoly-perf.md.
            fn_glBindBuffer(GL_ARRAY_BUFFER, 0);
            return;
        }

        // Excludes bShaderActive: this branch binds BMDMeshShader directly, which would silently
        // steal the GL program away from CItemSpecularShader's already-bound program (TASK-28
        // increment 2) for any item-specular mesh that falls through here (e.g. lacking a static
        // GPU VBO). Such meshes must instead fall all the way through to the legacy
        // CItemSpecularShader CPU path below, which is unaffected by this exclusion.
        if (!bShaderActive && m_StaticGeomReady && m_MeshIndexCount && m_MeshIndexOffset
            && m_MeshIndexCount[meshIndex] > 0)
        {
            materializeCpuLightingAndChrome(); // DXP-20 inc4: UploadDynamicBuffers() reads LightTransform/g_chrome
            UploadDynamicBuffers(meshIndex, alpha, renderFlags);

            // Only draw via VBO path if dynamic buffers were successfully uploaded this frame
            const bool dynVBReady = (m_VBO_Dynamic != 0);
            if (m_DynBufsReady && dynVBReady)
            {
                int shaderRenderMode = 0;
                if (finalRenderFlags == RENDER_BRIGHT)
                {
                    shaderRenderMode = 2;
                }
                else if (finalRenderFlags == RENDER_CHROME || finalRenderFlags == RENDER_CHROME4 || finalRenderFlags == RENDER_OIL)
                {
                    shaderRenderMode = 1;
                }

                // TASK-27: MVP fetched via per-body cache — computed once per BeginRender/EndRender
                // bracket instead of re-querying the driver for every mesh.
                GLfloat mvp[16];
                GetCurrentMVP(mvp);

                GLuint activeTexID = texture ? texture->TextureNumber : 0;
                BMDMeshShader::Instance().Bind(shaderRenderMode, EnableWave ? blendMeshTextureCoordU : 0.0f, EnableWave ? blendMeshTextureCoordV : 0.0f, activeTexID, mvp, 0);

                BindVAO(m_VAO_Static);

                // Single interleaved VBO: Pos (vec3) | UV (vec2) | Color (vec4) = 9 floats (36 bytes stride)
                fn_glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Dynamic);

                constexpr GLsizei stride = 9 * sizeof(float);

                // Location 0: Position (3 floats) at offset 0
                fn_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
                fn_glEnableVertexAttribArray(0);

                // Location 1: UV (2 floats) at offset 12 bytes (3 * sizeof(float))
                fn_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
                fn_glEnableVertexAttribArray(1);

                // Location 2: Color (4 floats) at offset 20 bytes (5 * sizeof(float))
                fn_glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
                fn_glEnableVertexAttribArray(2);

                // Flat expansion — glDrawArrays starting at index 0.
                glDrawArrays(GL_TRIANGLES, 0, m_MeshIndexCount[meshIndex]);

                // DXP-22 step 4: no reflexive BindVAO(0)/Unbind() here -- see the GPU-skinned path
                // above for the full reasoning (self-corrects via the wrapper cache).
                fn_glBindBuffer(GL_ARRAY_BUFFER, 0);
                return;
            }
        }
    }

#ifdef _DEBUG
    // DXP-08a Category 4 investigation (temporary): one-shot log of why this (model,mesh)
    // fell through to the legacy client-array path instead of a GPU VBO path, keyed on
    // (this, meshIndex) so repeats across frames don't spam. Remove once the trigger
    // condition is understood and the fallback itself is ported.
    {
        static std::set<std::pair<void*, int>> s_seenFallback;
        if (s_seenFallback.insert({ (void*)this, meshIndex }).second)
        {
            g_ErrorReport.Write(L"[DXP-08a-CAT4] RenderMesh fallback: this=%p meshIndex=%d NumMeshs=%d "
                L"bShaderActive=%d m_VAO_Static=%u m_MeshIndexCount=%p m_HasStaticGPUVBO=%d "
                L"m_VAO_StaticGPU=%u m_DynBufsReady=%d m_VBO_Dynamic=%u renderFlags=0x%X\r\n",
                (void*)this, meshIndex, NumMeshs, bShaderActive ? 1 : 0,
                m_VAO_Static, (void*)m_MeshIndexCount, m_HasStaticGPUVBO ? 1 : 0,
                m_VAO_StaticGPU, m_DynBufsReady ? 1 : 0, m_VBO_Dynamic, renderFlags);
        }
    }
#endif // _DEBUG

    // DXP-20 inc4: legacy client-array path -- reads VertexTransform/NormalTransform (RENDER_WAVE)
    // directly below, plus LightTransform/g_chrome via materializeCpuLightingAndChrome() (the
    // UploadDynamicBuffers call above already ran its own copy of this for its own fallthrough case;
    // both are idempotent per-mesh, so no double work if this is reached after that one).
    EnsureCpuVertices(meshIndex);
    materializeCpuLightingAndChrome();

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

    const int vertexCount = target_vertex_index + 1;

#ifdef _DEBUG
    // DXP-08a Category 4 investigation part 2 (temporary): the port didn't fix visibility on
    // first soak -- log what's actually about to be submitted, in case vertexCount is 0/bogus
    // or alpha is fully transparent, before assuming the IR:: submission itself is broken.
    {
        static std::set<std::pair<void*, int>> s_seenSubmit;
        if (s_seenSubmit.insert({ (void*)this, meshIndex }).second)
        {
            g_ErrorReport.Write(L"[DXP-08a-CAT4-SUBMIT] this=%p meshIndex=%d vertexCount=%d "
                L"NumTriangles=%d alpha=%.3f color0=(%.2f,%.2f,%.2f,%.2f) vertex0=(%.2f,%.2f,%.2f) "
                L"finalRenderFlags=0x%X enableColor=%d enableLight=%d\r\n",
                (void*)this, meshIndex, vertexCount, m->NumTriangles, alpha,
                vertexCount > 0 ? colors[0][0] : -1.f, vertexCount > 0 ? colors[0][1] : -1.f,
                vertexCount > 0 ? colors[0][2] : -1.f, vertexCount > 0 ? colors[0][3] : -1.f,
                vertexCount > 0 ? vertices[0][0] : 0.f, vertexCount > 0 ? vertices[0][1] : 0.f,
                vertexCount > 0 ? vertices[0][2] : 0.f, finalRenderFlags, enableColor ? 1 : 0,
                enableLight ? 1 : 0);
        }
    }
#endif // _DEBUG

    // DXP-08a Category 4: legacy client-array submit replaced with IR:: — the per-vertex
    // computation above (position/color/texcoord, including the CHROME/OIL/WAVE variants)
    // is unchanged; only how the already-computed vertices reach the GPU changes. This path
    // is reached when bShaderActive is true but the active CItemSpecularShader variant isn't
    // one of the GPU-eligible ones (see the m_VAO_Static gate above) — items render with the
    // plain textured look here rather than their specular effect, matching what the stale
    // "falls through to the legacy CItemSpecularShader CPU path" comment used to describe
    // before DXP-02 removed that path; the alternative was rendering nothing at all under
    // Core Profile, which is what this replaces.
    IR::Begin(GL_TRIANGLES);
    for (int i = 0; i < vertexCount; i++)
    {
        IR::Color4f(colors[i][0], colors[i][1], colors[i][2], colors[i][3]);
        IR::TexCoord2f(texCoords[i][0], texCoords[i][1]);
        IR::Vertex3fv(vertices[i]);
    }
    IR::End();
}

void BMD::RenderMeshEffect(int i, int iType, int iSubType, vec3_t Angle, VOID* obj)
{
    if (i >= NumMeshs || i < 0) return;

    Mesh_t* m = &Meshs[i];
    if (m->NumTriangles <= 0) return;

    EnsureCpuVertices(i); // DXP-20 inc4: spawn-position reads below (~20 sites) need mesh i materialized

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
    if (!CPlanarShadowShader::Instance().IsActive())
    {
        CalcShadowPosition(target, origin, sx, sy);
    }
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

    // DXP-06: when the shader is active, vertices are still raw (unskewed) world positions — the
    // GPU vertex shader does the ground-skew projection now instead of CalcShadowPosition() above.
    // Submit through CPlanarShadowShader's own VAO/VBO (explicit attribute 0), not the legacy
    // glVertexPointer path, which would read attribute 0 from unrelated client-array state.
    if (CPlanarShadowShader::Instance().IsActive())
    {
        CPlanarShadowShader::Instance().Draw(reinterpret_cast<const float*>(vertices), target_vertex_index + 1);
    }
    else
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glDrawArrays(GL_TRIANGLES, 0, target_vertex_index + 1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void BMD::AddMeshShadowTriangles(const int blendMesh, const int hiddenMesh, const int startMesh, const int endMesh, const float sx, const float sy) const
{
    auto vertices = RenderArrayVertices;
    int target_vertex_index = -1;

    const bool bShaderActive = CPlanarShadowShader::Instance().IsActive();

    // DXP-20 increment 3: a mesh with a ready static GPU VBO (built once by UploadStaticVBOs(),
    // independent of chrome/render-mode eligibility -- shadows are flat silhouettes, so texture
    // doesn't matter) skips the CPU VertexTransform replay entirely and draws straight from that
    // VBO, skinning + ground-skewing inside CPlanarShadowShader's vertex shader. Meshes without one
    // (below the static-VBO size threshold, or not yet uploaded) fall back to the CPU path below,
    // batched into one draw call exactly as before.
    const float* gpuBodyOrigin = nullptr;
    float gpuBodyScale = 1.0f;
    if (bShaderActive)
    {
        // Mirrors RenderMesh()'s GPU skinning world-space placement exactly (ZzzBMD.cpp ~1502-1507).
        gpuBodyOrigin = m_LastTranslate ? BodyOrigin : nullptr;
        gpuBodyScale  = m_LastTranslate ? BodyScale  : 1.0f;

        // Mirrors RenderMesh()'s active-bone-palette resolution (ZzzBMD.cpp ~1498-1500);
        // UploadBones() dedups on (pointer, g_BoneTransformVersion), so this is cheap even when
        // RenderMesh() already uploaded the same palette earlier this frame.
        const void* activeBones = g_pActiveBoneTransform ? (const void*)g_pActiveBoneTransform : (m_pCurrentBoneTransform ? (const void*)m_pCurrentBoneTransform : (const void*)BoneTransform);
        BoneUBO::Instance().UploadBones(activeBones, MAX_BONES, g_BoneTransformVersion);
    }

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

        if (bShaderActive && m_HasStaticGPUVBO && m_VAO_StaticGPU != 0 &&
            m_MeshIndexCount && m_MeshIndexOffset && m_MeshIndexCount[i] > 0)
        {
            CPlanarShadowShader::Instance().DrawGPUSkinned(m_VAO_StaticGPU, m_MeshIndexOffset[i], m_MeshIndexCount[i], gpuBodyOrigin, gpuBodyScale);
            continue;
        }

        EnsureCpuVertices(i); // DXP-20 inc4: CPU shadow fallback for meshes without a static GPU VBO

        for (int j = 0; j < mesh->NumTriangles; j++)
        {
            const auto* tp = &mesh->Triangles[j];
            for (int k = 0; k < tp->Polygon; k++)
            {
                const int source_vertex_index = tp->VertexIndex[k];
                target_vertex_index++;

                VectorCopy(VertexTransform[i][source_vertex_index], vertices[target_vertex_index]);

                if (!bShaderActive)
                {
                    CalcShadowPosition(&vertices[target_vertex_index], BodyOrigin, sx, sy);
                }
            }
        }
    }

    if (target_vertex_index < 0)
    {
        return;
    }

    // DXP-06: see AddClothesShadowTriangles() above for why this branches on shader activity.
    if (bShaderActive)
    {
        CPlanarShadowShader::Instance().Draw(reinterpret_cast<const float*>(vertices), target_vertex_index + 1);
    }
    else
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glDrawArrays(GL_TRIANGLES, 0, target_vertex_index + 1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
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

    // GPU Planar Shadow Integration: Offloads 100% of character & monster ground skew projection to GLSL vertex shader
    // DXP-06: MVP is the plain camera view*projection matrix (shadow vertices are pre-transformed
    // to world space on the CPU, same as BMD::RenderMesh()'s non-GPU-skinned dynamic-VBO path) —
    // reuse the existing per-body cache instead of a fresh glGetFloatv (TASK-27).
    GLfloat shadowMVP[16];
    GetCurrentMVP(shadowMVP);
    bool bShaderActive = CPlanarShadowShader::Instance().Begin(BodyOrigin, shadowMVP, sx, sy, 0.45f);

    if (clothesCount == 0)
    {
        AddMeshShadowTriangles(blendMesh, hiddenMesh, startMesh, endMesh, sx, sy);
    }
    else
    {
        AddClothesShadowTriangles(pClothes, clothesCount, sx, sy);
    }

    if (bShaderActive)
    {
        CPlanarShadowShader::Instance().End();
    }

    EndRender();
    EnableDepthMask();
}

void BMD::UploadStaticVBOs()
{
    if (!Meshs || NumMeshs <= 0) return;
    if (m_VAO_Static != 0) return;

    if (!LoadBMDGLFunctions() ||
        !fn_glGenVertexArrays ||
        !fn_glGenBuffers || !fn_glBindBuffer ||
        !fn_glBufferData || !fn_glVertexAttribPointer ||
        !fn_glEnableVertexAttribArray) return;

    ReleaseStaticVBOs();

    m_MeshUVOffset    = new int[NumMeshs]();
    m_MeshIndexOffset = new int[NumMeshs]();  // flat corner offset
    m_MeshIndexCount  = new int[NumMeshs]();  // flat corner count (drawn via glDrawArrays)

    // Count flat corners per mesh (triangles × 3, quads × 6).
    // UV is NOT built here — it is dynamic (chrome vs texture) and uploaded
    // per-mesh per-frame inside UploadDynamicBuffers.
    int flatOffset = 0;
    for (int i = 0; i < NumMeshs; i++)
    {
        Mesh_t* m = &Meshs[i];
        m_MeshUVOffset[i]    = flatOffset;
        m_MeshIndexOffset[i] = flatOffset;
        if (!m || !m->Triangles || !m->Vertices || m->NumTriangles <= 0 || m->NumVertices <= 0)
        {
            m_MeshIndexCount[i] = 0;
            continue;
        }

        int meshFlatCount = 0;
        for (int j = 0; j < m->NumTriangles; j++)
        {
            int p = m->Triangles[j].Polygon;
            if (p == 3)       meshFlatCount += 3;
            else if (p == 4)  meshFlatCount += 6;  // quad → 2 triangles
        }
        m_MeshIndexCount[i] = meshFlatCount;
        flatOffset += meshFlatCount;
    }

    if (flatOffset <= 0) return;  // Genuinely empty model — nothing to draw.

    // Pre-allocate CPU staging buffer once per model — 9 floats per flat corner.
    // This eliminates per-frame std::vector heap allocations during dynamic upload.
    m_Staging.assign(flatOffset * 9, 0.0f);

    // Create VAO for OpenGL core profile (dynamic VBO fallback path)
    fn_glGenVertexArrays(1, &m_VAO_Static);
    BindVAO(m_VAO_Static);
    BindVAO(0);

    // Build Static Geometry VBO (GL_STATIC_DRAW) for GPU Bone Skinning path
    // Interleaved layout per corner: RestPos (3 floats) | static UV (2 floats) | RestNormal (3 floats) | BoneIndex (1 int)
    // Stride = 9 floats/ints (36 bytes)
    std::vector<float> gpuGeomBuffer(flatOffset * 9, 0.0f);
    int gpuIdx = 0;

    for (int i = 0; i < NumMeshs; i++) {
        Mesh_t* m = &Meshs[i];
        if (!m || !m->Triangles || !m->Vertices || m->NumTriangles <= 0 || m->NumVertices <= 0) continue;

        for (int j = 0; j < m->NumTriangles; j++) {
            const Triangle_t& tri = m->Triangles[j];
            auto emitGPUCorner = [&](int k) {
                int vi  = tri.VertexIndex[k];
                int ni  = (tri.NormalIndex != nullptr) ? tri.NormalIndex[k] : -1;
                int tci = (tri.TexCoordIndex != nullptr) ? tri.TexCoordIndex[k] : -1;

                // Rest Position (3 floats) & Bone Index
                int boneNode = 0;
                if (vi >= 0 && vi < m->NumVertices) {
                    gpuGeomBuffer[gpuIdx * 9 + 0] = m->Vertices[vi].Position[0];
                    gpuGeomBuffer[gpuIdx * 9 + 1] = m->Vertices[vi].Position[1];
                    gpuGeomBuffer[gpuIdx * 9 + 2] = m->Vertices[vi].Position[2];
                    boneNode = m->Vertices[vi].Node;
                }
                // Static UV (2 floats)
                if (m->TexCoords && tci >= 0 && tci < m->NumTexCoords) {
                    gpuGeomBuffer[gpuIdx * 9 + 3] = m->TexCoords[tci].TexCoordU;
                    gpuGeomBuffer[gpuIdx * 9 + 4] = m->TexCoords[tci].TexCoordV;
                }
                // Rest Normal (3 floats)
                if (m->Normals && ni >= 0 && ni < m->NumNormals) {
                    gpuGeomBuffer[gpuIdx * 9 + 5] = m->Normals[ni].Normal[0];
                    gpuGeomBuffer[gpuIdx * 9 + 6] = m->Normals[ni].Normal[1];
                    gpuGeomBuffer[gpuIdx * 9 + 7] = m->Normals[ni].Normal[2];
                }
                // Bone Index (1 int bitcast to float storage)
                int32_t bNode = (int32_t)boneNode;
                float bFloat = 0.0f;
                memcpy(&bFloat, &bNode, sizeof(int32_t));
                gpuGeomBuffer[gpuIdx * 9 + 8] = bFloat;

                gpuIdx++;
            };

            if (tri.Polygon == 3) {
                emitGPUCorner(0); emitGPUCorner(1); emitGPUCorner(2);
            } else if (tri.Polygon == 4) {
                emitGPUCorner(0); emitGPUCorner(1); emitGPUCorner(2);
                emitGPUCorner(0); emitGPUCorner(2); emitGPUCorner(3);
            }
        }
    }

    if (!gpuGeomBuffer.empty()) {
        m_StaticGeomReady = true;

        fn_glGenVertexArrays(1, &m_VAO_StaticGPU);
        fn_glGenBuffers(1, &m_VBO_StaticGeom);

        BindVAO(m_VAO_StaticGPU);
        fn_glBindBuffer(GL_ARRAY_BUFFER, m_VBO_StaticGeom);
        fn_glBufferData(GL_ARRAY_BUFFER, gpuGeomBuffer.size() * sizeof(float), gpuGeomBuffer.data(), GL_STATIC_DRAW);

        constexpr GLsizei stride = 9 * sizeof(float); // 36 bytes

        // location 0: Rest Position (3 floats) at offset 0
        fn_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        fn_glEnableVertexAttribArray(0);

        // location 1: Static UV (2 floats) at offset 12
        fn_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        fn_glEnableVertexAttribArray(1);

        // location 3: Rest Normal (3 floats) at offset 20
        fn_glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        fn_glEnableVertexAttribArray(3);

        // location 4: Bone Index (1 int) at offset 32
        if (fn_glVertexAttribIPointer != nullptr) {
            fn_glVertexAttribIPointer(4, 1, GL_INT, stride, (void*)(8 * sizeof(float)));
            fn_glEnableVertexAttribArray(4);
        }

        BindVAO(0);
        fn_glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_HasStaticGPUVBO = true; // TASK-20 re-enabled 2026-07-31 for retest — was unconditionally disabled here, making the GPU-skin path (BoneUBO, u_UseGPUSkin, bone-index VAO) dead code despite the task file claiming it was "verified OK". Watch armor/linked items/procedural effects specifically — that's what the disabling comment cited.

        // Pre-allocate color buffer for GPU skinning path — sized for all flat corners
        // so glBufferSubData(offset=0, size=meshFlatCount*16) is always in-bounds.
        fn_glGenBuffers(1, &m_VBO_Color);
        fn_glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Color);
        fn_glBufferData(GL_ARRAY_BUFFER, flatOffset * 4 * sizeof(float), nullptr, GL_STREAM_DRAW);
        
        BindVAO(m_VAO_StaticGPU);
        fn_glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        fn_glEnableVertexAttribArray(2);
        BindVAO(0);
        
        fn_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}


void BMD::UploadDynamicBuffers(int meshIndex, float alpha, int renderFlags)
{
    if (!Meshs || meshIndex < 0 || meshIndex >= NumMeshs) return;
    Mesh_t* m = &Meshs[meshIndex];
    if (!m || !m->Triangles || !m->Vertices || m->NumTriangles <= 0 || m->NumVertices <= 0) return;

    // VertexTransform, LightTransform, g_chrome are GLOBAL STATIC ARRAYS — never null.
    if (!m_MeshIndexCount || m_MeshIndexCount[meshIndex] == 0) return;

    if (!LoadBMDGLFunctions() || !fn_glGenBuffers || !fn_glBindBuffer || !fn_glBufferData) return;

    // DXP-20 inc4: only called from RenderMesh()'s CPU dynamic-VBO fallback branch (never the
    // GPU-skinned path), so an unconditional Ensure here can't defeat the gate for GPU-eligible
    // meshes. g_chrome/LightTransform (read below) are populated by RenderMesh()'s own prologue
    // loops, handled separately (DXP-20 inc4 Step C).
    EnsureCpuVertices(meshIndex);

    // Lazy-create single dynamic interleaved VBO on first use
    if (m_VBO_Dynamic == 0) fn_glGenBuffers(1, &m_VBO_Dynamic);

    const int flatCount  = m_MeshIndexCount[meshIndex];
    const int baseCorner = m_MeshIndexOffset[meshIndex];

    // Determine UV source:
    //   Chrome modes: g_chrome[NormalIndex[k]] — per-frame, computed from NormalTransform
    //   Texture mode: TexCoords[TexCoordIndex[k]] — static mesh UVs
    const bool useChrome = (renderFlags & RENDER_CHROME)  || (renderFlags & RENDER_CHROME2) ||
                           (renderFlags & RENDER_CHROME3)  || (renderFlags & RENDER_CHROME4) ||
                           (renderFlags & RENDER_CHROME5)  || (renderFlags & RENDER_CHROME6) ||
                           (renderFlags & RENDER_CHROME7)  || (renderFlags & RENDER_OIL)     ||
                           (renderFlags & RENDER_METAL);

    // RENDER_BRIGHT uses BodyLight as color for ALL corners (matching FFP enableColor=false).
    const bool isBright  = (renderFlags & RENDER_BRIGHT) != 0;
    const bool useLight  = !isBright && (LightEnable && meshIndex != StreamMesh);

    // Ensure m_Staging is large enough
    if ((int)m_Staging.size() < (baseCorner + flatCount) * 9)
    {
        m_Staging.resize((baseCorner + flatCount) * 9, 0.0f);
    }

    float* dst = m_Staging.data() + baseCorner * 9;
    int idx = 0;

    for (int j = 0; j < m->NumTriangles && idx < flatCount; j++)
    {
        const Triangle_t& tri = m->Triangles[j];

        auto emitCorner = [&](int k)
        {
            int vi   = tri.VertexIndex[k];
            int ni   = (tri.NormalIndex   != nullptr) ? tri.NormalIndex[k]   : -1;
            int tci  = (tri.TexCoordIndex != nullptr) ? tri.TexCoordIndex[k] : -1;

            // Position (3 floats)
            if (vi >= 0 && vi < m->NumVertices)
            {
                dst[0] = VertexTransform[meshIndex][vi][0];
                dst[1] = VertexTransform[meshIndex][vi][1];
                dst[2] = VertexTransform[meshIndex][vi][2];
            }
            else
            {
                dst[0] = dst[1] = dst[2] = 0.0f;
            }

            // UV (2 floats)
            if (useChrome && ni >= 0 && ni < m->NumNormals)
            {
                dst[3] = g_chrome[ni][0];
                dst[4] = g_chrome[ni][1];
            }
            else if (!useChrome && m->TexCoords && tci >= 0 && tci < m->NumTexCoords)
            {
                dst[3] = m->TexCoords[tci].TexCoordU;
                dst[4] = m->TexCoords[tci].TexCoordV;
            }
            else
            {
                dst[3] = dst[4] = 0.0f;
            }

            // Color (4 floats)
            if (useLight && ni >= 0 && ni < m->NumNormals)
            {
                dst[5] = LightTransform[meshIndex][ni][0];
                dst[6] = LightTransform[meshIndex][ni][1];
                dst[7] = LightTransform[meshIndex][ni][2];
            }
            else
            {
                dst[5] = BodyLight[0];
                dst[6] = BodyLight[1];
                dst[7] = BodyLight[2];
            }
            dst[8] = alpha;

            dst += 9;
            idx++;
        };

        if (tri.Polygon == 3)
        {
            emitCorner(0); emitCorner(1); emitCorner(2);
        }
        else if (tri.Polygon == 4)
        {
            emitCorner(0); emitCorner(1); emitCorner(2);
            emitCorner(0); emitCorner(2); emitCorner(3);
        }
    }

    // Single 1x interleaved VBO orphan-upload per mesh (map-discard semantics)
    fn_glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Dynamic);
    fn_glBufferData(GL_ARRAY_BUFFER, flatCount * 9 * sizeof(float), m_Staging.data() + baseCorner * 9, GL_STREAM_DRAW);
    fn_glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_DynBufsReady = true;
}



void BMD::ReleaseDynamicVBOs()
{
    LoadBMDGLFunctions();
    if (m_VBO_Dynamic != 0 && fn_glDeleteBuffers) {
        fn_glDeleteBuffers(1, &m_VBO_Dynamic);
        m_VBO_Dynamic = 0;
    }
    m_Staging.clear();
    m_Staging.shrink_to_fit();
    m_DynBufsReady = false;
}

void BMD::ReleaseStaticVBOs()
{
    m_StaticGeomReady = false;

    LoadBMDGLFunctions();
    if (m_EBO != 0 && fn_glDeleteBuffers) {
        fn_glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    if (m_VBO_StaticGeom != 0 && fn_glDeleteBuffers) {
        fn_glDeleteBuffers(1, &m_VBO_StaticGeom);
        m_VBO_StaticGeom = 0;
    }
    if (m_VBO_Color != 0 && fn_glDeleteBuffers) {
        fn_glDeleteBuffers(1, &m_VBO_Color);
        m_VBO_Color = 0;
    }
    if (m_VAO_StaticGPU != 0 && fn_glDeleteVertexArrays) {
        fn_glDeleteVertexArrays(1, &m_VAO_StaticGPU);
        m_VAO_StaticGPU = 0;
        InvalidateVAOCache(); // DXP-22 fix: deleted id may get recycled by a later model's VAO
    }
    m_HasStaticGPUVBO = false;
    if (m_VAO_Static != 0 && fn_glDeleteVertexArrays) {
        fn_glDeleteVertexArrays(1, &m_VAO_Static);
        m_VAO_Static = 0;
        InvalidateVAOCache();
    }
    if (m_MeshUVOffset) { delete[] m_MeshUVOffset; m_MeshUVOffset = nullptr; }
    if (m_MeshIndexOffset) { delete[] m_MeshIndexOffset; m_MeshIndexOffset = nullptr; }
    if (m_MeshIndexCount) { delete[] m_MeshIndexCount; m_MeshIndexCount = nullptr; }
}

void BMD::Release()
{
    ReleaseDynamicVBOs();
    ReleaseStaticVBOs();

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


bool BMD::Open2(const wchar_t* DirName, const wchar_t* ModelFileName, bool bReAlloc)
{
    if (m_bCompletedAlloc)
    {
        if (!bReAlloc)
            return true;
        Release();
    }

    wchar_t ModelPath[260] = {};
    _snwprintf(ModelPath, std::size(ModelPath), L"%ls%ls", DirName, ModelFileName);

    FILE* fp = _wfopen(ModelPath, L"rb");
    if (!fp)
    {
        //// wprintf(L"[Open2] ERROR: Unable to open file: %ls\n", ModelPath);
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
        // The on-disk size field is 32-bit; `long` is 8 bytes on LP64 (Linux
        // x64), which would read past the field and produce a garbage size.
        std::int32_t encSize = *(std::int32_t*)(fileData.get() + ptr); ptr += sizeof(std::int32_t);
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

    if (NumMeshs > 0)
    {
        UploadStaticVBOs();
    }

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
    // The on-disk size field is 32-bit; writing a `long` would emit 8 bytes on
    // LP64 (Linux x64) and corrupt the file.
    auto lEncSize = (std::int32_t)MapFileEncrypt(nullptr, pbyBuffer, lSize);
    auto* pbyEnc = new BYTE[lEncSize];
    MapFileEncrypt(pbyEnc, pbyBuffer, lSize);
    fwrite(&lEncSize, sizeof(std::int32_t), 1, fp);
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