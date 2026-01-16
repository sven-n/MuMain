#pragma once

// CameraUtility.h - Camera movement and positioning utilities

// Main camera controller
// Returns true if camera is locked
bool MoveMainCamera();

// Camera variables (extern - defined in ZzzScene.cpp)
extern float CameraDistanceTarget;
extern float CameraDistance;
extern float Camera3DFov;
extern bool Camera3DRoll;
