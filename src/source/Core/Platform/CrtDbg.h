// Portable shim for the MSVC debug-CRT header (issue #462, Phase 3).
//
// The engine includes <crtdbg.h> only for the _ASSERT/_ASSERTE macros. On
// Windows that header is used unchanged; elsewhere the macros map onto the
// standard assert (a no-op under NDEBUG, exactly as on a Windows release build).
#pragma once

#ifdef _WIN32

#include <crtdbg.h>

#else  // ---- non-Windows ----------------------------------------------------

#include <cassert>

#ifndef _ASSERT
#define _ASSERT(expr)  assert(expr)
#endif
#ifndef _ASSERTE
#define _ASSERTE(expr) assert(expr)
#endif

#endif // _WIN32
