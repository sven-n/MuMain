#ifdef _WIN32

#include "Win32EventLoop.h"

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

namespace mu
{

bool Win32EventLoop::PollEvents()
{
    // Win32 message pumping is still handled by MainLoop() in MuMain.cpp.
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
