#pragma once

// WebzenScene.h - Splash screen scene

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

// Splash screen scene
void WebzenScene(HDC hDC);
