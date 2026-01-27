// OrbitalCamera.cpp - Spherical coordinate orbital camera implementation

#include "stdafx.h"
#include "OrbitalCamera.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzCharacter.h"

// External globals
extern int MouseWheel;
extern bool MouseMButton;
extern bool MouseMButtonPush;
extern int MouseX;
extern int MouseY;
// Hero is declared in ZzzCharacter.h as CHARACTER*

OrbitalCamera::OrbitalCamera(CameraState& state)
    : m_State(state)
    , m_Yaw(0.0f)
    , m_Pitch(-30.0f)
    , m_Radius(DEFAULT_RADIUS)
    , m_bRotating(false)
    , m_LastMouseX(0)
    , m_LastMouseY(0)
{
    IdentityVector3D(m_Target);
}

void OrbitalCamera::Reset()
{
    m_Yaw = 0.0f;
    m_Pitch = -30.0f;
    m_Radius = DEFAULT_RADIUS;
    m_bRotating = false;
}

void OrbitalCamera::OnActivate(const CameraState& previousState)
{
    // Inherit radius from previous camera distance
    m_Radius = previousState.Distance;
    m_Radius = std::clamp(m_Radius, MIN_RADIUS, MAX_RADIUS);

    // Try to calculate initial yaw/pitch from previous camera angle
    m_Yaw = -previousState.Angle[2];  // Convert to yaw
    m_Pitch = previousState.Angle[0]; // Use pitch as-is
    m_Pitch = std::clamp(m_Pitch, MIN_PITCH, MAX_PITCH);
}

void OrbitalCamera::OnDeactivate()
{
    m_bRotating = false;
}

bool OrbitalCamera::Update()
{
    HandleInput();
    UpdateTarget();
    ComputeCameraTransform();
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

    // Middle mouse drag rotation
    bool buttonHeld = (MouseMButton || MouseMButtonPush);
    if (buttonHeld)
    {
        if (!m_bRotating)
        {
            m_bRotating = true;
            m_LastMouseX = MouseX;
            m_LastMouseY = MouseY;
        }
        else
        {
            int deltaX = MouseX - m_LastMouseX;
            int deltaY = MouseY - m_LastMouseY;

            const float sensitivity = 0.5f;
            m_Yaw += deltaX * sensitivity;
            m_Pitch -= deltaY * sensitivity;  // Inverted Y

            // Normalize yaw to [0, 360)
            m_Yaw = fmodf(m_Yaw + 360.0f, 360.0f);

            // Clamp pitch
            m_Pitch = std::clamp(m_Pitch, MIN_PITCH, MAX_PITCH);

            m_LastMouseX = MouseX;
            m_LastMouseY = MouseY;
        }
    }
    else
    {
        m_bRotating = false;
    }
}

void OrbitalCamera::UpdateTarget()
{
    // Target is character position with height offset
    if (Hero)
    {
        m_Target[0] = Hero->Object.Position[0];
        m_Target[1] = Hero->Object.Position[1];
        m_Target[2] = Hero->Object.Position[2] + 80.0f; // Height offset
    }
}

void OrbitalCamera::ComputeCameraTransform()
{
    // Convert spherical to Cartesian coordinates
    float yawRad = m_Yaw * (Q_PI / 180.0f);
    float pitchRad = m_Pitch * (Q_PI / 180.0f);

    // Calculate position relative to target
    float horizontalDist = m_Radius * cosf(pitchRad);
    float x = horizontalDist * sinf(yawRad);
    float y = -horizontalDist * cosf(yawRad);
    float z = m_Radius * sinf(pitchRad);

    // Set camera position
    m_State.Position[0] = m_Target[0] + x;
    m_State.Position[1] = m_Target[1] + y;
    m_State.Position[2] = m_Target[2] + z;

    // Set camera angles to look at target
    m_State.Angle[0] = m_Pitch;
    m_State.Angle[1] = 0.0f;
    m_State.Angle[2] = -m_Yaw;

    // Set other state
    m_State.Distance = m_Radius;
    m_State.DistanceTarget = m_Radius;
    m_State.ViewFar = 3000.0f;
    m_State.FOV = 55.0f;
}
