#pragma once

#ifdef _EDITOR

#include "ICamera.h"
#include "CameraState.h"
#include "CameraConfig.h"
#include "Frustum.h"

/**
 * @brief Free-fly camera for editor mode (spectator tool)
 *
 * Features:
 * - WASD movement
 * - Q/E for up/down
 * - Right mouse drag for look
 * - Shift for speed boost
 * - Own frustum for correct culling from FreeFly viewpoint
 * - Editor only (_EDITOR flag)
 */
class FreeFlyCamera : public ICamera
{
public:
    FreeFlyCamera(CameraState& state);
    ~FreeFlyCamera() override = default;

    // ICamera interface
    bool Update() override;
    void Reset() override;
    void OnActivate(const CameraState& previousState) override;
    void OnDeactivate() override;
    const char* GetName() const override { return "FreeFly"; }
    void InheritFOV(float hFov);
    void SnapToPosition(const vec3_t pos, float yaw, float pitch);
    // Positions the camera high above the map centre looking (almost) straight
    // down, framing the whole 256x256 terrain, with far/cull/fog pushed out - for
    // capturing a top-down minimap screenshot.
    void SnapTopDown();
    // Leaves top-down pan/zoom mode (arrows go back to fly-along-forward).
    void ClearTopDown() { m_bTopDownPan = false; }
    // Zooms the top-down view by mouse-wheel ticks (positive = zoom in). Driven by
    // the editor from ImGui's wheel; no-op unless in top-down mode.
    void ZoomTopDown(float wheelTicks);

    // Projects the four map corners (z=0) through the current view and returns the
    // bottom-origin pixel rect (glReadPixels convention) that bounds the map, made
    // square and clamped to the window. Used to auto-crop a minimap capture.
    // Returns false if the map isn't sensibly in front of the camera.
    bool ComputeMapScreenRect(int& outX, int& outY, int& outSize) const;
    // Rotates the view around the vertical axis (for orienting a top-down shot).
    void RotateYaw(float deltaDeg) { m_Yaw += deltaDeg; }
    // Faces the view north (yaw 0) - the orientation a minimap capture must use.
    void FaceNorth() { m_Yaw = 0.0f; }

    // Configuration & Frustum Management
    const CameraConfig& GetConfig() const override { return m_Config; }
    void SetConfig(const CameraConfig& config) override;
    const Frustum& GetFrustum() const override { return m_Frustum; }

    bool ShouldCullObject(const vec3_t position, float radius) const override
    {
        return !m_Frustum.TestSphere(position, radius);
    }

    bool ShouldCullTerrain(int tileX, int tileY) const override
    {
        return !m_Frustum.TestPoint2D(tileX + 0.5f, tileY + 0.5f, -40.0f);
    }

    bool ShouldCullObject2D(float x, float y, float radius) const override
    {
        vec3_t position;
        position[0] = x;
        position[1] = y;
        position[2] = 0.0f;
        return !m_Frustum.TestSphere(position, radius);
    }

private:
    CameraState& m_State;

    // Configuration and frustum
    CameraConfig m_Config;
    Frustum m_Frustum;

    // Transform
    vec3_t m_Position;
    float m_Yaw;                  // Horizontal rotation (degrees)
    float m_Pitch;                // Vertical rotation (degrees)

    // Movement
    static constexpr float BASE_SPEED = 125.0f;
    static constexpr float SPRINT_MULTIPLIER = 3.0f;

    // Input state
    bool m_bLooking;
    int m_LastMouseX;
    int m_LastMouseY;

    // Top-down minimap mode: arrows pan across the map (XY) instead of flying along
    // the near-straight-down forward vector, and the mouse wheel zooms (height).
    bool m_bTopDownPan = false;

    // Zoom (height) limits and per-tick step for the top-down mouse-wheel zoom.
    static constexpr float TOPDOWN_MIN_HEIGHT = 4000.0f;
    static constexpr float TOPDOWN_MAX_HEIGHT = 58000.0f;
    static constexpr float TOPDOWN_WHEEL_STEP = 1800.0f;

    // State preservation across toggles
    bool m_bHasSavedState = false;

    // Constraints
    // Engine convention: Angle[0]=-90 = horizontal, more negative = looking up
    static constexpr float MIN_PITCH = -160.0f;  // Looking steeply upward
    static constexpr float MAX_PITCH = -0.3f;    // Near straight-down (pitch 0 =
                                                 // exactly down but gimbal-locks; a
                                                 // tiny tilt keeps the up-vector well
                                                 // defined while minimising the
                                                 // perspective skew in minimap capture)

    // Helper methods
    void HandleInput();
    void HandleMovement();
    void HandleTopDownMovement();
    void ReadMovementInput(float& outForward, float& outStrafe, float& outVertical);
    void ComputeCameraTransform();
    void UpdateFrustum();
};

#endif // _EDITOR
