#pragma once

// Forward declaration
class CameraState;

/**
 * @brief Camera projection and viewport utilities
 *
 * Handles projection matrix setup, screen-space transformations,
 * and viewport management. Extracted from ZzzOpenglUtil.cpp.
 */
class CameraProjection
{
public:
    /**
     * @brief Sets up OpenGL perspective projection
     *
     * Replaces gluPerspective2(). Updates both OpenGL state and
     * caches perspective factors in CameraState.
     *
     * @param state Camera state to update with cached factors
     * @param fov Field of view (degrees)
     * @param aspect Aspect ratio (width/height)
     * @param zNear Near clipping plane
     * @param zFar Far clipping plane
     */
    static void SetupPerspective(CameraState& state, float fov, float aspect,
                                  float zNear, float zFar);

    /**
     * @brief Sets OpenGL viewport with Y-axis flip
     *
     * Replaces glViewport2(). Takes pixel coordinates directly (not reference coords).
     *
     * @param x Viewport X offset (pixels)
     * @param y Viewport Y offset (pixels)
     * @param width Viewport width (pixels)
     * @param height Viewport height (pixels)
     */
    static void SetViewport(int x, int y, int width, int height);

    /**
     * @brief Converts screen coordinates to world ray direction
     *
     * Replaces CreateScreenVector().
     *
     * @param state Camera state with cached perspective factors
     * @param sx Screen X coordinate (in 640×480 reference coordinates)
     * @param sy Screen Y coordinate (in 640×480 reference coordinates)
     * @param outTarget Output world direction vector
     * @param bFixView Use camera view far (true) or item view far (false)
     */
    static void ScreenToWorldRay(const CameraState& state, int sx, int sy,
                                  vec3_t outTarget, bool bFixView = true);

    /**
     * @brief Projects world position to screen coordinates (reference)
     *
     * Replaces Projection(). Returns coordinates in 640×480 reference space.
     *
     * @param state Camera state with cached transform and perspective
     * @param worldPos World position
     * @param outX Output screen X (in 640×480 reference coordinates)
     * @param outY Output screen Y (in 640×480 reference coordinates)
     */
    static void WorldToScreen(const CameraState& state, const vec3_t worldPos,
                               int* outX, int* outY);

    /**
     * @brief Transforms position relative to camera (pixel coordinates)
     *
     * Replaces TransformPosition(). Returns coordinates in actual pixel space.
     *
     * @param state Camera state
     * @param position World position
     * @param outWorldPosition Output camera-relative position
     * @param outX Output screen X (in actual pixels)
     * @param outY Output screen Y (in actual pixels)
     */
    static void TransformPosition(const CameraState& state, const vec3_t position,
                                   vec3_t outWorldPosition, int* outX, int* outY);

    /**
     * @brief Tests if a world position is visible in depth buffer
     *
     * Replaces TestDepthBuffer().
     */
    static bool TestDepthBuffer(const CameraState& state, const vec3_t position);

    /**
     * @brief Reads OpenGL modelview matrix
     *
     * Replaces GetOpenGLMatrix().
     *
     * @param outMatrix Output 3×4 matrix
     */
    static void GetOpenGLMatrix(float outMatrix[3][4]);
};
