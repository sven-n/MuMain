// CameraUtility.cpp - Camera movement and positioning utilities
// Extracted from ZzzScene.cpp as part of scene refactoring

#include "stdafx.h"
#include "CameraUtility.h"
#include "CameraManager.h"
#include "Camera/CameraMove.h"
#include "Render/Terrain/ZzzLodTerrain.h"

// External variable declarations
extern short g_shCameraLevel;
extern float g_fSpecialHeight;

// Function-key edge-trigger state. We poll via GetAsyncKeyState every frame,
// so we need to remember whether the key was already held last frame to fire
// each binding once per press instead of on every tick.
namespace
{
    bool s_bF9KeyPressed  = false;  // F9  — cycle camera mode
    bool s_bF11KeyPressed = false;  // F11 — reset active view
}

/**
 * @brief Handles the camera-related function keys (F9, F11).
 *
 * Lives at this layer so the bindings work the same regardless of which
 * camera is active. F10 (zoom lock) is handled directly in Winmain's
 * WM_SYSKEYDOWN case because Windows reserves F10 as a system key and
 * polling GetAsyncKeyState was racing with focus/loading frames.
 */
static void HandleCameraHotkeys()
{
    const bool bF9Down  = (GetAsyncKeyState(VK_F9)  & 0x8000) != 0;
    const bool bF11Down = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;

    if (bF9Down && !s_bF9KeyPressed)
        CameraManager::Instance().CycleToNextMode();
    if (bF11Down && !s_bF11KeyPressed)
        CameraManager::Instance().ResetActiveView();

    s_bF9KeyPressed  = bF9Down;
    s_bF11KeyPressed = bF11Down;
}

/**
 * @brief Main camera controller - delegates to CameraManager
 * @return True if camera is locked
 */
bool MoveMainCamera()
{
    HandleCameraHotkeys();

    // Update active camera through manager
    bool result = CameraManager::Instance().Update();

    // Cache active camera pointer for TestFrustrum shim
    CacheActiveFrustum();

    return result;
}
