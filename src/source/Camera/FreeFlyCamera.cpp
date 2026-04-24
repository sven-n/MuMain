// FreeFlyCamera.cpp - Free-fly camera for editor mode (spectator tool)

#include "stdafx.h"

#ifdef _EDITOR

#include "FreeFlyCamera.h"
#include "CameraManager.h"
#include "../ZzzOpenglUtil.h"

// External globals
extern bool MouseRButton;
extern int MouseX;
extern int MouseY;
extern float FPS_ANIMATION_FACTOR;
extern CameraState g_Camera;

FreeFlyCamera::FreeFlyCamera(CameraState& state)
    : m_State(state)
    , m_Yaw(0.0f)
    , m_Pitch(0.0f)
    , m_bLooking(false)
    , m_LastMouseX(0)
    , m_LastMouseY(0)
{
    IdentityVector3D(m_Position);

    // Extended config for editor spectator mode
    // Large far plane ensures terrain within the spectated camera's view is always
    // rendered, even when FreeFly is far away from the spectated camera position
    m_Config.hFov = 90.0f;
    m_Config.nearPlane = 10.0f;
    m_Config.farPlane = 25000.0f;
    m_Config.terrainCullRange = 25000.0f;
    m_Config.objectCullRange = 25000.0f;
    m_Config.fogStart = 20000.0f;
    m_Config.fogEnd = 25000.0f;
}

void FreeFlyCamera::Reset()
{
    IdentityVector3D(m_Position);
    m_Yaw = 0.0f;
    m_Pitch = 0.0f;
    m_bLooking = false;
}

void FreeFlyCamera::OnActivate(const CameraState& previousState)
{
    if (m_bHasSavedState)
    {
        // Resume from where we left off — m_Position/m_Yaw/m_Pitch are still valid
        return;
    }

    // First activation: start from exact same viewpoint as the previous camera
    VectorCopy(previousState.Position, m_Position);

    // Inherit angles directly — no negation so the view direction matches exactly
    m_Yaw = previousState.Angle[2];
    m_Pitch = previousState.Angle[0];
    m_Pitch = std::clamp(m_Pitch, MIN_PITCH, MAX_PITCH);
}

void FreeFlyCamera::InheritFOV(float hFov)
{
    m_Config.hFov = hFov;
}

void FreeFlyCamera::SnapToPosition(const vec3_t pos, float yaw, float pitch)
{
    VectorCopy(pos, m_Position);
    m_Yaw = yaw;
    m_Pitch = std::clamp(pitch, MIN_PITCH, MAX_PITCH);
}

void FreeFlyCamera::OnDeactivate()
{
    m_bLooking = false;
    m_bHasSavedState = true;  // Position/yaw/pitch preserved in member vars
}

void FreeFlyCamera::SetConfig(const CameraConfig& config)
{
    m_Config = config;
    UpdateFrustum();
}

bool FreeFlyCamera::Update()
{
    HandleInput();
    HandleMovement();
    ComputeCameraTransform();

    // Sync to g_Camera so BeginOpengl uses FreeFly's viewpoint
    VectorCopy(m_State.Position, g_Camera.Position);
    VectorCopy(m_State.Angle, g_Camera.Angle);
    g_Camera.FOV = m_State.FOV;
    g_Camera.ViewFar = m_State.ViewFar;

    return false;
}

void FreeFlyCamera::HandleInput()
{
    // Right mouse look
    if (MouseRButton)
    {
        if (!m_bLooking)
        {
            m_bLooking = true;
            m_LastMouseX = MouseX;
            m_LastMouseY = MouseY;
        }
        else
        {
            int deltaX = MouseX - m_LastMouseX;
            int deltaY = MouseY - m_LastMouseY;

            const float sensitivity = 0.3f;
            m_Yaw += deltaX * sensitivity;
            m_Pitch += deltaY * sensitivity;

            // Normalize yaw
            m_Yaw = fmodf(m_Yaw + 360.0f, 360.0f);

            // Clamp pitch
            m_Pitch = std::clamp(m_Pitch, MIN_PITCH, MAX_PITCH);

            m_LastMouseX = MouseX;
            m_LastMouseY = MouseY;
        }
    }
    else
    {
        m_bLooking = false;
    }
}

void FreeFlyCamera::HandleMovement()
{
    // Arrow keys for XY movement (avoids WASD conflict with game UI)
    // Forward arrow moves in camera look direction (projected to XY plane)
    float forward = 0.0f, strafe = 0.0f, vertical = 0.0f;

    if (GetAsyncKeyState(VK_UP) & 0x8000)
        forward += 1.0f;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
        forward -= 1.0f;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        strafe -= 1.0f;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        strafe += 1.0f;

    // PageUp/PageDown for vertical movement
    if (GetAsyncKeyState(VK_PRIOR) & 0x8000)  // Page Up
        vertical += 1.0f;
    if (GetAsyncKeyState(VK_NEXT) & 0x8000)   // Page Down
        vertical -= 1.0f;

    float inputLength = sqrtf(forward * forward + strafe * strafe + vertical * vertical);
    if (inputLength > 0.0f)
    {
        // Normalize input
        forward /= inputLength;
        strafe /= inputLength;
        vertical /= inputLength;

        // Apply speed
        float speed = BASE_SPEED;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            speed *= SPRINT_MULTIPLIER;

        speed *= FPS_ANIMATION_FACTOR;

        // Forward vector derived from OpenGL view matrix (matches rendering direction)
        // BeginOpengl applies: glRotatef(pitch, 1,0,0) then glRotatef(yaw, 0,0,1)
        // Camera forward = inverse rotation applied to eye-space -Z
        float yawRad = m_Yaw * (Q_PI / 180.0f);
        float pitchRad = m_Pitch * (Q_PI / 180.0f);

        float forwardX = -sinf(yawRad) * sinf(pitchRad);
        float forwardY = -cosf(yawRad) * sinf(pitchRad);
        float forwardZ = -cosf(pitchRad);

        // Right vector (perpendicular to forward in XY plane)
        float rightX = cosf(yawRad);
        float rightY = -sinf(yawRad);

        // Compose world-space movement
        m_Position[0] += (forwardX * forward + rightX * strafe) * speed;
        m_Position[1] += (forwardY * forward + rightY * strafe) * speed;
        m_Position[2] += (forwardZ * forward + vertical) * speed;
    }
}

void FreeFlyCamera::ComputeCameraTransform()
{
    // Copy position
    VectorCopy(m_Position, m_State.Position);

    // For FreeFly camera, we set angles that make the camera look in the direction
    // specified by yaw and pitch, similar to a first-person camera
    m_State.Angle[0] = m_Pitch;
    m_State.Angle[1] = 0.0f;
    m_State.Angle[2] = m_Yaw;

    // Update camera matrix from angles
    m_State.UpdateMatrix();

    // Set other state
    m_State.Distance = 0.0f;  // No target
    m_State.DistanceTarget = 0.0f;
    m_State.ViewFar = m_Config.farPlane;
    {
        extern unsigned int WindowWidth, WindowHeight;
        float aspect = (float)WindowWidth / (float)WindowHeight;
        m_State.FOV = HFovToVFov(m_Config.hFov, aspect);
    }

    // Build frustum for correct culling from FreeFly viewpoint
    UpdateFrustum();
}

void FreeFlyCamera::UpdateFrustum()
{
    // Calculate forward vector from yaw and pitch
    float yawRad = m_Yaw * (Q_PI / 180.0f);
    float pitchRad = m_Pitch * (Q_PI / 180.0f);

    // Forward vector matches OpenGL rendering direction
    vec3_t forward;
    forward[0] = -sinf(yawRad) * sinf(pitchRad);
    forward[1] = -cosf(yawRad) * sinf(pitchRad);
    forward[2] = -cosf(pitchRad);
    VectorNormalize(forward);

    // World up
    vec3_t worldUp = { 0.0f, 0.0f, 1.0f };

    // Calculate right vector
    vec3_t right, forwardTemp, upTemp;
    VectorCopy(forward, forwardTemp);
    VectorCopy(worldUp, upTemp);
    CrossProduct(forwardTemp, upTemp, right);
    VectorNormalize(right);

    // Calculate actual up vector
    VectorCopy(right, forwardTemp);
    VectorCopy(forward, upTemp);
    CrossProduct(forwardTemp, upTemp, worldUp);
    VectorNormalize(worldUp);

    extern unsigned int WindowWidth, WindowHeight;
    float aspectRatio = (float)WindowWidth / (float)WindowHeight;
    float vFov = HFovToVFov(m_Config.hFov, aspectRatio);

    m_Frustum.BuildFromCamera(
        m_State.Position,
        forward,
        worldUp,
        vFov,
        aspectRatio,
        m_Config.nearPlane,
        m_Config.farPlane,
        m_Config.terrainCullRange
    );
}

#endif // _EDITOR
