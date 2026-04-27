#pragma once

#include "ICamera.h"
#include "CameraState.h"
#include "CullingConstants.h"

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

    // Current mount camera offset (smoothly lerped). Used by OrbitalCamera
    // to apply the same lift without duplicating the lerp logic.
    float GetMountCameraOffset() const { return m_CurrentMountOffset; }

    // Snap m_CurrentMountOffset to the current target (skip the lerp).
    // Call on camera activation so the internal state matches immediately.
    void SyncMountOffset();

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

    // ResetForScene helpers: each loads the scene-specific config and initial transform.
    void ApplyConfigToState();   // Apply m_Config to m_State (FOV/ViewFar/TopView)
    void InvalidateFrustumCache();
    void InitCharacterScene();
    void InitMainScene();
    void InitLoginScene();

    // Mount height offset — smooth lerp when mounting/dismounting
    float GetTargetMountOffset() const;
    float m_CurrentMountOffset = 0.0f;
    int   m_LastMountType = -1;

#ifdef ENABLE_EDIT2
    void HandleEditorMode();
#endif

    // Phase 5: Cache last frustum state + editor config to avoid unnecessary rebuilds
    mutable struct FrustumCache
    {
        vec3_t Position = {};
        vec3_t Angle = {};
        float ViewFar = 0.0f;
        float AspectRatio = 0.0f;
        float EditorFOV = 0.0f;
        float EditorFarPlane = 0.0f;
        float EditorNearPlane = 0.0f;
        float EditorTerrainCullRange = 0.0f;
    } m_FrustumCache;
};
