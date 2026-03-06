#include "MuPlatform.h"
#include "IPlatformWindow.h"
#include "IPlatformEventLoop.h"

#include <memory>

#ifdef MU_ENABLE_SDL3
#include "sdl3/SDLWindow.h"
#include "sdl3/SDLEventLoop.h"
#include <SDL3/SDL.h>
#else
#ifdef _WIN32
#include "win32/Win32Window.h"
#include "win32/Win32EventLoop.h"
#endif
#endif

namespace mu
{

static std::unique_ptr<IPlatformWindow> s_pWindow;
static std::unique_ptr<IPlatformEventLoop> s_pEventLoop;
static bool s_bInitialized = false;

bool MuPlatform::Initialize()
{
#ifdef MU_ENABLE_SDL3
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        // SDL3 returns false on failure
        return false;
    }
#endif
    s_bInitialized = true;
    return true;
}

void MuPlatform::Shutdown()
{
    s_pEventLoop.reset();
    if (s_pWindow)
    {
        s_pWindow->Destroy();
        s_pWindow.reset();
    }

#ifdef MU_ENABLE_SDL3
    SDL_Quit();
#endif
    s_bInitialized = false;
}

bool MuPlatform::CreateWindow(const char* title, int width, int height, uint32_t flags)
{
    if (!s_bInitialized)
    {
        return false;
    }

#ifdef MU_ENABLE_SDL3
    auto window = std::make_unique<SDLWindow>();
    if (!window->Create(title, width, height, flags))
    {
        return false;
    }
    s_pWindow = std::move(window);
    s_pEventLoop = std::make_unique<SDLEventLoop>();
#else
#ifdef _WIN32
    auto window = std::make_unique<Win32Window>();
    if (!window->Create(title, width, height, flags))
    {
        return false;
    }
    s_pWindow = std::move(window);
    s_pEventLoop = std::make_unique<Win32EventLoop>();
#endif
#endif
    return true;
}

IPlatformWindow* MuPlatform::GetWindow()
{
    return s_pWindow.get();
}

bool MuPlatform::PollEvents()
{
    if (!s_pEventLoop)
    {
        return false;
    }
    return s_pEventLoop->PollEvents();
}

} // namespace mu
