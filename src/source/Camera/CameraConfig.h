#pragma once

/**
 * @brief Rendering distance multiplier applied to camera far plane
 *
 * This multiplier extends the OpenGL projection matrix far plane beyond the camera's
 * logical ViewFar to prevent pop-in artifacts at screen edges. The multiplier creates
 * a buffer zone where terrain/objects slightly outside the frustum are still rendered.
 *
 * Used by:
 * - BeginOpengl() for projection matrix setup (ZzzOpenglUtil.cpp)
 * - ScreenToWorldRay() for mouse picking ray distance (CameraProjection.cpp)
 * - ForMainScene() for terrain culling range calculation (CameraConfig.h)
 *
 * @note Value of 1.4 provides 40% buffer (e.g., 1700 → 2380 units)
 */
constexpr float RENDER_DISTANCE_MULTIPLIER = 1.4f;

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
    float fov = 60.0f;

    /** Near clipping plane distance */
    float nearPlane = 10.0f;

    /** Far clipping plane distance */
    float farPlane = 2400.0f;

    /**
     * Aspect ratio (width / height) - calculated dynamically from viewport dimensions
     *
     * This value is NOT used directly! BeginOpengl() calculates aspect ratio every frame
     * from the current viewport (Width / Height) and passes it to SetupPerspective().
     * This field is kept for reference/compatibility but the live aspect comes from window size.
     *
     * Window resizing (WM_SIZE) automatically updates WindowWidth/WindowHeight, and the
     * next BeginOpengl() call will use the new dimensions to calculate the correct aspect ratio.
     */
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
     * @brief MainScene default camera configuration
     *
     * Optimized configuration for MainScene DefaultCamera.
     * Balanced for performance and visibility.
     *
     * Values (user-specified):
     * - FOV: 72 degrees
     * - Near Plane: 10
     * - Far Plane: 1700 (3D object culling)
     * - Terrain Cull Range: 1700 (2D terrain culling)
     */
    static CameraConfig ForMainScene()
    {
        CameraConfig config;
        config.fov = 72.0f;
        config.nearPlane = 500.0f;
        config.farPlane = 3000.0f;
        // Use RENDER_DISTANCE_MULTIPLIER to ensure terrain culling matches rendering/picking distance
        config.terrainCullRange = 3000.0f * RENDER_DISTANCE_MULTIPLIER;  // = 4200.0f
        config.objectCullRange = 3000.0f;
        config.frustumWidthNear = 330.0f;
        config.frustumWidthFar = 700.0f;
        return config;
    }

    /**
     * @brief LoginScene and CharacterScene camera configuration
     *
     * Extended visibility configuration for LoginScene and CharacterScene.
     * Provides wider view distance for cinematic tour and character preview.
     *
     * Phase 5 Values (user-specified):
     * - FOV: 72 degrees (optimal field of view)
     * - Near Plane: 10 (close clipping)
     * - Far Plane: 4800 (3D object culling distance)
     * - Terrain Cull Range: 4200 (2D terrain culling distance)
     */
    static CameraConfig ForGameplay()
    {
        CameraConfig config;
        config.fov = 72.0f;  // User-specified optimal FOV
        config.nearPlane = 1000.0f;
        config.farPlane = 4800.0f;  // 3D frustum culling
        config.terrainCullRange = 4200.0f;  // 2D terrain culling
        config.objectCullRange = 4800.0f;
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
        config.fov = 60.0f;
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
     * Values (Phase 5 update):
     * - FOV: 71 degrees (optimized for character viewing)
     * - Far plane: 4100 (balanced visibility)
     */
    static CameraConfig ForCharacterScene()
    {
        CameraConfig config;
        config.fov = 71.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 4100.0f;
        config.terrainCullRange = 4100.0f;
        config.objectCullRange = 4100.0f;
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
        config.fov = 60.0f;
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
        config.fov = 60.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 5100.0f;
        config.terrainCullRange = 5100.0f;
        config.objectCullRange = 5100.0f;
        config.frustumWidthNear = 540.0f;
        config.frustumWidthFar = 2250.0f;
        return config;
    }
};
