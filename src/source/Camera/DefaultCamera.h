#pragma once

#include "ICamera.h"
#include "CameraState.h"
#include "CullingConstants.h"

#ifdef _EDITOR
extern "C" float DevEditor_GetCullRadiusTerrain();
#endif

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

    // Phase 5: Scene-specific reset
    void ResetForScene(EGameScene scene);

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
        // Use cheap 2D ground-plane projection test for terrain
        // (same algorithm as original TestFrustrum2D, ~4 cross-products vs 6-plane sphere test)
        return !m_Frustum.TestPoint2D(tileX + 0.5f, tileY + 0.5f, -40.0f);
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

    // Phase 1: Configuration and frustum
    CameraConfig m_Config;
    Frustum m_Frustum;

    // Phase 5: Scene transition tracking and Hero validity
    int m_LastSceneFlag = -1;
    bool m_bJustActivated = false;  // Skip first frame update to preserve inherited position
    int m_FramesSinceActivation = 0;  // Count frames since activation to disable smoothing
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
    bool NeedsFrustumUpdate() const;  // Phase 5: Check if frustum needs rebuild

    // Phase 5: WASD+QE free camera movement
    void HandleFreeCameraMovement();
    bool m_bFreeCameraMode = false;

#ifdef ENABLE_EDIT2
    void HandleEditorMode();
#endif

    // Phase 5: Cache last frustum state to avoid unnecessary rebuilds
    mutable vec3_t m_LastFrustumPosition;
    mutable vec3_t m_LastFrustumAngle;
    mutable float m_LastFrustumViewFar;

    // Phase 5: Cache last DevEditor config to detect changes
    mutable float m_LastEditorFOV;
    mutable float m_LastEditorFarPlane;
    mutable float m_LastEditorNearPlane;
    mutable float m_LastEditorTerrainCullRange;
};
