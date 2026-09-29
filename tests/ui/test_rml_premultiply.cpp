#include <doctest.h>

#include "Render/RmlUi/RmlPremultiply.h"

#include <array>

TEST_CASE("RmlUi textures are premultiplied: colour scaled by alpha, alpha kept [ui][rmlui]")
{
    std::array<std::uint8_t, 12> pixels = {255, 255, 255, 0, 200, 100, 50, 128, 10, 20, 30, 255};
    Render::RmlUi::PremultiplyAlpha(pixels.data(), 3);
    // A transparent white pixel (mini_map_ui_npc's background) adds nothing any more.
    CHECK(pixels[0] == 0);
    CHECK(pixels[1] == 0);
    CHECK(pixels[3] == 0);
    CHECK(pixels[4] == 100);
    CHECK(pixels[5] == 50);
    CHECK(pixels[6] == 25);
    CHECK(pixels[7] == 128);
    CHECK(pixels[8] == 10);
    CHECK(pixels[11] == 255);
}
