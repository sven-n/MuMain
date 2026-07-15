#include "Core/Platform/MuPlatform.h"
#include "Core/Platform/IPlatformWindow.h"
#include "Core/Platform/IPlatformEventLoop.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <memory>

#ifndef _WIN32
#include "Core/Platform/posix/PosixSignalHandlers.h"
#endif

#include "Core/Platform/sdl3/SDLWindow.h"
#include "Core/Platform/sdl3/SDLEventLoop.h"
#include <SDL3/SDL.h>

namespace mu
{

static std::unique_ptr<IPlatformWindow> s_pWindow;
static std::unique_ptr<IPlatformEventLoop> s_pEventLoop;
static bool s_bInitialized = false;

bool MuPlatform::Initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        mu::log::Get("platform")->error("MU_ERR_SDL_INIT_FAILED: SDL3 initialization failed: {}", SDL_GetError());
        return false;
    }

#ifndef _WIN32
    // Story 7.1.2: Install POSIX signal handlers for crash diagnostics.
    // Must be called after SDL_Init (so .NET AOT runtime is initialized first — R8 mitigation)
    // and after mu::log::Init() (logging is initialized before MuPlatform::Initialize).
    // [VS0-QUAL-SIGNAL-HANDLERS]
    mu::platform::InstallSignalHandlers();
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

    SDL_Quit();
    s_bInitialized = false;
}

bool MuPlatform::CreateWindow(const char* title, int width, int height, uint32_t flags)
{
    if (!s_bInitialized)
    {
        return false;
    }

    if (title == nullptr || width <= 0 || height <= 0)
    {
        return false;
    }

    auto window = std::make_unique<SDLWindow>();
    if (!window->Create(title, width, height, flags))
    {
        mu::log::Get("platform")->error("MU_ERR_WINDOW_CREATE_FAILED: Window creation failed: {}", SDL_GetError());
        return false;
    }
    s_pWindow = std::move(window);
    s_pEventLoop = std::make_unique<SDLEventLoop>();
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

void MuPlatform::SetFullscreen(bool fullscreen)
{
    if (s_pWindow)
    {
        s_pWindow->SetFullscreen(fullscreen);
    }
}

void MuPlatform::SetMouseGrab(bool grab)
{
    if (s_pWindow)
    {
        s_pWindow->SetMouseGrab(grab);
    }
}

bool MuPlatform::GetDisplaySize(int& outWidth, int& outHeight)
{
    if (!s_pWindow)
    {
        return false;
    }
    return s_pWindow->GetDisplaySize(outWidth, outHeight);
}

} // namespace mu

// AC-STD-5: char16_t marshaling error log — emitted by mu_wchar_to_char16 defensive guard.
// Implemented here (not inline) to keep MuLogger.h out of every TU that
// includes PlatformCompat.h.
// [VS1-NET-CHAR16T-ENCODING]
void MuPlatformLogChar16MarshalingMismatch(const char* context)
{
    mu::log::Get("platform")->error("NET: char16_t marshaling -- encoding mismatch for {}", context);
}
