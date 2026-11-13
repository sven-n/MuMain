#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

namespace Core::Input
{
SDL_Scancode VkToScancode(int virtualKey);
// A press seen in the SDL events keeps its key or button down until ClearPressEdges, once a
// frame, so a press that starts and ends between two frames still reaches the per-frame readers.
// Takes key and mouse-button presses; other events and key repeats are ignored.
void RecordPressEdge(const SDL_Event& event);
// A mouse button, as VK_LBUTTON, VK_RBUTTON or VK_MBUTTON.
void RecordButtonPressEdge(int virtualKey);
void ClearButtonPressEdge(int virtualKey);
void ClearPressEdges();

// Portable replacement for the Win32 "is this key currently down" check
// (HIBYTE(GetAsyncKeyState(vk)) == 128 / & 0x8000), backed by SDL keyboard
// state. The argument is a Win32 virtual-key code (VK_*) or an ASCII letter
// or digit, matching what the existing call sites already pass.
bool IsKeyDown(int virtualKey);

#ifdef _EDITOR
// Whether a key or button (a virtual-key code as IsKeyDown takes) is with a
// window that is not the game's, so that IsKeyDown leaves it out. Set by the
// editor for its own windows; without one every key is the game's.
using OtherWindowQuery = bool (*)(int virtualKey);
void SetOtherWindowQuery(OtherWindowQuery query);
#endif
}
