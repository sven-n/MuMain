#pragma once

#include <SDL3/SDL_video.h>

namespace Core::Platform
{

[[nodiscard]] constexpr SDL_WindowFlags BuildSDLWindowFlags(bool fullscreen, bool resizable)
{
    // No SDL_WINDOW_HIGH_PIXEL_DENSITY until the 2D pipeline handles a pixel density above 1:
    // the game UI transforms, the RmlUi context and the mouse mapping work in window points
    // while the swapchain-sized projections work in pixels, so above density 1 those spaces
    // diverge, the UI draws into the top-left quarter and clicks land offset (#869). The flag
    // raises the density on macOS and Wayland only; the Windows and X11 backends ignore it.
    SDL_WindowFlags flags = 0;
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

[[nodiscard]] constexpr bool HasSDLWindowInputFocus(SDL_WindowFlags flags)
{
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

} // namespace Core::Platform
