#pragma once

// CameraUtility.h - Camera movement and positioning utilities

#include "CameraState.h"

// Note: Camera variables now in g_Camera (see backward compatibility layer in ZzzOpenglUtil.h)

// Main camera controller
// Returns true if camera is locked
bool MoveMainCamera();
