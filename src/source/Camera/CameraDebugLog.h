#pragma once

// Editor-only debug logging helper for camera classes.
// In non-editor builds, expands to nothing.

#ifdef _EDITOR
#include "UI/Console/MuEditorConsoleUI.h"
#include <cstdio>

#define CAMERA_LOG(fmt, ...) \
    do { \
        char _camDbgMsg[512]; \
        sprintf_s(_camDbgMsg, fmt, ##__VA_ARGS__); \
        g_MuEditorConsoleUI.LogEditor(_camDbgMsg); \
    } while (0)
#else
#define CAMERA_LOG(fmt, ...) ((void)0)
#endif
