#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewScene.h"

#include "EffectBrowserAssets.h"
#include "EffectPoolGuard.h"
#include "EffectPreviewGeometry.h"
#include "EffectPreviewItems.h"
#include "EffectPreviewObject.h"
#include "SavedGameRenderState.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/w_CharacterInfo.h"
#include "Render/Effects/EffectRegistry.h"
#include "Render/Items/ItemDisplay.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/RenderUtils.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>

extern float BoneScale;

namespace MuEditor::Effects
{
namespace
{
// Degrees the type and the item turn per frame at the game's frame rate.
constexpr float TurnDegreesPerFrame = 0.6f;
// What the camera frames at least, and the size of a model it cannot measure
// or of a type with nothing to show.
constexpr float MinFramingRadius = 40.0f;
constexpr float UnmeasuredHalfSize = 50.0f;
// The space an item takes around its origin, for the framing.
constexpr float ItemHalfSize = 60.0f;
// The plane reaches this many radii of the type around it; the cube's edge is
// the type's radius within limits.
constexpr float PlaneRadii = 4.0f;
constexpr float MinPlaneSize = 400.0f;
constexpr float MinCubeEdge = 60.0f;
constexpr float MaxCubeEdge = 300.0f;
// Ground decals lie this high above what they are on, so they do not sink
// into it.
constexpr float DecalLift = 0.5f;
// A level that turns on the excellent glow, and an ancient set.
constexpr int ExcellentFlags = 1;
constexpr int AncientDiscriminator = 1;

const std::uint32_t White = mu::PackABGR(1.0f, 1.0f, 1.0f, 1.0f);
const std::uint32_t OpaqueBlack = mu::PackABGR(0.0f, 0.0f, 0.0f, 1.0f);

PreviewVector CenterOf(const PreviewVector& min, const PreviewVector& max)
{
    return {(min[0] + max[0]) * 0.5f, (min[1] + max[1]) * 0.5f, (min[2] + max[2]) * 0.5f};
}

float RadiusOf(const PreviewVector& min, const PreviewVector& max)
{
    const float dx = max[0] - min[0];
    const float dy = max[1] - min[1];
    const float dz = max[2] - min[2];
    return 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);
}

float CubeEdge(float subjectRadius)
{
    return std::clamp(subjectRadius, MinCubeEdge, MaxCubeEdge);
}

// The fields of a model the preview sets. The game sets them before each of
// its own draws; the preview puts them back anyway.
class ScopedModelState
{
public:
    explicit ScopedModelState(BMD& model)
        : m_model(model), m_lightEnable(model.LightEnable), m_contrastEnable(model.ContrastEnable),
          m_hideSkin(model.HideSkin), m_bodyScale(model.BodyScale), m_bodyHeight(model.BodyHeight),
          m_currentAction(model.CurrentAction)
    {
        VectorCopy(model.BodyLight, m_bodyLight);
        VectorCopy(model.BodyOrigin, m_bodyOrigin);
    }

    ~ScopedModelState()
    {
        m_model.LightEnable = m_lightEnable;
        m_model.ContrastEnable = m_contrastEnable;
        m_model.HideSkin = m_hideSkin;
        m_model.BodyScale = m_bodyScale;
        m_model.BodyHeight = m_bodyHeight;
        m_model.CurrentAction = m_currentAction;
        VectorCopy(m_bodyLight, m_model.BodyLight);
        VectorCopy(m_bodyOrigin, m_model.BodyOrigin);
    }

    ScopedModelState(const ScopedModelState&) = delete;
    ScopedModelState& operator=(const ScopedModelState&) = delete;

private:
    BMD& m_model;
    bool m_lightEnable;
    bool m_contrastEnable;
    bool m_hideSkin;
    float m_bodyScale;
    float m_bodyHeight;
    unsigned short m_currentAction;
    vec3_t m_bodyLight;
    vec3_t m_bodyOrigin;
};

// Places the bones of `o`'s model as RenderObject does before it draws
// (Calc_RenderObject), without the terrain light.
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

// The extent of `o`'s model at its first frame, around its origin; false
// for a model without actions, whose bones would stay those of another.
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

void ApplySpriteBlend(PreviewSpriteBlend blend)
{
    switch (blend)
    {
    case PreviewSpriteBlend::Subtract:
        EnableAlphaBlendMinus();
        return;
    case PreviewSpriteBlend::AlphaTest:
        EnableAlphaTest();
        return;
    case PreviewSpriteBlend::Luminance:
        EnableAlphaBlend2();
        return;
    case PreviewSpriteBlend::Glow:
        break;
    }
    EnableAlphaBlend();
}
} // namespace

EffectPreviewScene::EffectPreviewScene() : m_effect(std::make_unique<OBJECT>()) {}

EffectPreviewScene::~EffectPreviewScene() = default;

PreviewFraming EffectPreviewScene::Prepare(const EffectPreviewRequest& request)
{
    if (request.kind != m_effectKind || request.type != m_effectType || request.subType != m_effectSubType ||
        request.subject.draw != m_effectDraw)
        RebuildEffectObject(request);
    const Extent extent = SubjectExtent(request);
    const PreviewVector anchor = Anchor(request);
    PreviewVector min = {extent.min[0] + anchor[0], extent.min[1] + anchor[1], extent.min[2] + anchor[2]};
    PreviewVector max = {extent.max[0] + anchor[0], extent.max[1] + anchor[1], extent.max[2] + anchor[2]};
    float baseHalf = 0.0f;
    if (request.showOn == PreviewShowOn::Cube)
        baseHalf = CubeEdge(RadiusOf(extent.min, extent.max)) * 0.5f;
    else if (request.showOn == PreviewShowOn::Item)
        baseHalf = ItemHalfSize;
    for (int axis = 0; axis < 3; ++axis)
    {
        min[axis] = std::min(min[axis], -baseHalf);
        max[axis] = std::max(max[axis], baseHalf);
    }
    return {CenterOf(min, max), std::max(RadiusOf(min, max), MinFramingRadius)};
}

void EffectPreviewScene::Draw(const EffectPreviewRequest& request, const EffectPreviewCamera& camera, float aspect)
{
    const SavedGameRenderState savedState;
    mu::IMuRenderer& renderer = mu::GetRenderer();
    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PushMatrix();
    renderer.LoadIdentity();
    gluPerspective(EffectPreviewCamera::FieldOfViewDegrees, aspect, camera.Near(), camera.Far());
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PushMatrix();
    const std::array<float, 16> view = camera.View();
    renderer.LoadMatrix(view.data());

    if (request.turn)
        m_turnDegrees = std::fmod(m_turnDegrees + TurnDegreesPerFrame * FPS_ANIMATION_FACTOR, 360.0f);
    const Extent extent = SubjectExtent(request);
    DrawBase(request, RadiusOf(extent.min, extent.max));
    if (request.showOn == PreviewShowOn::Item)
        DrawItem(request);
    DrawSubject(request, camera);
    CoverAlpha(camera, aspect);

    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PopMatrix();
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PopMatrix();
}

void EffectPreviewScene::RebuildEffectObject(const EffectPreviewRequest& request)
{
    m_effectKind = request.kind;
    m_effectType = request.type;
    m_effectSubType = request.subType;
    m_effectDraw = request.subject.draw;
    m_objectNotes = 0;
    m_modelExtent = {{-UnmeasuredHalfSize, -UnmeasuredHalfSize, -UnmeasuredHalfSize},
                     {UnmeasuredHalfSize, UnmeasuredHalfSize, UnmeasuredHalfSize}};
    if (request.kind != Data::Effects::EffectKind::Effect)
        return;
    m_objectNotes =
        BuildPreviewEffectObject(*m_effect, request.type, request.subType, Render::Effects::Lookup(request.type));
    m_creationYaw = m_effect->Angle[2];
    if (request.subject.draw != PreviewDraw::Model || !IsModelLoaded(request.type))
        return;
    Extent measured;
    if (MeasureModel(*m_effect, measured.min, measured.max))
        m_modelExtent = measured;
    else
        m_objectNotes |= NoteNoAnimation;
}

EffectPreviewScene::Extent EffectPreviewScene::SubjectExtent(const EffectPreviewRequest& request) const
{
    switch (request.subject.draw)
    {
    case PreviewDraw::Model:
        return m_modelExtent;
    case PreviewDraw::Sprite:
        if (const BITMAP_t* bitmap = Bitmaps.FindTexture(static_cast<GLuint>(request.type)))
        {
            const float halfWidth = bitmap->Width * SpriteScale(request) * 0.5f;
            const float halfHeight = bitmap->Height * SpriteScale(request) * 0.5f;
            return {{-halfWidth, -halfWidth, -halfHeight}, {halfWidth, halfWidth, halfHeight}};
        }
        break;
    case PreviewDraw::GroundDecal:
    {
        const float half = m_effect->Scale * TERRAIN_SCALE * 0.5f;
        return {{-half, -half, 0.0f}, {half, half, 0.0f}};
    }
    case PreviewDraw::None:
        break;
    }
    return {{-UnmeasuredHalfSize, -UnmeasuredHalfSize, -UnmeasuredHalfSize},
            {UnmeasuredHalfSize, UnmeasuredHalfSize, UnmeasuredHalfSize}};
}

float EffectPreviewScene::BaseTop(const EffectPreviewRequest& request, float subjectRadius) const
{
    return request.showOn == PreviewShowOn::Cube ? CubeEdge(subjectRadius) : 0.0f;
}

// The type stands on the plane and on the cube, and sits at the centre of an
// item (a ground decal too); on the plane and the cube a ground decal lies on
// top.
PreviewVector EffectPreviewScene::Anchor(const EffectPreviewRequest& request) const
{
    const Extent extent = SubjectExtent(request);
    if (request.showOn == PreviewShowOn::Item)
    {
        const PreviewVector center = CenterOf(extent.min, extent.max);
        return {-center[0], -center[1], -center[2]};
    }
    const float lift = request.subject.draw == PreviewDraw::GroundDecal ? DecalLift : 0.0f;
    return {0.0f, 0.0f, BaseTop(request, RadiusOf(extent.min, extent.max)) - extent.min[2] + lift};
}

// Effects are drawn at the scale of their creation values, sprites,
// particles and lightning at the size of their texture.
float EffectPreviewScene::SpriteScale(const EffectPreviewRequest& request) const
{
    return request.kind == Data::Effects::EffectKind::Effect ? m_effect->Scale : 1.0f;
}

void EffectPreviewScene::DrawBase(const EffectPreviewRequest& request, float subjectRadius)
{
    if (request.showOn != PreviewShowOn::Plane && request.showOn != PreviewShowOn::Cube)
        return;
    DisableAlphaBlend();
    DisableTexture();
    DisableCullFace();
    EnableDepthTest();
    if (request.showOn == PreviewShowOn::Plane)
    {
        const PreviewGeometry::PlaneQuads plane =
            PreviewGeometry::Plane(std::max(subjectRadius * PlaneRadii * 2.0f, MinPlaneSize));
        mu::GetRenderer().RenderQuad3D(plane, 0u);
        return;
    }
    const PreviewGeometry::CubeQuads cube = PreviewGeometry::Cube(CubeEdge(subjectRadius));
    mu::GetRenderer().RenderQuad3D(cube, 0u);
}

// As the inventory draws an item (RenderObjectScreen), at the origin and with
// the turn and scale it has on the ground. The item's own sprites, particles
// and effects are removed again.
void EffectPreviewScene::DrawItem(const EffectPreviewRequest& request)
{
    const PreviewItemModels models = GetPreviewItemModels(request.itemType, request.itemLevel);
    if (!models.CanDraw())
        return;
    // Armor is drawn with its own meshes on the bones of the skeleton; both
    // keep their fields.
    BMD& b = Models[models.skeleton];
    const ScopedModelState savedSkeleton(b);
    const ScopedModelState savedModel(Models[models.model]);
    const Render::Items::Display::GroundDisplay ground = Render::Items::Display::GetGroundDisplay(models.model);

    CHARACTER item;
    OBJECT* o = &item.Object;
    o->Type = models.model;
    ItemObjectAttribute(o);
    o->LightEnable = false;
    item.Class = CLASS_ELF;
    if (ground.scale)
        o->Scale = *ground.scale;
    Vector(ground.rotation[0], ground.rotation[1], ground.rotation[2] + m_turnDegrees, o->Angle);
    Vector(0.0f, 0.0f, 0.0f, o->Position);
    b.CurrentAction = 0;
    b.BodyHeight = ground.bodyHeight;
    b.Animation(BoneTransform, 0.0f, 0.0f, 0, o->Angle, o->HeadAngle, false, false);

    vec3_t light = {1.0f, 1.0f, 1.0f};
    const EffectPoolGuard removeItemEffects;
    RenderPartObject(o, models.model, nullptr, light, o->Alpha, request.itemLevel,
                     request.itemExcellent ? ExcellentFlags : 0, request.itemAncient ? AncientDiscriminator : 0, true,
                     true, true);
}

void EffectPreviewScene::DrawSubject(const EffectPreviewRequest& request, const EffectPreviewCamera& camera)
{
    const PreviewVector anchor = Anchor(request);
    switch (request.subject.draw)
    {
    case PreviewDraw::Model:
        DrawModel(anchor);
        return;
    case PreviewDraw::Sprite:
        DrawSprite(request, anchor, camera);
        return;
    case PreviewDraw::GroundDecal:
        DrawGroundDecal(request, anchor);
        return;
    case PreviewDraw::None:
        return;
    }
}

// RenderObject's generic draw (Draw_RenderObject), without its per-type
// branches, which create sprites, particles and effects; animated as
// MoveEffect animates it, but from the start again when it ends.
void EffectPreviewScene::DrawModel(const PreviewVector& anchor)
{
    OBJECT& o = *m_effect;
    if (!IsModelLoaded(o.Type) || (m_objectNotes & (NoteNoAnimation | NoteHiddenModel)) != 0)
        return;
    BMD& b = Models[o.Type];
    const ScopedModelState saved(b);
    if (IsAnimatedByMoveEffect(o.Type, o.SubType))
    {
        b.CurrentAction = o.CurrentAction;
        if (!b.PlayAnimation(&o.AnimationFrame, &o.PriorAnimationFrame, &o.PriorAction, o.Velocity, o.Position,
                             o.Angle))
        {
            o.AnimationFrame = 0.0f;
            o.PriorAnimationFrame = 0.0f;
        }
    }
    Vector(anchor[0], anchor[1], anchor[2], o.Position);
    o.Angle[2] = m_creationYaw + m_turnDegrees;
    PoseLikeRenderObject(o, b);
    b.Transform(BoneTransform, o.BoundingBoxMin, o.BoundingBoxMax, &o.OBB, false);
    EnableDepthTest();
    const int renderFlags = o.RenderType == RENDER_DARK ? (RENDER_TEXTURE | RENDER_DARK) : RENDER_TEXTURE;
    b.RenderBody(renderFlags, o.Alpha, o.BlendMesh, o.BlendMeshLight, o.BlendMeshTexCoordU, o.BlendMeshTexCoordV,
                 o.HiddenMesh);
}

void EffectPreviewScene::DrawSprite(const EffectPreviewRequest& request, const PreviewVector& anchor,
                                    const EffectPreviewCamera& camera)
{
    const BITMAP_t* bitmap = Bitmaps.FindTexture(static_cast<GLuint>(request.type));
    if (bitmap == nullptr)
        return;
    ApplySpriteBlend(request.spriteBlend);
    EnableDepthTest();
    const float scale = SpriteScale(request);
    const PreviewGeometry::Quad quad =
        PreviewGeometry::Billboard(anchor, camera.GetBasis(), bitmap->Width * scale, bitmap->Height * scale, White);
    mu::GetRenderer().RenderQuad3D(quad, static_cast<std::uint32_t>(request.type));
}

// RenderEffectShadows lays the texture on the ground at the effect's scale
// in terrain tiles and with the glow blend.
void EffectPreviewScene::DrawGroundDecal(const EffectPreviewRequest& request, const PreviewVector& anchor)
{
    if (Bitmaps.FindTexture(static_cast<GLuint>(request.type)) == nullptr)
        return;
    EnableAlphaBlend();
    EnableDepthTest();
    const PreviewGeometry::Quad quad =
        PreviewGeometry::GroundQuad(anchor, m_effect->Scale * TERRAIN_SCALE, m_turnDegrees, White);
    mu::GetRenderer().RenderQuad3D(quad, static_cast<std::uint32_t>(request.type));
}

// The blends of the effects leave the picture's alpha below 1 where they
// draw, and ImGui would let the window shine through. Adding an opaque black
// quad with the glow blend over everything sets the alpha to 1 and leaves the
// colours.
void EffectPreviewScene::CoverAlpha(const EffectPreviewCamera& camera, float aspect)
{
    EnableAlphaBlend();
    DisableTexture();
    DisableDepthTest();
    const PreviewGeometry::Quad cover =
        PreviewGeometry::ScreenCover(camera.Eye(), camera.GetBasis(), camera.Near(), aspect, OpaqueBlack);
    mu::GetRenderer().RenderQuad3D(cover, 0u);
}
} // namespace MuEditor::Effects

#endif // _EDITOR
