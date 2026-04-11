#ifdef MU_ENABLE_SDL3

#include "SDLWindow.h"
#include "../IPlatformWindow.h"
#include "MuLogger.h"
#include <SDL3/SDL.h>

namespace mu
{

SDLWindow::~SDLWindow()
{
    Destroy();
}

bool SDLWindow::Create(const char* title, int width, int height, uint32_t flags)
{
    if (m_pWindow != nullptr)
    {
        return false;
    }

    SDL_WindowFlags sdlFlags = SDL_WINDOW_RESIZABLE;
    if (flags & MU_WINDOW_FULLSCREEN)
    {
        sdlFlags |= SDL_WINDOW_FULLSCREEN;
    }

    m_pWindow = SDL_CreateWindow(title, width, height, sdlFlags);
    if (m_pWindow == nullptr)
    {
        return false;
    }

    return true;
}

void SDLWindow::Destroy()
{
    if (m_pWindow != nullptr)
    {
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }
}

void* SDLWindow::GetNativeHandle() const
{
    return m_pWindow;
}

void SDLWindow::SetTitle(const char* title)
{
    if (m_pWindow != nullptr)
    {
        SDL_SetWindowTitle(m_pWindow, title);
    }
}

void SDLWindow::SetSize(int width, int height)
{
    if (m_pWindow != nullptr)
    {
        SDL_SetWindowSize(m_pWindow, width, height);
    }
}

void SDLWindow::SetFullscreen(bool fullscreen)
{
    if (m_pWindow == nullptr)
    {
        return;
    }

    if (!SDL_SetWindowFullscreen(m_pWindow, fullscreen))
    {
        mu::log::Get("platform")->error("MU_ERR_FULLSCREEN_FAILED [VS1-SDL-WINDOW-FOCUS]: {}", SDL_GetError());
    }
}

void SDLWindow::SetMouseGrab(bool grab)
{
    if (m_pWindow != nullptr)
    {
        SDL_SetWindowMouseGrab(m_pWindow, grab);
    }
}

bool SDLWindow::GetDisplaySize(int& outWidth, int& outHeight) const
{
    if (m_pWindow == nullptr)
    {
        return false;
    }

    SDL_DisplayID displayId = SDL_GetDisplayForWindow(m_pWindow);
    if (displayId == 0)
    {
        mu::log::Get("platform")->error("MU_ERR_DISPLAY_QUERY_FAILED [VS1-SDL-WINDOW-FOCUS]: {}", SDL_GetError());
        return false;
    }

    const SDL_DisplayMode* pMode = SDL_GetCurrentDisplayMode(displayId);
    if (pMode == nullptr)
    {
        mu::log::Get("platform")->error("MU_ERR_DISPLAY_QUERY_FAILED [VS1-SDL-WINDOW-FOCUS]: {}", SDL_GetError());
        return false;
    }

    outWidth = pMode->w;
    outHeight = pMode->h;
    return true;
}

} // namespace mu

#endif // MU_ENABLE_SDL3
