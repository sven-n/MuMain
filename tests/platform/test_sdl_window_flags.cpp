#include <doctest.h>
#include <SDL3/SDL_video.h>

#include "Core/Platform/sdl3/SDLWindowFlags.h"

TEST_CASE("SDL windows request a 1x drawable on every platform [platform][sdl]")
{
    const SDL_WindowFlags flags = Core::Platform::BuildSDLWindowFlags(false, false);

    // Asking for a higher density breaks the 2D pipeline above density 1 (#869).
    CHECK((flags & SDL_WINDOW_HIGH_PIXEL_DENSITY) == 0);
}

TEST_CASE("SDL window flags preserve fullscreen and resizable requests [platform][sdl]")
{
    const SDL_WindowFlags flags = Core::Platform::BuildSDLWindowFlags(true, true);

    CHECK((flags & SDL_WINDOW_FULLSCREEN) != 0);
    CHECK((flags & SDL_WINDOW_RESIZABLE) != 0);
}

TEST_CASE("SDL startup focus follows the input-focus window flag [platform][sdl]")
{
    const SDL_WindowFlags unfocusedFullscreen = SDL_WINDOW_FULLSCREEN;
    const SDL_WindowFlags focusedFullscreen = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_INPUT_FOCUS;

    CHECK_FALSE(Core::Platform::HasSDLWindowInputFocus(unfocusedFullscreen));
    CHECK(Core::Platform::HasSDLWindowInputFocus(focusedFullscreen));
}
