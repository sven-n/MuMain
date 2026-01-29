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

    // Public accessors for UI
    float GetRadius() const { return m_Radius; }
    float GetTotalYaw() const { return m_BaseYaw + m_DeltaYaw; }
    float GetTotalPitch() const { return m_BasePitch + m_DeltaPitch; }

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
        int minX, minY, maxX, maxY;
        m_Frustum.GetTerrainTileBounds(&minX, &minY, &maxX, &maxY);
        return tileX < minX || tileX > maxX || tileY < minY || tileY > maxY;
    }

    bool ShouldCullObject2D(float x, float y, float radius) const override
    {
        return !m_Frustum.Test2D(x, y, radius);
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
    float m_LastEffectivePitch;   // Last effective pitch applied (after constraints)

    // Helper methods
    void HandleInput();
    void UpdateTarget();
    void ComputeCameraTransform();
    void UpdateFrustum();  // Phase 1: Rebuild frustum from current state
    void UpdateConfigForZoom();  // Phase 1: Adjust config based on zoom level

    // Phase 5: Hero validity check and target position getter
    bool IsHeroValid() const;
    void GetTargetPosition(vec3_t outTarget) const;
};
