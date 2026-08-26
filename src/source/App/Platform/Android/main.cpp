// Android entry point (AH-1118 spike).
//
// Mirrors the Linux entry: the portable bootstrap lives in Winmain.cpp and its
// WinMain is a plain function off Windows. SDL3's SDL_main glue provides the
// JNI bridge from SDLActivity; gl4es supplies desktop GL over the ES context.
#include "Core/Platform/WinCompat.h"

#include <cstdlib>
#include <unistd.h>
#include <sys/system_properties.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow);

// The engine's global constructors (e.g. CMixRecipeMgr) read game data relative
// to the working directory, and they run while libmain.so is still being
// dlopen'd -- before main() and before any Java-side hook can act. An explicit
// low-numbered constructor priority runs before every default-priority C++
// global initializer in this library, so the chdir happens first.
extern "C" __attribute__((constructor(101))) void mu_android_early_chdir(void)
{
    chdir("/data/data/com.alin.mumain/files");
}

int main(int /*argc*/, char* /*argv*/[])
{
    // Inside an emulator, GL calls cross a per-call transport whose round trips
    // dominate; the mapped streaming writes trade one call for two there, so
    // keep the plain SubData path. Real devices keep the stall-free mapping.
    {
        char qemu[PROP_VALUE_MAX] = {0};
        __system_property_get("ro.kernel.qemu", qemu);
        if (qemu[0] == '1')
        {
            setenv("MU_STREAM_MAP", "0", 1);
        }
    }
    // gl4es: back the desktop-GL API with an ES2 context it finds itself.
    setenv("LIBGL_ES", "2", 1);
    setenv("LIBGL_GL", "21", 1);
    // Diagnostics for the rendering-compat pass: surface shader conversion
    // failures and GL errors in logcat.
    setenv("LIBGL_LOGSHADERERROR", "1", 1);
    setenv("LIBGL_DBGSHADERCONV", "1", 1);


    // The engine's font discovery probes desktop Linux paths; point it at the
    // Android system font so UI text renders.
    setenv("MU_FONT", "/system/fonts/Roboto-Regular.ttf", 0);


    // The engine loads Data/ and config.ini relative to the working directory;
    // point it at the app's internal storage, where the data was pushed.
    const char* internal = SDL_GetAndroidInternalStoragePath();
    if (internal != nullptr)
    {
        chdir(internal);
    }

    // Without the trap, Android's back gesture minimizes the game mid-session;
    // trapped, it arrives as SDL_SCANCODE_AC_BACK for the engine to handle.
    // (SDL_SetHint, not setenv: SDLActivity queries this through the hint
    // table, which the environment does not reach here.)
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");

    return WinMain(nullptr, nullptr, nullptr, SW_SHOW);
}
