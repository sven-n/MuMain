// CameraUtility.cpp - Camera movement and positioning utilities
// Extracted from ZzzScene.cpp as part of scene refactoring

#include "stdafx.h"
#include "CameraUtility.h"
#include "CameraManager.h"
#include "CameraMove.h"

// External variable declarations
extern short g_shCameraLevel;
extern float g_fSpecialHeight;

// F9 key state tracking
static bool s_bF9KeyPressed = false;

/**
 * @brief Handles F9 key press for camera mode cycling
 */
static void HandleCameraModeToggle()
{
    // Check F9 key state
    bool bF9Down = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;

    // Detect key press (not held)
    if (bF9Down && !s_bF9KeyPressed)
    {
        // Cycle to next camera mode
        CameraManager::Instance().CycleToNextMode();

        // Optional: Log mode change for debugging
        // CameraMode newMode = CameraManager::Instance().GetCurrentMode();
        // printf("Camera mode switched to: %s\n", CameraModeToString(newMode));
    }

    s_bF9KeyPressed = bF9Down;
}

/**
 * @brief Main camera controller - delegates to CameraManager
 * @return True if camera is locked
 */
bool MoveMainCamera()
{
    // Handle F9 camera mode toggle
    HandleCameraModeToggle();

    // Update active camera through manager
    // Note: Camera's Update() method will check tour mode internally
    // and skip position calculations, only updating frustum
    return CameraManager::Instance().Update();
}
