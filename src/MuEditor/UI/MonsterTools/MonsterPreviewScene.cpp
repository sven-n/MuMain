#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterPreviewScene.h"

#include "../MuEditor/UI/EffectBrowser/EffectPreviewGeometry.h"
#include "../MuEditor/UI/EffectBrowser/PreviewModelPose.h"
#include "../MuEditor/UI/EffectBrowser/SavedGameRenderState.h"
#include "Core/Globals/_define.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/RenderUtils.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace MuEditor::Monsters
{
namespace
{
using Effects::PreviewFraming;
using Effects::PreviewVector;

// The size of a model the preview cannot measure, and what the camera
// frames at least.
constexpr float UnmeasuredHalfSize = 60.0f;
constexpr float MinFramingRadius = 40.0f;
// The plane reaches this many radii of the model around it.
constexpr float PlaneRadii = 3.0f;
constexpr float MinPlaneSize = 300.0f;

const std::uint32_t OpaqueBlack = mu::PackABGR(0.0f, 0.0f, 0.0f, 1.0f);

float RadiusOf(const PreviewVector& min, const PreviewVector& max)
{
    const float dx = max[0] - min[0];
    const float dy = max[1] - min[1];
    const float dz = max[2] - min[2];
    return 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool IsDrawable(int slot)
{
    return Models != nullptr && slot >= 0 && slot < MAX_MODELS && Models[slot].NumActions > 0 &&
           Models[slot].NumMeshs > 0;
}
} // namespace

MonsterPreviewScene::MonsterPreviewScene() : m_object(std::make_unique<OBJECT>()) {}

MonsterPreviewScene::~MonsterPreviewScene() = default;

// As CreateCharacter sets up a character's object, at the origin and
// without the terrain's light.
void MonsterPreviewScene::SetUp(int slot, int animation, float frame, float priorFrame, float turnDegrees)
{
    OBJECT& o = *m_object;
    o.Initialize();
    o.Live = true;
    o.Type = slot;
    o.Scale = 1.0f;
    o.Alpha = 1.0f;
    o.LightEnable = true;
    o.ContrastEnable = false;
    o.HiddenMesh = -1;
    o.BlendMesh = -1;
    o.BlendMeshLight = 1.0f;
    o.BlendMeshTexCoordU = 0.0f;
    o.BlendMeshTexCoordV = 0.0f;
    o.RenderType = 0;
    o.CurrentAction = static_cast<unsigned short>(animation);
    o.PriorAction = static_cast<unsigned short>(animation);
    o.AnimationFrame = frame;
    o.PriorAnimationFrame = priorFrame;
    Vector(1.0f, 1.0f, 1.0f, o.Light);
    Vector(0.0f, 0.0f, turnDegrees, o.Angle);
    Vector(0.0f, 0.0f, 0.0f, o.HeadAngle);
    Vector(0.0f, 0.0f, 0.0f, o.Position);
}

PreviewFraming MonsterPreviewScene::Frame(int slot)
{
    PreviewVector min = {-UnmeasuredHalfSize, -UnmeasuredHalfSize, 0.0f};
    PreviewVector max = {UnmeasuredHalfSize, UnmeasuredHalfSize, 2.0f * UnmeasuredHalfSize};
    if (IsDrawable(slot))
    {
        SetUp(slot, 0, 0.0f, 0.0f, 0.0f);
        PreviewVector measuredMin{};
        PreviewVector measuredMax{};
        if (Effects::MeasureModel(*m_object, measuredMin, measuredMax))
        {
            min = measuredMin;
            max = measuredMax;
        }
    }
    // The plane under the model is part of the picture.
    min[2] = std::min(min[2], 0.0f);
    const PreviewVector center = {(min[0] + max[0]) * 0.5f, (min[1] + max[1]) * 0.5f, (min[2] + max[2]) * 0.5f};
    return {center, std::max(RadiusOf(min, max), MinFramingRadius)};
}

void MonsterPreviewScene::Draw(int slot, int animation, float frame, float priorFrame, float turnDegrees,
                               const Effects::EffectPreviewCamera& camera, float aspect)
{
    const Effects::SavedGameRenderState savedState;
    mu::IMuRenderer& renderer = mu::GetRenderer();
    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PushMatrix();
    renderer.LoadIdentity();
    gluPerspective(Effects::EffectPreviewCamera::FieldOfViewDegrees, aspect, camera.Near(), camera.Far());
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PushMatrix();
    const std::array<float, 16> view = camera.View();
    renderer.LoadMatrix(view.data());

    // The plane.
    DisableAlphaBlend();
    DisableTexture();
    DisableCullFace();
    EnableDepthTest();
    const float planeSize = std::max(camera.Distance() * PlaneRadii, MinPlaneSize);
    const Effects::PreviewGeometry::PlaneQuads plane = Effects::PreviewGeometry::Plane(planeSize);
    renderer.RenderQuad3D(plane, 0u);

    // The model, as RenderObject's generic draw draws it.
    if (IsDrawable(slot))
    {
        BMD& b = Models[slot];
        const Effects::ScopedModelState saved(b);
        SetUp(slot, animation, frame, priorFrame, turnDegrees);
        OBJECT& o = *m_object;
        Effects::PoseLikeRenderObject(o, b);
        b.Transform(BoneTransform, o.BoundingBoxMin, o.BoundingBoxMax, &o.OBB, false);
        EnableDepthTest();
        b.RenderBody(RENDER_TEXTURE, o.Alpha, o.BlendMesh, o.BlendMeshLight, o.BlendMeshTexCoordU, o.BlendMeshTexCoordV,
                     o.HiddenMesh);
    }

    // The blends leave the picture's alpha below 1 where they draw; an opaque
    // black quad with the glow blend over everything sets it to 1 and leaves
    // the colours (as the effect preview does).
    EnableAlphaBlend();
    DisableTexture();
    DisableDepthTest();
    const Effects::PreviewGeometry::Quad cover =
        Effects::PreviewGeometry::ScreenCover(camera.Eye(), camera.GetBasis(), camera.Near(), aspect, OpaqueBlack);
    renderer.RenderQuad3D(cover, 0u);

    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PopMatrix();
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PopMatrix();
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
