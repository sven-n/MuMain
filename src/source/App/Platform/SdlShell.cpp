// Portable SDL window + OpenGL-context shell (issue #442). No Win32 here.
#include "SdlShell.h"

#include <SDL3/SDL.h>
#include <cstdio>

namespace Platform
{
    bool SdlShell::Create(const char* title, int width, int height)
    {
        // The fixed-function renderer needs a compatibility-profile context,
        // matching what the Windows bootstrap requests.
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

        window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL);
        if (window == nullptr)
        {
            std::fprintf(stderr, "[SdlShell] SDL_CreateWindow failed: %s\n", SDL_GetError());
            return false;
        }

        glContext = SDL_GL_CreateContext(window);
        if (glContext == nullptr)
        {
            std::fprintf(stderr, "[SdlShell] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            window = nullptr;
            return false;
        }

        SDL_GL_MakeCurrent(window, static_cast<SDL_GLContext>(glContext));
        return true;
    }

    void SdlShell::Destroy()
    {
        if (glContext != nullptr)
        {
            SDL_GL_DestroyContext(static_cast<SDL_GLContext>(glContext));
            glContext = nullptr;
        }
        if (window != nullptr)
        {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
    }

    int RunSmokeTest()
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::fprintf(stderr, "[SdlShell] SDL_Init(VIDEO) failed: %s\n", SDL_GetError());
            return 1;
        }

        SdlShell shell;
        if (!shell.Create("MU SDL shell smoke test", 320, 240))
        {
            SDL_Quit();
            return 2;
        }

        // A few non-interactive frames: drain events and swap, then exit so the
        // test terminates on its own under CI.
        constexpr int kFrames = 3;
        bool running = true;
        for (int frame = 0; frame < kFrames && running; ++frame)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                    running = false;
            }
            SDL_GL_SwapWindow(shell.window);
            SDL_Delay(16);
        }

        shell.Destroy();
        SDL_Quit();
        std::fprintf(stderr, "[SdlShell] smoke test OK\n");
        return 0;
    }
}
