// DefaultCamera.cpp - Legacy third-person follow camera implementation
// Extracted from CameraUtility.cpp

#include "stdafx.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "DefaultCamera.h"
#include "CameraProjection.h"
#include "../Scenes/SceneCore.h"
#include "../ZzzInterface.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzObject.h"
#include "../ZzzCharacter.h"
#include "../ZzzLodTerrain.h"
#include "../MapManager.h"
#include "../CameraMove.h"
#include "../NewUISystem.h"
#include "../CDirection.h"
#include "../w_MapHeaders.h"
#include "../UIManager.h"
#include "CameraDebugLog.h"

// External variable declarations
extern short g_shCameraLevel;
extern float g_fSpecialHeight;

#ifdef _EDITOR
// DevEditor per-camera config override (global scope required for extern "C").
// DefaultCamera always passes "Default" so the DevEditor routes to this
// camera's override state.
extern "C" bool DevEditor_IsCameraOverrideEnabled(const char* cameraName);
extern "C" void DevEditor_ApplyCameraOverride(const char* cameraName, CameraConfig* cfg);
extern "C" void DevEditor_GetDefaultCameraOffset(float* outX, float* outY, float* outZ);
#endif

namespace
{
    // LoginScene fallback when no tour waypoints available
    constexpr float LOGIN_SCENE_FALLBACK_Z = 500.0f;
    constexpr float LOGIN_SCENE_FALLBACK_PITCH = -80.0f;

    // Sentinel value used to invalidate the frustum cache (forces next-frame rebuild)
    constexpr float CACHE_INVALIDATE_SENTINEL = -999999.0f;

    // Camera position tuning constants. The values were inherited as magic
    // numbers in the legacy code; they're kept identical here, just named.
    constexpr float TOUR_VIEWFAR_PER_LEVEL = 390.f;          // ViewFar = 390 * tourLevel
    constexpr float DIRECTION_MODE_HERO_Z = 300.0f;          // Hero Z when in direction mode
    constexpr float TW_HEIGHT_CAMERA_Z = 1200.f;             // Camera Z on tiles flagged TW_HEIGHT
    constexpr float CAMERA_DISTANCE_HEIGHT_OFFSET = 150.f;   // Subtracted from Distance for Z
    constexpr float CUSTOM_DISTANCE_PITCH_DEG = -45.f;       // Pitch used for custom-distance offset

    constexpr float TOUR_BASE_DISTANCE = 1100.f;             // Base distance in tour & Battle Castle
    constexpr float CAMERA_DISTANCE_LEVEL_BASE = 1300.f;     // Distance at g_shCameraLevel == 0
    constexpr float CAMERA_DISTANCE_LEVEL_STEP = 100.f;      // Per-level increment
}

DefaultCamera::DefaultCamera(CameraState& state)
    : m_State(state)
    , m_Config(CameraConfig::ForMainSceneOrbitalCamera())  // Phase 1: Initialize with gameplay config
{
    m_FrustumCache = {};
}

bool DefaultCamera::IsHeroValid() const
{
    extern CHARACTER* Hero;
    return (Hero != nullptr && Hero->Object.Live);
}

void DefaultCamera::Reset()
{
    m_State.Reset();
    // Phase 5: Reset scene tracking to force config reload on next Update()
    m_LastSceneFlag = -1;
}

void DefaultCamera::ApplyConfigToState()
{
    // Apply the loaded m_Config to m_State (FOV/ViewFar/TopViewEnable).
    extern unsigned int WindowWidth, WindowHeight;
    float aspect = (float)WindowWidth / (float)WindowHeight;
    m_State.ViewFar = m_Config.farPlane;
    m_State.FOV = HFovToVFov(m_Config.hFov, aspect);
    m_State.TopViewEnable = false;
}

void DefaultCamera::InvalidateFrustumCache()
{
    m_FrustumCache.Position[0] = m_FrustumCache.Position[1] = m_FrustumCache.Position[2] = CACHE_INVALIDATE_SENTINEL;
    m_FrustumCache.Angle[0] = m_FrustumCache.Angle[1] = m_FrustumCache.Angle[2] = CACHE_INVALIDATE_SENTINEL;
    m_FrustumCache.ViewFar = CACHE_INVALIDATE_SENTINEL;
#ifdef _EDITOR
    m_FrustumCache.EditorFOV = -1.0f;
    m_FrustumCache.EditorFarPlane = -1.0f;
    m_FrustumCache.EditorNearPlane = -1.0f;
    m_FrustumCache.EditorTerrainCullRange = -1.0f;
#endif
}

void DefaultCamera::InitCharacterScene()
{
    m_Config = CameraConfig::ForCharacterScene();
    ApplyConfigToState();

    m_State.Angle[0] = CharacterSceneCamera::ANGLE_PITCH;
    m_State.Angle[1] = 0.0f;
    m_State.Angle[2] = CharacterSceneCamera::ANGLE_ROLL;
    m_State.Position[0] = CharacterSceneCamera::POSITION_X;
    m_State.Position[1] = CharacterSceneCamera::POSITION_Y;
    m_State.Position[2] = CharacterSceneCamera::POSITION_Z;
}

void DefaultCamera::InitMainScene()
{
    m_Config = CameraConfig::ForMainSceneDefaultCamera();
    ApplyConfigToState();

    // Initial position will be recalculated from Hero on the next Update() frame
    if (IsHeroValid())
    {
        extern CHARACTER* Hero;
        VectorCopy(Hero->Object.Position, m_State.Position);
    }
}

void DefaultCamera::InitLoginScene()
{
    m_Config = CameraConfig::ForLoginScene();
    ApplyConfigToState();

    // Initialize to LoginScene WALK_PATHS[0] starting position.
    // Tour mode will update this, but we need a valid initial position.
    if (!CCameraMove::GetInstancePtr()->IsTourMode())
        return;

    vec3_t tourPos;
    CCameraMove::GetInstancePtr()->GetCurrentCameraPos(tourPos);
    if (tourPos[0] != 0.0f || tourPos[1] != 0.0f || tourPos[2] != 0.0f)
    {
        VectorCopy(tourPos, m_State.Position);
        m_State.Angle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
        m_State.Angle[1] = 0.0f;
        m_State.Angle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();
        return;
    }

    // Fallback: Use WALK_PATHS[0] with transformation
    vec3_t startPos = {0.f, 0.f, LOGIN_SCENE_FALLBACK_Z};
    vec3_t startAngle = {LOGIN_SCENE_FALLBACK_PITCH, 0.f, 0.f};
    float tempAngle[3] = {0.f, 0.f, startAngle[2]};
    float Matrix[3][4];
    AngleMatrix(tempAngle, Matrix);
    VectorIRotate(startPos, Matrix, m_State.Position);
    VectorCopy(startAngle, m_State.Angle);
}

void DefaultCamera::ResetForScene(EGameScene scene)
{
    switch (scene)
    {
        case CHARACTER_SCENE:
            InitCharacterScene();
            break;
        case MAIN_SCENE:
            InitMainScene();
            break;
        case LOG_IN_SCENE:
            InitLoginScene();
            break;
        case SERVER_LIST_SCENE:
        case WEBZEN_SCENE:
        case LOADING_SCENE:
        default:
            m_Config = CameraConfig::ForMainSceneOrbitalCamera();
            m_State.TopViewEnable = false;
            break;
    }

    InvalidateFrustumCache();
}

void DefaultCamera::OnActivate(const CameraState& previousState)
{
    // Phase 5: When activating, ensure camera is configured for current scene
    extern EGameScene SceneFlag;

    CAMERA_LOG("[CAM] DefaultCamera::OnActivate - Scene=%d, PrevPos=(%.1f,%.1f,%.1f), PrevAngle=(%.1f,%.1f,%.1f), PrevDist=%.0f",
               (int)SceneFlag,
               previousState.Position[0], previousState.Position[1], previousState.Position[2],
               previousState.Angle[0], previousState.Angle[1], previousState.Angle[2],
               previousState.Distance);

    // FIX: Save previousState before ResetForScene overwrites m_State
    vec3_t savedPosition, savedAngle;
    float savedDistance = previousState.Distance;
    float savedDistanceTarget = previousState.DistanceTarget;
    VectorCopy(previousState.Position, savedPosition);
    VectorCopy(previousState.Angle, savedAngle);

    // ALWAYS call ResetForScene to ensure config is properly loaded
    // This guarantees correct config for each scene (MainScene uses ForMainScene, others use ForGameplay)
    // ResetForScene also handles position/angle initialization for each scene
    ResetForScene(SceneFlag);

    CAMERA_LOG("[CAM]   Config: Far=%.0f, hFOV=%.1f, TerrainCull=%.0f",
               m_Config.farPlane, m_Config.hFov, m_Config.terrainCullRange);

    // FIX: Inherit position and angles from previous camera for seamless transition
    VectorCopy(savedPosition, m_State.Position);
    VectorCopy(savedAngle, m_State.Angle);
    m_State.Distance = savedDistance;
    m_State.DistanceTarget = savedDistanceTarget;

    CAMERA_LOG("[CAM]   After inherit: Pos=(%.1f,%.1f,%.1f), Angle=(%.1f,%.1f,%.1f), Dist=%.0f",
               m_State.Position[0], m_State.Position[1], m_State.Position[2],
               m_State.Angle[0], m_State.Angle[1], m_State.Angle[2],
               m_State.Distance);

    // Update scene tracking to prevent redundant reset in Update()
    m_LastSceneFlag = (int)SceneFlag;

    // FIX: Mark as just activated to skip first frame position recalculation and disable smoothing
    m_bJustActivated = true;
    m_FramesSinceActivation = 0;
}

void DefaultCamera::OnDeactivate()
{
    // Nothing to clean up.
}

bool DefaultCamera::Update()
{
    // Phase 5: Detect scene transitions and properly reset for new scene
    extern EGameScene SceneFlag;
    if (m_LastSceneFlag != (int)SceneFlag)
    {
        // Scene changed - use dedicated reset function
        m_LastSceneFlag = (int)SceneFlag;
        ResetForScene(SceneFlag);
    }

    // FIX: Check if LoginScene is using WALK_PATHS animation (tour mode OFF)
    // In this case, MoveCamera() in LoginScene updates g_Camera directly
    // We should only update frustum and skip position/angle calculations
    if (SceneFlag == LOG_IN_SCENE && !CCameraMove::GetInstancePtr()->IsTourMode())
    {
        // LoginScene WALK_PATHS animation is active
        // g_Camera is updated by LoginScene::MoveCamera() -> MoveCharacterCamera()
        // Always update frustum since camera is moving every frame
        UpdateFrustum();
        return false;  // Camera not locked
    }

    // Note: Tour mode is handled internally by CalculateCameraPosition() and SetCameraAngle()
    // No need to return early - let normal flow continue

    bool bLockCamera = false;

    // FIX: Skip position/angle recalculation on first frame to preserve inherited state
    if (!m_bJustActivated)
    {
        // Initialize default angles (SetCameraAngle will override these for specific scenes)
        m_State.Angle[0] = 0.f;
        m_State.Angle[1] = 0.f;
        m_State.Angle[2] = -45.f;

        SetCameraFOV();

#ifdef ENABLE_EDIT2
        HandleEditorMode();
#endif

        if (m_State.TopViewEnable)
        {
            m_State.ViewFar = 3200.f;
            // Phase 5: NULL check for Hero (may not exist in LoginScene/CharacterScene)
            if (IsHeroValid())
            {
                m_State.Position[0] = Hero->Object.Position[0];
                m_State.Position[1] = Hero->Object.Position[1];
                m_State.Position[2] = m_State.ViewFar;
            }
            // else: maintain current position (fallback for non-MainScene)
        }
        else
        {
            AdjustHeroHeight();
            CalculateCameraPosition();
            SetCameraAngle();
            UpdateCustomCameraDistance();
        }
    }
    else
    {
        // First frame after activation - preserve inherited position/angles
        // Still need to call SetCameraFOV for proper FOV setup
        SetCameraFOV();

        CAMERA_LOG("[CAM] DefaultCamera first frame: Pos=(%.1f,%.1f,%.1f), Angle=(%.1f,%.1f,%.1f)",
                   m_State.Position[0], m_State.Position[1], m_State.Position[2],
                   m_State.Angle[0], m_State.Angle[1], m_State.Angle[2]);

        m_bJustActivated = false;
    }

    // Increment frame counter for smoothing control
    if (m_FramesSinceActivation < 10)  // Cap to prevent overflow
        m_FramesSinceActivation++;

    UpdateCameraDistance();

    // Phase 5 fix: Update frustum only when camera state actually changes
    // This avoids expensive frustum rebuild every frame (20-25% performance gain)
    if (NeedsFrustumUpdate())
    {
        UpdateFrustum();
    }

#ifdef _EDITOR
    // Debug text rendering to verify camera values (editor only)
    {
        g_pRenderText->SetFont(g_hFixFont);
        g_pRenderText->SetTextColor(255, 255, 0, 255);  // Yellow text
        g_pRenderText->SetBgColor(0, 0, 0, 180);        // Semi-transparent black background

        wchar_t debugText[256];
        int yPos = 10;
        const int lineHeight = 15;

        // Camera type and scene
        swprintf(debugText, 256, L"Camera: DefaultCamera | Scene: %d", (int)SceneFlag);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // State values
        swprintf(debugText, 256, L"State.ViewFar: %.0f | State.FOV: %.1f", m_State.ViewFar, m_State.FOV);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // Config values
        swprintf(debugText, 256, L"Config.farPlane: %.0f | Config.hFov: %.1f", m_Config.farPlane, m_Config.hFov);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // Near plane and culling
        swprintf(debugText, 256, L"Config.nearPlane: %.0f | Config.terrainCullRange: %.0f",
                 m_Config.nearPlane, m_Config.terrainCullRange);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // Rendering value (what BeginOpengl actually uses)
        swprintf(debugText, 256, L"g_Camera.ViewFar (rendering): %.0f", g_Camera.ViewFar);
        g_pRenderText->RenderText(10, yPos, debugText);
    }
#endif

    // Phase 5: Sync camera state to legacy g_Camera global
    // This is needed because BeginOpengl() still uses g_Camera.FOV for perspective setup
    VectorCopy(m_State.Position, g_Camera.Position);
    VectorCopy(m_State.Angle, g_Camera.Angle);
    g_Camera.FOV = m_State.FOV;
    // g_Camera.ViewNear intentionally NOT set from m_Config.nearPlane — the
    // CameraConfig nearPlane is for frustum culling; gluPerspective uses the
    // legacy ViewNear (20.0) for depth precision. Syncing would clip geometry.

    // FIX Issue #1: Use m_Config.farPlane for rendering, not zoom-adjusted m_State.ViewFar
    float effectiveFarPlane = m_Config.farPlane;

#ifdef _EDITOR
    // Log only when the value changes
    static float lastLoggedValue = -1.0f;
    if (effectiveFarPlane != lastLoggedValue)
    {
        CAMERA_LOG("[CAM] DefaultCamera: Setting g_Camera.ViewFar=%.0f (from m_Config.farPlane=%.0f)",
                   effectiveFarPlane, m_Config.farPlane);
        lastLoggedValue = effectiveFarPlane;
    }
#endif
    g_Camera.ViewFar = effectiveFarPlane;

    return bLockCamera;
}

void DefaultCamera::CalculateCameraViewFar()
{
    // Use config's farPlane as single source of truth.
    // Per-map ViewFar multipliers (BattleCastle/PK Field/6th-char-home/etc)
    // were removed so all gameplay maps share the same zoom-level scaling.
    float baseFarPlane = m_Config.farPlane;

    // Handle camera level based view distance (zoom levels)
    switch (g_shCameraLevel)
    {
    case 0:
        if (SceneFlag == LOG_IN_SCENE)
        {
            // LoginScene uses its own config with massive far plane
            m_State.ViewFar = m_Config.farPlane;
        }
        else if (SceneFlag == CHARACTER_SCENE)
        {
            // CharacterScene uses its own config (FOV 71, Far 4100)
            m_State.ViewFar = m_Config.farPlane;
        }
        else if (g_Direction.m_CKanturu.IsMayaScene())
        {
            m_State.ViewFar = baseFarPlane * 0.96f;  // Slightly less for Kanturu Maya
        }
        else
        {
            // Default gameplay: use config's farPlane directly
            m_State.ViewFar = baseFarPlane;
        }
        break;
    case 1: m_State.ViewFar = baseFarPlane * 1.04f; break;  // Zoom level 1
    case 2: m_State.ViewFar = baseFarPlane * 1.08f; break;  // Zoom level 2
    case 3: m_State.ViewFar = baseFarPlane * 1.23f; break;  // Zoom level 3
    case 4:
    case 5: m_State.ViewFar = baseFarPlane * 1.33f; break;  // Zoom level 4-5
    default: m_State.ViewFar = baseFarPlane; break;
    }
}

namespace
{
    // Per-mount Z offsets (world units above terrain).
    // Camera lift for ground mounts (character stays on terrain, camera rises
    // for a better vantage point). Dinorant gets 0 because MoveCharacterPosition
    // already lifts the character model (+30 normal, +90 sky maps).
    constexpr float MOUNT_OFFSET_GROUND         = 30.0f;

    // Time-based smooth transition for mount/dismount (frame-rate independent).
    // Uses exponential decay: reaches ~95% of target in MOUNT_TRANSITION_MS.
    constexpr float MOUNT_TRANSITION_MS         = 500.0f;
    constexpr float MOUNT_LERP_TAU              = MOUNT_TRANSITION_MS / 3000.0f;  // time constant (seconds)

    // Snap threshold — when the offset is close enough, snap to target
    // to avoid endless micro-oscillation.
    constexpr float MOUNT_LERP_SNAP_THRESHOLD   = 0.5f;
}

float DefaultCamera::GetTargetMountOffset() const
{
    if (!IsHeroValid() || Hero->SafeZone || gMapManager.WorldActive == -1)
        return 0.0f;

    switch (Hero->Helper.Type)
    {
    case MODEL_HORN_OF_DINORANT:
        return 0.0f;  // Character model already floats via MoveCharacterPosition
    case MODEL_HORN_OF_UNIRIA:
    case MODEL_DARK_HORSE_ITEM:
    case MODEL_HORN_OF_FENRIR:
        return MOUNT_OFFSET_GROUND;
    default:
        return 0.0f;
    }
}

void DefaultCamera::SyncMountOffset()
{
    m_CurrentMountOffset = GetTargetMountOffset();
    m_LastMountType = IsHeroValid() ? Hero->Helper.Type : -1;
}

void DefaultCamera::AdjustHeroHeight()
{
    if (!IsHeroValid())
        return;

    // Chaos Castle and Kanturu 3rd: special Z handling, skip normal logic
    if (gMapManager.InChaosCastle() && Hero->Object.m_bActionStart)
        return;
    if (gMapManager.WorldActive == WD_39KANTURU_3RD && Hero->Object.m_bActionStart)
        return;

    // Set hero height. Must match MoveCharacterPosition() in ZzzCharacter.cpp
    // so there's no jump between standing (this code) and moving (that code).
    // Dinorant flies above terrain; all other mounts stay on the ground.
    float baseHeight = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]);
    if (Hero->Helper.Type == MODEL_HORN_OF_DINORANT && !Hero->SafeZone && gMapManager.WorldActive != -1)
    {
        if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
            Hero->Object.Position[2] = baseHeight + 90.f;
        else
            Hero->Object.Position[2] = baseHeight + 30.f;
    }
    else
    {
        Hero->Object.Position[2] = baseHeight;
    }

    // Compute target mount offset and lerp for camera
    float targetOffset = GetTargetMountOffset();

    if (Hero->Helper.Type != m_LastMountType)
        m_LastMountType = Hero->Helper.Type;

    // Time-based exponential lerp (frame-rate independent).
    extern float FPS_ANIMATION_FACTOR;
    float dt = FPS_ANIMATION_FACTOR / 25.0f;  // REFERENCE_FPS = 25
    float delta = targetOffset - m_CurrentMountOffset;
    if (fabsf(delta) < MOUNT_LERP_SNAP_THRESHOLD)
        m_CurrentMountOffset = targetOffset;
    else
        m_CurrentMountOffset += delta * (1.0f - expf(-dt / MOUNT_LERP_TAU));
}

void DefaultCamera::CalculateCameraPosition()
{
    vec3_t Position, TransformPosition;
    float Matrix[3][4];

    CalculateCameraViewFar();

    Vector(0.f, -m_State.Distance, 0.f, Position);
    AngleMatrix(m_State.Angle, Matrix);
    VectorIRotate(Position, Matrix, TransformPosition);

    // Phase 5 fix: Handle tour mode (LoginScene) BEFORE Hero check
    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CCameraMove::GetInstancePtr()->UpdateTourWayPoint();
        CCameraMove::GetInstancePtr()->GetCurrentCameraPos(Position);
        m_State.ViewFar = TOUR_VIEWFAR_PER_LEVEL * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();

        // Tour mode handles camera completely - set position and return
        VectorAdd(Position, TransformPosition, m_State.Position);
        return;
    }

    // Phase 5: NULL check - if Hero invalid and not in tour mode, use fallback
    if (!IsHeroValid())
    {
        // Fallback: Use CharacterScene or LoginScene static positions
        // (SetCameraAngle() sets hardcoded positions for CHARACTER_SCENE)
        return;
    }

    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));

    if (SceneFlag == MAIN_SCENE)
    {
        g_pCatapultWindow->GetCameraPos(Position);
    }

    if (g_Direction.IsDirection() && !g_Direction.m_bDownHero)
    {
        Hero->Object.Position[2] = DIRECTION_MODE_HERO_Z;
        g_shCameraLevel = g_Direction.GetCameraPosition(Position);
    }
    else g_shCameraLevel = 0;

    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        vec3_t temp = { 0.0f, 0.0f, -100.0f };
        VectorAdd(TransformPosition, temp, TransformPosition);
    }

    VectorAdd(Position, TransformPosition, m_State.Position);

    if (!CCameraMove::GetInstancePtr()->IsTourMode())
    {
        m_State.Position[2] = Hero->Object.Position[2];
    }

    if ((TerrainWall[iIndex] & TW_HEIGHT) == TW_HEIGHT)
    {
        m_State.Position[2] = g_fSpecialHeight = TW_HEIGHT_CAMERA_Z + 1;
    }
    m_State.Position[2] += m_State.Distance - CAMERA_DISTANCE_HEIGHT_OFFSET;

    // Raise camera when mounted (smooth lerp computed in AdjustHeroHeight).
    // Character stays on the ground; only the camera view lifts.
    m_State.Position[2] += m_CurrentMountOffset;

    // Apply custom camera distance for special terrain
    if (m_State.CustomDistance != 0.f)
    {
        vec3_t angle = { 0.f, 0.f, CUSTOM_DISTANCE_PITCH_DEG };
        Vector(0.f, m_State.CustomDistance, 0.f, Position);
        AngleMatrix(angle, Matrix);
        VectorIRotate(Position, Matrix, TransformPosition);
        VectorAdd(m_State.Position, TransformPosition, m_State.Position);
    }

#ifdef _EDITOR
    // DevEditor: additive world-space camera offset (no-op when override disabled).
    float ox = 0.0f, oy = 0.0f, oz = 0.0f;
    DevEditor_GetDefaultCameraOffset(&ox, &oy, &oz);
    m_State.Position[0] += ox;
    m_State.Position[1] += oy;
    m_State.Position[2] += oz;
#endif
}

void DefaultCamera::SetCameraAngle()
{
    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        // Apply angle offsets for LoginScene camera tuning
        extern float g_LoginSceneAnglePitch;
        extern float g_LoginSceneAngleYaw;
        extern CMapManager gMapManager;

        CCameraMove::GetInstancePtr()->SetAngleFrustum(-112.5f);
        m_State.Angle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
        m_State.Angle[1] = 0.0f;
        m_State.Angle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();

        // Apply offsets only in LoginScene (WD_73NEW_LOGIN_SCENE)
        if (gMapManager.WorldActive == 73)
        {
            m_State.Angle[0] += g_LoginSceneAnglePitch;
            m_State.Angle[2] += g_LoginSceneAngleYaw;
        }
    }
    else if (SceneFlag == CHARACTER_SCENE)
    {
        m_State.Angle[0] = -84.5f;
        m_State.Angle[1] = 0.0f;
        m_State.Angle[2] = -75.0f;
        m_State.Position[0] = 9758.93f;
        m_State.Position[1] = 18913.11f;
        m_State.Position[2] = 675.5f;
    }
    else
    {
        m_State.Angle[0] = -48.5f;
    }

    m_State.Angle[0] += EarthQuake;
}

void DefaultCamera::UpdateCustomCameraDistance()
{
    // Phase 5: NULL check - custom distance only applies when Hero exists
    if (!IsHeroValid())
        return;

    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));

    if ((TerrainWall[iIndex] & TW_CAMERA_UP) == TW_CAMERA_UP)
    {
        if (m_State.CustomDistance <= CUSTOM_CAMERA_DISTANCE1)
        {
            m_State.CustomDistance += 10;
        }
    }
    else
    {
        if (m_State.CustomDistance > 0)
        {
            m_State.CustomDistance -= 10;
        }
    }
}

void DefaultCamera::UpdateCameraDistance()
{
    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        m_State.DistanceTarget = TOUR_BASE_DISTANCE * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel() * 0.1f;
        m_State.Distance = m_State.DistanceTarget;
        return;
    }

    switch (g_shCameraLevel)
    {
    case 0: m_State.DistanceTarget = CAMERA_DISTANCE_LEVEL_BASE + 0 * CAMERA_DISTANCE_LEVEL_STEP; break;
    case 1: m_State.DistanceTarget = CAMERA_DISTANCE_LEVEL_BASE + 1 * CAMERA_DISTANCE_LEVEL_STEP; break;
    case 2: m_State.DistanceTarget = CAMERA_DISTANCE_LEVEL_BASE + 2 * CAMERA_DISTANCE_LEVEL_STEP; break;
    case 3: m_State.DistanceTarget = CAMERA_DISTANCE_LEVEL_BASE + 3 * CAMERA_DISTANCE_LEVEL_STEP; break;
    case 4: m_State.DistanceTarget = CAMERA_DISTANCE_LEVEL_BASE + 4 * CAMERA_DISTANCE_LEVEL_STEP; break;
    case 5: m_State.DistanceTarget = g_Direction.m_fCameraViewFar; break;
    }

    // Disable distance smoothing for first 2 frames after activation to
    // prevent visible interpolation when switching from OrbitalCamera.
    if (m_FramesSinceActivation < 2)
        m_State.Distance = m_State.DistanceTarget;
    else
        m_State.Distance += (m_State.DistanceTarget - m_State.Distance) / 3;
}

void DefaultCamera::SetCameraFOV()
{
    extern unsigned int WindowWidth, WindowHeight;
    float aspect = (float)WindowWidth / (float)WindowHeight;

    if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE
        && CCameraMove::GetInstancePtr()->IsTourMode())
    {
        // Tour mode uses wider FOV for cinematic feel
        m_State.FOV = HFovToVFov(m_Config.hFov, aspect);
    }
    else
    {
        // Compute vertical FOV from horizontal FOV and current aspect ratio
        m_State.FOV = HFovToVFov(m_Config.hFov, aspect);
    }
}

#ifdef ENABLE_EDIT2
void DefaultCamera::HandleEditorMode()
{
    bool EditMove = false;

    if (!g_pUIManager->IsInputEnable())
    {
        // Handle camera angle rotation
        if (HIBYTE(GetAsyncKeyState(VK_INSERT)) == 128)
            m_State.Angle[2] += 15;
        if (HIBYTE(GetAsyncKeyState(VK_DELETE)) == 128)
            m_State.Angle[2] -= 15;
        if (HIBYTE(GetAsyncKeyState(VK_HOME)) == 128)
            m_State.Angle[2] = -45;

        m_State.Angle[2] = fmodf(m_State.Angle[2] + 360.0f, 360.0f) - 360.0f;

        // Handle movement input
        vec3_t p1, p2;
        Vector(0.f, 0.f, 0.f, p1);
        FLOAT Velocity = TERRAIN_SCALE * 1.25f * FPS_ANIMATION_FACTOR;

        struct KeyMapping {
            int vKey;
            float x;
            float y;
        };

        const KeyMapping keyMappings[] = {
            {VK_LEFT,  -Velocity, -Velocity},
            {VK_RIGHT,  Velocity,  Velocity},
            {VK_UP,    -Velocity,  Velocity},
            {VK_DOWN,   Velocity, -Velocity}
        };

        for (const auto& mapping : keyMappings)
        {
            if (HIBYTE(GetAsyncKeyState(mapping.vKey)) == 128)
            {
                Vector(mapping.x, mapping.y, 0.f, p1);
                EditMove = true;
                break;
            }
        }

        // Phase 5: Only apply movement if Hero is valid
        if (IsHeroValid())
        {
            // Apply rotation and movement
            glPushMatrix();
            glLoadIdentity();
            glRotatef(-m_State.Angle[2], 0.f, 0.f, 1.f);
            float Matrix[3][4];
            CameraProjection::GetOpenGLMatrix(Matrix);
            glPopMatrix();
            VectorRotate(p1, Matrix, p2);
            VectorAdd(Hero->Object.Position, p2, Hero->Object.Position);
        }
    }

    AdjustHeroHeight();

    if (EditMove && IsHeroValid())
    {
        BYTE PathX[1];
        BYTE PathY[1];
        PathX[0] = (BYTE)(Hero->Object.Position[0] / TERRAIN_SCALE);
        PathY[0] = (BYTE)(Hero->Object.Position[1] / TERRAIN_SCALE);

        SendCharacterMove(Hero->Key, Hero->Object.Angle[2], 1, PathX, PathY, PathX[0], PathY[0]);

        Hero->Path.PathNum = 0;
    }
}
#endif //ENABLE_EDIT2

// ========== Phase 1: Configuration & Frustum Management ==========

void DefaultCamera::SetConfig(const CameraConfig& config)
{
    m_Config = config;
    UpdateFrustum();
}

void DefaultCamera::UpdateFrustum()
{
#ifdef _EDITOR
    // DevEditor may be overriding any subset of this camera's CameraConfig.
    if (DevEditor_IsCameraOverrideEnabled("Default"))
        DevEditor_ApplyCameraOverride("Default", &m_Config);
#endif

    // Derive forward/up from camera angles matching GL rotation convention.
    // BeginOpengl applies: glRotatef(A1,0,1,0) * glRotatef(A0,1,0,0) * glRotatef(A2,0,0,1)
    // => R = Ry(A1) * Rx(A0) * Rz(A2).  Forward = R^T*(0,0,-1), Up = R^T*(0,1,0).
    float a0 = m_State.Angle[0] * (Q_PI / 180.0f);
    float a1 = m_State.Angle[1] * (Q_PI / 180.0f);
    float a2 = m_State.Angle[2] * (Q_PI / 180.0f);
    float s0 = sinf(a0), c0 = cosf(a0);
    float s1 = sinf(a1), c1 = cosf(a1);
    float s2 = sinf(a2), c2 = cosf(a2);

    vec3_t forward, up;
    forward[0] = s1 * c2 - c1 * s0 * s2;
    forward[1] = -(s1 * s2 + c1 * s0 * c2);
    forward[2] = -c1 * c0;
    VectorNormalize(forward);

    up[0] = c0 * s2;
    up[1] = c0 * c2;
    up[2] = -s0;
    VectorNormalize(up);

    // Build frustum from current configuration
    extern unsigned int WindowWidth;
    extern unsigned int WindowHeight;
    float aspectRatio = (float)WindowWidth / (float)WindowHeight;

    // Phase 5 FIX: ALWAYS use m_Config values for frustum culling
    // (Override was already applied at the top of this function.)
    float effectiveFarPlane = m_Config.farPlane;
    float effectiveTerrainCullRange = m_Config.terrainCullRange;

    // Convert horizontal FOV to vertical FOV for frustum building
    float vFov = HFovToVFov(m_Config.hFov, aspectRatio);

    m_Frustum.BuildFromCamera(
        m_State.Position,
        forward,
        up,
        vFov,
        aspectRatio,
        m_Config.nearPlane,
        effectiveFarPlane,
        effectiveTerrainCullRange
    );

    // Phase 5: Cache current state for next frame's comparison
    VectorCopy(m_State.Position, m_FrustumCache.Position);
    VectorCopy(m_State.Angle, m_FrustumCache.Angle);
    m_FrustumCache.ViewFar = effectiveFarPlane;
    m_FrustumCache.AspectRatio = aspectRatio;

#ifdef _EDITOR
    if (DevEditor_IsCameraOverrideEnabled("Default"))
    {
        m_FrustumCache.EditorFOV = m_Config.hFov;
        m_FrustumCache.EditorFarPlane = m_Config.farPlane;
        m_FrustumCache.EditorNearPlane = m_Config.nearPlane;
        m_FrustumCache.EditorTerrainCullRange = m_Config.terrainCullRange;
    }
#endif
}

bool DefaultCamera::NeedsFrustumUpdate() const
{
    // Phase 5: Check if camera state changed since last frustum rebuild
    const float EPSILON = 0.01f;  // Small threshold to avoid floating point comparison issues

#ifdef _EDITOR
    // If the DevEditor override is active, force a rebuild every frame —
    // any of {hFov, near, far, terrainCull, objectCull} may have been nudged.
    if (DevEditor_IsCameraOverrideEnabled("Default"))
        return true;
#endif

    // Check position change
    if (fabs(m_State.Position[0] - m_FrustumCache.Position[0]) > EPSILON ||
        fabs(m_State.Position[1] - m_FrustumCache.Position[1]) > EPSILON ||
        fabs(m_State.Position[2] - m_FrustumCache.Position[2]) > EPSILON)
    {
        return true;
    }

    // Check angle change
    if (fabs(m_State.Angle[0] - m_FrustumCache.Angle[0]) > EPSILON ||
        fabs(m_State.Angle[1] - m_FrustumCache.Angle[1]) > EPSILON ||
        fabs(m_State.Angle[2] - m_FrustumCache.Angle[2]) > EPSILON)
    {
        return true;
    }

    // Check ViewFar change
    if (fabs(m_State.ViewFar - m_FrustumCache.ViewFar) > EPSILON)
    {
        return true;
    }

    // Check aspect ratio change (window resize / runtime resolution switch).
    // Frustum width depends on aspect; without this the cache would stay valid
    // through a resize and culling at the screen edges would go stale.
    extern unsigned int WindowWidth;
    extern unsigned int WindowHeight;
    const float aspectRatio = (float)WindowWidth / (float)WindowHeight;
    if (fabs(aspectRatio - m_FrustumCache.AspectRatio) > EPSILON)
    {
        return true;
    }

    return false;  // No significant change, skip frustum rebuild
}
