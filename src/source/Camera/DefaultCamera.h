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
