// Linux entry point (issue #442, the per-platform entry seam from #441).
//
// The game bootstrap lives in Winmain.cpp; off Windows its WinMain is a plain
// function (no Win32 entry contract), so this just forwards into it. The
// HINSTANCE/command-line parameters are Win32 artifacts the portable code path
// does not read.
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
