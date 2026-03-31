#ifdef _WIN32

#include "Win32Window.h"

// Existing global window handle from MuMain.cpp
extern HWND g_hWnd;

namespace mu
{

bool Win32Window::Create(const char* /*title*/, int /*width*/, int /*height*/, uint32_t /*flags*/)
{
    // Win32 window creation is still handled by WinMain() directly.
    // This stub exists so MuPlatform can provide the existing HWND via GetNativeHandle().
    // The actual CreateWindowEx call remains in MuMain.cpp on Windows.
    return (g_hWnd != nullptr);
}

void Win32Window::Destroy()
{
    // Win32 window destruction is handled by the existing WM_DESTROY handler in WndProc.
}

void* Win32Window::GetNativeHandle() const
{
    return g_hWnd;
}

void Win32Window::SetTitle(const char* title)
{
    if (g_hWnd != nullptr && title != nullptr)
    {
        SetWindowTextA(g_hWnd, title);
    }
}

void Win32Window::SetSize(int width, int height)
{
    if (g_hWnd != nullptr)
    {
        SetWindowPos(g_hWnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Win32Window::SetFullscreen(bool /*fullscreen*/)
{
    // Win32 fullscreen is handled by ChangeDisplaySettings in MuMain.cpp before window creation.
    // No runtime toggle needed on the Win32 path at this stage.
}

void Win32Window::SetMouseGrab(bool /*grab*/)
{
    // Win32 uses SetCapture/ReleaseCapture in game logic for mouse confinement.
    // This SDL3-specific interface method is a no-op on Windows.
}

bool Win32Window::GetDisplaySize(int& outWidth, int& outHeight) const
{
    outWidth = GetSystemMetrics(SM_CXSCREEN);
    outHeight = GetSystemMetrics(SM_CYSCREEN);
    return (outWidth > 0 && outHeight > 0);
}

} // namespace mu

#endif // _WIN32
