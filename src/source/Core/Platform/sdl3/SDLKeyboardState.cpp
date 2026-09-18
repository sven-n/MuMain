// The original keyboard-state array, text-input buffer, and
// MuStartTextInput()/MuStopTextInput() lifecycle functions were deleted -- confirmed
// zero readers/callers anywhere (their only writer, Core::Platform::SDLEventLoop, was itself dead:
// zero call sites; the live input loop is Winmain.cpp::MainLoop()'s own inline SDL_PollEvent loop,
// which already calls SDL_StartTextInput()/SDL_StopTextInput() directly rather than through these
// functions).
//
// Separated from PlatformCompat.h to keep logging implementation details
// out of every translation unit that includes PlatformCompat.h.
// Compiled with the project PCH (stdafx.h) via MUPlatform REUSE_FROM MUCore.

#include "Core/Platform/WinCompat.h"
#include "Core/Utilities/Log/MuLogger.h"

// Logs a cursor warp failure via the post-mortem error log.
// Called by the SetCursorPos() shim in PlatformCompat.h.
void MuPlatformLogMouseWarpFailed(const char* sdlError)
{
    mu::log::Get("platform")->error("MU_ERR_MOUSE_WARP_FAILED [VS1-SDL-INPUT-MOUSE]: cursor warp failed: {}", sdlError);
}
