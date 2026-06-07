// Portable SDL window + OpenGL-context shell (issue #442).
//
// This is the platform-neutral core of the client's window/GL/event bootstrap,
// with no Win32 dependency, so it compiles and runs on Windows, Linux and
// macOS. The Windows entry point still drives the full game through Winmain for
// now; this unit backs the cross-platform smoke test (and the Linux entry) that
// guards the portable path until the rest of the engine is platform-neutral.
#pragma once

struct SDL_Window;

namespace Platform
{
    // Owns an SDL window and a compatibility-profile OpenGL context (the
    // renderer is fixed-function). Create() returns false on failure and leaves
    // the shell empty; Destroy() is idempotent.
    struct SdlShell
    {
        SDL_Window* window = nullptr;
        void* glContext = nullptr;  // SDL_GLContext (opaque)

        bool Create(const char* title, int width, int height);
        void Destroy();
    };

    // Minimal portability smoke test: initialize SDL video, open a window with a
    // GL context, pump the event loop for a few frames (swapping buffers), then
    // tear everything down. Returns 0 on success, non-zero on failure. Used by
    // the cross-platform smoke-test target as a CI guard for the portable shell.
    int RunSmokeTest();
}
