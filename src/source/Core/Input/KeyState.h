#pragma once

namespace Core::Input
{
    void RecordLeftMouseButtonPressEdge();
    void ClearLeftMouseButtonPressEdge();

    // Portable replacement for the Win32 "is this key currently down" check
    // (HIBYTE(GetAsyncKeyState(vk)) == 128 / & 0x8000), backed by SDL keyboard
    // state. The argument is a Win32 virtual-key code (VK_*) or an ASCII letter
    // or digit, matching what the existing call sites already pass.
    bool IsKeyDown(int virtualKey);

    // SDL scancode for a Win32 virtual-key code (or an ASCII letter/digit), as an int so this
    // header stays free of SDL. SDL_SCANCODE_UNKNOWN (0) for keys the shim does not translate.
    // Exported for the synthetic injector, which has to build real SDL key events.
    int ScancodeForVirtualKey(int virtualKey);
}
