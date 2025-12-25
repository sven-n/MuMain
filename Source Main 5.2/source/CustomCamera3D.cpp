#include "stdafx.h"
#include "CustomCamera3D.h"
#include "ZzzOpenglUtil.h"
#include "ZzzScene.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Initialize static members
bool CCustomCamera3D::m_bEnabled = false;
float CCustomCamera3D::m_fZoomDistance = 100.0f;     // Default zoom (100 = 1.0x scale)
float CCustomCamera3D::m_fRotationAngle = 0.0f;      // Default rotation (0 degrees)
float CCustomCamera3D::m_fPitchAngle = 0.0f;         // Default pitch (0 degrees)
float CCustomCamera3D::m_fMinZoom = 50.0f;           // Minimum zoom (0.5x - closest)
float CCustomCamera3D::m_fMaxZoom = 200.0f;          // Maximum zoom (2.0x - farthest)
float CCustomCamera3D::m_fMinPitch = -25.0f;         // Minimum pitch (-45 degrees, looking down)
float CCustomCamera3D::m_fMaxPitch = 45.0f;          // Maximum pitch (20 degrees, looking up - limited to prevent clipping)
bool CCustomCamera3D::m_bRotating = false;
int CCustomCamera3D::m_iLastMouseX = 0;
int CCustomCamera3D::m_iLastMouseY = 0;
bool CCustomCamera3D::m_bF9KeyPressed = false;
float CCustomCamera3D::m_fInitialCameraOffset[3] = { 0.0f, 0.0f, 0.0f };

void CCustomCamera3D::Initialize()
{
}

void CCustomCamera3D::Toggle()
{
    m_bEnabled = !m_bEnabled;

    if (m_bEnabled)
    {
        // Start with 1.0x zoom (100 units), zero rotation, and zero pitch
        m_fZoomDistance = 100.0f;
        m_fRotationAngle = 0.0f;
        m_fPitchAngle = 0.0f;

        // Clear initial offset (will be set on first frame)
        m_fInitialCameraOffset[0] = 0.0f;
        m_fInitialCameraOffset[1] = 0.0f;
        m_fInitialCameraOffset[2] = 0.0f;
    }
    else
    {
        StopRotation();
    }
}

void CCustomCamera3D::Update()
{
    // Currently unused - reserved for future smooth transitions if needed
}

void CCustomCamera3D::ProcessMouseWheel(int delta)
{
    if (!m_bEnabled)
        return;

    // Input validation: clamp extreme delta values to prevent unexpected behavior
    const int maxDelta = 10;  // Reasonable limit for wheel delta per event
    delta = std::max(-maxDelta, std::min(maxDelta, delta));

    // Zoom speed: 10.0 units per wheel notch
    const float zoomSpeed = 10.0f;

    // delta > 0 = scroll up = zoom in (decrease distance)
    // delta < 0 = scroll down = zoom out (increase distance)
    m_fZoomDistance -= (float)delta * zoomSpeed;

    // Clamp zoom distance to min/max bounds
    m_fZoomDistance = std::max(m_fMinZoom, std::min(m_fMaxZoom, m_fZoomDistance));
}

void CCustomCamera3D::ProcessMouseRotation(int currentMouseX, int currentMouseY)
{
    if (!m_bEnabled || !m_bRotating)
        return;

    // Input validation: ensure mouse positions are within reasonable screen bounds
    const int maxScreenDimension = 4096;  // Reasonable maximum for modern displays
    if (currentMouseX < -maxScreenDimension || currentMouseX > maxScreenDimension ||
        currentMouseY < -maxScreenDimension || currentMouseY > maxScreenDimension)
        return;

    // Calculate mouse movement deltas
    int deltaX = currentMouseX - m_iLastMouseX;
    int deltaY = currentMouseY - m_iLastMouseY;

    // Validate deltas to prevent extreme jumps (e.g., from teleporting cursor)
    const int maxDelta = 500;  // Maximum reasonable pixel movement per frame
    if (abs(deltaX) > maxDelta || abs(deltaY) > maxDelta)
    {
        // Likely a cursor jump - just update positions without rotating
        m_iLastMouseX = currentMouseX;
        m_iLastMouseY = currentMouseY;
        return;
    }

    // Only process if there's actual movement
    if (deltaX == 0 && deltaY == 0)
        return;

    // Rotation and pitch speed: 0.5 degrees per pixel (smoother control)
    const float rotationSpeed = 0.5f;
    const float pitchSpeed = 0.5f;

    // Update horizontal rotation angle
    m_fRotationAngle += (float)deltaX * rotationSpeed;

    // Normalize angle to 0-360 range
    while (m_fRotationAngle >= 360.0f)
        m_fRotationAngle -= 360.0f;
    while (m_fRotationAngle < 0.0f)
        m_fRotationAngle += 360.0f;

    // Update vertical pitch angle (inverted Y - moving mouse up looks up)
    m_fPitchAngle -= (float)deltaY * pitchSpeed;

    // Clamp pitch to min/max bounds
    m_fPitchAngle = std::max(m_fMinPitch, std::min(m_fMaxPitch, m_fPitchAngle));

    // Update last mouse positions
    m_iLastMouseX = currentMouseX;
    m_iLastMouseY = currentMouseY;
}

void CCustomCamera3D::StartRotation(int mouseX, int mouseY)
{
    if (!m_bEnabled)
        return;

    // Input validation: ensure mouse positions are within reasonable bounds
    const int maxScreenDimension = 4096;
    if (mouseX < -maxScreenDimension || mouseX > maxScreenDimension ||
        mouseY < -maxScreenDimension || mouseY > maxScreenDimension)
        return;

    // Only initialize if we're not already rotating
    if (!m_bRotating)
    {
        m_bRotating = true;
        m_iLastMouseX = mouseX;
        m_iLastMouseY = mouseY;
    }
}

void CCustomCamera3D::StopRotation()
{
    m_bRotating = false;
}

void CCustomCamera3D::GetModifiedCameraPosition(float inPos[3], float charPos[3], float outPos[3])
{
    // Only apply 3D camera in main game scene
    if (!m_bEnabled || SceneFlag != MAIN_SCENE)
    {
        outPos[0] = inPos[0];
        outPos[1] = inPos[1];
        outPos[2] = inPos[2];
        return;
    }

    // Calculate camera offset relative to character position
    float relativeX = inPos[0] - charPos[0];
    float relativeY = inPos[1] - charPos[1];
    float relativeZ = inPos[2] - charPos[2];

    // If initial offset not set yet (first frame or not rotating), save it
    if (m_fInitialCameraOffset[0] == 0.0f && m_fInitialCameraOffset[1] == 0.0f && m_fInitialCameraOffset[2] == 0.0f)
    {
        m_fInitialCameraOffset[0] = relativeX;
        m_fInitialCameraOffset[1] = relativeY;
        m_fInitialCameraOffset[2] = relativeZ;
    }

    // Calculate zoom scale from reference distance (100 units = 1.0x)
    // 50 = 0.5x (zoom in closer), 100 = 1.0x (normal), 200 = 2.0x (zoom out further)
    float zoomScale = m_fZoomDistance / 100.0f;

    // Apply zoom to the initial offset (not the current offset)
    float scaledX = m_fInitialCameraOffset[0] * zoomScale;
    float scaledY = m_fInitialCameraOffset[1] * zoomScale;
    float scaledZ = m_fInitialCameraOffset[2] * zoomScale;

    // Apply horizontal rotation around Z axis (vertical axis) to orbit around character
    // Rotate from the initial camera offset direction
    float angleRad = m_fRotationAngle * (float)M_PI / 180.0f;

    float rotatedX = scaledX * cosf(angleRad) - scaledY * sinf(angleRad);
    float rotatedY = scaledX * sinf(angleRad) + scaledY * cosf(angleRad);
    float rotatedZ = scaledZ;

    // Apply vertical pitch rotation to orbit up/down around character
    // We need to orbit around a point above the character's feet (their center)
    // Calculate total distance from camera to character
    float totalDist = sqrtf(rotatedX * rotatedX + rotatedY * rotatedY + rotatedZ * rotatedZ);

    // Apply pitch: rotate the entire camera position vector around character's center
    // First, get the current elevation angle
    float horizontalDist = sqrtf(rotatedX * rotatedX + rotatedY * rotatedY);
    float currentElevation = atan2f(rotatedZ, horizontalDist);

    // Add pitch angle to current elevation
    float pitchRad = m_fPitchAngle * (float)M_PI / 180.0f;
    float newElevation = currentElevation + pitchRad;

    // Convert back to cartesian coordinates maintaining total distance
    float newHorizontalDist = totalDist * cosf(newElevation);
    float newVerticalDist = totalDist * sinf(newElevation);

    // Scale the XY components to maintain direction while adjusting distance
    float xyScale = (horizontalDist > 0.001f) ? (newHorizontalDist / horizontalDist) : 1.0f;
    rotatedX *= xyScale;
    rotatedY *= xyScale;
    rotatedZ = newVerticalDist;

    // Calculate additional vertical offset beyond the default camera's 40 units
    // The default camera already has 40 units baked into the initial offset
    const float maxHeightOffset = 150.0f;     // Maximum offset at min zoom in (50)

    // Calculate zoom factor
    float zoomFactor = 0.0f;
    if (m_fZoomDistance < 100.0f)  // Only add extra offset when zooming in from default
    {
        zoomFactor = (100.0f - m_fZoomDistance) / (100.0f - m_fMinZoom);
        zoomFactor = std::max(0.0f, std::min(1.0f, zoomFactor));  // Clamp to 0-1
    }

    // Calculate the target offset for current zoom level
    float targetHeightOffset = CAMERA_HEIGHT_OFFSET + ((maxHeightOffset - CAMERA_HEIGHT_OFFSET) * zoomFactor);

    // Calculate additional offset beyond what's already in the initial position
    // Since the initial position already has CAMERA_HEIGHT_OFFSET units, we only add the difference
    float additionalOffset = targetHeightOffset - CAMERA_HEIGHT_OFFSET;

    // Convert back to world coordinates with additional height offset
    outPos[0] = charPos[0] + rotatedX;
    outPos[1] = charPos[1] + rotatedY;
    outPos[2] = charPos[2] + rotatedZ + additionalOffset;

    // Ensure camera doesn't go below a minimum height above character
    // This prevents black squares (clipping issues) when looking up
    const float minCameraHeight = 50.0f;
    if (outPos[2] < charPos[2] + minCameraHeight)
    {
        outPos[2] = charPos[2] + minCameraHeight;
    }
}

void CCustomCamera3D::GetModifiedCameraAngle(float inAngle[3], float outAngle[3])
{
    // Only apply 3D camera in main game scene
    if (!m_bEnabled || SceneFlag != MAIN_SCENE)
    {
        outAngle[0] = inAngle[0];
        outAngle[1] = inAngle[1];
        outAngle[2] = inAngle[2];
        return;
    }

    // Adjust the pitch angle (CameraAngle[0]) by the pitch amount
    // When camera moves up (positive pitch), view angle must tilt down (add pitch)
    // When camera moves down (negative pitch), view angle must tilt up (subtract pitch)
    outAngle[0] = inAngle[0] + m_fPitchAngle;

    // Keep yaw the same
    outAngle[1] = inAngle[1];

    // Adjust the horizontal rotation (CameraAngle[2]) by the rotation amount
    // This makes the camera look at the character while orbiting horizontally
    // Note: Inverted rotation (-) because camera view rotates opposite to position
    outAngle[2] = inAngle[2] - m_fRotationAngle;
}

