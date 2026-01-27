#pragma once

#include "CameraState.h"

/**
 * @brief Base interface for all camera implementations
 *
 * Defines the contract that all cameras must fulfill. Each camera
 * computes its own position and orientation, then applies it to
 * the shared CameraState.
 */
class ICamera
{
public:
    virtual ~ICamera() = default;

    /**
     * @brief Updates camera logic and applies to state
     * @return True if camera is locked (e.g., during cinematic)
     */
    virtual bool Update() = 0;

    /**
     * @brief Resets camera to default state
     */
    virtual void Reset() = 0;

    /**
     * @brief Called when camera becomes active
     * @param previousState Previous camera state for smooth transitions
     */
    virtual void OnActivate(const CameraState& previousState) {}

    /**
     * @brief Called when camera becomes inactive
     */
    virtual void OnDeactivate() {}

    /**
     * @brief Gets human-readable camera name
     */
    virtual const char* GetName() const = 0;
};
