// macOS entry point.
//
// The shared game bootstrap lives in Winmain.cpp. Off Windows, WinMain is a
// plain function, so the platform entry point forwards into it with unused
// Win32-compatible parameters.
#include "Core/Platform/WinCompat.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow);

int main(int /*argc*/, char* /*argv*/[])
{
    return WinMain(nullptr, nullptr, nullptr, SW_SHOW);
}
