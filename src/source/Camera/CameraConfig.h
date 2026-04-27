#pragma once

#include <cmath>

/// Convert horizontal FOV (degrees) to vertical FOV (degrees) for gluPerspective.
/// hFovDeg is the horizontal FOV at the 4:3 reference aspect. The `aspectRatio`
/// argument is intentionally ignored — we want vFov tied to the reference
/// aspect so wider resolutions extend the side view via gluPerspective rather
/// than stretching or cropping vertically. The UI is still scaled to actual
/// aspect separately (g_fScreenRate_x/y) which is the intended behavior there.
inline float HFovToVFov(float hFovDeg, float /*aspectRatio*/)
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float REFERENCE_ASPECT = 4.0f / 3.0f;
    float hHalfRad = hFovDeg * 0.5f * PI / 180.0f;
    float vHalfRad = atanf(tanf(hHalfRad) / REFERENCE_ASPECT);
    return vHalfRad * 2.0f * 180.0f / PI;
}

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
 * @brief Fixed camera transform used in CharacterScene
 *
 * CharacterScene uses a static hardcoded camera pose (the original game didn't move
 * the camera on this screen). Both DefaultCamera and OrbitalCamera need these values:
 * DefaultCamera snaps to them directly, OrbitalCamera uses them as a ray-cast origin
 * to compute where to place the orbit pivot.
 */
namespace CharacterSceneCamera
{
    constexpr float POSITION_X = 9758.93f;
    constexpr float POSITION_Y = 18913.11f;
    constexpr float POSITION_Z = 675.5f;
    constexpr float ANGLE_PITCH = -84.5f;  // Angle[0]
    constexpr float ANGLE_ROLL  = -75.0f;  // Angle[2]
}

/**
 * @brief Camera rendering configuration
 *
 * Encapsulates all parameters that define how a camera renders the scene.
 * Replaces hardcoded scene-specific frustum values with clear, configurable parameters.
 */
struct CameraConfig
{
    // ========== View Frustum Parameters ==========

    /** Horizontal field of view in degrees. Vertical FOV is derived at runtime via HFovToVFov(hFov, aspect). */
    float hFov = 90.0f;

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

    // ========== Fog Parameters ==========

    /**
     * Fog start distance (where fog transition begins)
     * Default: 100% of farPlane
     */
    float fogStart = 2400.0f;

    /**
     * Fog end distance (where fog reaches full density)
     * Default: 125% of farPlane
     */
    float fogEnd = 3000.0f;

    // ========== Comparison Operator ==========

    bool operator==(const CameraConfig& other) const
    {
        return hFov == other.hFov &&
               nearPlane == other.nearPlane &&
               farPlane == other.farPlane &&
               aspectRatio == other.aspectRatio &&
               terrainCullRange == other.terrainCullRange &&
               objectCullRange == other.objectCullRange &&
               fogStart == other.fogStart &&
               fogEnd == other.fogEnd;
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
    static CameraConfig ForMainSceneDefaultCamera()
    {
        CameraConfig config;
        config.hFov = 40.0f;  // ~30° vFOV on 4:3, preserves original game look
        // Match the projection's near plane (g_Camera.ViewNear = 20). Higher
        // values here would CPU-cull objects that the GPU still renders,
        // making them vanish as they approach the camera.
        config.nearPlane = 20.0f;
        config.farPlane = 3000.0f;
        // Use RENDER_DISTANCE_MULTIPLIER to ensure terrain culling matches rendering/picking distance
        config.terrainCullRange = 3000.0f * RENDER_DISTANCE_MULTIPLIER;  // = 4200.0f
        config.objectCullRange = 3000.0f;
        config.fogStart = config.farPlane * 1.0f;
        config.fogEnd = config.farPlane * 1.25f;
        return config;
    }

    /**
 * @brief MainScene orbital camera configuration
 *
 * Optimized configuration for MainScene OrbitalCamera.
 * Balanced for performance and visibility.
 *
 * Values:
 * - FOV: 90 degrees
 * - Near Plane: 20 (matches projection's ViewNear)
 * - Far Plane: 3800 (3D object culling)
 * - Terrain Cull Range: 5320 (farPlane * RENDER_DISTANCE_MULTIPLIER)
 */
    static CameraConfig ForMainSceneOrbitalCamera()
    {
        CameraConfig config;
        config.hFov = 90.0f;  // Good 3D game default; ~59° vFOV on 16:9
        // Match the projection's near plane (g_Camera.ViewNear = 20). Higher
        // values here would CPU-cull objects the GPU still renders — visible
        // in zoomed-in orbital where the camera sits ~200 units from hero.
        config.nearPlane = 20.0f;
        config.farPlane = 3800.0f;  // Direct value — RENDER_DISTANCE_MULTIPLIER already applied in BeginOpengl()
        config.objectCullRange = 3800.0f;
        config.fogStart = config.farPlane * 1.0f;
        config.fogEnd = config.farPlane * 1.25f;
        config.terrainCullRange = config.farPlane * RENDER_DISTANCE_MULTIPLIER;
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
        config.hFov = 90.0f;
        config.nearPlane = 10.0f;
        config.farPlane = 20000.0f;
        config.terrainCullRange = 20000.0f;
        config.objectCullRange = 20000.0f;
        config.fogStart = config.farPlane * 1.0f;
        config.fogEnd = config.farPlane * 1.25f;
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
        config.hFov = 40.0f;  // ~30° vFOV on 4:3, matches original character scene
        config.nearPlane = 10.0f;
        config.farPlane = 4100.0f;
        config.terrainCullRange = 4100.0f;
        config.objectCullRange = 4100.0f;
        config.fogStart = config.farPlane * 1.0f;
        config.fogEnd = config.farPlane * 1.25f;
        return config;
    }
};
