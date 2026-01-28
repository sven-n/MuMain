#pragma once

#include "CameraState.h"
#include "CameraConfig.h"
#include "Frustum.h"

/**
 * @brief Base interface for all camera implementations
 *
 * Defines the contract that all cameras must fulfill. Each camera
 * computes its own position and orientation, then applies it to
 * the shared CameraState.
 *
 * Phase 1 Enhancement: Cameras now own their rendering configuration
 * and provide frustum for culling.
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

    // ========== Phase 1: Configuration & Frustum Management ==========

    /**
     * @brief Gets the camera's rendering configuration
     *
     * Configuration controls FOV, near/far planes, and culling parameters.
     */
    virtual const CameraConfig& GetConfig() const = 0;

    /**
     * @brief Sets the camera's rendering configuration
     *
     * @param config New configuration to apply
     *
     * NOTE: Derived classes should rebuild frustum when config changes.
     */
    virtual void SetConfig(const CameraConfig& config) = 0;

    /**
     * @brief Gets the camera's view frustum
     *
     * Frustum is used for culling objects and terrain tiles.
     * Derived classes should update frustum in Update() method.
     */
    virtual const Frustum& GetFrustum() const = 0;

    /**
     * @brief Tests if an object should be culled (not rendered)
     *
     * @param position Object center position in world space
     * @param radius Object bounding sphere radius
     * @return true if object should be culled, false if visible
     */
    virtual bool ShouldCullObject(const vec3_t position, float radius) const = 0;

    /**
     * @brief Tests if a terrain tile should be culled (not rendered)
     *
     * @param tileX Terrain tile X coordinate (in tile space, not world space)
     * @param tileY Terrain tile Y coordinate (in tile space, not world space)
     * @return true if tile should be culled, false if visible
     */
    virtual bool ShouldCullTerrain(int tileX, int tileY) const = 0;
};
