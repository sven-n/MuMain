#pragma once

// MainScene.h - Main game scene

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

// Main scene lifecycle
void MoveMainScene();
bool RenderMainScene();
