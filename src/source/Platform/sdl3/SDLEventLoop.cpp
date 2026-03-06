#ifdef MU_ENABLE_SDL3

#include "SDLEventLoop.h"
#include <SDL3/SDL.h>

// External game state — set to true to trigger clean shutdown
extern bool Destroy;

namespace mu
{

bool SDLEventLoop::PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            Destroy = true;
            return false;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            Destroy = true;
            return false;

        case SDL_EVENT_WINDOW_RESIZED:
            // No-op — will be mapped in story 2.1.2
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            // No-op — will be mapped in story 2.1.2
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            // No-op — will be mapped in story 2.1.2
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            // No-op — will be mapped in story 2.1.2
            break;

        case SDL_EVENT_WINDOW_RESTORED:
            // No-op — will be mapped in story 2.1.2
            break;

        default:
            break;
        }
    }
    return true;
}

} // namespace mu

#endif // MU_ENABLE_SDL3
