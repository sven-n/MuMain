#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterPreviewScene.h"

#include "../MuEditor/UI/EffectBrowser/EffectPreviewGeometry.h"
#include "../MuEditor/UI/EffectBrowser/PreviewModelPose.h"
#include "../MuEditor/UI/EffectBrowser/SavedGameRenderState.h"
#include "Camera/CameraState.h"
#include "Character/CSParts.h"
#include "Core/Globals/_define.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ObjectPose.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Monsters/MonsterSetupApply.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/RenderUtils.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/MainScene.h"
#include "Scenes/SceneCore.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

// ZzzCharacter.cpp: the tick's visual part of MoveCharacterClient.
void MoveCharacterVisual(CHARACTER* c, OBJECT* o);

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
constexpr float PlayerHeight = 180.0f;

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

// The game's effects move in the main scene's tick only.
bool InWorld()
{
    return SceneFlag == MAIN_SCENE && Hero != nullptr;
}
} // namespace

MonsterPreviewScene::MonsterPreviewScene() : m_object(std::make_unique<OBJECT>()) {}

MonsterPreviewScene::~MonsterPreviewScene()
{
    ReleaseCharacter();
    if (m_character != nullptr)
        Engine::Object::Attachments::Untrack(m_character.get());
    if (m_character != nullptr && m_character->Object.BoneTransform != nullptr)
    {
        delete[] m_character->Object.BoneTransform;
        m_character->Object.BoneTransform = nullptr;
    }
}

void MonsterPreviewScene::DrawCharacter(int slot, int animation, float frame, float priorFrame, float turnDegrees,
                                        const MonsterPreviewLook& look)
{
    if (m_character == nullptr)
    {
        m_character = std::make_unique<CHARACTER>();
        // A preview's character: parts of its own, updated by its owner.
        Engine::Object::Attachments::Track(m_character.get());
    }
    CHARACTER& c = *m_character;
    OBJECT& o = c.Object;
    // As CreateCharacter and Create make it, again only when it changed: it
    // gets new bones each time.
    if (m_characterSlot != slot || m_characterLook.number != look.number || m_characterLook.kind != look.kind ||
        !(m_characterLook.setup == look.setup))
    {
        ReleaseCharacter();
        m_characterSlot = slot;
        CreateCharacterPointer(&c, slot, 0, 0, turnDegrees);
        c.MonsterIndex = static_cast<EMonsterType>(look.number);
        o.Kind = static_cast<BYTE>(look.kind);
        GameLogic::Monsters::Setup::Dress(c, look.setup);
        o.Alpha = 1.0f;
        // As a spawn makes them, on its first pose.
        m_spawnPending = true;
    }
    m_characterLook = look;
    o.Live = true;
    o.Visible = true;
    // The game's ground shadow falls by the map's sun onto its terrain: on the
    // preview's floor and camera it is a black sheet, so it is left out, as
    // for the bare model.
    c.HideShadow = true;
    o.m_bRenderShadow = false;
    o.CurrentAction = static_cast<unsigned short>(animation);
    o.PriorAction = static_cast<unsigned short>(animation);
    o.AnimationFrame = frame;
    o.PriorAnimationFrame = priorFrame;
    Vector(0.0f, 0.0f, turnDegrees, o.Angle);
    Vector(0.0f, 0.0f, 0.0f, o.Position);
    Engine::Object::Pose::Invalidate(&o);
    // Its bones, parts and cloth, as a preview's owner updates them; then
    // the preview's light over the place's it set.
    UpdatePreviewCharacter(&c);
    Vector(look.light, look.light, look.light, o.Light);
    RenderCharacter(&c, &o, look.pointedAt ? 1 : 0);
    if (!InWorld())
        return;
    // On the pose it was drawn with: the replays asked for, then what the
    // game's tick does for it (its eyes, the trails that follow them, its
    // particles, its fade).
    if (m_spawnPending)
    {
        m_spawnPending = false;
        GameLogic::Monsters::Setup::ReplaySpawnEffects(c, look.setup);
    }
    if (m_appearPending)
    {
        m_appearPending = false;
        GameLogic::Monsters::Setup::ReplayAppear(c, look.setup, m_appearSound);
    }
    MoveCharacterVisual(&c, &o);
    Alpha(&o);
}

void MonsterPreviewScene::ReplaySpawn()
{
    if (m_characterSlot < 0)
        return;
    GameLogic::Monsters::Setup::RemoveEffectsOf(m_character->Object);
    m_spawnPending = true;
}

void MonsterPreviewScene::ReplayAppear(bool sound)
{
    if (m_characterSlot < 0)
        return;
    m_appearPending = true;
    m_appearSound = sound;
}

void MonsterPreviewScene::ReleaseCharacter()
{
    if (m_character != nullptr && m_characterSlot >= 0)
    {
        CHARACTER& c = *m_character;
        GameLogic::Monsters::Setup::RemoveEffectsOf(c.Object);
        // Its cloth and part models: CreateCharacterPointer drops the parts
        // without freeing them, and would keep the old cloth.
        DeleteCloth(&c, &c.Object);
        DeleteParts(&c);
    }
    m_characterSlot = -1;
    m_spawnPending = false;
    m_appearPending = false;
}

// The billboards face the preview's camera, and nothing is left out as off
// the game's screen: the effects far away in the world are there too, beyond
// the camera's far plane.
void MonsterPreviewScene::DrawGameEffects(const Effects::EffectPreviewCamera& camera)
{
    const std::array<float, 16> view = camera.View();
    float cameraMatrix[3][4];
    std::memcpy(cameraMatrix, g_Camera.Matrix, sizeof(cameraMatrix));
    vec3_t cameraPosition;
    VectorCopy(g_Camera.Position, cameraPosition);
    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 4; ++column)
            g_Camera.Matrix[row][column] = view[static_cast<size_t>(column * 4 + row)];
    }
    const Effects::PreviewVector eye = camera.Eye();
    Vector(eye[0], eye[1], eye[2], g_Camera.Position);
    const bool culling = !IsEffectCullingDisabledDebug();
    SetDisableEffectCulling(true);
    std::array<bool, MAX_EFFECTS> visible{};
    for (int i = 0; i < MAX_EFFECTS; ++i)
    {
        visible[static_cast<size_t>(i)] = ::Effects[i].Visible;
        ::Effects[i].Visible = true;
    }
    const SpriteDrawFlags sprites = SpritesDrawnThisFrame();

    // In the world pass's order.
    RenderJoints();
    RenderEffects();
    BeginSprite();
    RenderSprites();
    RenderParticles();
    EndSprite();

    SetSpritesDrawnThisFrame(sprites);
    for (int i = 0; i < MAX_EFFECTS; ++i)
        ::Effects[i].Visible = visible[static_cast<size_t>(i)];
    SetDisableEffectCulling(!culling);
    VectorCopy(cameraPosition, g_Camera.Position);
    std::memcpy(g_Camera.Matrix, cameraMatrix, sizeof(cameraMatrix));
}

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

PreviewFraming MonsterPreviewScene::Frame(int slot, float scale)
{
    PreviewVector min = {-UnmeasuredHalfSize, -UnmeasuredHalfSize, 0.0f};
    PreviewVector max = {UnmeasuredHalfSize, UnmeasuredHalfSize, 2.0f * UnmeasuredHalfSize};
    // The player model has no meshes of its own: a player's height.
    if (slot == MODEL_PLAYER)
        max[2] = PlayerHeight * scale;
    else if (IsDrawable(slot))
    {
        SetUp(slot, 0, 0.0f, 0.0f, 0.0f);
        m_object->Scale = scale;
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
                               const Effects::EffectPreviewCamera& camera, float aspect, const MonsterPreviewLook& look)
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

    // The character, as the game draws it; or the model alone, as
    // RenderObject's generic draw draws it.
    if (Models != nullptr && slot >= 0 && slot < MAX_MODELS && Models[slot].NumActions > 0 && look.asCharacter)
    {
        EnableDepthTest();
        DrawCharacter(slot, animation, frame, priorFrame, turnDegrees, look);
        if (InWorld())
            DrawGameEffects(camera);
    }
    else if (IsDrawable(slot))
    {
        ReleaseCharacter();
        BMD& b = Models[slot];
        const Effects::ScopedModelState saved(b);
        SetUp(slot, animation, frame, priorFrame, turnDegrees);
        OBJECT& o = *m_object;
        Vector(look.light, look.light, look.light, o.Light);
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
