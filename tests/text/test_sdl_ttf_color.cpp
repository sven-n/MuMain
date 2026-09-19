#include "doctest.h"

#include "Render/Text/SDLTtfColorPack.h"

TEST_CASE("SDL_ttf colors use the renderer ABGR byte order")
{
    CHECK(mu::sdlttf::PackColorDWORD(0x12, 0x34, 0x56, 0x78) == 0x78563412u);
    CHECK(mu::sdlttf::PackColorDWORD(0xFF, 0xFF, 0xFF, 0xFF) == 0xFFFFFFFFu);
    CHECK(mu::sdlttf::PackColorDWORD(0, 0, 0, 0) == 0u);
}
