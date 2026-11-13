#include "doctest.h"

#include "Core/Platform/Quit.h"

#include <SDL3/SDL.h>

TEST_CASE("A quit request is kept before SDL runs and reaches the loop as one SDL quit event after [core][quit]")
{
    REQUIRE_FALSE(Core::Platform::QuitRequested());
    Core::Platform::RequestQuit();
    CHECK(Core::Platform::QuitRequested());

    REQUIRE(SDL_Init(SDL_INIT_EVENTS));
    SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);

    Core::Platform::RequestQuit();

    SDL_Event events[4];
    CHECK(SDL_PeepEvents(events, 4, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) == 1);
    CHECK(events[0].type == SDL_EVENT_QUIT);

    SDL_Quit();
}
