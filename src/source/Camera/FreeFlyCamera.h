#pragma once

#ifdef _EDITOR

#include "ICamera.h"
#include "CameraState.h"

/**
 * @brief Free-fly camera for editor mode
 *
 * Features:
 * - WASD movement
 * - Q/E for up/down
 * - Right mouse drag for look
 * - Shift for speed boost
 * - Editor only (_EDITOR flag)
 */
class FreeFlyCamera : public ICamera
{
public:
    FreeFlyCamera(CameraState& state);
    ~FreeFlyCamera() override = default;

    // ICamera interface
    bool Update() override;
    void Reset() override;
    void OnActivate(const CameraState& previousState) override;
    void OnDeactivate() override;
    const char* GetName() const override { return "FreeFly"; }

private:
    CameraState& m_State;

    // Transform
    vec3_t m_Position;
    float m_Yaw;                  // Horizontal rotation (degrees)
    float m_Pitch;                // Vertical rotation (degrees)

    // Movement
    static constexpr float BASE_SPEED = 500.0f;
    static constexpr float SPRINT_MULTIPLIER = 3.0f;

    // Input state
    bool m_bLooking;
    int m_LastMouseX;
    int m_LastMouseY;

    // Constraints
    static constexpr float MIN_PITCH = -89.0f;
    static constexpr float MAX_PITCH = 89.0f;

    // Helper methods
    void HandleInput();
    void HandleMovement();
    void ComputeCameraTransform();
};

#endif // _EDITOR
