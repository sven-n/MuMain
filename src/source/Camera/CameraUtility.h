#pragma once

// CameraUtility.h - Camera movement and positioning utilities

// Legacy global variables (extern - defined in ZzzScene.cpp)
extern float CameraDistanceTarget;
extern float CameraDistance;
extern float Camera3DFov;
extern bool Camera3DRoll;

// Camera state structure (encapsulates camera variables)
struct CameraState
{
    float& DistanceTarget;
    float& Distance;
    float& Fov3D;
    bool& Roll3D;

    CameraState()
        : DistanceTarget(::CameraDistanceTarget)
        , Distance(::CameraDistance)
        , Fov3D(::Camera3DFov)
        , Roll3D(::Camera3DRoll)
    {
    }
};

// Global camera state instance (extern - defined in ZzzScene.cpp)
extern CameraState g_CameraState;

// Main camera controller
// Returns true if camera is locked
bool MoveMainCamera();
