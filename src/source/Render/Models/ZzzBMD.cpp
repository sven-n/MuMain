///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <cstdint>
// u_char is used below. On glibc it arrives transitively through other
// headers; musl targets (Emscripten) need its declaring header explicitly.
#include <sys/types.h>
#include <cstring>
#include <cmath>
#include <cassert>
#include <climits>
#include <map>
#include <optional>
#include <limits>
#include <set>
#include <span>
#include <tuple>
#include <utility>
#include "Render/Textures/ZzzOpenglUtil.h"

#include "Engine/Object/ZzzInfomation.h"
#include "ZzzBMD.h"
#include "Render/Models/AnimationSampling.h"
#include "Render/Models/Gltf/SkinnedModel.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "SMD.h"
#include "BmdModulusCipher.h"
#include "Render/Effects/ZzzEffect.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Camera/CameraState.h"

#include "Camera/CameraMove.h"
#include "Engine/Physics/PhysicsManager.h"
#include "UI/Core/WindowSystem.h"
#include "Render/Models/GpuSkinningPath.h"
#include "Render/Models/SkinnedVertexPacking.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/RenderUtils.h"
#include "Render/Terrain/TerrainHeightGrid.h"
#include "Core/Utilities/FrameProfiler.h"
#include "Scenes/MainScene.h"
#include "Render/Lighting/LightMoodSetting.h"

using mu::PackABGR;

BMD* Models;
BMD* ModelsDump;

vec4_t BoneQuaternion[MAX_BONES];
short  BoundingVertices[MAX_BONES];
vec3_t BoundingMin[MAX_BONES];
vec3_t BoundingMax[MAX_BONES];

float  BoneTransform[MAX_BONES][3][4];
// BoneTransform's slots from here on are identity; Animation's tail fill only rewrites what lies
// below it. Starts at MAX_BONES: the zero-initialised array is not identity.
static int s_boneTransformWrittenEnd = MAX_BONES;
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

// Lazy CPU-skin materialization. Bumped by every TransformCheap() call;
// a BMD's m_SkinStamp matching this value means it currently owns VertexTransform/NormalTransform/
// IntensityTransform (last-writer-wins global scratch, same sharing model as before this increment).
// g_LazyCpuSkin is a kill switch -- false reproduces pre-increment-4 eager behavior exactly.
static uint32_t g_SkinStampCounter = 0;
static bool g_LazyCpuSkin = true; // Lazy CPU-skin gate, on by default.

vec3_t RenderArrayVertices[MAX_VERTICES * 3];
vec4_t RenderArrayColors[MAX_VERTICES * 3];
vec2_t RenderArrayTexCoords[MAX_VERTICES * 3];

namespace
{
std::span<mu::Vertex3D> GetRendererVertexScratch(std::size_t requiredVertexCount)
{
    static thread_local std::vector<mu::Vertex3D> vertices;
    if (vertices.size() < requiredVertexCount)
    {
        vertices.resize(requiredVertexCount);
    }
    return {vertices.data(), requiredVertexCount};
}

template <typename Vertex = mu::SkinnedVertex3D>
std::span<Vertex> GetRendererSkinnedVertexScratch(std::size_t requiredVertexCount)
{
    static thread_local std::vector<Vertex> vertices;
    if (vertices.size() < requiredVertexCount)
    {
        vertices.resize(requiredVertexCount);
    }
    return {vertices.data(), requiredVertexCount};
}

mu::SkinningTextureCoordinates GetSkinningTextureCoordinates(int renderFlags)
{
    if ((renderFlags & RENDER_CHROME2) == RENDER_CHROME2)
        return mu::SkinningTextureCoordinates::Chrome2;
    if ((renderFlags & RENDER_CHROME3) == RENDER_CHROME3)
        return mu::SkinningTextureCoordinates::Chrome3;
    if ((renderFlags & RENDER_CHROME4) == RENDER_CHROME4)
        return mu::SkinningTextureCoordinates::Chrome4;
    if ((renderFlags & RENDER_CHROME5) == RENDER_CHROME5)
        return mu::SkinningTextureCoordinates::Chrome5;
    if ((renderFlags & RENDER_CHROME6) == RENDER_CHROME6)
        return mu::SkinningTextureCoordinates::Chrome6;
    if ((renderFlags & RENDER_CHROME7) == RENDER_CHROME7)
        return mu::SkinningTextureCoordinates::Chrome7;
    if ((renderFlags & RENDER_OIL) == RENDER_OIL)
        return mu::SkinningTextureCoordinates::Oil;
    if ((renderFlags & RENDER_CHROME) == RENDER_CHROME)
        return mu::SkinningTextureCoordinates::Chrome;
    if ((renderFlags & RENDER_METAL) == RENDER_METAL)
        return mu::SkinningTextureCoordinates::Metal;
    return mu::SkinningTextureCoordinates::Mesh;
}

bool CanGpuSkinMesh(int finalRenderFlags, int renderFlags, const float (*boneMatrices)[3][4])
{
    const bool supportedMaterial = finalRenderFlags == RENDER_TEXTURE || finalRenderFlags == RENDER_COLOR ||
                                   finalRenderFlags == RENDER_BRIGHT ||
                                   GetSkinningTextureCoordinates(renderFlags) != mu::SkinningTextureCoordinates::Mesh;
    return boneMatrices && supportedMaterial && !(renderFlags & (RENDER_SHADOWMAP | RENDER_WAVE));
}

// The bone palette of a GPU-skinned draw and the version the renderer's upload cache keys on.
struct SkinPalette
{
    std::span<const float> matrices;
    std::uint32_t version = 0;
};

// The weighted palette of a glTF model's current bones, rebuilt only when the
// bones or scales changed since the last draw (one body draws several meshes).
SkinPalette CurrentWeightedSkinPalette(const Render::Models::Gltf::SkinnedModel& model,
                                       const float (*boneMatrices)[3][4], float boneScale, float restPoseScale)
{
    static thread_local struct
    {
        const Render::Models::Gltf::SkinnedModel* model = nullptr;
        const float (*boneMatrices)[3][4] = nullptr;
        unsigned int boneVersion = 0;
        float boneScale = 0.f;
        float restPoseScale = 0.f;
        std::vector<float> palette;
        std::uint32_t version = 0;
    } cache;
    const bool current = cache.model == &model && cache.boneMatrices == boneMatrices &&
                         cache.boneVersion == g_BoneTransformVersion && cache.boneScale == boneScale &&
                         cache.restPoseScale == restPoseScale && !cache.palette.empty();
    if (!current)
    {
        Render::Models::BuildWeightedSkinPalette(model, boneMatrices, boneScale, restPoseScale, cache.palette);
        cache.model = &model;
        cache.boneMatrices = boneMatrices;
        cache.boneVersion = g_BoneTransformVersion;
        cache.boneScale = boneScale;
        cache.restPoseScale = restPoseScale;
        ++cache.version;
    }
    return {cache.palette, cache.version};
}

// A packed mesh's triangle corners (the mesh cache draws without indices), plus a scrolling texture's offset.
std::span<const mu::SkinnedWeightedVertex3D> WeightedCorners(const Render::Models::Gltf::PackedMesh& mesh, float waveU,
                                                             float waveV)
{
    const std::size_t cornerCount = mesh.indices.empty() ? mesh.vertices.size() : mesh.indices.size();
    auto corners = GetRendererSkinnedVertexScratch<mu::SkinnedWeightedVertex3D>(cornerCount);
    for (std::size_t i = 0; i < cornerCount; ++i)
    {
        corners[i] = mesh.vertices[mesh.indices.empty() ? i : mesh.indices[i]];
        corners[i].u += waveU;
        corners[i].v += waveV;
    }
    return corners;
}

std::pair<int, int> ResolveMeshRange(int meshCount, int requestedStart, int requestedEnd)
{
    return {requestedStart == -1 ? 0 : requestedStart, requestedEnd == -1 ? meshCount : requestedEnd};
}

// One cached shadow stream per model and RenderBodyShadow() mesh selection (model, start, end, hidden mesh,
// blend texture); an entry's address is its key in the renderer's mesh cache. Kept outside BMD, which
// OpenPlayers() zero-fills, and never destroyed, so a model released at exit still finds it.
using ShadowStreamId = std::tuple<std::uintptr_t, int, int, int, int>;
std::map<ShadowStreamId, char>& ShadowStreams()
{
    static auto* streams = new std::map<ShadowStreamId, char>();
    return *streams;
}
} // namespace

bool  StopMotion = false;
thread_local float ParentMatrix[3][4];

static vec3_t LightVector = { 0.f, -0.1f, -0.8f };
static vec3_t LightVector2 = { 0.f, -0.5f, -0.8f };

// A joint's local pose in a model from a glTF binary at one sample of the
// playback rule: MU key time t of the action is clip time t / K x cycle.
static void GltfSample(const Render::Models::Gltf::SkinnedModel& gltf, const Action_t* Actions, int joint,
                       const Render::Models::AnimationSampling::KeySample& sample,
                       Render::Models::Gltf::JointPose& pose)
{
    const Action_t& action = Actions[sample.action];
    const int intervals = Render::Models::Gltf::SkinnedModel::ClipKeyIntervals(action.NumAnimationKeys,
                                                                               action.LockPositions, action.Loop);
    gltf.SampleJoint(joint, sample.action, sample.time, intervals, pose);
}

// The key time of the crossfade into `action` that the model file sets, or
// nothing (a BMD, or the playback rule's fade).
static std::optional<float> GltfCrossfadeKeyTime(const BMD& model, unsigned short action)
{
    const auto* gltf = Render::Models::Gltf::SkinnedModelOf(model);
    if (!gltf || action >= gltf->actionCrossfades.size() || !gltf->actionCrossfades[action])
    {
        return std::nullopt;
    }
    const Action_t& a = model.Actions[action];
    return Render::Models::AnimationSampling::CrossfadeKeyTime(
        {a.NumAnimationKeys, a.LockPositions, a.PlaySpeed, a.Loop}, *gltf->actionCrossfades[action]);
}

void BMD::Animation(float (*BoneMatrix)[3][4], float AnimationFrame, float PriorFrame, unsigned short PriorAction,
                    vec3_t Angle, vec3_t HeadAngle, bool Parent, bool Translate, const float (*ExtParentMatrix)[4],
                    short CurrentActionArg, int BoneMatrixCapacity)
{
    if (NumActions <= 0) return;

    unsigned short currentAction = (CurrentActionArg < 0) ? CurrentAction : (unsigned short)CurrentActionArg;
    if (PriorAction >= NumActions) PriorAction = 0;
    if (currentAction >= NumActions) currentAction = 0;

    // Which keys of which action the pose is made of (the playback rule, see
    // AnimationSampling.h): MU's blend of key floor(PriorFrame) into key
    // floor(AnimationFrame) except for skipped keys and an action change.
    namespace Sampling = Render::Models::AnimationSampling;
    const auto timelineOf = [this](unsigned short action)
    {
        const Action_t& a = Actions[action];
        return Sampling::ActionTimeline{a.NumAnimationKeys, a.LockPositions, a.PlaySpeed, a.Loop};
    };
    Sampling::PosePlan pose = Sampling::PlanPose(timelineOf(PriorAction), PriorAction, PriorFrame,
                                                 timelineOf(currentAction), currentAction, AnimationFrame);
    // A model file may set the crossfade's length (Data/Models/ModelFiles.json).
    if (const auto fade = GltfCrossfadeKeyTime(*this, currentAction))
    {
        Sampling::SetCrossfadeLength(pose, AnimationFrame, *fade);
    }
    // A model from a glTF binary samples MU's blend of neighbouring keys at the
    // continuous key time it stands for, so detail between its keys shows.
    if (Render::Models::Gltf::SkinnedModelOf(*this))
    {
        if (const auto phase = Sampling::MuBlendPhase(timelineOf(currentAction), pose))
        {
            pose.from = *phase;
            pose.to = *phase;
            pose.weight = 0.f;
        }
    }
    const float s1 = pose.weight;
    const float s2 = 1.f - s1;

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

    // Pre-calculate Head bone quaternions ONCE outside loop. HeadAngle turns every
    // sampled key before it becomes a quaternion.
    vec4_t headQ1, headQ2;
    const bool hasHeadBone = (BoneHead >= 0 && BoneHead < NumBones && !Bones[BoneHead].Dummy);
    constexpr float radFactor = 1.0f / (180.f / Q_PI);
    const float HeadAngleX = HeadAngle[0] * radFactor;
    const float HeadAngleY = HeadAngle[1] * radFactor;
    // A model from a glTF binary samples its clips at the same points (see GltfSample).
    const auto* gltf = Render::Models::Gltf::SkinnedModelOf(*this);
    if (hasHeadBone && !gltf)
    {
        const Bone_t* hb = &Bones[BoneHead];
        const auto headKey = [&](const Sampling::KeySample& sample, int key, vec4_t out)
        {
            vec3_t angle;
            VectorCopy(hb->BoneMatrixes[sample.action].Rotation[key], angle);
            angle[0] -= HeadAngleX;
            angle[2] -= HeadAngleY;
            AngleQuaternion(angle, out);
        };
        const auto headSample = [&](const Sampling::KeySample& sample, vec4_t out)
        {
            headKey(sample, sample.key0, out);
            if (sample.fraction == 0.f || sample.key0 == sample.key1)
                return;
            vec4_t next, from;
            headKey(sample, sample.key1, next);
            QuaternionCopy(out, from);
            if (!QuaternionCompare(from, next))
                QuaternionNLERP(from, next, sample.fraction, out);
        };
        headSample(pose.from, headQ1);
        headSample(pose.to, headQ2);
    }

    // One action's rotation and position of a bone at a sample: the key itself,
    // or its two keys blended by the fraction.
    const auto sampleRotation = [](const Bone_t* bone, const Sampling::KeySample& sample,
                                   vec4_t scratch) -> const float*
    {
        const BoneMatrix_t* bm = &bone->BoneMatrixes[sample.action];
        const float* q0 = bm->Quaternion[sample.key0];
        if (sample.fraction == 0.f || sample.key0 == sample.key1)
            return q0;
        const float* q1 = bm->Quaternion[sample.key1];
        if (QuaternionCompare(q0, q1))
            return q0;
        QuaternionNLERP(q0, q1, sample.fraction, scratch);
        return scratch;
    };
    const auto samplePosition = [](const Bone_t* bone, const Sampling::KeySample& sample,
                                   vec3_t scratch) -> const float*
    {
        const BoneMatrix_t* bm = &bone->BoneMatrixes[sample.action];
        const float* p0 = bm->Position[sample.key0];
        if (sample.fraction == 0.f || sample.key0 == sample.key1)
            return p0;
        const float* p1 = bm->Position[sample.key1];
        const float t1 = sample.fraction;
        const float t0 = 1.f - t1;
        scratch[0] = p0[0] * t0 + p1[0] * t1;
        scratch[1] = p0[1] * t0 + p1[1] * t1;
        scratch[2] = p0[2] * t0 + p1[2] * t1;
        return scratch;
    };

    const bool bLockPositions = (Actions[PriorAction].LockPositions || Actions[currentAction].LockPositions);

    // bones loop
    for (int i = 0; i < NumBones; i++)
    {
        const Bone_t* b = &Bones[i];
        if (b->Dummy)
        {
            // Dummy bones carry no name/parent/animation data (see Open2's
            // loader -- the !Dummy branch is the only one that reads anything), so this slot was
            // previously left holding whatever the LAST model to animate into this shared buffer
            // wrote there. If that was a differently-positioned character (character-select roster),
            // any vertex or child bone reading this slot picks up a real matrix anchored thousands
            // of units away -- which PlanarShadowShader's skew division then amplifies into the
            // "infinity shadow". The tail fill below only covers slots >= NumBones; Dummy slots
            // live INSIDE [0, NumBones) and need the same identity treatment.
            BoneMatrix[i][0][0] = 1.f;
            BoneMatrix[i][0][1] = 0.f;
            BoneMatrix[i][0][2] = 0.f;
            BoneMatrix[i][0][3] = 0.f;
            BoneMatrix[i][1][0] = 0.f;
            BoneMatrix[i][1][1] = 1.f;
            BoneMatrix[i][1][2] = 0.f;
            BoneMatrix[i][1][3] = 0.f;
            BoneMatrix[i][2][0] = 0.f;
            BoneMatrix[i][2][1] = 0.f;
            BoneMatrix[i][2][2] = 1.f;
            BoneMatrix[i][2][3] = 0.f;
            continue;
        }
        vec4_t rotationScratch1, rotationScratch2;
        vec3_t positionScratch1, positionScratch2;
        const float* q1;
        const float* q2;
        const float* Position1;
        const float* Position2;
        // A locked root keeps x and y of the current action's key 0.
        vec3_t lockedRoot;

        if (gltf)
        {
            Render::Models::Gltf::JointPose from, to;
            GltfSample(*gltf, Actions, i, pose.from, from);
            GltfSample(*gltf, Actions, i, pose.to, to);
            if (i == BoneHead && hasHeadBone)
            {
                Render::Models::Gltf::SkinnedModel::TurnHead(from.rotation, HeadAngleX, HeadAngleY);
                Render::Models::Gltf::SkinnedModel::TurnHead(to.rotation, HeadAngleX, HeadAngleY);
            }
            QuaternionCopy(from.rotation.data(), rotationScratch1);
            QuaternionCopy(to.rotation.data(), rotationScratch2);
            VectorCopy(from.position.data(), positionScratch1);
            VectorCopy(to.position.data(), positionScratch2);
            q1 = rotationScratch1;
            q2 = rotationScratch2;
            Position1 = positionScratch1;
            Position2 = positionScratch2;
            if (i == 0 && bLockPositions)
            {
                Render::Models::Gltf::JointPose key0;
                GltfSample(*gltf, Actions, i, Sampling::KeyAt(currentAction, 0), key0);
                VectorCopy(key0.position.data(), lockedRoot);
            }
        }
        else
        {
            if (i == BoneHead)
            {
                q1 = headQ1;
                q2 = headQ2;
            }
            else
            {
                q1 = sampleRotation(b, pose.from, rotationScratch1);
                q2 = sampleRotation(b, pose.to, rotationScratch2);
            }
            Position1 = samplePosition(b, pose.from, positionScratch1);
            Position2 = samplePosition(b, pose.to, positionScratch2);
            if (i == 0 && bLockPositions)
            {
                VectorCopy(b->BoneMatrixes[currentAction].Position[0], lockedRoot);
            }
        }

        vec4_t boneQuaternion;
        if (!QuaternionCompare(q1, q2))
        {
            QuaternionNLERP(q1, q2, s1, boneQuaternion);
        }
        else
        {
            QuaternionCopy(q1, boneQuaternion);
        }

        float Matrix[3][4];
        QuaternionMatrix(boneQuaternion, Matrix);

        if (i == 0 && bLockPositions)
        {
            Matrix[0][3] = lockedRoot[0];
            Matrix[1][3] = lockedRoot[1];
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

    // This model's own skeleton may have fewer than MAX_BONES real bones, and the loop
    // above only ever writes BoneMatrix[0..NumBones). BoneMatrix is caller-supplied and shared/reused
    // across different models' Animation() calls (not cleared between them), so slots >= NumBones
    // would otherwise keep holding a PREVIOUS, differently-boned model's real (not garbage) transform
    // matrix. GPU skinning uploads the caller's full palette capacity, so any mesh vertex whose
    // bone index lands in that untouched tail would read a foreign character's matrix. Identity-fill
    // the unused tail
    // so any out-of-range bone index reads a safe no-op transform instead. Bounded by the caller's
    // declared buffer capacity -- NOT MAX_BONES -- because several internal callers pass
    // NumBones-sized heap scratch buffers (AnimationTransformWithAttachHighModel* /
    // AnimationTransformOnlySelf), which a MAX_BONES-bound loop would overflow.
    // The shared BoneTransform tracks how far the last models wrote, so the tail fill stops there
    // instead of rewriting slots that are already identity.
    const bool sharedPalette = BoneMatrix == BoneTransform && BoneMatrixCapacity == MAX_BONES;
    const int fillEnd = sharedPalette ? std::max<int>(NumBones, s_boneTransformWrittenEnd) : BoneMatrixCapacity;
    if (sharedPalette)
    {
        s_boneTransformWrittenEnd = NumBones;
    }
    for (int i = NumBones; i < fillEnd; i++)
    {
        BoneMatrix[i][0][0] = 1.f;
        BoneMatrix[i][0][1] = 0.f;
        BoneMatrix[i][0][2] = 0.f;
        BoneMatrix[i][0][3] = 0.f;
        BoneMatrix[i][1][0] = 0.f;
        BoneMatrix[i][1][1] = 1.f;
        BoneMatrix[i][1][2] = 0.f;
        BoneMatrix[i][1][3] = 0.f;
        BoneMatrix[i][2][0] = 0.f;
        BoneMatrix[i][2][1] = 0.f;
        BoneMatrix[i][2][2] = 1.f;
        BoneMatrix[i][2][3] = 0.f;
    }
}

extern EGameScene SceneFlag;
extern float g_fSpecialHeight;
extern int EditFlag;

bool HighLight = true;

// The light mood's highlight shoulder on a CPU-lit model colour; a knee of 1 (Classic) leaves it.
static void RollOffModelLight(vec3_t light, float knee)
{
    if (knee >= 1.f)
    {
        return;
    }
    for (int i = 0; i < 3; ++i)
    {
        light[i] = Render::Lighting::LightMood::RollOffHighlight(light[i], knee);
    }
}
float BoneScale = 1.f;

void BMD::ClaimSkinStamp() const
{
    if (m_SkinStamp == g_SkinStampCounter)
        return;
#ifdef _DEBUG
    // Reaching here means some OTHER BMD's TransformCheap() ran since this BMD's own last one --
    // this BMD's slice of the shared scratch arrays was evicted, and we're about to re-derive it
    // from our own stashed skin request (self-heal). This is a pre-existing sharing model (last
    // Transform() wins), not newly introduced by the lazy-skin change -- but a consumer reaching this branch means it
    // read/wrote the arrays OUTSIDE the Calc/Draw (or equivalent) bracket that owns this BMD's
    // data, which is worth knowing about if the soak turns up anything odd.
    g_ErrorReport.Write(
        L"[DXP-20-inc4] ClaimSkinStamp: stale skin stamp (this=%p, stamp=%u vs current=%u) -- self-healing\r\n",
        (void*)this, m_SkinStamp, g_SkinStampCounter);
#endif
    m_SkinStamp = ++g_SkinStampCounter;
    for (int i = 0; i < MAX_MESH; i++)
    {
        m_CpuVertsReady[i] = false;
        m_CpuNormalsReady[i] = false;
    }
}

void BMD::TransformCheap(float (*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB,
                         bool Translate, float _Scale)
{
    m_pCurrentBoneTransform = BoneMatrix;
    SetActiveBoneTransform(BoneMatrix);
    m_LastTranslate = Translate;        // persist for RenderMesh GPU skinning path
    m_LastSkinScale = _Scale;           // stashed for EnsureCpuVertices()
    m_LastBoneScale = BoneScale;        // snapshot of the global -- callers mutate it right
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

        // The light mood's sun scales the directional part of the model light.
        Render::Lighting::LightMood::ScaleSunDirection(Render::Lighting::LightMood::Active(), Position);
        AngleMatrix(ShadowAngle, Matrix);
        VectorIRotate(Position, Matrix, m_LastLightPosition); // for RenderMesh's GPU-skinned in-shader lighting
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

void BMD::SkinVertex(int mesh, int vertexIndex, float (*BoneMatrix)[3][4], bool Translate, float _Scale,
                     vec3_t out) const
{
    const Vertex_t* v = &Meshs[mesh].Vertices[vertexIndex];

    // A model from a glTF binary: up to four joints per vertex, same arithmetic per joint.
    if (const auto* gltf = Render::Models::Gltf::SkinnedModelOf(*this))
    {
        gltf->SkinPosition(mesh, vertexIndex, BoneMatrix, m_LastBoneScale, _Scale, out);
        if (Translate)
        {
            VectorScale(out, BodyScale, out);
            VectorAdd(out, BodyOrigin, out);
        }
        return;
    }

    // Reads the BoneScale snapshotted at TransformCheap() time, not the live global --
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

void BMD::SkinVertices(int mesh, float (*BoneMatrix)[3][4], bool Translate, float _Scale) const
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

    if (mesh >= MAX_MESH || mesh >= NumMeshs)
        return;
    if (m_CpuVertsReady[mesh])
        return;

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

    if (mesh >= MAX_MESH || mesh >= NumMeshs)
        return;
    if (m_CpuNormalsReady[mesh])
        return;

    const Mesh_t* m = &Meshs[mesh];
    const auto* gltf = Render::Models::Gltf::SkinnedModelOf(*this);
    for (int j = 0; j < m->NumNormals; j++)
    {
        const Normal_t* sn = &m->Normals[j];
        float* tn = NormalTransform[mesh][j];
        if (gltf)
            gltf->SkinNormal(mesh, j, m_pCurrentBoneTransform, tn);
        else
            VectorRotate(sn->Normal, m_pCurrentBoneTransform[sn->Node], tn);
        if (LightEnable)
        {
            float Luminosity = VectorDotProduct(tn, m_LastLightPosition) * 0.8f + 0.4f;
            if (Luminosity < 0.2f)
                Luminosity = 0.2f;
            IntensityTransform[mesh][j] = Luminosity;
        }
    }
    m_CpuNormalsReady[mesh] = true;
}

void BMD::MarkCpuVerticesExternallyWritten(int mesh) const
{
    if (mesh < 0 || mesh >= MAX_MESH)
        return;
    ClaimSkinStamp();
    m_CpuVertsReady[mesh] = true;
}

void BMD::Transform(float (*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate,
                    float _Scale)
{
    FRAME_PROFILE(Skinning); // baseline measurement
    TransformCheap(BoneMatrix, BoundingBoxMin, BoundingBoxMax, OBB, Translate, _Scale);

    // With the lazy-skin gate on, ordinary (EditFlag != 2) bodies defer the
    // vertex/normal loops below to EnsureCpuVertices()/EnsureCpuNormals() at each consumer site
    // instead of running them here unconditionally -- TransformCheap() already stashed everything
    // those need. EditFlag == 2 (map editor) always takes the eager path below: it needs the
    // vertex-loop-derived OBB override further down, which EnsureCpu*() never computes (see
    // TransformCheap()'s header comment -- same restriction that already applied to it in
    // increment 2). fTransformedSize is intentionally left stale on the lazy path in both Debug
    // and Release builds -- its only consumer already floors the result.
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
    const auto* gltf = Render::Models::Gltf::SkinnedModelOf(*this);
    for (int i = 0; i < NumMeshs; i++)
    {
        Mesh_t* m = &Meshs[i];
        for (int j = 0; j < m->NumVertices; j++)
        {
            Vertex_t* v = &m->Vertices[j];
            float* vp = VertexTransform[i][j];

            if (gltf)
            {
                gltf->SkinPosition(i, j, BoneMatrix, BoneScale, _Scale, vp);
                if (Translate)
                    VectorScale(vp, BodyScale, vp);
            }
            else if (BoneScale == 1.f)
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
            if (gltf)
                gltf->SkinNormal(i, j, BoneMatrix, tn);
            else
                VectorRotate(sn->Normal, BoneMatrix[sn->Node], tn);
            if (LightEnable)
            {
                float Luminosity;
                Luminosity = VectorDotProduct(tn, m_LastLightPosition) * 0.8f + 0.4f;

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
    fTransformedSize =
        std::max<float>(std::max<float>(BoundingMax[0] - BoundingMin[0], BoundingMax[1] - BoundingMin[1]),
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
    // A crossfade of a set length keeps the prior action until it has run (see SetCrossfadeLength).
    const auto fade = *PriorAction != CurrentAction ? GltfCrossfadeKeyTime(*this, CurrentAction) : std::nullopt;
    const bool crossfadeContinues =
        fade && Render::Models::AnimationSampling::CrossfadeContinues(*AnimationFrame, *fade);
    if (priorAnimationFrame != (int)*AnimationFrame && !crossfadeContinues)
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

    s_boneTransformWrittenEnd = std::max<int>(s_boneTransformWrittenEnd, NumBones);
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

    Animation(arrBonesTMLocal, oSelf->AnimationFrame, oSelf->PriorAnimationFrame, oSelf->PriorAction,
              (const_cast<OBJECT*>(oSelf))->Angle, Temp, false, true, nullptr, -1, NumBones);

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

        Animation(arrBonesTMLocal, fAnimationFrame, fPiriorAnimationFrame, iPiriorAction, v3RootAngle, Temp, false,
                  true, nullptr, -1, NumBones);
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

    n = VectorDotProduct(normal, g_chromeright[bone]);
    pchrome[0] = (n + 1.f); // FIX: make this a float

    n = VectorDotProduct(normal, g_chromeup[bone]);
    pchrome[1] = (n + 1.f); // FIX: make this a float
}

void BMD::Lighting(float* pLight, Light_t* lp, vec3_t Position, vec3_t Normal)
{
    vec3_t Light;
    VectorSubtract(lp->Position, Position, Light);
    float Length = sqrtf(Light[0] * Light[0] + Light[1] * Light[1] + Light[2] * Light[2]);

    float LightCos = (VectorDotProduct(Normal, Light) / Length) * 0.8f + 0.3f;
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
    EnsureCpuVertices(-1); // mouse-picking/lightmap-bake reader, not in the original spec's consumer list
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
    EnsureCpuVertices(i); // lightmap bake reader, not in the original spec's consumer list
    EnsureCpuNormals(i);
    int k, l;
    Triangle_t* tp = &m->Triangles[j];
    float* np = NormalTransform[i][tp->NormalIndex[0]];
    float* vp = VertexTransform[i][tp->VertexIndex[0]];
    float d = -VectorDotProduct(vp, np);

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

            mu::GetRenderer().BindTexture(i + IndexLightMap);
            mu::GetRenderer().SetTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            mu::GetRenderer().SetTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            mu::GetRenderer().SetTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            mu::GetRenderer().SetTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            mu::GetRenderer().SetTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    mu::GetRenderer().PushMatrix();
}

void BMD::EndRender()
{
    mu::GetRenderer().PopMatrix();
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

    // DXP-20 inc4: RenderZen's Transform() defers skinning, and VertexTransform is shared scratch.
    // Without this the heap copies whatever mesh 0 another model left there, often the hero's after
    // its shadow or a skill effect materialized it: an opaque gold plate over the character.
    EnsureCpuVertices(meshIndex);

    for (int j = 0; j < m->NumTriangles; j++)
    {
        const auto triangle = &m->Triangles[j];
        for (int k = 0; k < triangle->Polygon; k++)
        {
            const int source_vertex_index = triangle->VertexIndex[k];
            target_vertex_index++;

            VectorCopy(VertexTransform[meshIndex][source_vertex_index], vertices[target_vertex_index]);

            Vector4Set(BodyLight[0], BodyLight[1], BodyLight[2], alpha, colors[target_vertex_index]);

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

    const int numVerts = m->NumTriangles * 3 * coinCount;
    auto muVerts = GetRendererVertexScratch(static_cast<std::size_t>(numVerts));
    for (int i = 0; i < numVerts; ++i)
    {
        const vec4_t& c = colors[i];
        const std::uint32_t color = PackABGR(c[0], c[1], c[2], c[3]);
        muVerts[static_cast<std::size_t>(i)] =
            {vertices[i][0], vertices[i][1], vertices[i][2], 0.f, 0.f, 0.f, texCoords[i][0], texCoords[i][1], color};
    }
    mu::GetRenderer().RenderTriangles(muVerts, 0u);
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
        enableLight = false;
    }
    // The LightTransform-materializing loop that used to run right here
    // unconditionally (even for meshes that end up on the GPU-skinned draw path, which computes
    // lighting in-shader and never reads LightTransform) has moved into the
    // materializeCpuLightingAndChrome() lambda below, called only from the CPU-fallback sub-paths
    // that actually read it.

    int finalRenderFlags = renderFlags;
    bool useBlendMeshColor = false;
    // A chrome pass reflects light off the surface and is no bloom source; held until the mesh is drawn.
    std::optional<mu::ScopedBloomExclusion> chromePass;
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
        if (alpha < 0.99f)
        {
            EnableAlphaTest();
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

        chromePass.emplace();
        finalRenderFlags = RENDER_CHROME;
        if ((renderFlags & RENDER_CHROME4) == RENDER_CHROME4)
        {
            finalRenderFlags = RENDER_CHROME4;
        }
        if ((renderFlags & RENDER_OIL) == RENDER_OIL)
        {
            finalRenderFlags = RENDER_OIL;
        }

        // The g_chrome-writing loop that used to run right here unconditionally
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

        useBlendMeshColor = true;
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

    const bool shadowMap = (renderFlags & RENDER_SHADOWMAP) == RENDER_SHADOWMAP;
    const float colorScale = useBlendMeshColor ? blendMeshAlpha : 1.0f;
    const float baseAlpha = (useBlendMeshColor || meshIndex == StreamMesh) ? 1.0f : alpha;
    const bool useChrome = (renderFlags & RENDER_CHROME) || (renderFlags & RENDER_CHROME2) ||
                           (renderFlags & RENDER_CHROME3) || (renderFlags & RENDER_CHROME4) ||
                           (renderFlags & RENDER_CHROME5) || (renderFlags & RENDER_CHROME6) ||
                           (renderFlags & RENDER_CHROME7) || (renderFlags & RENDER_OIL) || (renderFlags & RENDER_METAL);
    const bool usesCpuLighting = enableLight && finalRenderFlags == RENDER_TEXTURE;

    auto materializeCpuDerivedData = [&]()
    {
        if (!usesCpuLighting && !useChrome && !(renderFlags & RENDER_WAVE))
        {
            return;
        }
        EnsureCpuNormals(meshIndex);

        if (usesCpuLighting)
        {
            // Lit per vertex here, so the point lights the base light leaves out come back as a whole.
            vec3_t light;
            VectorCopy(BodyLight, light);
            AddVertexLitDynamicLight(BodyOrigin[0], BodyOrigin[1], light);
            const float knee = Render::Lighting::LightMood::Active().highlightKnee;
            for (int j = 0; j < m->NumNormals; ++j)
            {
                VectorScale(light, IntensityTransform[meshIndex][j], LightTransform[meshIndex][j]);
                RollOffModelLight(LightTransform[meshIndex][j], knee);
            }
        }

        if (!useChrome)
        {
            return;
        }

        const float wave2 = static_cast<int>(WorldTime) % 5000 * 0.00024f - 0.4f;
        vec3_t light = {static_cast<float>(cos(WorldTime * 0.001f)), static_cast<float>(sin(WorldTime * 0.002f)), 1.0f};
        for (int j = 0; j < m->NumNormals && j < MAX_VERTICES; ++j)
        {
            const auto normal = NormalTransform[meshIndex][j];
            if ((renderFlags & RENDER_CHROME2) == RENDER_CHROME2)
            {
                g_chrome[j][0] = (normal[2] + normal[0]) * 0.8f + wave2 * 2.0f;
                g_chrome[j][1] = (normal[1] + normal[0]) + wave2 * 3.0f;
            }
            else if ((renderFlags & RENDER_CHROME3) == RENDER_CHROME3)
            {
                g_chrome[j][0] = VectorDotProduct(normal, LightVector);
                g_chrome[j][1] = 1.0f - g_chrome[j][0];
            }
            else if ((renderFlags & RENDER_CHROME4) == RENDER_CHROME4)
            {
                g_chrome[j][0] = VectorDotProduct(normal, light);
                g_chrome[j][1] = 1.0f - g_chrome[j][0];
                g_chrome[j][1] -= normal[2] * 0.5f + wave * 3.0f;
                g_chrome[j][0] += normal[1] * 0.5f + light[1] * 3.0f;
            }
            else if ((renderFlags & RENDER_CHROME5) == RENDER_CHROME5)
            {
                g_chrome[j][0] = VectorDotProduct(normal, light);
                g_chrome[j][1] = 1.0f - g_chrome[j][0];
                g_chrome[j][1] -= normal[2] * 2.5f + wave;
                g_chrome[j][0] += normal[1] * 3.0f + light[1] * 5.0f;
            }
            else if ((renderFlags & RENDER_CHROME6) == RENDER_CHROME6)
            {
                g_chrome[j][0] = (normal[2] + normal[0]) * 0.8f + wave2 * 2.0f;
                g_chrome[j][1] = g_chrome[j][0];
            }
            else if ((renderFlags & RENDER_CHROME7) == RENDER_CHROME7)
            {
                g_chrome[j][0] = (normal[2] + normal[0]) * 0.8f + static_cast<float>(WorldTime) * 0.00006f;
                g_chrome[j][1] = g_chrome[j][0];
            }
            else if ((renderFlags & RENDER_OIL) == RENDER_OIL)
            {
                g_chrome[j][0] = normal[0];
                g_chrome[j][1] = normal[1];
            }
            else if ((renderFlags & RENDER_CHROME) == RENDER_CHROME)
            {
                g_chrome[j][0] = normal[2] * 0.5f + wave;
                g_chrome[j][1] = normal[1] * 0.5f + wave * 2.0f;
            }
            else
            {
                g_chrome[j][0] = normal[2] * 0.5f + 0.2f;
                g_chrome[j][1] = normal[1] * 0.5f + 0.5f;
            }
        }
    };

    const std::size_t maxVertexCount = static_cast<std::size_t>(m->NumTriangles) * 3;
    const Render::Models::Gltf::SkinnedModel* gltfModel = Render::Models::Gltf::SkinnedModelOf(*this);
    const bool gpuSkinningEligible = CanGpuSkinMesh(finalRenderFlags, renderFlags, m_pCurrentBoneTransform);
    bool gpuSkinningSubmitted = false;
    if (gpuSkinningEligible)
    {
        // A coarser LOD of a model file draws its own mesh, or nothing beyond its meshes.
        bool absentFromLod = false;
        const Render::Models::Gltf::LodMesh* lodMesh =
            gltfModel ? gltfModel->LodMeshFor(ActiveLod, meshIndex, absentFromLod) : nullptr;
        if (absentFromLod)
        {
            return;
        }
        if (lodMesh && explicitTextureIndex == -1 && finalRenderFlags == RENDER_TEXTURE)
        {
            const unsigned int lodTexture = gltfModel->lodTextures[ActiveLod - 1][meshIndex];
            if (lodTexture != BITMAP_UNKNOWN)
            {
                BindTexture(static_cast<int>(lodTexture));
            }
        }
        const auto textureCoordinates = GetSkinningTextureCoordinates(renderFlags);
        const bool useMeshTextureCoordinates = textureCoordinates == mu::SkinningTextureCoordinates::Mesh;
        // A scrolling texture bakes this frame's offset into the UVs, so only the other meshes are cached.
        const bool waveTextureCoordinates = useMeshTextureCoordinates && EnableWave;
        const float waveU = waveTextureCoordinates ? blendMeshTextureCoordU : 0.0f;
        const float waveV = waveTextureCoordinates ? blendMeshTextureCoordV : 0.0f;
        const std::uint32_t color =
            PackABGR(BodyLight[0] * colorScale, BodyLight[1] * colorScale, BodyLight[2] * colorScale, baseAlpha);
        // A model file's mesh at this LOD is packed once (Render::Models::PackedWeightedMeshOf) and cached
        // under its vertices' address; a BMD mesh is cached under the mesh.
        const Render::Models::Gltf::PackedMesh* packedMesh =
            gltfModel
                ? &Render::Models::PackedWeightedMeshOf(*gltfModel, *m, meshIndex, lodMesh ? ActiveLod : 0, lodMesh)
                : nullptr;
        const void* cacheKey = packedMesh && !packedMesh->vertices.empty()
                                   ? static_cast<const void*>(packedMesh->vertices.data())
                                   : static_cast<const void*>(m);
        auto buildWeightedVertices = [&] { return WeightedCorners(*packedMesh, waveU, waveV); };
        auto buildSkinnedVertices = [&]() -> std::span<const mu::SkinnedVertex3D>
        {
            auto skinnedVertices = GetRendererSkinnedVertexScratch(maxVertexCount);
            std::size_t skinnedVertexCount = 0;
            for (int j = 0; j < m->NumTriangles; ++j)
            {
                const auto* triangle = &m->Triangles[j];
                for (int k = 0; k < triangle->Polygon; ++k)
                {
                    const auto& vertex = m->Vertices[triangle->VertexIndex[k]];
                    const auto& normal = m->Normals[triangle->NormalIndex[k]];
                    const auto& texCoord = m->TexCoords[triangle->TexCoordIndex[k]];
                    skinnedVertices[skinnedVertexCount++] =
                        Render::Models::PackBmdSkinnedVertex(vertex.Position, vertex.Node, normal.Normal, normal.Node,
                                                             texCoord.TexCoordU + waveU, texCoord.TexCoordV + waveV);
                }
            }
            return skinnedVertices.first(skinnedVertexCount);
        };

        const SkinPalette palette =
            gltfModel
                ? CurrentWeightedSkinPalette(*gltfModel, m_pCurrentBoneTransform, m_LastBoneScale, m_LastSkinScale)
                : SkinPalette{{&m_pCurrentBoneTransform[0][0][0], static_cast<std::size_t>(MAX_BONES) * 12u},
                              g_BoneTransformVersion};
        mu::SkinningParameters skinning{
            .boneMatrices = palette.matrices,
            .paletteVersion = palette.version,
            .bodyOrigin = {BodyOrigin[0], BodyOrigin[1], BodyOrigin[2]},
            .bodyScale = BodyScale,
            .boneScale = m_LastBoneScale,
            .restPoseScale = m_LastSkinScale,
            .lightDirection = {m_LastLightPosition[0], m_LastLightPosition[1], m_LastLightPosition[2]},
            .highlightKnee = Render::Lighting::LightMood::Active().highlightKnee,
            .textureCoordinateOffset = {blendMeshTextureCoordU, blendMeshTextureCoordV},
            .chromeWave = wave,
            .chromeWave2 = static_cast<int>(WorldTime) % 5000 * 0.00024f - 0.4f,
            .chromeLight = {static_cast<float>(cos(WorldTime * 0.001f)), static_cast<float>(sin(WorldTime * 0.002f))},
            .chromeTimeTerm = static_cast<float>(WorldTime) * 0.00006f,
            .textureCoordinates = textureCoordinates,
            .translate = m_LastTranslate,
            .lightEnabled = usesCpuLighting,
            .specularStrength = texture->IsSkin || texture->IsHair ? 0.0f : SpecularStrength,
        };
        // A model file's normal map (Enhanced lighting only), for draws on the mesh's own coordinates.
        if (gltfModel && useMeshTextureCoordinates)
        {
            const Render::Models::Gltf::NormalMapTexture normalMap =
                gltfModel->NormalMapTextureFor(ActiveLod, meshIndex);
            skinning.normalTextureId = normalMap.texture;
            skinning.normalMapScale = normalMap.scale;
        }
        auto& renderer = mu::GetRenderer();
        const bool cached = !waveTextureCoordinates &&
                            (renderer.IsSkinnedMeshCached(cacheKey) ||
                             (packedMesh ? renderer.CacheWeightedSkinnedMesh(cacheKey, buildWeightedVertices())
                                         : renderer.CacheSkinnedMesh(cacheKey, buildSkinnedVertices())));
        if (cached)
        {
            gpuSkinningSubmitted = renderer.RenderCachedSkinnedMesh(cacheKey, color, 0u, skinning);
        }
        else if (packedMesh)
        {
            gpuSkinningSubmitted =
                renderer.RenderWeightedSkinnedTriangles(buildWeightedVertices(), color, 0u, skinning);
        }
        else
        {
            gpuSkinningSubmitted = renderer.RenderSkinnedTriangles(buildSkinnedVertices(), color, 0u, skinning);
        }
    }

    const Render::Models::GpuSkinningPath skinningPath =
        Render::Models::ResolveGpuSkinningPath(gpuSkinningEligible, gpuSkinningSubmitted);
    if (skinningPath == Render::Models::GpuSkinningPath::GpuSubmitted)
    {
        FrameProfiler::Count(FrameProfiler::Counter::GpuSkinningSubmissions);
        return;
    }
    if (skinningPath == Render::Models::GpuSkinningPath::GpuFailed)
    {
        FrameProfiler::Count(FrameProfiler::Counter::GpuSkinningFailures);
        return;
    }

    FrameProfiler::Count(FrameProfiler::Counter::CpuSkinningIneligible);
    EnsureCpuVertices(meshIndex);
    materializeCpuDerivedData();
    auto rendererVertices = GetRendererVertexScratch(maxVertexCount);

    int target_vertex_index = -1;
    for (int j = 0; j < m->NumTriangles; j++)
    {
        const auto triangle = &m->Triangles[j];
        for (int k = 0; k < triangle->Polygon; k++)
        {
            const int source_vertex_index = triangle->VertexIndex[k];
            target_vertex_index++;

            vec3_t position;
            vec4_t colorComponents;
            vec2_t texCoord;
            VectorCopy(VertexTransform[meshIndex][source_vertex_index], position);
            Vector4Set(shadowMap ? 0.0f : BodyLight[0] * colorScale,
                    shadowMap ? 0.0f : BodyLight[1] * colorScale,
                    shadowMap ? 0.0f : BodyLight[2] * colorScale,
                    baseAlpha,
                    colorComponents);

            auto texco = m->TexCoords[triangle->TexCoordIndex[k]];
            texCoord[0] = texco.TexCoordU;
            texCoord[1] = texco.TexCoordV;

            int normalIndex = triangle->NormalIndex[k];

            switch (finalRenderFlags)
            {
                case RENDER_TEXTURE:
                {
                    if (EnableWave)
                    {
                        texCoord[0] += blendMeshTextureCoordU;
                        texCoord[1] += blendMeshTextureCoordV;
                    }

                    if (enableLight)
                    {
                        auto light = LightTransform[meshIndex][normalIndex];
                        Vector4Set(light[0], light[1], light[2], alpha, colorComponents);
                    }

                    break;
                }
                case RENDER_CHROME:
                {
                    texCoord[0] = g_chrome[normalIndex][0];
                    texCoord[1] = g_chrome[normalIndex][1];
                    break;
                }
                case RENDER_CHROME4:
                {
                    texCoord[0] = g_chrome[normalIndex][0] + blendMeshTextureCoordU;
                    texCoord[1] = g_chrome[normalIndex][1] + blendMeshTextureCoordV;
                    break;
                }
                case RENDER_OIL:
                {
                    texCoord[0] = g_chrome[normalIndex][0] * texCoord[0] + blendMeshTextureCoordU;
                    texCoord[1] = g_chrome[normalIndex][1] * texCoord[1] + blendMeshTextureCoordV;
                    break;
                }
            }

            if (shadowMap)
            {
                vec3_t pos;
                VectorSubtract(position, BodyOrigin, pos);

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
                    position[iCoord] += normal[iCoord] * time_sin;
                }
            }

            const std::uint32_t color = PackABGR(colorComponents[0], colorComponents[1], colorComponents[2],
                                                  colorComponents[3]);
            rendererVertices[static_cast<std::size_t>(target_vertex_index)] =
                {position[0], position[1], position[2], 0.f, 0.f, 0.f, texCoord[0], texCoord[1], color};
        }
    }

    const std::size_t renderedVertexCount = static_cast<std::size_t>(target_vertex_index + 1);
    mu::GetRenderer().RenderTriangles(rendererVertices.first(renderedVertexCount), 0u);
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
    {
        EnableWave = true;
    }

    bool EnableLight = LightEnable;
    if (i == StreamMesh)
    {
        EnableLight = false;
    }
    else if (EnableLight)
    {
        const float knee = Render::Lighting::LightMood::Active().highlightKnee;
        for (int j = 0; j < m->NumNormals; j++)
        {
            VectorScale(BodyLight, IntensityTransform[i][j], LightTransform[i][j]);
            RollOffModelLight(LightTransform[i][j], knee);
        }
    }

    int Render = RenderFlag;
    bool useBlendMeshColor = false;
    // A chrome pass reflects light off the surface and is no bloom source; held until the mesh is drawn.
    std::optional<mu::ScopedBloomExclusion> chromePass;
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
        if (Alpha < 0.99f)
        {
            EnableAlphaTest();
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
        chromePass.emplace();
        Render = RENDER_CHROME;
        if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4)
        {
            Render = RENDER_CHROME4;
        }
        float Wave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;

        vec3_t L = { (float)(cos(WorldTime * 0.001f)), (float)(sin(WorldTime * 0.002f)), 1.f };
        for (int j = 0; j < m->NumNormals; j++)
        {
            if (j >= MAX_VERTICES)
                break;
            float* Normal = NormalTransform[i][j];

            if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2)
            {
                g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
            }
            else if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3)
            {
                g_chrome[j][0] = VectorDotProduct(Normal, LightVector);
                g_chrome[j][1] = 1.f - VectorDotProduct(Normal, LightVector);
            }
            else if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4)
            {
                g_chrome[j][0] = VectorDotProduct(Normal, L);
                g_chrome[j][1] = 1.f - VectorDotProduct(Normal, L);
                g_chrome[j][1] -= Normal[2] * 0.5f + Wave * 3.f;
                g_chrome[j][0] += Normal[1] * 0.5f + L[1] * 3.f;
            }
            else if ((RenderFlag & RENDER_CHROME5) == RENDER_CHROME5)
            {
                Vector(0.1f, -0.23f, 0.22f, LightVector2);

                g_chrome[j][0] = (VectorDotProduct(Normal, LightVector2) /*+ Normal[1] + LightVector2[1]*3.f */) / 1.08f;
                g_chrome[j][1] = (1.f - VectorDotProduct(Normal, LightVector2) /*- Normal[2]*0.5f + 3.f */) / 1.08f;
            }
            else if ((RenderFlag & RENDER_CHROME6) == RENDER_CHROME6)
            {
                g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
                g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
            }
            else if ((RenderFlag & RENDER_CHROME7) == RENDER_CHROME7)
            {
                Vector(0.1f, -0.23f, 0.22f, LightVector2);

                g_chrome[j][0] = (VectorDotProduct(Normal, LightVector2)) / 1.08f;
                g_chrome[j][1] = (1.f - VectorDotProduct(Normal, LightVector2)) / 1.08f;
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

        useBlendMeshColor = true;
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

    auto muVerts = GetRendererVertexScratch(static_cast<std::size_t>(m->NumTriangles) * 3);
    std::size_t vertexIndex = 0;
    for (int j = 0; j < m->NumTriangles; j++)
    {
        Triangle_t* tp = &m->Triangles[j];
        for (int k = 0; k < tp->Polygon; k++)
        {
            int vi = tp->VertexIndex[k];
            int ni = tp->NormalIndex[k];

            float u = 0.f;
            float v = 0.f;
            const float colorScale = useBlendMeshColor ? BlendMeshLight : 1.0f;
            const float baseAlpha = (useBlendMeshColor || i == StreamMesh) ? 1.0f : Alpha;
            std::uint32_t color = PackABGR(BodyLight[0] * colorScale, BodyLight[1] * colorScale,
                                           BodyLight[2] * colorScale, baseAlpha);

            switch (Render)
            {
            case RENDER_TEXTURE:
            {
                TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
                u = EnableWave ? texp->TexCoordU + BlendMeshTexCoordU : texp->TexCoordU;
                v = EnableWave ? texp->TexCoordV + BlendMeshTexCoordV : texp->TexCoordV;
                if (EnableLight)
                {
                    float* Light = LightTransform[i][ni];
                    color = (Alpha >= 0.99f) ? PackABGR(Light[0], Light[1], Light[2], 1.f)
                                             : PackABGR(Light[0], Light[1], Light[2], Alpha);
                }
                break;
            }
            case RENDER_CHROME:
            {
                u = g_chrome[ni][0];
                v = g_chrome[ni][1];
                color = (Alpha >= 0.99f) ? PackABGR(BodyLight[0], BodyLight[1], BodyLight[2], 1.f)
                                         : PackABGR(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
                break;
            }
            }
            float px;
            float py;
            float pz;
            if ((iRndExtFlag & RNDEXT_WAVE))
            {
                float fParam = static_cast<float>(static_cast<int>(WorldTime) + vi * 931) * 0.007f;
                float fSin = sinf(fParam);
                float* Normal = NormalTransform[i][ni];
                px = VertexTransform[i][vi][0] + Normal[0] * fSin * 28.0f;
                py = VertexTransform[i][vi][1] + Normal[1] * fSin * 28.0f;
                pz = VertexTransform[i][vi][2] + Normal[2] * fSin * 28.0f;
            }
            else
            {
                px = VertexTransform[i][vi][0];
                py = VertexTransform[i][vi][1];
                pz = VertexTransform[i][vi][2];
            }

            float* n = NormalTransform[i][ni];
            muVerts[vertexIndex++] = {px, py, pz, n[0], n[1], n[2], u, v, color};
        }
    }
    mu::GetRenderer().RenderTriangles(muVerts.first(vertexIndex), 0u);
}

void BMD::RenderMeshEffect(int i, int iType, int iSubType, vec3_t Angle, VOID* obj)
{
    if (i >= NumMeshs || i < 0) return;

    Mesh_t* m = &Meshs[i];
    if (m->NumTriangles <= 0) return;

    EnsureCpuVertices(i); // spawn-position reads below (~20 sites) need mesh i materialized

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
    (void)LightEnable;
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

                    RenderMesh(i, RENDER_COLOR | RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV);
                }
                else if (shadowType == SHADOW_RENDER_TEXTURE)
                {
                    DisableAlphaBlend();

                    RenderMesh(i, RENDER_TEXTURE | RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV);
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
    (void)LightEnable;
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
        EnableLight = false;
    }
    else if (EnableLight)
    {
        const float knee = Render::Lighting::LightMood::Active().highlightKnee;
        for (int j = 0; j < m->NumNormals; j++)
        {
            VectorScale(BodyLight, IntensityTransform[i][j], LightTransform[i][j]);
            RollOffModelLight(LightTransform[i][j], knee);
        }
    }

    int Render = RenderFlag;
    bool useBlendMeshColor = false;
    // A chrome pass reflects light off the surface and is no bloom source; held until the mesh is drawn.
    std::optional<mu::ScopedBloomExclusion> chromePass;
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
        chromePass.emplace();
        Render = RENDER_CHROME;

        float Wave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;

        for (int j = 0; j < m->NumNormals; j++)
        {
            //			Normal_t *np = &m->Normals[j];
            if (j >= MAX_VERTICES)
                break;
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
        useBlendMeshColor = true;
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

    auto muVerts = GetRendererVertexScratch(static_cast<std::size_t>(m->NumTriangles) * 3);
    std::size_t vertexIndex = 0;
    for (int j = 0; j < m->NumTriangles; j++)
    {
        vec3_t  pos;
        Triangle_t* tp = &m->Triangles[j];
        for (int k = 0; k < tp->Polygon; k++)
        {
            int vi = tp->VertexIndex[k];
            int ni = tp->NormalIndex[k];

            float u = 0.f;
            float v = 0.f;
            const float colorScale = useBlendMeshColor ? BlendMeshLight : 1.0f;
            const float baseAlpha = (useBlendMeshColor || i == StreamMesh) ? 1.0f : Alpha;
            std::uint32_t color = PackABGR(BodyLight[0] * colorScale, BodyLight[1] * colorScale,
                                           BodyLight[2] * colorScale, baseAlpha);

            switch (Render)
            {
            case RENDER_TEXTURE:
            {
                TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
                u = EnableWave ? texp->TexCoordU + BlendMeshTexCoordU : texp->TexCoordU;
                v = EnableWave ? texp->TexCoordV + BlendMeshTexCoordV : texp->TexCoordV;
                if (EnableLight)
                {
                    float* Light = LightTransform[i][ni];
                    color = (Alpha >= 0.99f) ? PackABGR(Light[0], Light[1], Light[2], 1.f)
                                             : PackABGR(Light[0], Light[1], Light[2], Alpha);
                }
                break;
            }
            case RENDER_CHROME:
            {
                u = g_chrome[ni][0];
                v = g_chrome[ni][1];
                color = (Alpha >= 0.99f) ? PackABGR(BodyLight[0], BodyLight[1], BodyLight[2], 1.f)
                                         : PackABGR(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
                break;
            }
            }
            VectorAdd(VertexTransform[i][vi], BodyOrigin, pos);

            float* n = NormalTransform[i][ni];
            muVerts[vertexIndex++] = {pos[0], pos[1], pos[2], n[0], n[1], n[2], u, v, color};
        }
    }
    mu::GetRenderer().RenderTriangles(muVerts.first(vertexIndex), 0u);
}

void BMD::RenderBodyTranslate(int Flag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int HiddenMesh, int Texture)
{
    if (NumMeshs == 0) return;

    BeginRender(Alpha);
    (void)LightEnable;
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

void BMD::AddClothesShadowTriangles(void* pClothes, const int clothesCount, const float sx, const float sy,
    const std::uint32_t color) const
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

    const int numVerts = target_vertex_index + 1;
    auto muVerts = GetRendererVertexScratch(static_cast<std::size_t>(numVerts));
    for (int i = 0; i < numVerts; ++i)
    {
        muVerts[static_cast<std::size_t>(i)] =
            {vertices[i][0], vertices[i][1], vertices[i][2], 0.f, 0.f, 0.f, 0.f, 0.f, color};
    }
    mu::GetRenderer().RenderTriangles(muVerts, 0u);
}

namespace
{
// What CalcShadowPosition needs besides the position.
struct ShadowGround
{
    const float* origin;
    float sx;
    float sy;
};

void ProjectOntoGround(float position[3], const void* context)
{
    const auto* ground = static_cast<const ShadowGround*>(context);
    CalcShadowPosition(reinterpret_cast<vec3_t*>(position), ground->origin, ground->sx, ground->sy);
}

// CalcShadowPosition's projection for skinned_shadow.vert.
mu::ShadowProjection BodyShadowProjection(const vec3_t origin, const float sx, const float sy)
{
    return {
        .origin = {origin[0], origin[1], origin[2]},
        .shearX = sx,
        .shearHeight = sy,
        .specialHeight = g_fSpecialHeight,
        .terrain = SceneFlag != SERVER_LIST_SCENE && SceneFlag != WEBZEN_SCENE && SceneFlag != LOADING_SCENE,
    };
}
} // namespace

void BMD::ProjectWeightedShadowMesh(const int mesh, const ShadowProjection project, const void* context,
                                    std::vector<std::array<float, 3>>& vertices,
                                    std::vector<std::uint32_t>& indices) const
{
    vertices.clear();
    indices.clear();
    const auto* gltf = Render::Models::Gltf::SkinnedModelOf(*this);
    if (!gltf || mesh < 0 || mesh >= NumMeshs)
    {
        return;
    }
    bool absent = false;
    const Render::Models::Gltf::LodMesh* lodMesh = gltf->LodMeshFor(ActiveLod, mesh, absent);
    if (absent)
    {
        return;
    }

    if (lodMesh)
    {
        vertices.resize(lodMesh->vertices.size());
        for (std::size_t v = 0; v < lodMesh->vertices.size(); ++v)
        {
            float* position = vertices[v].data();
            Render::Models::Gltf::SkinnedModel::SkinPosition(lodMesh->vertices[v], m_pCurrentBoneTransform,
                                                             m_LastBoneScale, m_LastSkinScale, position);
            // As BMD::SkinVertex finishes a skinned position.
            if (m_LastTranslate)
            {
                VectorScale(position, BodyScale, position);
                VectorAdd(position, BodyOrigin, position);
            }
            project(position, context);
        }
        indices.reserve(lodMesh->triangles.size() * 3);
        for (const auto& triangle : lodMesh->triangles)
        {
            for (const int vertex : triangle)
            {
                indices.push_back(static_cast<std::uint32_t>(vertex));
            }
        }
        return;
    }

    EnsureCpuVertices(mesh);
    const Mesh_t& m = Meshs[mesh];
    vertices.resize(static_cast<std::size_t>(std::max<short>(m.NumVertices, 0)));
    for (int v = 0; v < m.NumVertices; ++v)
    {
        float* position = vertices[static_cast<std::size_t>(v)].data();
        VectorCopy(VertexTransform[mesh][v], position);
        project(position, context);
    }
    indices.reserve(static_cast<std::size_t>(m.NumTriangles) * 3);
    for (int t = 0; t < m.NumTriangles; ++t)
    {
        const Triangle_t& triangle = m.Triangles[t];
        for (int k = 0; k < triangle.Polygon; ++k)
        {
            indices.push_back(static_cast<std::uint32_t>(triangle.VertexIndex[k]));
        }
    }
}

void BMD::AddWeightedMeshShadowTriangles(const int blendMesh, const int hiddenMesh, const int startMesh,
                                         const int endMesh, const float sx, const float sy,
                                         const std::uint32_t color) const
{
    constexpr std::size_t indexReach = std::size_t{std::numeric_limits<std::uint16_t>::max()} + 1;
    static thread_local std::vector<std::array<float, 3>> meshVertices;
    static thread_local std::vector<std::uint32_t> meshIndices;
    static thread_local std::vector<mu::Vertex3D> batchVertices;
    static thread_local std::vector<std::uint16_t> batchIndices;
    batchVertices.clear();
    batchIndices.clear();
    const auto shadowVertex = [color](const std::array<float, 3>& position)
    { return mu::Vertex3D{position[0], position[1], position[2], 0.f, 0.f, 0.f, 0.f, 0.f, color}; };
    const auto flush = []
    {
        if (!batchIndices.empty())
        {
            mu::GetRenderer().RenderIndexedTriangles(batchVertices, batchIndices, 0u);
        }
        batchVertices.clear();
        batchIndices.clear();
    };

    const ShadowGround ground{BodyOrigin, sx, sy};
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

        ProjectWeightedShadowMesh(i, ProjectOntoGround, &ground, meshVertices, meshIndices);
        if (meshIndices.empty())
        {
            continue;
        }
        if (meshVertices.size() > indexReach)
        {
            // Beyond 16-bit indices: this mesh's corners, drawn in their place in the order.
            flush();
            auto corners = GetRendererVertexScratch(meshIndices.size());
            for (std::size_t c = 0; c < meshIndices.size(); ++c)
            {
                corners[c] = shadowVertex(meshVertices[meshIndices[c]]);
            }
            mu::GetRenderer().RenderTriangles(corners, 0u);
            continue;
        }
        if (batchVertices.size() + meshVertices.size() > indexReach)
        {
            flush();
        }
        const auto base = static_cast<std::uint32_t>(batchVertices.size());
        for (const auto& position : meshVertices)
        {
            batchVertices.push_back(shadowVertex(position));
        }
        for (const std::uint32_t index : meshIndices)
        {
            batchIndices.push_back(static_cast<std::uint16_t>(base + index));
        }
    }
    flush();
}

void BMD::AddMeshShadowTriangles(const int blendMesh, const int hiddenMesh, const int startMesh,
    const int endMesh, const float sx, const float sy, const std::uint32_t color) const
{
    if (Render::Models::Gltf::SkinnedModelOf(*this))
    {
        AddWeightedMeshShadowTriangles(blendMesh, hiddenMesh, startMesh, endMesh, sx, sy, color);
        return;
    }

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

        EnsureCpuVertices(i);

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

    const int numVerts = target_vertex_index + 1;
    auto muVerts = GetRendererVertexScratch(static_cast<std::size_t>(numVerts));
    for (int i = 0; i < numVerts; ++i)
    {
        muVerts[static_cast<std::size_t>(i)] =
            {vertices[i][0], vertices[i][1], vertices[i][2], 0.f, 0.f, 0.f, 0.f, 0.f, color};
    }
    mu::GetRenderer().RenderTriangles(muVerts, 0u);
}

bool BMD::RenderGpuMeshShadow(const int blendMesh, const int hiddenMesh, const int startMesh, const int endMesh,
    const float sx, const float sy, const std::uint32_t color) const
{
    if (!m_pCurrentBoneTransform || startMesh < 0 || endMesh > NumMeshs || endMesh > MAX_MESH)
    {
        return false;
    }

    const auto drawn = [&](int i)
    {
        return i != hiddenMesh && Meshs[i].NumTriangles > 0 && Meshs[i].Texture != blendMesh;
    };
    const bool cpuVerticesCurrent = m_SkinStamp == g_SkinStampCounter;
    for (int i = startMesh; i < endMesh; i++)
    {
        // Already skinned this frame, maybe at another BodyOrigin, or written by the cloth simulation: the CPU
        // shadow draws those positions.
        if (drawn(i) && cpuVerticesCurrent && m_CpuVertsReady[i])
        {
            return false;
        }
    }

    auto& renderer = mu::GetRenderer();
    if (!renderer.SetTerrainHeights({BackTerrainHeight, Render::kTerrainCells}, {TerrainWall, Render::kTerrainCells},
                                    TerrainHeightVersion()))
    {
        return false;
    }

    const auto stream = ShadowStreams()
                            .try_emplace({reinterpret_cast<std::uintptr_t>(this), startMesh, endMesh, hiddenMesh,
                                          blendMesh})
                            .first;
    const void* key = &*stream;
    if (!renderer.IsSkinnedMeshCached(key))
    {
        std::size_t vertexCount = 0;
        for (int i = startMesh; i < endMesh; i++)
        {
            for (int j = 0; drawn(i) && j < Meshs[i].NumTriangles; j++)
            {
                vertexCount += static_cast<std::size_t>(Meshs[i].Triangles[j].Polygon);
            }
        }

        auto vertices = GetRendererSkinnedVertexScratch(vertexCount);
        std::size_t count = 0;
        for (int i = startMesh; i < endMesh; i++)
        {
            const Mesh_t* mesh = &Meshs[i];
            for (int j = 0; drawn(i) && j < mesh->NumTriangles; j++)
            {
                const auto* tp = &mesh->Triangles[j];
                for (int k = 0; k < tp->Polygon; k++)
                {
                    const auto& vertex = mesh->Vertices[tp->VertexIndex[k]];
                    vertices[count++] = {vertex.Position[0], vertex.Position[1], vertex.Position[2], 0.f, 0.f, 0.f,
                                         0.f, 0.f, vertex.Node, -1};
                }
            }
        }
        if (!renderer.CacheSkinnedMesh(key, vertices.first(count)))
        {
            return false;
        }
    }

    // What EnsureCpuVertices() would skin with now.
    const mu::SkinningParameters skinning{
        .boneMatrices = {&m_pCurrentBoneTransform[0][0][0], MAX_BONES * 12u},
        .paletteVersion = g_BoneTransformVersion,
        .bodyOrigin = {BodyOrigin[0], BodyOrigin[1], BodyOrigin[2]},
        .bodyScale = BodyScale,
        .boneScale = m_LastBoneScale,
        .restPoseScale = m_LastSkinScale,
        .translate = m_LastTranslate,
    };
    return renderer.RenderCachedSkinnedShadow(key, color, skinning, BodyShadowProjection(BodyOrigin, sx, sy));
}

void BMD::RenderWeightedMeshShadow(const int blendMesh, const int hiddenMesh, const int startMesh, const int endMesh,
                                   const float sx, const float sy, const std::uint32_t color) const
{
    const auto* gltf = Render::Models::Gltf::SkinnedModelOf(*this);
    auto& renderer = mu::GetRenderer();
    if (!gltf || !m_pCurrentBoneTransform || startMesh < 0 || endMesh > NumMeshs || endMesh > MAX_MESH ||
        !renderer.SetTerrainHeights({BackTerrainHeight, Render::kTerrainCells}, {TerrainWall, Render::kTerrainCells},
                                    TerrainHeightVersion()))
    {
        AddWeightedMeshShadowTriangles(blendMesh, hiddenMesh, startMesh, endMesh, sx, sy, color);
        return;
    }

    // What the body draw skins with (BMD::RenderMesh).
    const SkinPalette palette =
        CurrentWeightedSkinPalette(*gltf, m_pCurrentBoneTransform, m_LastBoneScale, m_LastSkinScale);
    const mu::SkinningParameters skinning{
        .boneMatrices = palette.matrices,
        .paletteVersion = palette.version,
        .bodyOrigin = {BodyOrigin[0], BodyOrigin[1], BodyOrigin[2]},
        .bodyScale = BodyScale,
        .boneScale = m_LastBoneScale,
        .restPoseScale = m_LastSkinScale,
        .translate = m_LastTranslate,
    };
    const mu::ShadowProjection projection = BodyShadowProjection(BodyOrigin, sx, sy);
    const bool cpuVerticesCurrent = m_SkinStamp == g_SkinStampCounter;
    for (int i = startMesh; i < endMesh; i++)
    {
        if (i == hiddenMesh || Meshs[i].NumTriangles <= 0 || Meshs[i].Texture == blendMesh)
        {
            continue;
        }
        bool absent = false;
        const Render::Models::Gltf::LodMesh* lodMesh = gltf->LodMeshFor(ActiveLod, i, absent);
        if (absent)
        {
            continue;
        }
        // As RenderGpuMeshShadow: CPU vertices already skinned this frame draw their own shadow.
        if (!lodMesh && cpuVerticesCurrent && m_CpuVertsReady[i])
        {
            AddWeightedMeshShadowTriangles(blendMesh, hiddenMesh, i, i + 1, sx, sy, color);
            continue;
        }

        const auto& packed = Render::Models::PackedWeightedMeshOf(*gltf, Meshs[i], i, lodMesh ? ActiveLod : 0, lodMesh);
        const void* key = packed.vertices.data();
        const bool cached =
            !packed.vertices.empty() && (renderer.IsSkinnedMeshCached(key) ||
                                         renderer.CacheWeightedSkinnedMesh(key, WeightedCorners(packed, 0.f, 0.f)));
        if (!cached || !renderer.RenderCachedSkinnedShadow(key, color, skinning, projection))
        {
            AddWeightedMeshShadowTriangles(blendMesh, hiddenMesh, i, i + 1, sx, sy, color);
        }
    }
}

void BMD::RenderBodyShadow(const int blendMesh, const int hiddenMesh, const int startMeshNumber,
    const int endMeshNumber, void* pClothes, const int clothesCount, const float alpha)
{
    if (!g_pOption->GetRenderAllEffects() || IsBodyShadowsDisabledDebug())
    {
        return;
    }

    if (NumMeshs == 0 && clothesCount == 0)
    {
        return;
    }

    EnableAlphaTest(false);

    DisableTexture();
    DisableDepthMask();
    BeginRender(1.f);

    const auto [startMesh, endMesh] = ResolveMeshRange(NumMeshs, startMeshNumber, endMeshNumber);

    const float sx = gMapManager.InBattleCastle() ? 2500.f : 2000.f;
    const float sy = 4000.f;
    const std::uint32_t shadowColor = PackABGR(0.f, 0.f, 0.f, std::clamp(alpha, 0.f, 1.f));

    if (clothesCount == 0 && Render::Models::Gltf::SkinnedModelOf(*this))
    {
        RenderWeightedMeshShadow(blendMesh, hiddenMesh, startMesh, endMesh, sx, sy, shadowColor);
    }
    else if (clothesCount == 0)
    {
        if (!RenderGpuMeshShadow(blendMesh, hiddenMesh, startMesh, endMesh, sx, sy, shadowColor))
        {
            AddMeshShadowTriangles(blendMesh, hiddenMesh, startMesh, endMesh, sx, sy, shadowColor);
        }
    }
    else
    {
        AddClothesShadowTriangles(pClothes, clothesCount, sx, sy, shadowColor);
    }

    EndRender();
    EnableDepthMask();
}

void BMD::RenderObjectBoundingBox()
{
    DisableTexture();
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().Translate(BodyOrigin[0], BodyOrigin[1], BodyOrigin[2]);
    mu::GetRenderer().Scale(BodyScale, BodyScale, BodyScale);
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

            auto MakeVtx = [&](const vec3_t& pos, float u, float v, std::uint32_t c) -> mu::Vertex3D
            { return {pos[0], pos[1], pos[2], 0.f, 0.f, 1.f, u, v, c}; };

            constexpr std::uint32_t cDark = 0xFF333333u;
            constexpr std::uint32_t cMid = 0xFF999999u;
            constexpr std::uint32_t cLight = 0xFF666666u;

            auto verts = GetRendererVertexScratch(36);
            std::size_t vertexIndex = 0;

            auto EmitQuad = [&](const vec3_t& q0, float u0, float v0, const vec3_t& q1, float u1, float v1,
                                const vec3_t& q2, float u2, float v2, const vec3_t& q3, float u3, float v3,
                                std::uint32_t col)
            {
                verts[vertexIndex++] = MakeVtx(q0, u0, v0, col);
                verts[vertexIndex++] = MakeVtx(q1, u1, v1, col);
                verts[vertexIndex++] = MakeVtx(q2, u2, v2, col);
                verts[vertexIndex++] = MakeVtx(q0, u0, v0, col);
                verts[vertexIndex++] = MakeVtx(q2, u2, v2, col);
                verts[vertexIndex++] = MakeVtx(q3, u3, v3, col);
            };

            EmitQuad(BoundingVertices[7], 1.f, 1.f, BoundingVertices[6], 1.f, 0.f, BoundingVertices[4], 0.f, 0.f,
                     BoundingVertices[5], 0.f, 1.f, cDark);
            EmitQuad(BoundingVertices[0], 0.f, 1.f, BoundingVertices[2], 1.f, 1.f, BoundingVertices[3], 1.f, 0.f,
                     BoundingVertices[1], 0.f, 0.f, cDark);
            EmitQuad(BoundingVertices[7], 1.f, 1.f, BoundingVertices[3], 1.f, 0.f, BoundingVertices[2], 0.f, 0.f,
                     BoundingVertices[6], 0.f, 1.f, cMid);
            EmitQuad(BoundingVertices[0], 0.f, 1.f, BoundingVertices[1], 1.f, 1.f, BoundingVertices[5], 1.f, 0.f,
                     BoundingVertices[4], 0.f, 0.f, cMid);
            EmitQuad(BoundingVertices[7], 1.f, 1.f, BoundingVertices[5], 1.f, 0.f, BoundingVertices[1], 0.f, 0.f,
                     BoundingVertices[3], 0.f, 1.f, cLight);
            EmitQuad(BoundingVertices[0], 0.f, 1.f, BoundingVertices[4], 1.f, 1.f, BoundingVertices[6], 1.f, 0.f,
                     BoundingVertices[2], 0.f, 0.f, cLight);

            mu::GetRenderer().RenderTriangles(verts, 0u);
        }
    }
    mu::GetRenderer().PopMatrix();
    DisableAlphaBlend();
}

void BMD::RenderBone(float(*BoneMatrix)[3][4])
{
    DisableTexture();
    mu::GetRenderer().SetDepthFunc(GL_ALWAYS);

    constexpr std::uint32_t boneColor = 0xFF33CCCCu;
    auto allLines = GetRendererVertexScratch(static_cast<std::size_t>(NumBones) * 6);
    std::size_t vertexIndex = 0;

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
                for (auto& BoneVertice : BoneVertices)
                {
                    VectorMA(BodyOrigin, BodyScale, BoneVertice, BoneVertice);
                }
                auto MakeVtx = [&](const vec3_t& pos) -> mu::Vertex3D
                { return {pos[0], pos[1], pos[2], 0.f, 0.f, 1.f, 0.f, 0.f, boneColor}; };
                allLines[vertexIndex++] = MakeVtx(BoneVertices[0]);
                allLines[vertexIndex++] = MakeVtx(BoneVertices[1]);
                allLines[vertexIndex++] = MakeVtx(BoneVertices[1]);
                allLines[vertexIndex++] = MakeVtx(BoneVertices[2]);
                allLines[vertexIndex++] = MakeVtx(BoneVertices[2]);
                allLines[vertexIndex++] = MakeVtx(BoneVertices[0]);
            }
        }
    }
    if (vertexIndex != 0)
    {
        mu::GetRenderer().RenderLines(allLines.first(vertexIndex), 0u);
    }

    mu::GetRenderer().SetDepthFunc(GL_LEQUAL);
}

void BMD::ShareFrom(BMD& owner)
{
    if (&owner == this || owner.Meshs == nullptr)
    {
        return;
    }
    Release();
    if (!owner.m_sharedDataUsers)
    {
        owner.m_sharedDataUsers = std::make_shared<char>();
    }
    m_sharedDataUsers = owner.m_sharedDataUsers;
    // What Open2 reads from the file; the rest is this slot's own.
    memcpy(Name, owner.Name, sizeof(Name));
    Version = owner.Version;
    NumMeshs = owner.NumMeshs;
    NumBones = owner.NumBones;
    NumActions = owner.NumActions;
    Meshs = owner.Meshs;
    Bones = owner.Bones;
    Actions = owner.Actions;
    Textures = owner.Textures;
    IndexTexture = owner.IndexTexture;
    GltfModel = owner.GltfModel;
    m_bSharedData = true;
    // What Init sets for this slot; the bounding boxes of the bones are the owner's.
    renderCount = 0;
    BoneHead = -1;
    StreamMesh = -1;
    m_bCompletedAlloc = true;
}

void BMD::SwapData(BMD& other)
{
    if (&other == this)
    {
        return;
    }
    // They were built from the meshes each slot had.
    ReleaseShadowStreams();
    other.ReleaseShadowStreams();
    std::swap(Name, other.Name);
    std::swap(Version, other.Version);
    std::swap(NumMeshs, other.NumMeshs);
    std::swap(NumBones, other.NumBones);
    std::swap(NumActions, other.NumActions);
    std::swap(Meshs, other.Meshs);
    std::swap(Bones, other.Bones);
    std::swap(Actions, other.Actions);
    std::swap(Textures, other.Textures);
    std::swap(IndexTexture, other.IndexTexture);
    std::swap(GltfModel, other.GltfModel);
    m_boneLookup.clear();
    m_boneLookupBones = nullptr;
    m_boneLookupBoneCount = 0;
    other.m_boneLookup.clear();
    other.m_boneLookupBones = nullptr;
    other.m_boneLookupBoneCount = 0;
    std::swap(m_bSharedData, other.m_bSharedData);
    std::swap(m_sharedDataUsers, other.m_sharedDataUsers);
    std::swap(m_bCompletedAlloc, other.m_bCompletedAlloc);
}

long BMD::GetDataUserCount() const
{
    if (m_sharedDataUsers)
    {
        return m_sharedDataUsers.use_count();
    }
    return Meshs != nullptr ? 1 : 0;
}

void BMD::ReleaseShadowStreams()
{
    auto& shadowStreams = ShadowStreams();
    const auto self = reinterpret_cast<std::uintptr_t>(this);
    for (auto stream = shadowStreams.lower_bound({self, INT_MIN, INT_MIN, INT_MIN, INT_MIN});
         stream != shadowStreams.end() && std::get<0>(stream->first) == self;)
    {
        mu::GetRenderer().ReleaseSkinnedMesh(&*stream);
        stream = shadowStreams.erase(stream);
    }
}

void BMD::Release()
{
    ReleaseShadowStreams();

    const bool othersUseTheData = m_sharedDataUsers && m_sharedDataUsers.use_count() > 1;
    m_sharedDataUsers.reset();
    GltfModel.reset();
    m_boneLookup.clear();
    m_boneLookupBones = nullptr;
    m_boneLookupBoneCount = 0;
    if (othersUseTheData)
    {
        // The last slot that uses the data frees it.
        Meshs = nullptr;
        Bones = nullptr;
        Actions = nullptr;
        Textures = nullptr;
        IndexTexture = nullptr;
        NumBones = 0;
        NumActions = 0;
        NumMeshs = 0;
        m_bSharedData = false;
        m_bCompletedAlloc = false;
        return;
    }

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

            mu::GetRenderer().ReleaseSkinnedMesh(m);
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
    m_bSharedData = false;
    m_bCompletedAlloc = false;
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

namespace
{
// Decrypts the payload of a version 14 model, which follows the int32 payload size at ptr, in place.
// On success, ptr points to the decrypted model data.
bool DecryptModulusBody(unsigned char* fileData, int fileSize, int& ptr)
{
    std::int32_t payloadSize = 0;
    if (fileSize - ptr < static_cast<int>(sizeof(payloadSize)))
    {
        return false;
    }

    std::memcpy(&payloadSize, fileData + ptr, sizeof(payloadSize));
    const int payloadStart = ptr + static_cast<int>(sizeof(payloadSize));
    if (payloadSize < 0 || payloadSize > fileSize - payloadStart)
    {
        return false;
    }

    if (!Render::Models::BmdModulusCipher::DecryptInPlace(fileData + payloadStart,
                                                          static_cast<std::size_t>(payloadSize)))
    {
        return false;
    }

    ptr = payloadStart + static_cast<int>(Render::Models::BmdModulusCipher::BodyOffset);
    return true;
}
} // namespace

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

    const size_t bytesRead = fread(fileData.get(), 1, dataSize, fp);
    fclose(fp);

    // *** Check the "BMD" header *** (and its version byte); a file cut short has neither.
    constexpr int HeaderSize = 4;
    if (dataSize < HeaderSize || bytesRead != static_cast<size_t>(dataSize) ||
        !(fileData[0] == 'B' && fileData[1] == 'M' && fileData[2] == 'D'))
    {
        wprintf(L"[Open2] ERROR: Invalid file header (expected 'BMD') in file %.64ls\n", ModelPath);
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
        std::int32_t encSize = 0;
        // The size field and then no more than the bytes after it, as for version 14.
        if (dataSize - ptr < static_cast<int>(sizeof(encSize)))
        {
            wprintf(L"[Open2] ERROR: Version 12 model without its size in %.64ls\n", ModelPath);
            m_bCompletedAlloc = false;
            return false;
        }
        std::memcpy(&encSize, &fileData[ptr], sizeof(encSize));
        ptr += sizeof(std::int32_t);
        if (encSize < 0 || encSize > dataSize - ptr)
        {
            wprintf(L"[Open2] ERROR: Invalid version 12 model size in %.64ls\n", ModelPath);
            m_bCompletedAlloc = false;
            return false;
        }
        unsigned char* encData = &fileData[ptr];
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
        if (!DecryptModulusBody(fileData.get(), dataSize, ptr))
        {
            wprintf(L"[Open2] ERROR: Invalid version 14 model data in %.64ls\n", ModelPath);
            m_bCompletedAlloc = false;
            return false;
        }
    }
    else if (Version == 0xA)
    {
        // wprintf(L"[Open2] Version: %d\n", Version);
        ptr = 4;
    }
    else
    {
        wprintf(L"[Open2] Unknown BMD version: %d\n in %.64ls\n", static_cast<int>(Version), ModelPath);
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

BMD::BoneLookupEntry& BMD::LookupBone(std::string_view name) const
{
    if (m_boneLookupBones != Bones || m_boneLookupBoneCount != NumBones)
    {
        m_boneLookup.clear();
        m_boneLookupBones = Bones;
        m_boneLookupBoneCount = NumBones;
    }
    for (BoneLookupEntry& entry : m_boneLookup)
    {
        if (entry.name == name)
        {
            return entry;
        }
    }

    int found = -1;
    for (int i = 0; i < NumBones && Bones != nullptr; ++i)
    {
        const Bone_t& bone = Bones[i];
        if (bone.Dummy)
        {
            continue;
        }
        const std::string_view boneName(bone.Name, strnlen(bone.Name, sizeof(bone.Name)));
        if (boneName == name)
        {
            found = i;
            break;
        }
    }
    return m_boneLookup.emplace_back(BoneLookupEntry{std::string(name), found, false});
}

int BMD::FindBone(std::string_view name) const
{
    return LookupBone(name).index;
}

int BMD::FindBoneOrRoot(std::string_view name) const
{
    BoneLookupEntry& entry = LookupBone(name);
    if (entry.index >= 0)
    {
        return entry.index;
    }
    if (!entry.missLogged)
    {
        entry.missLogged = true;
        MU_LOG_WARN(mu::log::Get("render"), "Model {} has no bone \"{}\"; the root bone stands in for it.",
                    std::string_view(Name, strnlen(Name, sizeof(Name))), name);
    }
    return 0;
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
