// OrbitalCamera.cpp - Spherical coordinate orbital camera implementation

#include "stdafx.h"
#include "OrbitalCamera.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzCharacter.h"
#include <cmath>

#include "UIControls.h"
#include "GameConfig/GameConfig.h"
#include "CameraDebugLog.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// External globals
extern int MouseWheel;
extern bool MouseMButton;
extern bool MouseMButtonPush;
extern int MouseX;
extern int MouseY;
// Hero is declared in ZzzCharacter.h as CHARACTER*

#ifdef _EDITOR
// DevEditor per-camera config override (global scope required for extern "C").
extern "C" bool DevEditor_IsCameraOverrideEnabled(const char* cameraName);
extern "C" void DevEditor_ApplyCameraOverride(const char* cameraName, CameraConfig* cfg);
extern "C" bool DevEditor_GetOrbitalHullTrapezoid(float* outFarDist, float* outFarWidth,
                                                  float* outNearDist, float* outNearWidth);
#endif

namespace
{
    // Match the natural-pyramid near-edge width used by ZzzLodTerrain's Orbital
    // path so FreeFly-spectator hull equals the active-render hull.
    constexpr float NATURAL_NEAR_HALF_WIDTH = 400.0f;

    // Fallback orbit-origin parameters used by non-MainScene activation.
    // We synthesize a "static camera" and ray-cast from it to find where to
    // place the orbit pivot at the character's Z height.
    constexpr float NON_CHAR_STATIC_CAM_HEIGHT_OFFSET = 300.0f;   // above the character
    constexpr float NON_CHAR_STATIC_CAM_PITCH = -45.0f;

    // Sanity-check ranges for ray-cast intersection t values (world units).
    constexpr float ORBIT_ORIGIN_MAX_T = 5000.0f;
    constexpr float LOOKAT_MAX_T = 2000.0f;
}

OrbitalCamera::OrbitalCamera(CameraState& state)
    : m_State(state)
    , m_pDefaultCamera(std::make_unique<DefaultCamera>(state))
    , m_Config(CameraConfig::ForMainSceneOrbitalCamera())  // OrbitalCamera defaults to extended visibility
    , m_bInitialOffsetSet(false)
    , m_BaseYaw(0.0f)
    , m_BasePitch(0.0f)
    , m_DeltaYaw(0.0f)
    , m_DeltaPitch(0.0f)
    , m_Radius((float)GameConfig::GetInstance().GetZoom())
    , m_LastSceneFlag(-1)  // Phase 5: Initialize to invalid scene
    , m_bJustActivated(false)  // Initialize activation flag
{
    IdentityVector3D(m_Target);
    IdentityVector3D(m_InitialCameraOffset);

    // Sync ViewFar with loaded zoom level
    UpdateConfigForView();
}

void OrbitalCamera::Reset()
{
    m_BaseYaw = 0.0f;
    m_BasePitch = 0.0f;
    m_DeltaYaw = 0.0f;
    m_DeltaPitch = 0.0f;
    m_Radius = (float)GameConfig::GetInstance().GetZoom();
    m_Input.Rotating = false;
    m_bInitialOffsetSet = false;
    // Phase 5: Reset scene tracking to force config reload
    m_LastSceneFlag = -1;
    m_pDefaultCamera->Reset();
}

void OrbitalCamera::ResetForScene(EGameScene scene)
{
    // Save current zoom before scene transition
    GameConfig::GetInstance().SetZoom((int)m_Radius);
    GameConfig::GetInstance().Save();

    // Phase 5: Proper scene-specific reset using switch statement
    switch (scene)
    {
        case CHARACTER_SCENE:
        case MAIN_SCENE:
            LoadConfigForScene(scene);
            ApplyConfigToState();

            // Reset orbital parameters for new scene
            m_DeltaYaw = 0.0f;
            m_DeltaPitch = 0.0f;
            m_Radius = (float)GameConfig::GetInstance().GetZoom();
            m_bInitialOffsetSet = false;
            break;

        case LOG_IN_SCENE:
        case SERVER_LIST_SCENE:
        case WEBZEN_SCENE:
        case LOADING_SCENE:
        default:
            LoadConfigForScene(scene);
            ApplyConfigToState();
            break;
    }

    // Update target immediately with new scene context
    UpdateTarget();

    // Force DefaultCamera to recalculate as well
    m_pDefaultCamera->ResetForScene(scene);
}

void OrbitalCamera::LoadConfigForScene(EGameScene scene)
{
    switch (scene)
    {
        case CHARACTER_SCENE:
            m_Config = CameraConfig::ForCharacterScene();
            break;
        case MAIN_SCENE:
            m_Config = CameraConfig::ForMainSceneOrbitalCamera();
            break;
        default:
            m_Config = CameraConfig::ForMainSceneOrbitalCamera();
            break;
    }
}

void OrbitalCamera::ApplyConfigToState()
{
    extern unsigned int WindowWidth, WindowHeight;
    float aspect = (float)WindowWidth / (float)WindowHeight;
    m_State.ViewFar = m_Config.farPlane;
    m_State.FOV = HFovToVFov(m_Config.hFov, aspect);
    m_State.TopViewEnable = false;
}

void OrbitalCamera::OnActivate(const CameraState& previousState)
{
    extern EGameScene SceneFlag;

    CAMERA_LOG("[CAM] OrbitalCamera::OnActivate - Scene=%d, PrevPos=(%.1f,%.1f,%.1f), PrevAngle=(%.1f,%.1f,%.1f), PrevDist=%.0f",
               (int)SceneFlag,
               previousState.Position[0], previousState.Position[1], previousState.Position[2],
               previousState.Angle[0], previousState.Angle[1], previousState.Angle[2],
               previousState.Distance);

    // Step 1: Load config (don't use ResetForScene — it would call UpdateTarget with wrong pos)
    LoadConfigForScene(SceneFlag);
    ApplyConfigToState();
    CAMERA_LOG("[CAM]   Config: Far=%.0f, hFOV=%.1f, TerrainCull=%.0f",
               m_Config.farPlane, m_Config.hFov, m_Config.terrainCullRange);

    // Step 2: Update target to Hero/Character position
    UpdateTarget();

    // Step 3: Either inherit the previous camera pose (MainScene) or ray-cast from a static
    // camera pose to find a sensible orbit origin (CharacterScene/LoginScene).
    if (SceneFlag == MAIN_SCENE)
    {
        VectorCopy(previousState.Position, m_State.Position);
        VectorCopy(previousState.Angle, m_State.Angle);
        CAMERA_LOG("[CAM]   MainScene: Inherited Pos=(%.1f,%.1f,%.1f), Angle=(%.1f,%.1f,%.1f)",
                   m_State.Position[0], m_State.Position[1], m_State.Position[2],
                   m_State.Angle[0], m_State.Angle[1], m_State.Angle[2]);
    }
    else
    {
        CalculateOrbitOriginForStaticScene(SceneFlag, previousState);
    }

    // Step 4: Compute look-at point and initialize orbital parameters from it
    m_DeltaYaw = 0.0f;
    m_DeltaPitch = 0.0f;

    vec3_t lookAtPoint;
    CalculateLookAtPoint(SceneFlag, lookAtPoint);
    InitializeOrbitalFromCurrentState(lookAtPoint, previousState);

    // Apply zoom-scaled farPlane / cull ranges now, before the first render. The
    // first Update() skips ComputeCameraTransform (and therefore UpdateConfigForView)
    // to preserve the inherited pose — without this call the first frame would
    // render with the unscaled base config and pop to zoom-scaled values on frame 2.
    UpdateConfigForView();

    // Step 5: Update matrix so rendering uses inherited pose, then sync to g_Camera
    m_State.UpdateMatrix();
    SyncStateToGlobalCamera();

    // Step 6: Initialize frustum and scene tracking
    UpdateFrustum();
    m_LastSceneFlag = (int)SceneFlag;

    // Skip DefaultCamera's first-frame update so our inherited pose isn't overwritten
    m_bJustActivated = true;

    // Sync the internal DefaultCamera's mount state immediately so
    // GetMountCameraOffset() returns the correct value — without this,
    // on the first-ever activation the internal DefaultCamera hasn't run
    // Update() yet and would return 0 even though the inherited position
    // already has the offset baked in from the main DefaultCamera.
    m_pDefaultCamera->SyncMountOffset();
    m_ActivationMountOffset = m_pDefaultCamera->GetMountCameraOffset();
}

void OrbitalCamera::CalculateOrbitOriginForStaticScene(EGameScene scene, const CameraState& previousState)
{
    // CharacterScene/LoginScene: DefaultCamera uses static hardcoded positions, not
    // character-relative. Inheriting them would place the camera far from the orbit pivot,
    // so instead we ray-cast from the static camera through its forward direction to
    // find where on the character's Z plane to place the pivot.

    vec3_t staticCameraPos, staticCameraAngle;
    if (scene == CHARACTER_SCENE)
    {
        staticCameraPos[0] = CharacterSceneCamera::POSITION_X;
        staticCameraPos[1] = CharacterSceneCamera::POSITION_Y;
        staticCameraPos[2] = CharacterSceneCamera::POSITION_Z;
        staticCameraAngle[0] = CharacterSceneCamera::ANGLE_PITCH;
        staticCameraAngle[1] = 0.0f;
        staticCameraAngle[2] = CharacterSceneCamera::ANGLE_ROLL;
    }
    else
    {
        // LoginScene/others: synthesize a camera above the character
        VectorCopy(m_Target, staticCameraPos);
        staticCameraPos[2] += NON_CHAR_STATIC_CAM_HEIGHT_OFFSET;
        staticCameraAngle[0] = NON_CHAR_STATIC_CAM_PITCH;
        staticCameraAngle[1] = 0.0f;
        staticCameraAngle[2] = 0.0f;
    }

    float Matrix[3][4];
    AngleMatrix(staticCameraAngle, Matrix);
    vec3_t forward = { Matrix[1][0], Matrix[1][1], Matrix[1][2] };
    VectorNormalize(forward);

    // Ray from static camera to the character's Z plane
    float targetZ = m_Target[2];
    float t = (targetZ - staticCameraPos[2]) / forward[2];

    vec3_t orbitOrigin;
    if (t > 0.0f && t < ORBIT_ORIGIN_MAX_T)
    {
        orbitOrigin[0] = staticCameraPos[0] + t * forward[0];
        orbitOrigin[1] = staticCameraPos[1] + t * forward[1];
        orbitOrigin[2] = targetZ;
    }
    else
    {
        // Fallback: character position
        VectorCopy(m_Target, orbitOrigin);
    }

    // Inherit the previous camera pose, but orbit around the calculated origin
    VectorCopy(previousState.Position, m_State.Position);
    VectorCopy(previousState.Angle, m_State.Angle);
    VectorCopy(orbitOrigin, m_Target);

    CAMERA_LOG("[CAM]   CharScene: StaticCam=(%.1f,%.1f,%.1f), CalcOrigin=(%.1f,%.1f,%.1f), InheritedPos=(%.1f,%.1f,%.1f)",
               staticCameraPos[0], staticCameraPos[1], staticCameraPos[2],
               orbitOrigin[0], orbitOrigin[1], orbitOrigin[2],
               m_State.Position[0], m_State.Position[1], m_State.Position[2]);
}

void OrbitalCamera::CalculateLookAtPoint(EGameScene scene, vec3_t outLookAt) const
{
    // For CharacterScene/LoginScene, m_Target already IS the orbit pivot (set above).
    if (scene != MAIN_SCENE)
    {
        VectorCopy(m_Target, outLookAt);
        return;
    }

    // MainScene: project forward from the inherited camera pose onto the Hero's Z plane
    // to find what point the camera was actually looking at.
    float Matrix[3][4];
    AngleMatrix(m_State.Angle, Matrix);
    vec3_t forward = { Matrix[1][0], Matrix[1][1], Matrix[1][2] };
    VectorNormalize(forward);

    float targetZ = m_Target[2];
    float t = (targetZ - m_State.Position[2]) / forward[2];

    if (t > 0.0f && t < LOOKAT_MAX_T)
    {
        outLookAt[0] = m_State.Position[0] + t * forward[0];
        outLookAt[1] = m_State.Position[1] + t * forward[1];
        outLookAt[2] = targetZ;
    }
    else
    {
        // Fallback: Hero position
        VectorCopy(m_Target, outLookAt);
    }
}

void OrbitalCamera::InitializeOrbitalFromCurrentState(const vec3_t lookAtPoint, const CameraState& previousState)
{
    // Save initial offset from the look-at pivot so orbital rotations preserve visual pos
    m_InitialCameraOffset[0] = m_State.Position[0] - lookAtPoint[0];
    m_InitialCameraOffset[1] = m_State.Position[1] - lookAtPoint[1];
    m_InitialCameraOffset[2] = m_State.Position[2] - lookAtPoint[2];
    m_bInitialOffsetSet = true;

    // Diagnostic distances for logging (horizontal matches DefaultCamera's distance convention)
    float dx = m_State.Position[0] - m_Target[0];
    float dy = m_State.Position[1] - m_Target[1];
    float horizontalDistance = sqrtf(dx * dx + dy * dy);
    float fullDistance = sqrtf(
        m_InitialCameraOffset[0] * m_InitialCameraOffset[0] +
        m_InitialCameraOffset[1] * m_InitialCameraOffset[1] +
        m_InitialCameraOffset[2] * m_InitialCameraOffset[2]);

    // Load radius from config so zoom persists across activations
    m_Radius = (float)GameConfig::GetInstance().GetZoom();
    m_Radius = std::clamp(m_Radius, MIN_RADIUS, MAX_RADIUS);

    // Use the look-at point as the pivot for orbital rotation
    VectorCopy(lookAtPoint, m_Target);

    CAMERA_LOG("[CAM]   LookAtPoint: (%.1f,%.1f,%.1f), Hero/Target: (%.1f,%.1f,%.1f)",
               lookAtPoint[0], lookAtPoint[1], lookAtPoint[2],
               m_Target[0], m_Target[1], m_Target[2]);
    CAMERA_LOG("[CAM]   InitialOffset (to LookAt): (%.1f,%.1f,%.1f)",
               m_InitialCameraOffset[0], m_InitialCameraOffset[1], m_InitialCameraOffset[2]);
    CAMERA_LOG("[CAM]   Cam->Hero: dx=%.1f, dy=%.1f | HorizontalDist=%.1f, FullDist=%.1f, m_Radius=%.1f, prevDist=%.1f",
               dx, dy, horizontalDistance, fullDistance, m_Radius, previousState.Distance);
}

void OrbitalCamera::SyncStateToGlobalCamera()
{
    VectorCopy(m_State.Position, g_Camera.Position);
    VectorCopy(m_State.Angle, g_Camera.Angle);
    g_Camera.FOV = m_State.FOV;
    g_Camera.ViewFar = m_Config.farPlane;
}

void OrbitalCamera::OnDeactivate()
{
    m_Input.Rotating = false;

    // Save zoom level to config.ini
    GameConfig::GetInstance().SetZoom((int)m_Radius);
    GameConfig::GetInstance().Save();
}

bool OrbitalCamera::Update()
{
    // Phase 5: Detect scene transitions and properly reset for new scene
    extern EGameScene SceneFlag;
    if (m_LastSceneFlag != (int)SceneFlag)
    {
        // Scene changed - use dedicated reset function
        m_LastSceneFlag = (int)SceneFlag;
        ResetForScene(SceneFlag);
    }

    HandleInput();

    UpdateTarget();

    // First, let DefaultCamera calculate the base camera position
    // EXCEPT on the first frame after activation - we want to preserve inherited position
    bool skipTransformThisFrame = m_bJustActivated;

    if (!m_bJustActivated)
    {
        m_pDefaultCamera->Update();
    }
    else
    {
        // First frame after activation - skip DefaultCamera update to preserve inherited state
        CAMERA_LOG("[CAM] OrbitalCamera: Skipping DefaultCamera update on first frame. Current Pos=(%.1f,%.1f,%.1f), Angle=(%.1f,%.1f,%.1f)",
                   m_State.Position[0], m_State.Position[1], m_State.Position[2],
                   m_State.Angle[0], m_State.Angle[1], m_State.Angle[2]);
        m_bJustActivated = false;  // Clear flag for next frame
    }

    // Then modify it with our orbital transformations.
    // BUT skip ComputeCameraTransform on first frame to preserve inherited position
    if (!skipTransformThisFrame)
    {
        ComputeCameraTransform();

        // Apply mount camera offset as a DELTA from a fixed activation baseline.
        //
        // m_ActivationMountOffset is captured once at OnActivate and is intentionally
        // never updated while the orbital camera is active — it is a *reference
        // point*, not a "current" value. The reasoning:
        //
        //   - At activation, the inherited pose already had m_ActivationMountOffset
        //     baked into Position[2] (DefaultCamera applied it before handoff). The
        //     spherical (BaseYaw, BasePitch, Radius) we derived from that pose
        //     therefore reproduces an inheritedZ that includes the activation offset.
        //   - GetMountCameraOffset() is the DefaultCamera's current (lerped) offset,
        //     updated by AdjustHeroHeight() each frame.
        //   - mountDelta = current - activation. While the player is in the same
        //     mount state as activation, delta == 0 and Z matches the inherited pose.
        //     When the player dismounts, current → 0 and delta → -activation, so the
        //     camera correctly drops by the lost mount lift. When they remount,
        //     delta returns to 0 and the camera returns to the activation height.
        //
        // Updating m_ActivationMountOffset on mount changes (a reasonable-sounding
        // "fix" against staleness) would make delta always 0 and freeze the camera
        // height across mount/dismount transitions — the opposite of what we want.
        float mountDelta = m_pDefaultCamera->GetMountCameraOffset() - m_ActivationMountOffset;
        if (fabsf(mountDelta) > 0.5f)
            m_State.Position[2] += mountDelta;

        // Clamp m_DeltaPitch based on what was actually achieved
        // If we tried to pitch but didn't move much, we're stuck at a constraint
        // This prevents accumulation when hitting ground/ceiling
        const float tolerance = 0.1f;
        if (std::abs(m_DeltaPitch - m_Input.LastEffectivePitch) > tolerance)
        {
            // We tried to pitch more but hit a constraint, clamp to effective value
            m_DeltaPitch = m_Input.LastEffectivePitch;
        }
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
        swprintf(debugText, 256, L"Camera: OrbitalCamera | Scene: %d", (int)SceneFlag);
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

        // Orbital-specific values
        swprintf(debugText, 256, L"Radius: %.0f | DeltaYaw: %.1f | DeltaPitch: %.1f",
                 m_Radius, m_DeltaYaw, m_DeltaPitch);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // Rendering value (what BeginOpengl actually uses)
        swprintf(debugText, 256, L"g_Camera.ViewFar (rendering): %.0f", g_Camera.ViewFar);
        g_pRenderText->RenderText(10, yPos, debugText);
    }
#endif

    return false; // Camera not locked
}

void OrbitalCamera::HandleInput()
{
    // Mouse wheel zoom
    if (MouseWheel != 0)
    {
        const float zoomSpeed = 50.0f;
        m_Radius -= MouseWheel * zoomSpeed;
        m_Radius = std::clamp(m_Radius, MIN_RADIUS, MAX_RADIUS);
        MouseWheel = 0;

        // FIX: Update terrain culling range based on zoom level and pitch
        UpdateConfigForView();
        UpdateFrustum();
    }

    // Middle mouse drag rotation - only rotate when button is held AND mouse moves
    // Check if button is currently pressed (not just was pressed)
    bool buttonHeld = MouseMButton;

    if (buttonHeld)
    {
        if (!m_Input.Rotating)
        {
            // Button just pressed - record starting position
            m_Input.Rotating = true;
            m_Input.LastMouseX = MouseX;
            m_Input.LastMouseY = MouseY;
        }
        else
        {
            // Button held - only rotate if mouse actually moved
            int deltaX = MouseX - m_Input.LastMouseX;
            int deltaY = MouseY - m_Input.LastMouseY;

            // Only apply rotation if there's actual mouse movement
            if (deltaX != 0 || deltaY != 0)
            {
                const float sensitivity = 0.5f;
                m_DeltaYaw += deltaX * sensitivity;
                m_DeltaPitch -= deltaY * sensitivity;  // Inverted Y

                // Clamp pitch delta to prevent extreme angles
                float totalPitch = m_BasePitch + m_DeltaPitch;
                if (totalPitch < MIN_PITCH)
                    m_DeltaPitch = MIN_PITCH - m_BasePitch;
                else if (totalPitch > MAX_PITCH)
                    m_DeltaPitch = MAX_PITCH - m_BasePitch;

                // Update last position
                m_Input.LastMouseX = MouseX;
                m_Input.LastMouseY = MouseY;
            }
        }
    }
    else
    {
        // Button released - reset rotation state
        m_Input.Rotating = false;
    }
}

bool OrbitalCamera::IsHeroValid() const
{
    extern CHARACTER* Hero;
    return (Hero != nullptr && Hero->Object.Live);
}

void OrbitalCamera::GetTargetPosition(vec3_t outTarget) const
{
    extern CHARACTER* Hero;
    if (IsHeroValid())
    {
        // Priority 3: Hero exists - use Hero position as target
        VectorCopy(Hero->Object.Position, outTarget);
    }
    else
    {
        // Priority 4: In CharacterScene, use CharactersClient[0] as target
        extern EGameScene SceneFlag;
        if (SceneFlag == CHARACTER_SCENE)
        {
            extern CHARACTER* CharactersClient;
            if (CharactersClient && CharactersClient[0].Object.Live)
            {
                VectorCopy(CharactersClient[0].Object.Position, outTarget);
                return;
            }
        }

        // Priority 5 (Fallback): Use current camera position as pivot
        // This allows orbital camera to work in LoginScene when no character exists
        VectorCopy(m_State.Position, outTarget);
    }
}

void OrbitalCamera::UpdateTarget()
{
    // Phase 5: Use safe GetTargetPosition() instead of direct Hero access
    GetTargetPosition(m_Target);
}

void OrbitalCamera::ComputeCameraTransform()
{
    // At this point, DefaultCamera has calculated m_State.Position, m_State.Angle, and m_State.ViewFar
    // Save the ViewFar that DefaultCamera calculated (we'll multiply it later)
    float defaultCameraViewFar = m_State.ViewFar;

    // Calculate camera offset relative to character
    float relativeX = m_State.Position[0] - m_Target[0];
    float relativeY = m_State.Position[1] - m_Target[1];
    float relativeZ = m_State.Position[2] - m_Target[2];

    // Save initial offset on first frame
    if (!m_bInitialOffsetSet)
    {
        m_InitialCameraOffset[0] = relativeX;
        m_InitialCameraOffset[1] = relativeY;
        m_InitialCameraOffset[2] = relativeZ;
        m_bInitialOffsetSet = true;
    }

    // Calculate zoom scale (100 = 1.0x normal)
    float zoomScale = m_Radius / DEFAULT_RADIUS;

    // Apply zoom to initial offset
    float scaledX = m_InitialCameraOffset[0] * zoomScale;
    float scaledY = m_InitialCameraOffset[1] * zoomScale;
    float scaledZ = m_InitialCameraOffset[2] * zoomScale;

    // Apply horizontal rotation around Z axis
    float angleRad = m_DeltaYaw * (M_PI / 180.0f);
    float rotatedX = scaledX * cosf(angleRad) - scaledY * sinf(angleRad);
    float rotatedY = scaledX * sinf(angleRad) + scaledY * cosf(angleRad);
    float rotatedZ = scaledZ;

    // Apply vertical pitch rotation
    float horizontalDist = sqrtf(rotatedX * rotatedX + rotatedY * rotatedY);
    float totalDist = sqrtf(rotatedX * rotatedX + rotatedY * rotatedY + rotatedZ * rotatedZ);
    float currentElevation = atan2f(rotatedZ, horizontalDist);

    // Calculate additional height offset when zooming in close
    // This makes the camera focus on character's head/upper body instead of feet
    // Increased from 80 to 150 to center character on screen (was showing feet at center)
    const float CAMERA_HEIGHT_OFFSET = 150.0f;
    const float maxHeightOffset = 270.0f;  // Increased proportionally

    float zoomFactor = 0.0f;
    if (m_Radius < DEFAULT_RADIUS)  // Only add extra offset when zooming in from default
    {
        zoomFactor = (DEFAULT_RADIUS - m_Radius) / (DEFAULT_RADIUS - MIN_RADIUS);
        zoomFactor = std::max(0.0f, std::min(1.0f, zoomFactor));  // Clamp to 0-1
    }

    // Calculate the target offset for current zoom level
    float targetHeightOffset = CAMERA_HEIGHT_OFFSET + ((maxHeightOffset - CAMERA_HEIGHT_OFFSET) * zoomFactor);
    float additionalOffset = targetHeightOffset - CAMERA_HEIGHT_OFFSET;

    // Check if requested pitch would hit ground or ceiling constraints
    float pitchRad = m_DeltaPitch * (M_PI / 180.0f);
    float requestedElevation = currentElevation + pitchRad;
    float requestedVerticalDist = totalDist * sinf(requestedElevation);
    float requestedZ = m_Target[2] + requestedVerticalDist + additionalOffset;

    const float minCameraHeight = 50.0f;
    float finalElevation = requestedElevation;
    float effectivePitchDelta = m_DeltaPitch;

    // Clamp to prevent looking too far down (below minimum height)
    if (requestedZ < m_Target[2] + minCameraHeight)
    {
        // Calculate maximum downward elevation that keeps us above minimum height
        float maxRelativeZ = minCameraHeight - additionalOffset;
        finalElevation = asinf(std::clamp(maxRelativeZ / totalDist, -1.0f, 1.0f));

        // Calculate effective pitch delta
        effectivePitchDelta = (finalElevation - currentElevation) * (180.0f / M_PI);
    }

    // Clamp to prevent looking straight down at character (limit to ~80-90 degrees)
    // When camera is directly above, elevation approaches 90 degrees (PI/2)
    // We want to stop before reaching that to keep character visible
    const float maxElevationRad = 90.0f * (M_PI / 180.0f);  // ~90 degrees maximum
    if (finalElevation > maxElevationRad)
    {
        finalElevation = maxElevationRad;
        effectivePitchDelta = (finalElevation - currentElevation) * (180.0f / M_PI);
    }

    // Apply the final (possibly clamped) elevation
    float newHorizontalDist = totalDist * cosf(finalElevation);
    float newVerticalDist = totalDist * sinf(finalElevation);

    float xyScale = (horizontalDist > 0.001f) ? (newHorizontalDist / horizontalDist) : 1.0f;
    rotatedX *= xyScale;
    rotatedY *= xyScale;
    rotatedZ = newVerticalDist;

    // Set final camera position with additional height offset
    m_State.Position[0] = m_Target[0] + rotatedX;
    m_State.Position[1] = m_Target[1] + rotatedY;
    m_State.Position[2] = m_Target[2] + rotatedZ + additionalOffset;

    // Modify camera angles - use effective pitch that accounts for ground collision
    m_State.Angle[0] = m_State.Angle[0] + effectivePitchDelta;
    m_State.Angle[1] = 0.0f;
    m_State.Angle[2] = m_State.Angle[2] - m_DeltaYaw;  // Inverted like CustomCamera3D

    // Store the effective pitch for next frame's constraint checking
    m_Input.LastEffectivePitch = effectivePitchDelta;

    // Update transformation matrix
    m_State.UpdateMatrix();

    // Update distance properties
    m_State.Distance = m_Radius;
    m_State.DistanceTarget = m_Radius;

    // Sync ViewFar and fog distances with current zoom level
    UpdateConfigForView();
    UpdateFrustum();
}

// ========== Phase 1: Configuration & Frustum Management ==========

void OrbitalCamera::SetConfig(const CameraConfig& config)
{
    m_Config = config;
    UpdateFrustum();
}

void OrbitalCamera::UpdateConfigForView()
{
    float zoomRatio = m_Radius / DEFAULT_RADIUS;
    CameraConfig baseConfig = CameraConfig::ForMainSceneOrbitalCamera();

    float zoomScale;
    if (zoomRatio >= 1.0f)
    {
        // Zooming OUT: scale up gently
        zoomScale = 1.0f + (zoomRatio - 1.0f) * 0.33f;
    }
    else
    {
        // Zooming IN: reduce proportionally
        zoomScale = 0.5f + (zoomRatio * 0.5f);
    }

    // Scale far plane with zoom only (not pitch).
    // The 2D frustum projection already adapts its shape to the actual camera pitch.
    m_Config.farPlane = baseConfig.farPlane * zoomScale;

    // Sync g_Camera.ViewFar so BeginOpengl uses the updated far plane for
    // projection and percentage-based fog (fog = ViewFar * 80%/90%).
    m_State.ViewFar = m_Config.farPlane;
    g_Camera.ViewFar = m_Config.farPlane;

    // Keep cull ranges in lockstep with farPlane. Previously objectCullRange was
    // set once at config load and never updated, so zooming the camera widened
    // the terrain hull but left object culling at the static initial value.
    m_Config.terrainCullRange = m_Config.farPlane * RENDER_DISTANCE_MULTIPLIER;
    m_Config.objectCullRange  = m_Config.farPlane;
}

void OrbitalCamera::UpdateFrustum()
{
#ifdef _EDITOR
    // Scaffolding: the Orbital override is wired but has no sliders at present,
    // so this is a no-op until a future pass adds Orbital-specific tuning.
    if (DevEditor_IsCameraOverrideEnabled("Orbital"))
        DevEditor_ApplyCameraOverride("Orbital", &m_Config);
#endif

    // Derive forward and up vectors from m_State.Angle to match the OpenGL view setup.
    // BeginOpengl() applies: glRotatef(A1, Y) * glRotatef(A0, X) * glRotatef(A2, Z)
    // The GL rotation R = Ry(A1) * Rx(A0) * Rz(A2) transforms world→view.
    // Forward (view -Z) in world space = R^T * (0,0,-1), Up (view +Y) = R^T * (0,1,0).
    vec3_t forward, up;

    float a0 = m_State.Angle[0] * (Q_PI / 180.0f);  // X rotation (pitch)
    float a1 = m_State.Angle[1] * (Q_PI / 180.0f);  // Y rotation (usually 0)
    float a2 = m_State.Angle[2] * (Q_PI / 180.0f);  // Z rotation (yaw)
    float sa0 = sinf(a0), ca0 = cosf(a0);
    float sa1 = sinf(a1), ca1 = cosf(a1);
    float sa2 = sinf(a2), ca2 = cosf(a2);

    // forward = -(third row of R) = -R[2][*]
    forward[0] = sa1 * ca2 - ca1 * sa0 * sa2;
    forward[1] = -(sa1 * sa2 + ca1 * sa0 * ca2);
    forward[2] = -(ca1 * ca0);

    // up = second row of R = R[1][*]
    up[0] = ca0 * sa2;
    up[1] = ca0 * ca2;
    up[2] = -sa0;

    // Build frustum using the same viewport aspect ratio that BeginOpengl() will use.
    // BeginOpengl() scales reference coords (640×480) to actual window pixels.
    // MainScene uses height=432 (480-48 for UI bar), CharacterScene uses 430, etc.
    extern unsigned int WindowWidth;
    extern unsigned int WindowHeight;
    extern EGameScene SceneFlag;
    extern int GetScreenWidth();

    int refWidth = REFERENCE_WIDTH;
    int refHeight = REFERENCE_HEIGHT;
    if (SceneFlag == MAIN_SCENE)
    {
        refWidth = GetScreenWidth();
        refHeight = g_Camera.TopViewEnable ? REFERENCE_HEIGHT : (REFERENCE_HEIGHT - 48);
    }
    else if (SceneFlag == CHARACTER_SCENE || SceneFlag == LOG_IN_SCENE)
    {
        refHeight = 430;
    }
    float viewportWidth = (float)(refWidth * WindowWidth) / (float)REFERENCE_WIDTH;
    float viewportHeight = (float)(refHeight * WindowHeight) / (float)REFERENCE_HEIGHT;
    float aspectRatio = viewportWidth / viewportHeight;

    // Override was already applied at the top of this function (if enabled).
    float effectiveFarPlane = m_Config.farPlane;
    float effectiveTerrainCullRange = m_Config.terrainCullRange;

    // Use g_Camera.FOV (= m_State.FOV) directly — same vFov that BeginOpengl passes
    // to gluPerspective. Combined with viewport aspect, this matches the GL projection exactly.
    float vFov = m_State.FOV;

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

    // Inject Orbital's actual 2D terrain-cull hull into m_Frustum so FreeFly
    // spectator (which reads Frustum::Get2DX/Y) sees exactly what Orbital's
    // active-render path uses. Matches ZzzLodTerrain::CreateFrustrum2D logic:
    //   - override on  → user's trapezoid (farDist/farWidth/nearDist/nearWidth)
    //   - override off → natural pyramid + 400 half-width at near edge
    {
        float hullFarDist, hullFarHalfW, hullNearDist, hullNearHalfW;
#ifdef _EDITOR
        float ovFarD = 0, ovFarW = 0, ovNearD = 0, ovNearW = 0;
        if (DevEditor_GetOrbitalHullTrapezoid(&ovFarD, &ovFarW, &ovNearD, &ovNearW))
        {
            hullFarDist   = ovFarD;
            hullFarHalfW  = ovFarW * 0.5f;
            hullNearDist  = ovNearD;
            hullNearHalfW = ovNearW * 0.5f;
        }
        else
#endif
        {
            const float tanHalfV = tanf(vFov * 0.5f * Q_PI / 180.0f);
            hullFarDist   = effectiveTerrainCullRange;
            hullFarHalfW  = tanHalfV * effectiveTerrainCullRange * aspectRatio;
            hullNearDist  = 0.0f;
            hullNearHalfW = NATURAL_NEAR_HALF_WIDTH;
        }
        const float hullFarHalfH  = hullFarHalfW  / aspectRatio;
        const float hullNearHalfH = hullNearHalfW / aspectRatio;

        // Build the SAME rotation matrix BeginOpengl produces (Ry(A1) * Rx(A0) * Rz(A2))
        // so the transform matches g_Camera.Matrix exactly. Using AngleMatrix directly
        // would apply a different convention and the hull would counter-rotate in spectator.
        float mat[3][4];
        {
            const float a0 = m_State.Angle[0] * (Q_PI / 180.0f);
            const float a1 = m_State.Angle[1] * (Q_PI / 180.0f);
            const float a2 = m_State.Angle[2] * (Q_PI / 180.0f);
            const float s0 = sinf(a0), c0 = cosf(a0);
            const float s1 = sinf(a1), c1 = cosf(a1);
            const float s2 = sinf(a2), c2 = cosf(a2);
            mat[0][0] =  c1 * c2 + s1 * s0 * s2;
            mat[0][1] = -c1 * s2 + s1 * s0 * c2;
            mat[0][2] =  s1 * c0;
            mat[1][0] =  c0 * s2;
            mat[1][1] =  c0 * c2;
            mat[1][2] = -s0;
            mat[2][0] = -s1 * c2 + c1 * s0 * s2;
            mat[2][1] =  s1 * s2 + c1 * s0 * c2;
            mat[2][2] =  c1 * c0;
            mat[0][3] = mat[1][3] = mat[2][3] = 0.0f;
        }

        vec3_t viewPts[8];
        Vector(-hullNearHalfW,  hullNearHalfH, -hullNearDist, viewPts[0]);
        Vector( hullNearHalfW,  hullNearHalfH, -hullNearDist, viewPts[1]);
        Vector( hullNearHalfW, -hullNearHalfH, -hullNearDist, viewPts[2]);
        Vector(-hullNearHalfW, -hullNearHalfH, -hullNearDist, viewPts[3]);
        Vector(-hullFarHalfW,   hullFarHalfH,  -hullFarDist,  viewPts[4]);
        Vector( hullFarHalfW,   hullFarHalfH,  -hullFarDist,  viewPts[5]);
        Vector( hullFarHalfW,  -hullFarHalfH,  -hullFarDist,  viewPts[6]);
        Vector(-hullFarHalfW,  -hullFarHalfH,  -hullFarDist,  viewPts[7]);

        float hx[8], hy[8];
        for (int i = 0; i < 8; i++)
        {
            vec3_t world;
            VectorIRotate(viewPts[i], mat, world);
            VectorAdd(world, m_State.Position, world);
            hx[i] = world[0] * 0.01f;
            hy[i] = world[1] * 0.01f;
        }
        m_Frustum.SetCustom2DHull(hx, hy, 8);
    }
}
