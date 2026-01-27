// CameraUtility.cpp - Camera movement and positioning utilities
// Extracted from ZzzScene.cpp as part of scene refactoring

#include "stdafx.h"
#include "CameraUtility.h"
#include "DefaultCamera.h"

// Global DefaultCamera instance
static DefaultCamera g_DefaultCamera(g_Camera);

// External variable declarations
extern short g_shCameraLevel;
extern float g_fSpecialHeight;

/**
 * @brief Main camera controller - updates camera position, angle, and distance.
 * @return True if camera is locked
 */
bool MoveMainCamera()
{
    return g_DefaultCamera.Update();
}
