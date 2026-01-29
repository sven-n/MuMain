#pragma once

#include "ICamera.h"
#include "CameraState.h"

/**
 * @brief Default legacy third-person follow camera
 *
 * This is the original camera implementation extracted from CameraUtility.cpp.
 * It handles all the legacy game-specific behavior including terrain flags,
 * tour mode, direction system, scene-specific positioning, etc.
 *
 * This remains the DEFAULT camera mode. In Phase 2, we'll add OrbitalCamera
 * and FreeFlyCameraEditor as alternatives.
 */
class DefaultCamera : public ICamera
{
public:
    DefaultCamera(CameraState& state);
    ~DefaultCamera() override = default;

    // ICamera interface
    bool Update() override;
    void Reset() override;
    void OnActivate(const CameraState& previousState) override;
    void OnDeactivate() override;
    const char* GetName() const override { return "Default"; }

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

    // Phase 1: Configuration and frustum
    CameraConfig m_Config;
    Frustum m_Frustum;

    // Phase 5: Hero validity check for non-MainScene contexts
    bool IsHeroValid() const;

    // These are direct copies of the static functions from CameraUtility.cpp
    // We're NOT refactoring them yet - just moving them as-is
    void CalculateCameraViewFar();
    void AdjustHeroHeight();
    void CalculateCameraPosition();
    void SetCameraAngle();
    void UpdateCustomCameraDistance();
    void UpdateCameraDistance();
    void SetCameraFOV();
    void UpdateFrustum();  // Phase 1: Rebuild frustum from current state

#ifdef ENABLE_EDIT2
    void HandleEditorMode();
#endif
};
