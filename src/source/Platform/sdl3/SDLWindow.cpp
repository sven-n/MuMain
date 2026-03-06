#ifdef MU_ENABLE_SDL3

#include "SDLWindow.h"
#include "../IPlatformWindow.h"
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

} // namespace mu

#endif // MU_ENABLE_SDL3
