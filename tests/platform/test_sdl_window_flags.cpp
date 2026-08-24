#include <doctest.h>
#include <SDL3/SDL_video.h>

#include "Core/Platform/sdl3/SDLWindowFlags.h"

TEST_CASE("SDL windows always request high pixel density [platform][sdl]")
{
    const SDL_WindowFlags flags = Core::Platform::BuildSDLWindowFlags(false, false);

    CHECK((flags & SDL_WINDOW_HIGH_PIXEL_DENSITY) != 0);
}

TEST_CASE("SDL window flags preserve fullscreen and resizable requests [platform][sdl]")
{
    const SDL_WindowFlags flags = Core::Platform::BuildSDLWindowFlags(true, true);

    CHECK((flags & SDL_WINDOW_FULLSCREEN) != 0);
    CHECK((flags & SDL_WINDOW_RESIZABLE) != 0);
}
