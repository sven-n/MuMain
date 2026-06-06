#pragma once

#ifdef _EDITOR

#include "ICamera.h"
#include "CameraState.h"
#include "CameraConfig.h"
#include "Frustum.h"

/**
 * @brief Free-fly camera for editor mode (spectator tool)
 *
 * Features:
 * - WASD movement
 * - Q/E for up/down
 * - Right mouse drag for look
 * - Shift for speed boost
 * - Own frustum for correct culling from FreeFly viewpoint
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
    void InheritFOV(float hFov);
    void SnapToPosition(const vec3_t pos, float yaw, float pitch);

    // Configuration & Frustum Management
    const CameraConfig& GetConfig() const override { return m_Config; }
    void SetConfig(const CameraConfig& config) override;
    const Frustum& GetFrustum() const override { return m_Frustum; }

    bool ShouldCullObject(const vec3_t position, float radius) const override
    {
        return !m_Frustum.TestSphere(position, radius);
    }

    bool ShouldCullTerrain(int tileX, int tileY) const override
    {
        return !m_Frustum.TestPoint2D(tileX + 0.5f, tileY + 0.5f, -40.0f);
    }

    bool ShouldCullObject2D(float x, float y, float radius) const override
    {
        vec3_t position;
        position[0] = x;
        position[1] = y;
        position[2] = 0.0f;
        return !m_Frustum.TestSphere(position, radius);
    }

private:
    CameraState& m_State;

    // Configuration and frustum
    CameraConfig m_Config;
    Frustum m_Frustum;

    // Transform
    vec3_t m_Position;
    float m_Yaw;                  // Horizontal rotation (degrees)
    float m_Pitch;                // Vertical rotation (degrees)

    // Movement
    static constexpr float BASE_SPEED = 125.0f;
    static constexpr float SPRINT_MULTIPLIER = 3.0f;

    // Input state
    bool m_bLooking;
    int m_LastMouseX;
    int m_LastMouseY;

    // State preservation across toggles
    bool m_bHasSavedState = false;

    // Constraints
    // Engine convention: Angle[0]=-90 = horizontal, more negative = looking up
    static constexpr float MIN_PITCH = -160.0f;  // Looking steeply upward
    static constexpr float MAX_PITCH = -20.0f;   // Looking steeply downward

    // Helper methods
    void HandleInput();
    void HandleMovement();
    void ReadMovementInput(float& outForward, float& outStrafe, float& outVertical);
    void ComputeCameraTransform();
    void UpdateFrustum();
};

#endif // _EDITOR
