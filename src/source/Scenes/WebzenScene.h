#pragma once

// WebzenScene.h - Splash screen scene

#include "Core/Platform/WinCompat.h"

// Splash screen scene: draws the startup load's progress.
void WebzenScene(HDC hDC);
// Runs the startup load a few steps per frame, before the frame begins (load work never runs inside a frame),
// then switches to login.
void PumpWebzenScene();
// Releases the title screen once login has taken over (a frame later, so the frame that last drew it is
// submitted), and drops a startup load still running on a quit; the renderer must still be up.
void EndWebzenScene();
