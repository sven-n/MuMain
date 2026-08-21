// macOS entry point.
//
// The shared game bootstrap lives in Winmain.cpp. Off Windows, WinMain is a
// plain function, so the platform entry point forwards into it with unused
// Win32-compatible parameters.
#include "Core/Platform/WinCompat.h"

#include <string>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow);

int main(int argc, char* argv[])
{
    std::string commandLine;
    for (int index = 1; index < argc; ++index)
    {
        if (!commandLine.empty())
        {
            commandLine += ' ';
        }
        commandLine += argv[index];
    }

    return WinMain(nullptr, nullptr, commandLine.data(), SW_SHOW);
}
