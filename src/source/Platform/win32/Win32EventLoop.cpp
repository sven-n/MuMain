#ifdef _WIN32

#include "Win32EventLoop.h"

#include <windows.h>

namespace mu
{

bool Win32EventLoop::PollEvents()
{
    // Win32 message pumping is still handled by MainLoop() in Winmain.cpp.
    // This stub exists so MuPlatform::PollEvents() has a valid backend on Windows.
    // The actual GetMessage/PeekMessage/DispatchMessage loop remains unchanged.
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

} // namespace mu

#endif // _WIN32
