// FreeFlyCamera.cpp - Free-fly camera for editor mode (spectator tool)

#include "stdafx.h"
#include "Core/Input/KeyState.h"

#ifdef _EDITOR

#include "FreeFlyCamera.h"
#include "CameraManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"

// External globals
extern bool MouseRButton;
extern int MouseX;
extern int MouseY;
extern float FPS_ANIMATION_FACTOR;
extern CameraState g_Camera;

namespace
{
    // Degrees the camera rotates per pixel of mouse delta while right-clicking.
    constexpr float MOUSE_LOOK_SENSITIVITY = 0.3f;

    // Full rotation in degrees, used to wrap yaw into [0, 360).
    constexpr float FULL_ROTATION_DEG = 360.0f;

    // Compute the camera forward vector from yaw/pitch in degrees.
    // Matches the OpenGL rendering direction: glRotatef(pitch, 1,0,0) then glRotatef(yaw, 0,0,1).
    inline void ComputeForwardVector(float yawDeg, float pitchDeg, vec3_t outForward)
    {
        float yawRad = yawDeg * (Q_PI / 180.0f);
        float pitchRad = pitchDeg * (Q_PI / 180.0f);
        outForward[0] = -sinf(yawRad) * sinf(pitchRad);
        outForward[1] = -cosf(yawRad) * sinf(pitchRad);
        outForward[2] = -cosf(pitchRad);
    }

    // Compute the right vector (perpendicular to forward in the XY plane).
    inline void ComputeRightVectorXY(float yawDeg, float& outRightX, float& outRightY)
    {
        float yawRad = yawDeg * (Q_PI / 180.0f);
        outRightX = cosf(yawRad);
        outRightY = -sinf(yawRad);
    }
}

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
    m_bTopDownPan = false;
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
    m_bTopDownPan = false;
}

void FreeFlyCamera::SnapTopDown()
{
    // Map is TERRAIN_SIZE tiles of TERRAIN_SCALE units; centre it in view.
    const float mapCentre = TERRAIN_SIZE * 0.5f * TERRAIN_SCALE;  // 128 * 100
    m_Position[0] = mapCentre;
    m_Position[1] = mapCentre;
    m_Position[2] = 24000.0f;  // high enough for a 90-deg FOV to frame the map
    m_Yaw = 0.0f;
    m_Pitch = MAX_PITCH;       // as close to straight-down as the camera allows

    // Push far clip, culling and fog well past the map's far corners so nothing
    // is culled or fogged out from up here.
    m_Config.farPlane = 60000.0f;
    m_Config.terrainCullRange = 60000.0f;
    m_Config.objectCullRange = 60000.0f;
    m_Config.fogStart = 55000.0f;
    m_Config.fogEnd = 60000.0f;

    m_bTopDownPan = true;   // arrows pan the map, mouse wheel zooms

    UpdateFrustum();
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

            m_Yaw += deltaX * MOUSE_LOOK_SENSITIVITY;
            m_Pitch += deltaY * MOUSE_LOOK_SENSITIVITY;

            // Normalize yaw into [0, 360)
            m_Yaw = fmodf(m_Yaw + FULL_ROTATION_DEG, FULL_ROTATION_DEG);

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

void FreeFlyCamera::ReadMovementInput(float& outForward, float& outStrafe, float& outVertical)
{
    // Arrow keys for XY movement (avoids WASD conflict with game UI)
    outForward = 0.0f;
    outStrafe = 0.0f;
    outVertical = 0.0f;

    if (Core::Input::IsKeyDown(VK_UP))    outForward += 1.0f;
    if (Core::Input::IsKeyDown(VK_DOWN))  outForward -= 1.0f;
    if (Core::Input::IsKeyDown(VK_LEFT))  outStrafe -= 1.0f;
    if (Core::Input::IsKeyDown(VK_RIGHT)) outStrafe += 1.0f;

    // PageUp/PageDown for vertical movement
    if (Core::Input::IsKeyDown(VK_PRIOR)) outVertical += 1.0f;
    if (Core::Input::IsKeyDown(VK_NEXT)) outVertical -= 1.0f;
}

void FreeFlyCamera::HandleMovement()
{
    // Top-down minimap mode has its own scheme: all four arrows pan across the map
    // (looking straight down, "fly forward" would just dive into the terrain), the
    // mouse wheel zooms, and PageUp/PageDown still nudge the height.
    if (m_bTopDownPan)
    {
        HandleTopDownMovement();
        return;
    }

    float forward, strafe, vertical;
    ReadMovementInput(forward, strafe, vertical);

    float inputLength = sqrtf(forward * forward + strafe * strafe + vertical * vertical);
    if (inputLength == 0.0f)
        return;

    // Normalize input so diagonal movement isn't faster
    forward /= inputLength;
    strafe /= inputLength;
    vertical /= inputLength;

    float speed = BASE_SPEED;
    if (Core::Input::IsKeyDown(VK_SHIFT))
        speed *= SPRINT_MULTIPLIER;
    speed *= FPS_ANIMATION_FACTOR;

    vec3_t forwardVec;
    ComputeForwardVector(m_Yaw, m_Pitch, forwardVec);

    float rightX, rightY;
    ComputeRightVectorXY(m_Yaw, rightX, rightY);

    // Compose world-space movement
    m_Position[0] += (forwardVec[0] * forward + rightX * strafe) * speed;
    m_Position[1] += (forwardVec[1] * forward + rightY * strafe) * speed;
    m_Position[2] += (forwardVec[2] * forward + vertical) * speed;
}

void FreeFlyCamera::ZoomTopDown(float wheelTicks)
{
    // Wheel up (positive) zooms in = lower the camera. Driven by the editor from
    // ImGui's wheel, because the game HUD consumes the global MouseWheel earlier in
    // the frame than the camera update runs.
    if (!m_bTopDownPan || wheelTicks == 0.0f)
        return;
    m_Position[2] = std::clamp(m_Position[2] - wheelTicks * TOPDOWN_WHEEL_STEP,
                               TOPDOWN_MIN_HEIGHT, TOPDOWN_MAX_HEIGHT);
}

bool FreeFlyCamera::ComputeMapScreenRect(int& outX, int& outY, int& outSize) const
{
    extern unsigned int WindowWidth, WindowHeight;
    const float winW = static_cast<float>(WindowWidth);
    const float winH = static_cast<float>(WindowHeight);
    if (winW < 1.0f || winH < 1.0f)
        return false;

    // Camera basis.
    vec3_t F;
    ComputeForwardVector(m_Yaw, m_Pitch, F);
    VectorNormalize(F);
    vec3_t worldUp = { 0.0f, 0.0f, 1.0f };
    vec3_t R;
    CrossProduct(F, worldUp, R);
    VectorNormalize(R);
    vec3_t U;
    CrossProduct(R, F, U);
    VectorNormalize(U);

    const float aspect = winW / winH;
    const float hFovRad = m_Config.hFov * (Q_PI / 180.0f);
    const float vFovRad = HFovToVFov(m_Config.hFov, aspect) * (Q_PI / 180.0f);
    const float tanH = tanf(hFovRad * 0.5f);
    const float tanV = tanf(vFovRad * 0.5f);

    // The map spans [0, extent] in X and Y at z=0.
    const float extent = TERRAIN_SIZE * TERRAIN_SCALE;

    // Project a world point (z=0) to a bottom-origin screen pixel.
    auto project = [&](float wx, float wy, float& sx, float& sy) -> bool
    {
        vec3_t d = { wx - m_Position[0], wy - m_Position[1], -m_Position[2] };
        const float vz = DotProduct(d, F);
        if (vz <= 1.0f)
            return false;
        sx = ((DotProduct(d, R) / vz) / tanH * 0.5f + 0.5f) * winW;
        sy = ((DotProduct(d, U) / vz) / tanV * 0.5f + 0.5f) * winH;
        return true;
    };

    // Centre the crop on the projected MAP CENTRE (not the bounding box of the
    // corners): under the small camera tilt the projection is slightly asymmetric,
    // and centring on the true centre is what keeps the in-game position marker
    // aligned. Size it from the horizontal half-extent, which is the tilt-free axis
    // (the tilt is along Y only), so the square scale is correct.
    float cx, cy, ex, ey;
    if (!project(extent * 0.5f, extent * 0.5f, cx, cy))
        return false;                                   // map centre behind camera
    if (!project(extent, extent * 0.5f, ex, ey))
        return false;                                   // right-edge midpoint
    const float radius = fabsf(ex - cx);
    float size = 2.0f * radius;
    size = std::min(size, std::min(winW, winH));   // never read outside the window

    int x = static_cast<int>(cx - size * 0.5f);
    int y = static_cast<int>(cy - size * 0.5f);
    int s = static_cast<int>(size);
    if (s < 8)
        return false;
    x = std::clamp(x, 0, static_cast<int>(winW) - s);
    y = std::clamp(y, 0, static_cast<int>(winH) - s);

    outX = x; outY = y; outSize = s;
    return true;
}

void FreeFlyCamera::HandleTopDownMovement()
{
    float forward, strafe, vertical;
    ReadMovementInput(forward, strafe, vertical);

    float inputLength = sqrtf(forward * forward + strafe * strafe + vertical * vertical);
    if (inputLength == 0.0f)
        return;

    forward /= inputLength;
    strafe /= inputLength;
    vertical /= inputLength;

    float speed = BASE_SPEED;
    if (Core::Input::IsKeyDown(VK_SHIFT))
        speed *= SPRINT_MULTIPLIER;
    speed *= FPS_ANIMATION_FACTOR;

    // Pan in the ground plane. "forward" here is the screen-up direction projected
    // onto the map (perpendicular to the strafe/right vector), so Up/Down and
    // Left/Right all slide the map and rotate together with the view yaw.
    float yawRad = m_Yaw * (Q_PI / 180.0f);
    float panFwdX = sinf(yawRad);
    float panFwdY = cosf(yawRad);
    float rightX, rightY;
    ComputeRightVectorXY(m_Yaw, rightX, rightY);

    m_Position[0] += (panFwdX * forward + rightX * strafe) * speed;
    m_Position[1] += (panFwdY * forward + rightY * strafe) * speed;
    // PageUp/PageDown remain a fine keyboard zoom.
    m_Position[2] = std::clamp(m_Position[2] + vertical * speed,
                               TOPDOWN_MIN_HEIGHT, TOPDOWN_MAX_HEIGHT);
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
    vec3_t forward;
    ComputeForwardVector(m_Yaw, m_Pitch, forward);
    VectorNormalize(forward);

    // Orthonormal basis: build right from forward × world up, then real up from right × forward
    vec3_t worldUp = { 0.0f, 0.0f, 1.0f };
    vec3_t right;
    CrossProduct(forward, worldUp, right);
    VectorNormalize(right);
    CrossProduct(right, forward, worldUp);
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
