// Portable shim for the MSVC debug-CRT header (issue #462, Phase 3).
//
// The engine includes <crtdbg.h> only for the _ASSERT/_ASSERTE macros. <crtdbg.h>
// is MSVC-specific, so only MSVC uses it; every other compiler (MinGW, Clang,
// Linux GCC) maps the macros onto the standard assert (a no-op under NDEBUG,
// exactly as on a Windows release build).
#pragma once

#ifdef _MSC_VER

#include <crtdbg.h>

#else  // ---- non-MSVC -------------------------------------------------------

#include <assert.h>

#ifndef _ASSERT
#define _ASSERT(expr)  assert(expr)
#endif
#ifndef _ASSERTE
#define _ASSERTE(expr) assert(expr)
#endif

#endif // _MSC_VER
