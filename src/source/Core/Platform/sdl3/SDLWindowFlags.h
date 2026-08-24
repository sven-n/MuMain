#pragma once

#include <SDL3/SDL_video.h>

namespace Core::Platform
{

[[nodiscard]] constexpr SDL_WindowFlags BuildSDLWindowFlags(bool fullscreen, bool resizable)
{
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (resizable)
    {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    return flags;
}

} // namespace Core::Platform
