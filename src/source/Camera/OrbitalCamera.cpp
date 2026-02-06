// OrbitalCamera.cpp - Spherical coordinate orbital camera implementation

#include "stdafx.h"
#include "OrbitalCamera.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzCharacter.h"
#include <cmath>

#include "UIControls.h"
#include "UI/Console/MuEditorConsoleUI.h"

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
// DevEditor config override functions (global scope required for extern "C")
extern "C" bool DevEditor_IsConfigOverrideEnabled();
extern "C" void DevEditor_GetCameraConfig(float* outFOV, float* outNearPlane, float* outFarPlane, float* outTerrainCullRange);
extern "C" bool DevEditor_IsCustomOriginEnabled();
extern "C" void DevEditor_GetCustomOrigin(float* outX, float* outY, float* outZ);
extern "C" bool DevEditor_IsTargetCharacterEnabled();
extern "C" int DevEditor_GetTargetCharacterIndex();
#endif

OrbitalCamera::OrbitalCamera(CameraState& state)
    : m_State(state)
    , m_pDefaultCamera(std::make_unique<DefaultCamera>(state))
    , m_Config(CameraConfig::ForMainSceneOrbitalCamera())  // OrbitalCamera defaults to extended visibility
    , m_bInitialOffsetSet(false)
    , m_BaseYaw(0.0f)
    , m_BasePitch(0.0f)
    , m_DeltaYaw(0.0f)
    , m_DeltaPitch(0.0f)
    , m_Radius(DEFAULT_RADIUS)
    , m_LastSceneFlag(-1)  // Phase 5: Initialize to invalid scene
    , m_bJustActivated(false)  // Initialize activation flag
    , m_bRotating(false)
    , m_LastMouseX(0)
    , m_LastMouseY(0)
    , m_LastEffectivePitch(0.0f)
{
    IdentityVector3D(m_Target);
    IdentityVector3D(m_InitialCameraOffset);

    // Ensure ViewFar matches the config
    m_State.ViewFar = m_Config.farPlane;
}

void OrbitalCamera::Reset()
{
    m_BaseYaw = 0.0f;
    m_BasePitch = 0.0f;
    m_DeltaYaw = 0.0f;
    m_DeltaPitch = 0.0f;
    m_Radius = DEFAULT_RADIUS;
    m_bRotating = false;
    m_bInitialOffsetSet = false;
    m_bFreeCameraMode = false;
    // Phase 5: Reset scene tracking to force config reload
    m_LastSceneFlag = -1;
    m_pDefaultCamera->Reset();
}

void OrbitalCamera::ResetForScene(EGameScene scene)
{
    // Phase 5: Proper scene-specific reset using switch statement
    switch (scene)
    {
        case CHARACTER_SCENE:
        {
            // Load CharacterScene config
            m_Config = CameraConfig::ForCharacterScene();

            // FORCE all CharacterScene values DIRECTLY to state
            m_State.ViewFar = m_Config.farPlane;      // 4100
            m_State.FOV = 30.0f;                       // OpenGL perspective FOV
            m_State.TopViewEnable = false;

            // Reset orbital parameters for new scene
            m_DeltaYaw = 0.0f;
            m_DeltaPitch = 0.0f;
            m_Radius = DEFAULT_RADIUS;
            m_bInitialOffsetSet = false;
            break;
        }

        case MAIN_SCENE:
        {
            // FIX Issue #1: Use ForMainScene to match DefaultCamera's config
            m_Config = CameraConfig::ForMainSceneDefaultCamera();

            // Set MainScene defaults
            m_State.ViewFar = m_Config.farPlane;      // 1700
            m_State.FOV = 30.0f;
            m_State.TopViewEnable = false;

            // Reset orbital parameters for new scene
            m_DeltaYaw = 0.0f;
            m_DeltaPitch = 0.0f;
            m_Radius = DEFAULT_RADIUS;
            m_bInitialOffsetSet = false;
            break;
        }

        case LOG_IN_SCENE:
        case SERVER_LIST_SCENE:
        case WEBZEN_SCENE:
        case LOADING_SCENE:
        default:
        {
            // Use gameplay config as default
            m_Config = CameraConfig::ForMainSceneOrbitalCamera();
            m_State.ViewFar = m_Config.farPlane;
            m_State.FOV = 30.0f;
            m_State.TopViewEnable = false;
            break;
        }
    }

    // Update target immediately with new scene context
    UpdateTarget();

    // Force DefaultCamera to recalculate as well
    m_pDefaultCamera->ResetForScene(scene);
}

void OrbitalCamera::OnActivate(const CameraState& previousState)
{
    // Phase 5: When activating, ensure camera is configured for current scene
    extern EGameScene SceneFlag;

#ifdef _EDITOR
    // DEBUG: Log activation to Editor console
    char debugMsg[512];
    sprintf_s(debugMsg, "[CAM] OrbitalCamera::OnActivate - Scene=%d, PrevPos=(%.1f,%.1f,%.1f), PrevAngle=(%.1f,%.1f,%.1f), PrevDist=%.0f",
              (int)SceneFlag,
              previousState.Position[0], previousState.Position[1], previousState.Position[2],
              previousState.Angle[0], previousState.Angle[1], previousState.Angle[2],
              previousState.Distance);
    g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif

    // Step 1: Load config (but don't call ResetForScene yet - it calls UpdateTarget with wrong position)
    switch (SceneFlag)
    {
        case CHARACTER_SCENE:
            m_Config = CameraConfig::ForCharacterScene();
#ifdef _EDITOR
            g_MuEditorConsoleUI.LogEditor("[CAM]   Loaded ForCharacterScene config");
#endif
            break;
        case MAIN_SCENE:
            // FIX Issue #1: Use ForMainScene to match DefaultCamera's config
            m_Config = CameraConfig::ForMainSceneDefaultCamera();
#ifdef _EDITOR
            g_MuEditorConsoleUI.LogEditor("[CAM]   Loaded ForMainScene config for MAIN_SCENE");
#endif
            break;
        default:
            m_Config = CameraConfig::ForMainSceneOrbitalCamera();
#ifdef _EDITOR
            g_MuEditorConsoleUI.LogEditor("[CAM]   Loaded ForGameplay config (default)");
#endif
            break;
    }

#ifdef _EDITOR
    // DEBUG: Log loaded config
    sprintf_s(debugMsg, "[CAM]   Config: Far=%.0f, FOV=%.1f, TerrainCull=%.0f",
              m_Config.farPlane, m_Config.fov, m_Config.terrainCullRange);
    g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif

    // Step 2: Apply config to state
    m_State.ViewFar = m_Config.farPlane;
    m_State.FOV = 30.0f;
    m_State.TopViewEnable = false;

    // Step 3: Update target FIRST to get Hero/Character position
    UpdateTarget();

    // Step 3.5: FIX - Only inherit position for MAIN_SCENE where camera follows Hero
    // For CHARACTER_SCENE and LOG_IN_SCENE, DefaultCamera uses static hardcoded positions
    // that aren't relative to the character, so inheriting them causes the camera to be far away
    if (SceneFlag == MAIN_SCENE)
    {
        // MainScene: Inherit position and angles for seamless transition
        VectorCopy(previousState.Position, m_State.Position);
        VectorCopy(previousState.Angle, m_State.Angle);

#ifdef _EDITOR
        sprintf_s(debugMsg, "[CAM]   MainScene: Inherited Pos=(%.1f,%.1f,%.1f), Angle=(%.1f,%.1f,%.1f)",
                  m_State.Position[0], m_State.Position[1], m_State.Position[2],
                  m_State.Angle[0], m_State.Angle[1], m_State.Angle[2]);
        g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif
    }
    else
    {
        // CharacterScene/LoginScene: DefaultCamera uses static positions, not character-relative
        // Calculate the orbit origin by ray-casting from DefaultCamera's static position

        // Use DefaultCamera's static position and angles from ResetForScene
        vec3_t staticCameraPos, staticCameraAngle;
        if (SceneFlag == CHARACTER_SCENE)
        {
            staticCameraPos[0] = 9758.93f;
            staticCameraPos[1] = 18913.11f;
            staticCameraPos[2] = 675.5f;
            staticCameraAngle[0] = -84.5f;
            staticCameraAngle[1] = 0.0f;
            staticCameraAngle[2] = -75.0f;
        }
        else // LOGIN_SCENE or others - use character position as fallback
        {
            VectorCopy(m_Target, staticCameraPos);
            staticCameraPos[2] += 300.0f; // Offset above
            staticCameraAngle[0] = -45.0f;
            staticCameraAngle[1] = 0.0f;
            staticCameraAngle[2] = 0.0f;
        }

        // Calculate forward vector from static camera angles
        float Matrix[3][4];
        AngleMatrix(staticCameraAngle, Matrix);
        vec3_t forward = { Matrix[1][0], Matrix[1][1], Matrix[1][2] };
        VectorNormalize(forward);

        // Ray-cast forward from static position to find orbit origin
        // Project to character's Z height
        float targetZ = m_Target[2];
        float t = (targetZ - staticCameraPos[2]) / forward[2];

        vec3_t orbitOrigin;
        if (t > 0.0f && t < 5000.0f)
        {
            orbitOrigin[0] = staticCameraPos[0] + t * forward[0];
            orbitOrigin[1] = staticCameraPos[1] + t * forward[1];
            orbitOrigin[2] = targetZ;
        }
        else
        {
            // Fallback: use character position
            VectorCopy(m_Target, orbitOrigin);
        }

        // Now inherit the static camera position but orbit around the calculated origin
        VectorCopy(previousState.Position, m_State.Position);
        VectorCopy(previousState.Angle, m_State.Angle);

        // Override the target to be the orbit origin instead of character
        VectorCopy(orbitOrigin, m_Target);

#ifdef _EDITOR
        sprintf_s(debugMsg, "[CAM]   CharScene: StaticCam=(%.1f,%.1f,%.1f), CalcOrigin=(%.1f,%.1f,%.1f), InheritedPos=(%.1f,%.1f,%.1f)",
                  staticCameraPos[0], staticCameraPos[1], staticCameraPos[2],
                  orbitOrigin[0], orbitOrigin[1], orbitOrigin[2],
                  m_State.Position[0], m_State.Position[1], m_State.Position[2]);
        g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif
    }

    // Step 4: Reset deltas - user hasn't rotated yet (m_Radius will be calculated later from actual distance)
    m_DeltaYaw = 0.0f;
    m_DeltaPitch = 0.0f;

    // Step 6: Already called UpdateTarget above

    // Step 6.5: FIX Issue #3 - For MainScene, calculate what point the inherited camera was looking at
    // For other scenes, use the Target directly since we calculated the position around it
    vec3_t lookAtPoint;

    if (SceneFlag == MAIN_SCENE)
    {
        // MainScene: Calculate forward direction from inherited angles
        float Matrix[3][4];
        AngleMatrix(m_State.Angle, Matrix);
        vec3_t forward = { Matrix[1][0], Matrix[1][1], Matrix[1][2] };
        VectorNormalize(forward);

        // Project forward from camera position to find look-at point at Hero's Z height
        // Ray: P = CameraPos + t * forward, where P.z = Hero.z
        // Solve for t: CameraPos.z + t * forward.z = Hero.z
        float targetZ = m_Target[2];
        float t = (targetZ - m_State.Position[2]) / forward[2];

        // If t is positive, we're looking down toward Hero level
        if (t > 0.0f && t < 2000.0f)  // Sanity check: within reasonable distance
        {
            lookAtPoint[0] = m_State.Position[0] + t * forward[0];
            lookAtPoint[1] = m_State.Position[1] + t * forward[1];
            lookAtPoint[2] = targetZ;
        }
        else
        {
            // Fallback: use Hero position if ray doesn't intersect
            VectorCopy(m_Target, lookAtPoint);
        }
    }
    else
    {
        // CharacterScene/LoginScene: Use Target directly since we calculated position around it
        VectorCopy(m_Target, lookAtPoint);
    }

    // Step 7: Calculate and save initial offset from look-at point (not Hero position)
    // This ensures seamless transition - the orbital camera will maintain the same visual position
    m_InitialCameraOffset[0] = m_State.Position[0] - lookAtPoint[0];
    m_InitialCameraOffset[1] = m_State.Position[1] - lookAtPoint[1];
    m_InitialCameraOffset[2] = m_State.Position[2] - lookAtPoint[2];
    m_bInitialOffsetSet = true;

    // Step 7.5: FIX Issue #3 - Calculate orbital radius from inherited position
    // DefaultCamera uses 2D horizontal distance (m_State.Distance is Y-offset ~1000-1400)
    // Calculate horizontal distance from camera to Hero (not lookAtPoint) to match DefaultCamera
    float dx = m_State.Position[0] - m_Target[0];
    float dy = m_State.Position[1] - m_Target[1];
    float horizontalDistance = sqrtf(dx * dx + dy * dy);

    float fullDistance = sqrtf(
        m_InitialCameraOffset[0] * m_InitialCameraOffset[0] +
        m_InitialCameraOffset[1] * m_InitialCameraOffset[1] +
        m_InitialCameraOffset[2] * m_InitialCameraOffset[2]
    );

    // Use horizontal distance as radius to maintain similar zoom level as DefaultCamera
    // This prevents the camera from appearing zoomed out when including vertical offset
    m_Radius = DEFAULT_RADIUS;
    m_Radius = std::clamp(m_Radius, MIN_RADIUS, MAX_RADIUS);

    // Use the calculated look-at point as the pivot for orbital rotation
    VectorCopy(lookAtPoint, m_Target);

#ifdef _EDITOR
    sprintf_s(debugMsg, "[CAM]   LookAtPoint: (%.1f,%.1f,%.1f), Hero/Target: (%.1f,%.1f,%.1f)",
              lookAtPoint[0], lookAtPoint[1], lookAtPoint[2],
              m_Target[0], m_Target[1], m_Target[2]);
    g_MuEditorConsoleUI.LogEditor(debugMsg);
    sprintf_s(debugMsg, "[CAM]   InitialOffset (to LookAt): (%.1f,%.1f,%.1f)",
              m_InitialCameraOffset[0], m_InitialCameraOffset[1], m_InitialCameraOffset[2]);
    g_MuEditorConsoleUI.LogEditor(debugMsg);
    sprintf_s(debugMsg, "[CAM]   Cam->Hero: dx=%.1f, dy=%.1f | HorizontalDist=%.1f, FullDist=%.1f, m_Radius=%.1f, prevDist=%.1f",
              dx, dy, horizontalDistance, fullDistance, m_Radius, previousState.Distance);
    g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif

    // Step 8: DO NOT call m_pDefaultCamera->ResetForScene() here!
    // It would overwrite the inherited m_State.Position we just set above
    // DefaultCamera will load its config on first Update() call anyway

    // Step 8.5: FIX Issue #3 - Update matrix from inherited angles to prevent visual jump
    // This ensures the camera's transformation matrix matches the inherited position/angles
    m_State.UpdateMatrix();

    // Step 8.6: FIX Issue #3 - Sync inherited state to g_Camera immediately
    // This ensures rendering uses the correct inherited position on the first frame
    VectorCopy(m_State.Position, g_Camera.Position);
    VectorCopy(m_State.Angle, g_Camera.Angle);
    g_Camera.FOV = m_State.FOV;

    // Apply DevEditor override if enabled
    float effectiveFarPlane = m_Config.farPlane;
#ifdef _EDITOR
    if (DevEditor_IsConfigOverrideEnabled())
    {
        float fov, nearPlane, farPlane, terrainCull;
        DevEditor_GetCameraConfig(&fov, &nearPlane, &farPlane, &terrainCull);
        effectiveFarPlane = farPlane;
    }

    // Debug: Log what we're setting g_Camera.ViewFar to
    sprintf_s(debugMsg, "[CAM]   OnActivate: Setting g_Camera.ViewFar=%.0f (from m_Config.farPlane=%.0f)",
              effectiveFarPlane, m_Config.farPlane);
    g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif
    g_Camera.ViewFar = effectiveFarPlane;

    // Step 9: Initialize frustum immediately on activation
    UpdateFrustum();

    // Step 10: Update scene tracking to prevent redundant reset in Update()
    m_LastSceneFlag = (int)SceneFlag;

    // Step 11: Mark as just activated to skip DefaultCamera update on first frame
    // This preserves the inherited position/angles for seamless transition
    m_bJustActivated = true;
}

void OrbitalCamera::OnDeactivate()
{
    m_bRotating = false;
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

    // Phase 5: Handle WASD+QE free camera movement (toggle with F8)
    if (HIBYTE(GetAsyncKeyState(VK_F8)) == 128)
    {
        static bool bF8Pressed = false;
        if (!bF8Pressed)
        {
            m_bFreeCameraMode = !m_bFreeCameraMode;
            bF8Pressed = true;
        }
    }
    else
    {
        static bool bF8Pressed = false;
        bF8Pressed = false;
    }

    if (m_bFreeCameraMode)
    {
        HandleFreeCameraMovement();
    }
    else
    {
        HandleInput();
    }

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
#ifdef _EDITOR
        char posMsg[256];
        sprintf_s(posMsg, "[CAM] OrbitalCamera: Skipping DefaultCamera update on first frame. Current Pos=(%.1f,%.1f,%.1f), Angle=(%.1f,%.1f,%.1f)",
                  m_State.Position[0], m_State.Position[1], m_State.Position[2],
                  m_State.Angle[0], m_State.Angle[1], m_State.Angle[2]);
        g_MuEditorConsoleUI.LogEditor(posMsg);
#endif
        m_bJustActivated = false;  // Clear flag for next frame
    }

    // Then modify it with our orbital transformations (unless in free camera mode)
    // BUT skip ComputeCameraTransform on first frame to preserve inherited position
    if (!m_bFreeCameraMode && !skipTransformThisFrame)
    {
        ComputeCameraTransform();

        // Clamp m_DeltaPitch based on what was actually achieved
        // If we tried to pitch but didn't move much, we're stuck at a constraint
        // This prevents accumulation when hitting ground/ceiling
        const float tolerance = 0.1f;
        if (std::abs(m_DeltaPitch - m_LastEffectivePitch) > tolerance)
        {
            // We tried to pitch more but hit a constraint, clamp to effective value
            m_DeltaPitch = m_LastEffectivePitch;
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
        swprintf(debugText, 256, L"Camera: OrbitalCamera | Scene: %d | FreeMode: %s",
                 (int)SceneFlag, m_bFreeCameraMode ? L"ON" : L"OFF");
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // State values
        swprintf(debugText, 256, L"State.ViewFar: %.0f | State.FOV: %.1f", m_State.ViewFar, m_State.FOV);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // Config values
        swprintf(debugText, 256, L"Config.farPlane: %.0f | Config.fov: %.1f", m_Config.farPlane, m_Config.fov);
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

        // FIX: Update terrain culling range based on zoom level
        UpdateConfigForZoom();
        UpdateFrustum();
    }

    // Middle mouse drag rotation - only rotate when button is held AND mouse moves
    // Check if button is currently pressed (not just was pressed)
    bool buttonHeld = MouseMButton;

    if (buttonHeld)
    {
        if (!m_bRotating)
        {
            // Button just pressed - record starting position
            m_bRotating = true;
            m_LastMouseX = MouseX;
            m_LastMouseY = MouseY;
        }
        else
        {
            // Button held - only rotate if mouse actually moved
            int deltaX = MouseX - m_LastMouseX;
            int deltaY = MouseY - m_LastMouseY;

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
                m_LastMouseX = MouseX;
                m_LastMouseY = MouseY;
            }
        }
    }
    else
    {
        // Button released - reset rotation state
        m_bRotating = false;
    }
}

bool OrbitalCamera::IsHeroValid() const
{
    extern CHARACTER* Hero;
    return (Hero != nullptr && Hero->Object.Live);
}

void OrbitalCamera::GetTargetPosition(vec3_t outTarget) const
{
#ifdef _EDITOR
    extern EGameScene SceneFlag;

    // Priority 1: Target specific character (ONLY in CharacterScene)
    if (SceneFlag == CHARACTER_SCENE && DevEditor_IsTargetCharacterEnabled())
    {
        int charIndex = DevEditor_GetTargetCharacterIndex();
        extern CHARACTER* CharactersClient;
        if (CharactersClient && CharactersClient[charIndex].Object.Live)
        {
            VectorCopy(CharactersClient[charIndex].Object.Position, outTarget);
            return;
        }
        // If character not loaded, fall through to next priority
    }

    // Priority 2: Custom origin from DevEditor
    if (DevEditor_IsCustomOriginEnabled())
    {
        // Use custom origin from DevEditor
        DevEditor_GetCustomOrigin(&outTarget[0], &outTarget[1], &outTarget[2]);
        return;
    }
#endif

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
    m_LastEffectivePitch = effectivePitchDelta;

    // Update transformation matrix
    m_State.UpdateMatrix();

    // Update distance properties
    m_State.Distance = m_Radius;
    m_State.DistanceTarget = m_Radius;

    // FIX Issue #1/#2: Use m_Config.farPlane directly, don't scale with zoom
    // The scaling was causing frustum/rendering mismatch and black background
    m_State.ViewFar = m_Config.farPlane;

    // Phase 1: Update frustum after changing ViewFar
    UpdateFrustum();
}

// ========== Phase 1: Configuration & Frustum Management ==========

void OrbitalCamera::SetConfig(const CameraConfig& config)
{
    m_Config = config;
    UpdateFrustum();
}

void OrbitalCamera::UpdateConfigForZoom()
{
    float zoomRatio = m_Radius / DEFAULT_RADIUS;  // 0.25 (min) to 2.5 (max)

    // Adjust config based on zoom level
    // Use gameplay preset as base
    CameraConfig baseConfig = CameraConfig::ForMainSceneOrbitalCamera();

    if (zoomRatio >= 1.0f)
    {
        // Zooming OUT: Scale up to 1.5x max
        // At max zoom (2.5x), farPlane = base * 1.5x
        float scale = 1.0f + (zoomRatio - 1.0f) * 0.33f;  // 1.0x to 1.5x
        m_Config.farPlane = baseConfig.farPlane * scale;
        m_Config.terrainCullRange = baseConfig.terrainCullRange * scale;
    }
    else
    {
        // Zooming IN: reduce proportionally
        float scale = 0.5f + (zoomRatio * 0.5f);  // 0.5x to 1.0x
        m_Config.farPlane = baseConfig.farPlane * scale;
        m_Config.terrainCullRange = baseConfig.terrainCullRange * scale;
    }
}

void OrbitalCamera::HandleFreeCameraMovement()
{
    // Phase 5: WASD+QE free camera movement (same as DefaultCamera)
    const float moveSpeed = 50.0f;
    const float rotSpeed = 2.0f;

    // Forward/Backward: W/S
    if (HIBYTE(GetAsyncKeyState('W')) == 128)
    {
        float angle = m_State.Angle[2] * M_PI / 180.0f;
        m_State.Position[0] += sinf(angle) * moveSpeed;
        m_State.Position[1] += cosf(angle) * moveSpeed;
    }
    if (HIBYTE(GetAsyncKeyState('S')) == 128)
    {
        float angle = m_State.Angle[2] * M_PI / 180.0f;
        m_State.Position[0] -= sinf(angle) * moveSpeed;
        m_State.Position[1] -= cosf(angle) * moveSpeed;
    }

    // Strafe Left/Right: A/D
    if (HIBYTE(GetAsyncKeyState('A')) == 128)
    {
        float angle = (m_State.Angle[2] + 90.0f) * M_PI / 180.0f;
        m_State.Position[0] += sinf(angle) * moveSpeed;
        m_State.Position[1] += cosf(angle) * moveSpeed;
    }
    if (HIBYTE(GetAsyncKeyState('D')) == 128)
    {
        float angle = (m_State.Angle[2] - 90.0f) * M_PI / 180.0f;
        m_State.Position[0] += sinf(angle) * moveSpeed;
        m_State.Position[1] += cosf(angle) * moveSpeed;
    }

    // Up/Down: E/Q
    if (HIBYTE(GetAsyncKeyState('E')) == 128)
    {
        m_State.Position[2] += moveSpeed;
    }
    if (HIBYTE(GetAsyncKeyState('Q')) == 128)
    {
        m_State.Position[2] -= moveSpeed;
    }

    // Mouse look: Arrow keys for rotation
    if (HIBYTE(GetAsyncKeyState(VK_UP)) == 128)
    {
        m_State.Angle[0] += rotSpeed;
    }
    if (HIBYTE(GetAsyncKeyState(VK_DOWN)) == 128)
    {
        m_State.Angle[0] -= rotSpeed;
    }
    if (HIBYTE(GetAsyncKeyState(VK_LEFT)) == 128)
    {
        m_State.Angle[2] += rotSpeed;
    }
    if (HIBYTE(GetAsyncKeyState(VK_RIGHT)) == 128)
    {
        m_State.Angle[2] -= rotSpeed;
    }

    // Force frustum update when in free camera mode
    UpdateFrustum();
}

void OrbitalCamera::UpdateFrustum()
{
#ifdef _EDITOR
    // Check if DevEditor is overriding config values
    if (DevEditor_IsConfigOverrideEnabled())
    {
        DevEditor_GetCameraConfig(&m_Config.fov, &m_Config.nearPlane, &m_Config.farPlane, &m_Config.terrainCullRange);
    }
#endif

    // Calculate forward and up vectors from current camera state
    vec3_t forward, up, right;

    // Calculate forward as direction from camera to target (what we're looking at)
    forward[0] = m_Target[0] - m_State.Position[0];
    forward[1] = m_Target[1] - m_State.Position[1];
    forward[2] = m_Target[2] - m_State.Position[2];
    VectorNormalize(forward);

    // Calculate right vector (perpendicular to forward in XY plane)
    vec3_t worldUp = { 0.0f, 0.0f, 1.0f };
    vec3_t forwardTemp, worldUpTemp;
    VectorCopy(forward, forwardTemp);
    VectorCopy(worldUp, worldUpTemp);
    CrossProduct(forwardTemp, worldUpTemp, right);
    VectorNormalize(right);

    // Calculate actual up vector (perpendicular to both forward and right)
    VectorCopy(right, forwardTemp);  // reuse temp
    VectorCopy(forward, worldUpTemp);  // reuse temp
    CrossProduct(forwardTemp, worldUpTemp, up);
    VectorNormalize(up);

    // Build frustum from current configuration
    // Use screen aspect ratio (assuming 4:3, can be made dynamic later)
    extern unsigned int WindowWidth;
    extern unsigned int WindowHeight;
    float aspectRatio = (float)WindowWidth / (float)WindowHeight;

    // Phase 5 FIX: ALWAYS use m_Config values for frustum culling
    // m_State.ViewFar is only for OpenGL rendering (BeginOpengl)
    // Frustum should use the clean config values, not zoom-adjusted ViewFar
    float effectiveFarPlane = m_Config.farPlane;
    float effectiveTerrainCullRange = m_Config.terrainCullRange;
#ifdef _EDITOR
    // DevEditor can still override config values if needed
    if (DevEditor_IsConfigOverrideEnabled())
    {
        DevEditor_GetCameraConfig(&m_Config.fov, &m_Config.nearPlane, &m_Config.farPlane, &m_Config.terrainCullRange);
        effectiveFarPlane = m_Config.farPlane;
        effectiveTerrainCullRange = m_Config.terrainCullRange;
    }
#endif

    m_Frustum.BuildFromCamera(
        m_State.Position,
        forward,
        up,
        m_Config.fov,
        aspectRatio,
        m_Config.nearPlane,
        effectiveFarPlane,  // Use override or dynamic ViewFar
        effectiveTerrainCullRange  // Separate terrain culling distance
    );
}
