// Linux entry point (issue #442, the per-platform entry seam from #441).
//
// The engine is still largely Win32-bound, so the full game cannot link here
// yet. For now this entry runs the portable SDL shell smoke test, which proves
// the window/GL/event-loop path works on Linux. It is replaced with the real
// game bootstrap once the engine is platform-neutral.
#include "../SdlShell.h"

int main(int /*argc*/, char* /*argv*/[])
{
    return Platform::RunSmokeTest();
}
