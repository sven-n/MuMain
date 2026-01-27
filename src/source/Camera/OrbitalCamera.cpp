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

    // Start from the previous camera's angles
    m_Pitch = previousState.Angle[0];
    m_Yaw = previousState.Angle[2];

    // Clamp to valid range
    m_Pitch = std::clamp(m_Pitch, MIN_PITCH, MAX_PITCH);
    m_Yaw = fmodf(m_Yaw + 360.0f, 360.0f);
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

    // Middle mouse drag rotation - only rotate when button is held AND mouse moves
    bool buttonHeld = (MouseMButton || MouseMButtonPush);
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
    // Use the same approach as DefaultCamera:
    // 1. Create offset vector pointing backward (0, -Distance, 0)
    // 2. Rotate by camera angles
    // 3. Add to target position

    vec3_t offset, rotatedOffset;
    float matrix[3][4];

    // Set camera angles
    m_State.Angle[0] = m_Pitch;
    m_State.Angle[1] = 0.0f;
    m_State.Angle[2] = m_Yaw;

    // Create offset vector pointing backward
    Vector(0.f, -m_Radius, 0.f, offset);

    // Rotate offset by camera angles (same as DefaultCamera)
    AngleMatrix(m_State.Angle, matrix);
    VectorIRotate(offset, matrix, rotatedOffset);

    // Camera position = target + rotated offset
    VectorAdd(m_Target, rotatedOffset, m_State.Position);

    // Set Z position like DefaultCamera does: target Z + distance adjustment
    // Note: We use target[2] - 80 because UpdateTarget already adds 80
    m_State.Position[2] = (m_Target[2] - 80.0f) + m_Radius - 150.f;

    // Update transformation matrix
    m_State.UpdateMatrix();

    // Set other camera properties
    m_State.Distance = m_Radius;
    m_State.DistanceTarget = m_Radius;
    m_State.ViewFar = 3000.0f;
    m_State.FOV = 55.0f;
}
