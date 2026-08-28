// macOS entry point, the Apple arm of the per-platform entry seam that already
// carries source/App/Platform/Linux/main.cpp and .../Windows/Winmain.cpp.
//
// The CMake APPLE branch has referenced this file for a while, but the file was
// never added, so configuring for macOS failed on a missing source. This is that
// file.
//
// As on Linux, the game bootstrap lives in Winmain.cpp: off Windows its WinMain
// is a plain function rather than the Win32 entry contract, so the platform entry
// only has to forward into it. The HINSTANCE and command-line parameters are
// Win32 artifacts which the portable code path never reads.
//
// This is Objective-C++ (.mm) rather than .cpp because a macOS app needs a Cocoa
// application object to exist before it can own a window, take keyboard focus, or
// appear in the Dock. SDL creates and activates one from SDL_Init, so nothing has
// to be done here yet; keeping the translation unit Objective-C++ means the Cocoa
// work can land here without changing the build when it is needed.

#include "Core/Platform/WinCompat.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow);

int main(int /*argc*/, char* /*argv*/[])
{
    return WinMain(nullptr, nullptr, nullptr, SW_SHOW);
}
