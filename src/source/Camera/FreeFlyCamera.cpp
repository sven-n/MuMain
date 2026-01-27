// FreeFlyCamera.cpp - Free-fly camera for editor mode

#include "stdafx.h"

#ifdef _EDITOR

#include "FreeFlyCamera.h"
#include "../ZzzOpenglUtil.h"

// External globals
extern bool MouseRButton;
extern int MouseX;
extern int MouseY;
extern float FPS_ANIMATION_FACTOR;

FreeFlyCamera::FreeFlyCamera(CameraState& state)
    : m_State(state)
    , m_Yaw(0.0f)
    , m_Pitch(0.0f)
    , m_bLooking(false)
    , m_LastMouseX(0)
    , m_LastMouseY(0)
{
    IdentityVector3D(m_Position);
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
    // Start from previous camera position
    VectorCopy(previousState.Position, m_Position);

    // Inherit angles if possible
    m_Yaw = -previousState.Angle[2];
    m_Pitch = previousState.Angle[0];
    m_Pitch = std::clamp(m_Pitch, MIN_PITCH, MAX_PITCH);
}

void FreeFlyCamera::OnDeactivate()
{
    m_bLooking = false;
}

bool FreeFlyCamera::Update()
{
    HandleInput();
    HandleMovement();
    ComputeCameraTransform();
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
            m_Pitch -= deltaY * sensitivity;

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
    vec3_t movement = { 0.0f, 0.0f, 0.0f };

    // WASD movement in local space
    if (GetAsyncKeyState('W') & 0x8000)
        movement[1] += 1.0f;  // Forward
    if (GetAsyncKeyState('S') & 0x8000)
        movement[1] -= 1.0f;  // Backward
    if (GetAsyncKeyState('A') & 0x8000)
        movement[0] -= 1.0f;  // Left
    if (GetAsyncKeyState('D') & 0x8000)
        movement[0] += 1.0f;  // Right

    // Q/E for up/down
    if (GetAsyncKeyState('Q') & 0x8000)
        movement[2] -= 1.0f;  // Down
    if (GetAsyncKeyState('E') & 0x8000)
        movement[2] += 1.0f;  // Up

    // Apply movement if any input
    float length = sqrtf(movement[0]*movement[0] + movement[1]*movement[1] + movement[2]*movement[2]);
    if (length > 0.0f)
    {
        // Normalize
        movement[0] /= length;
        movement[1] /= length;
        movement[2] /= length;

        // Apply speed
        float speed = BASE_SPEED;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            speed *= SPRINT_MULTIPLIER;

        speed *= FPS_ANIMATION_FACTOR;

        // Transform to camera local space
        float yawRad = m_Yaw * (Q_PI / 180.0f);
        float cosYaw = cosf(yawRad);
        float sinYaw = sinf(yawRad);

        vec3_t localMovement;
        localMovement[0] = movement[0] * cosYaw - movement[1] * sinYaw;
        localMovement[1] = movement[0] * sinYaw + movement[1] * cosYaw;
        localMovement[2] = movement[2];  // Up/down stays in world space

        // Apply to position
        m_Position[0] += localMovement[0] * speed;
        m_Position[1] += localMovement[1] * speed;
        m_Position[2] += localMovement[2] * speed;
    }
}

void FreeFlyCamera::ComputeCameraTransform()
{
    // Copy position
    VectorCopy(m_Position, m_State.Position);

    // Set angles
    m_State.Angle[0] = m_Pitch;
    m_State.Angle[1] = 0.0f;
    m_State.Angle[2] = -m_Yaw;

    // Set other state
    m_State.Distance = 0.0f;  // No target
    m_State.DistanceTarget = 0.0f;
    m_State.ViewFar = 5000.0f;  // Extended view for editor
    m_State.FOV = 60.0f;
}

#endif // _EDITOR
