#pragma once

#include <SDL3/SDL_video.h>

namespace Core::Platform
{

[[nodiscard]] constexpr SDL_WindowFlags BuildSDLWindowFlags(bool fullscreen, bool resizable)
{
#if defined(__APPLE__)
    // The 2D pipeline (game UI transforms, RmlUi context and mouse mapping) works in window
    // points while the swapchain-sized projections work in pixels. With a pixel density above
    // 1 those spaces diverge: the UI draws into the top-left quarter and clicks land offset
    // (macOS Retina = density 2). Until the pipeline handles density, request a 1x drawable.
    SDL_WindowFlags flags = 0;
#else
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
#endif
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
