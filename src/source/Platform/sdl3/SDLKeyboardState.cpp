// Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
// Defines the g_sdl3KeyboardState array (indexed by SDL_Scancode, size 512)
// and the MuPlatformLogUnmappedVk() helper.
//
// Separated from PlatformCompat.h to avoid pulling CErrorReport/g_ErrorReport
// into every translation unit that includes PlatformCompat.h.
// Compiled with the project PCH (stdafx.h) via MUPlatform REUSE_FROM MUCore.

#ifdef MU_ENABLE_SDL3

#include "PlatformCompat.h"
#include "Core/ErrorReport.h"

// Keyboard state array: true when the key at that SDL_Scancode index is held.
// Populated by SDLEventLoop::PollEvents() on KEY_DOWN / KEY_UP events.
// Cleared in HandleFocusLoss() to prevent stuck keys on Alt-Tab.
// Size matches SDL_NUM_SCANCODES (512 in SDL3 release-3.2.8).
bool g_sdl3KeyboardState[512] = {};

// Logs an unmapped VK code warning via the post-mortem error log.
// Called by the GetAsyncKeyState() shim in PlatformCompat.h.
// [VS1-SDL-INPUT-KEYBOARD]
void MuPlatformLogUnmappedVk(int vk)
{
    g_ErrorReport.Write(L"MU_ERR_INPUT_UNMAPPED_VK [VS1-SDL-INPUT-KEYBOARD]: unmapped VK code 0x%02X\r\n",
                        static_cast<unsigned>(vk));
}

#endif // MU_ENABLE_SDL3
