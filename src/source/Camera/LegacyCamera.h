#pragma once

#include "ICamera.h"
#include "CameraState.h"
#include "CameraConfig.h"
#include "Frustum.h"

/**
 * @brief Legacy camera — exact replica of main branch MoveMainCamera()
 *
 * Replicates the original camera code from the main branch before the
 * camera system was introduced. Uses simple distance-based culling
 * instead of 3D frustum sphere tests. For A/B performance testing.
 */
class LegacyCamera : public ICamera
{
public:
    LegacyCamera(CameraState& state);
    ~LegacyCamera() override = default;

    // ICamera interface
    bool Update() override;
    void Reset() override;
    void OnActivate(const CameraState& previousState) override;
    void OnDeactivate() override;
    const char* GetName() const override { return "Legacy"; }

    // Configuration & Frustum (minimal — no expensive frustum building)
    const CameraConfig& GetConfig() const override { return m_Config; }
    void SetConfig(const CameraConfig& config) override { m_Config = config; }
    const Frustum& GetFrustum() const override { return m_Frustum; }

    // Culling — cheap distance-based checks (no 6-plane sphere test)
    bool ShouldCullObject(const vec3_t position, float radius) const override
    {
        float dx = position[0] - m_State.Position[0];
        float dy = position[1] - m_State.Position[1];
        float distSq = dx * dx + dy * dy;
        float maxDist = m_State.ViewFar + radius;
        return distSq > maxDist * maxDist;
    }

    bool ShouldCullTerrain(int tileX, int tileY) const override
    {
        float worldX = (tileX + 0.5f) * 100.0f;
        float worldY = (tileY + 0.5f) * 100.0f;
        float dx = worldX - m_State.Position[0];
        float dy = worldY - m_State.Position[1];
        float distSq = dx * dx + dy * dy;
        float maxDist = m_State.ViewFar + 200.0f;
        return distSq > maxDist * maxDist;
    }

    bool ShouldCullObject2D(float x, float y, float radius) const override
    {
        float dx = x - m_State.Position[0];
        float dy = y - m_State.Position[1];
        float distSq = dx * dx + dy * dy;
        float maxDist = m_State.ViewFar + radius;
        return distSq > maxDist * maxDist;
    }

private:
    CameraState& m_State;
    CameraConfig m_Config;
    Frustum m_Frustum;  // Unused — kept to satisfy interface

    // Original main branch camera functions (direct ports)
    float CalculateCameraViewFar();
    void AdjustHeroHeight();
    void CalculateCameraPosition();
    void SetCameraAngle();
    void UpdateCustomCameraDistance();
    void UpdateCameraDistance();
    void SetCameraFOV();
};
