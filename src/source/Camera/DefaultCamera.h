#pragma once

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
class DefaultCamera
{
public:
    DefaultCamera(CameraState& state);
    ~DefaultCamera() = default;

    /**
     * @brief Main update function - call this every frame
     * @return true if camera is locked (tour mode, etc.)
     */
    bool Update();

    /**
     * @brief Reset camera to default state
     */
    void Reset();

private:
    CameraState& m_State;

    // These are direct copies of the static functions from CameraUtility.cpp
    // We're NOT refactoring them yet - just moving them as-is
    void CalculateCameraViewFar();
    void AdjustHeroHeight();
    void CalculateCameraPosition();
    void SetCameraAngle();
    void UpdateCustomCameraDistance();
    void UpdateCameraDistance();
    void SetCameraFOV();

#ifdef ENABLE_EDIT2
    void HandleEditorMode();
#endif
};
