// OrbitalCamera.cpp - Spherical coordinate orbital camera implementation

#include "stdafx.h"
#include "OrbitalCamera.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzCharacter.h"
#include <cmath>

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
    , m_Config(CameraConfig::ForGameplay())  // Phase 1: Initialize with gameplay config
    , m_bInitialOffsetSet(false)
    , m_BaseYaw(0.0f)
    , m_BasePitch(0.0f)
    , m_DeltaYaw(0.0f)
    , m_DeltaPitch(0.0f)
    , m_Radius(DEFAULT_RADIUS)
    , m_LastSceneFlag(-1)  // Phase 5: Initialize to invalid scene
    , m_bRotating(false)
    , m_LastMouseX(0)
    , m_LastMouseY(0)
    , m_LastEffectivePitch(0.0f)
{
    IdentityVector3D(m_Target);
    IdentityVector3D(m_InitialCameraOffset);
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
    m_pDefaultCamera->Reset();
}

void OrbitalCamera::OnActivate(const CameraState& previousState)
{
    // Inherit radius from previous camera distance
    m_Radius = previousState.Distance;
    m_Radius = std::clamp(m_Radius, MIN_RADIUS, MAX_RADIUS);

    // Reset deltas - user hasn't rotated yet
    m_DeltaYaw = 0.0f;
    m_DeltaPitch = 0.0f;

    // Reset initial offset - will be captured on first frame
    m_bInitialOffsetSet = false;

    // Phase 5 fix: Update target immediately on activation to get correct scene context
    UpdateTarget();

    // Phase 3 fix: Initialize frustum immediately on activation
    // Without this, first frame(s) have uninitialized frustum and everything gets culled
    UpdateFrustum();
}

void OrbitalCamera::OnDeactivate()
{
    m_bRotating = false;
}

bool OrbitalCamera::Update()
{
    // Phase 5: Detect scene transitions and force target recalculation
    extern EGameScene SceneFlag;
    if (m_LastSceneFlag != (int)SceneFlag)
    {
        // Scene changed - reset ALL orbital state to prevent cross-scene contamination
        m_LastSceneFlag = (int)SceneFlag;
        m_bInitialOffsetSet = false;  // Force recapture of camera offset

        // Reset rotation deltas - don't carry rotation from previous scene
        m_DeltaYaw = 0.0f;
        m_DeltaPitch = 0.0f;

        // Reset radius to default for new scene
        m_Radius = DEFAULT_RADIUS;

        // Update target immediately with new scene context
        UpdateTarget();

        // Force DefaultCamera to recalculate as well
        m_pDefaultCamera->Reset();
    }

    HandleInput();
    UpdateTarget();

    // First, let DefaultCamera calculate the base camera position
    m_pDefaultCamera->Update();

    // Then modify it with our orbital transformations
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
        // Priority 4 (Fallback): Use current camera position as pivot
        // This allows orbital camera to work in LoginScene/CharacterScene
        // without crashing when Hero is invalid
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
    const float CAMERA_HEIGHT_OFFSET = 80.0f;
    const float maxHeightOffset = 200.0f;

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

    // Calculate dynamic ViewFar based on zoom distance
    // When zoomed IN (small radius): see LESS (smaller ViewFar)
    // When zoomed OUT (large radius): see MORE (larger ViewFar)
    // Use CustomCamera3D's aggressive scaling for zoom OUT only

    float baseViewFar = defaultCameraViewFar;
    float zoomRatio = m_Radius / DEFAULT_RADIUS;  // 0.25 (min) to 2.5 (max), 1.0 at default

    float viewMultiplier;
    if (zoomRatio >= 1.0f)
    {
        // Zooming OUT: minimal scaling for natural feel
        // At max zoom (2.5x radius), ViewFar = base * 1.3x
        viewMultiplier = 1.0f + (zoomRatio - 1.0f) * 1.0f;  // Range: 1.0x to 1.3x
    }
    else
    {
        // Zooming IN: reduce ViewFar proportionally
        // At radius=200 (0.25x): ViewFar = base * 0.5
        // At radius=800 (1.0x): ViewFar = base * 1.0
        viewMultiplier = 0.5f + (zoomRatio * 0.5f);  // Range: 0.5x to 1.0x
    }

    m_State.ViewFar = baseViewFar * viewMultiplier;

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
    CameraConfig baseConfig = CameraConfig::ForGameplay();

    if (zoomRatio >= 1.0f)
    {
        // Zooming OUT: gentle scaling
        float scale = 1.0f + (zoomRatio - 1.0f) * 1.0f;  // 1.0x to 2.5x
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

    // Use ViewFar for 3D culling distance (varies 2000-3700 based on map/zoom)
    // The Frustum will internally use terrainCullRange for 2D ground projection
    m_Frustum.BuildFromCamera(
        m_State.Position,
        forward,
        up,
        m_Config.fov,
        aspectRatio,
        m_Config.nearPlane,
        m_State.ViewFar  // Use dynamic ViewFar for proper distance culling
    );
}
