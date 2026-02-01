#pragma once

#include "ICamera.h"
#include "CameraState.h"
#include "DefaultCamera.h"
#include <memory>

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

    // Phase 5: Scene-specific reset
    void ResetForScene(EGameScene scene);

    // Public accessors for UI
    float GetRadius() const { return m_Radius; }
    float GetTotalYaw() const { return m_BaseYaw + m_DeltaYaw; }
    float GetTotalPitch() const { return m_BasePitch + m_DeltaPitch; }

    // Phase 5: Public accessor for debug display
    void GetTargetPosition(vec3_t outTarget) const;

    // Phase 1: Configuration & Frustum Management
    const CameraConfig& GetConfig() const override { return m_Config; }
    void SetConfig(const CameraConfig& config) override;
    const Frustum& GetFrustum() const override { return m_Frustum; }

    bool ShouldCullObject(const vec3_t position, float radius) const override
    {
        return !m_Frustum.TestSphere(position, radius);
    }

    bool ShouldCullTerrain(int tileX, int tileY) const override
    {
        // Use 3D frustum sphere test instead of 2D ground projection
        // Terrain tiles are indexed in world space / TERRAIN_SCALE (100)
        // Convert tile coords to world space center point
        vec3_t tileCenter;
        tileCenter[0] = (tileX + 0.5f) * 100.0f;  // TERRAIN_SCALE = 100
        tileCenter[1] = (tileY + 0.5f) * 100.0f;
        tileCenter[2] = 0.0f;  // Z will be ignored for terrain height variance

        // Use generous radius to account for terrain height variance
        return !m_Frustum.TestSphere(tileCenter, 100.0f);
    }

    bool ShouldCullObject2D(float x, float y, float radius) const override
    {
        // Use 3D frustum sphere test instead of 2D ground projection
        vec3_t position;
        position[0] = x;
        position[1] = y;
        position[2] = 0.0f;  // Objects on ground
        return !m_Frustum.TestSphere(position, radius);
    }

private:
    CameraState& m_State;
    std::unique_ptr<DefaultCamera> m_pDefaultCamera;  // Internal default camera for base calculation

    // Phase 1: Configuration and frustum
    CameraConfig m_Config;
    Frustum m_Frustum;

    // Orbital parameters
    vec3_t m_InitialCameraOffset;  // Saved offset from character on first frame
    bool m_bInitialOffsetSet;      // Has initial offset been captured?
    vec3_t m_Target;              // Orbit center (character position)
    float m_BaseYaw;              // Initial yaw when activated
    float m_BasePitch;            // Initial pitch when activated
    float m_DeltaYaw;             // User rotation delta from base
    float m_DeltaPitch;           // User pitch delta from base
    float m_Radius;               // Distance from target

    // Phase 5: Scene transition tracking
    int m_LastSceneFlag;          // Track scene changes to reset target
    bool m_bJustActivated;        // Skip DefaultCamera update on first frame after activation

    // Constraints
    static constexpr float MIN_PITCH = -80.0f;  // Look down limit
    static constexpr float MAX_PITCH = 80.0f;   // Look up limit
    static constexpr float MIN_RADIUS = 200.0f;
    static constexpr float MAX_RADIUS = 2000.0f;
    static constexpr float DEFAULT_RADIUS = 1100.0f;  // +300 to match Default Camera (3 scroll ticks further out)

    // Input state
    bool m_bRotating;             // Middle mouse button held?
    int m_LastMouseX;
    int m_LastMouseY;
    float m_LastEffectivePitch;   // Last effective pitch applied (after constraints)

    // Helper methods
    void HandleInput();
    void UpdateTarget();
    void ComputeCameraTransform();
    void UpdateFrustum();  // Phase 1: Rebuild frustum from current state
    void UpdateConfigForZoom();  // Phase 1: Adjust config based on zoom level

    // Phase 5: WASD+QE free camera movement
    void HandleFreeCameraMovement();
    bool m_bFreeCameraMode = false;

    // Phase 5: Hero validity check (private)
    bool IsHeroValid() const;
};
