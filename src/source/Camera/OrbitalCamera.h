#pragma once

#include "ICamera.h"
#include "CameraState.h"

/**
 * @brief Orbital camera - spherical coordinates around character
 *
 * Features:
 * - Middle mouse drag to rotate (yaw + pitch)
 * - Mouse wheel to zoom (radius)
 * - Maintains character focus
 * - Available in all builds
 */
class OrbitalCamera : public ICamera
{
public:
    OrbitalCamera(CameraState& state);
    ~OrbitalCamera() override = default;

    // ICamera interface
    bool Update() override;
    void Reset() override;
    void OnActivate(const CameraState& previousState) override;
    void OnDeactivate() override;
    const char* GetName() const override { return "Orbital"; }

private:
    CameraState& m_State;

    // Orbital parameters
    vec3_t m_Target;              // Orbit center (character position)
    float m_Yaw;                  // Horizontal rotation (degrees, 0-360)
    float m_Pitch;                // Vertical rotation (degrees, clamped)
    float m_Radius;               // Distance from target

    // Constraints
    static constexpr float MIN_PITCH = -80.0f;  // Look down limit
    static constexpr float MAX_PITCH = 80.0f;   // Look up limit
    static constexpr float MIN_RADIUS = 200.0f;
    static constexpr float MAX_RADIUS = 2000.0f;
    static constexpr float DEFAULT_RADIUS = 800.0f;

    // Input state
    bool m_bRotating;             // Middle mouse button held?
    int m_LastMouseX;
    int m_LastMouseY;

    // Helper methods
    void HandleInput();
    void UpdateTarget();
    void ComputeCameraTransform();
};
