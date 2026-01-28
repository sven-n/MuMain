#pragma once

/**
 * @brief Camera rendering configuration
 *
 * Encapsulates all parameters that define how a camera renders the scene.
 * Replaces hardcoded scene-specific frustum values with clear, configurable parameters.
 */
struct CameraConfig
{
    // ========== View Frustum Parameters ==========

    /** Vertical field of view in degrees */
    float fov = 33.0f;

    /** Near clipping plane distance */
    float nearPlane = 10.0f;

    /** Far clipping plane distance */
    float farPlane = 2400.0f;

    /** Aspect ratio (width / height) - usually calculated from screen dimensions */
    float aspectRatio = 1.33f;

    // ========== Culling Parameters ==========

    /**
     * Terrain culling range (2D ground projection far distance)
     * Controls how far terrain tiles are rendered
     */
    float terrainCullRange = 1100.0f;

    /**
     * Object culling range (3D frustum far distance)
     * Usually same as farPlane, but can be different for optimization
     */
    float objectCullRange = 2400.0f;

    // ========== Frustum Shape (for 2D terrain projection) ==========

    /**
     * Width of frustum trapezoid at near plane (in world units before aspect scaling)
     * Used for 2D ground projection calculations
     */
    float frustumWidthNear = 330.0f;

    /**
     * Width of frustum trapezoid at far plane (in world units before aspect scaling)
     * Used for 2D ground projection calculations
     */
    float frustumWidthFar = 700.0f;

    // ========== Comparison Operator ==========

    bool operator==(const CameraConfig& other) const
    {
        return fov == other.fov &&
               nearPlane == other.nearPlane &&
               farPlane == other.farPlane &&
               aspectRatio == other.aspectRatio &&
               terrainCullRange == other.terrainCullRange &&
               objectCullRange == other.objectCullRange &&
               frustumWidthNear == other.frustumWidthNear &&
               frustumWidthFar == other.frustumWidthFar;
    }

    bool operator!=(const CameraConfig& other) const
    {
        return !(*this == other);
    }

    // ========== Preset Configurations ==========

    /**
     * @brief Default gameplay camera configuration
     *
     * Optimized for standard third-person gameplay.
     * Based on testing, these values provide the best balance of
     * visibility and performance for orbital/default cameras.
     *
     * Values:
     * - ViewFar: 1100 (terrain), 2400 (objects)
     * - WidthFar: 700, WidthNear: 330
     */
    static CameraConfig ForGameplay()
    {
        CameraConfig config;
        config.fov = 33.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 2400.0f;
        config.terrainCullRange = 1100.0f;
        config.objectCullRange = 2400.0f;
        config.frustumWidthNear = 330.0f;
        config.frustumWidthFar = 700.0f;
        return config;
    }

    /**
     * @brief Login scene camera configuration
     *
     * Massive panoramic view for the cinematic login screen.
     * Camera moves through the environment on a pre-defined path.
     *
     * Values:
     * - ViewFar: 265200 (1200 * 17 * 13)
     * - WidthFar: 2500, WidthNear: 150
     */
    static CameraConfig ForLoginScene()
    {
        CameraConfig config;
        config.fov = 33.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 265200.0f;
        config.terrainCullRange = 265200.0f;
        config.objectCullRange = 265200.0f;
        config.frustumWidthNear = 150.0f;
        config.frustumWidthFar = 2500.0f;
        return config;
    }

    /**
     * @brief Character selection scene camera configuration
     *
     * Medium frustum for character preview.
     * Shows character and surrounding environment.
     *
     * Values:
     * - ViewFar: 7371 (2000 * 9.1 * 0.404998)
     * - WidthFar: 1190, WidthNear: 540 (with FOV scaling)
     */
    static CameraConfig ForCharacterScene()
    {
        CameraConfig config;
        config.fov = 33.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 7371.0f;
        config.terrainCullRange = 7371.0f;
        config.objectCullRange = 7371.0f;
        config.frustumWidthNear = 540.0f;
        config.frustumWidthFar = 1190.0f;
        return config;
    }

    /**
     * @brief Wide view configuration (for testing/debugging)
     *
     * Extended view distance for debugging or special situations.
     */
    static CameraConfig ForWideView()
    {
        CameraConfig config;
        config.fov = 33.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 6000.0f;
        config.terrainCullRange = 6000.0f;
        config.objectCullRange = 6000.0f;
        config.frustumWidthNear = 700.0f;
        config.frustumWidthFar = 3000.0f;
        return config;
    }

    /**
     * @brief Old default configuration (for comparison)
     *
     * The previous hardcoded values from before refactor.
     * Kept for testing and comparison purposes.
     */
    static CameraConfig ForOldDefault()
    {
        CameraConfig config;
        config.fov = 33.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 5100.0f;
        config.terrainCullRange = 5100.0f;
        config.objectCullRange = 5100.0f;
        config.frustumWidthNear = 540.0f;
        config.frustumWidthFar = 2250.0f;
        return config;
    }
};
