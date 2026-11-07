#pragma once

#include <SDL3/SDL_scancode.h>

namespace Core::Input
{
SDL_Scancode VkToScancode(int virtualKey);
void RecordLeftMouseButtonPressEdge();
void ClearLeftMouseButtonPressEdge();

// Portable replacement for the Win32 "is this key currently down" check
// (HIBYTE(GetAsyncKeyState(vk)) == 128 / & 0x8000), backed by SDL keyboard
// state. The argument is a Win32 virtual-key code (VK_*) or an ASCII letter
// or digit, matching what the existing call sites already pass.
bool IsKeyDown(int virtualKey);

// PrintScreen reaches us as a synthesized key-down followed by a key-up in the same SDL event
// batch on Windows, so a polled IsKeyDown(VK_SNAPSHOT) never sees it. The event path records the
// press here (once per physical press) and the screenshot handler consumes it.
void SetPrintScreenDown(bool down);
bool ConsumePrintScreenPress();

#ifdef _EDITOR
// Whether a key or button (a virtual-key code as IsKeyDown takes) is with a
// window that is not the game's, so that IsKeyDown leaves it out. Set by the
// editor for its own windows; without one every key is the game's.
using OtherWindowQuery = bool (*)(int virtualKey);
void SetOtherWindowQuery(OtherWindowQuery query);
#endif
}
