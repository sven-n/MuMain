#pragma once
#include "_types.h"

/**
 * @brief Encapsulates all camera state data
 *
 * Centralizes camera transformation data that was previously
 * scattered across global variables.
 */
class CameraState
{
public:
    CameraState();
    ~CameraState() = default;

    // ========== Transform ==========
    vec3_t Position;              // Camera world position
    vec3_t Angle;                 // Euler angles [pitch, yaw, roll] in degrees
    float Matrix[3][4];           // Cached transform matrix

    // ========== View Frustum ==========
    float ViewNear;               // Near clipping plane
    float ViewFar;                // Far clipping plane
    float FOV;                    // Field of view (degrees)
    bool TopViewEnable;           // Top-down orthographic view

    // ========== Camera Behavior ==========
    float Distance;               // Current distance from target
    float DistanceTarget;         // Target distance (smooth interpolation)
    short ZoomLevel;              // Camera zoom level (0-5)

    // Legacy 3D camera flags (unused in Phase 1, but kept for compatibility)
    float FOV3D;                  // 3D FOV override
    bool Roll3D;                  // 3D roll enable

    // Custom camera distance (for special terrain)
    float CustomDistance;

    // ========== Projection Cache ==========
    // These are computed by CameraProjection and cached here
    float PerspectiveX;           // Perspective factor X
    float PerspectiveY;           // Perspective factor Y
    int ScreenCenterX;            // Screen center X (pixels)
    int ScreenCenterY;            // Screen center Y (pixels)
    int ScreenCenterYFlip;        // Screen center Y flipped

    /**
     * @brief Updates the cached transform matrix from position and angle
     *
     * Call this after modifying Position or Angle.
     */
    void UpdateMatrix();

    /**
     * @brief Resets camera to default state
     */
    void Reset();
};

// Global camera state instance
// This replaces the scattered global variables
extern CameraState g_Camera;
