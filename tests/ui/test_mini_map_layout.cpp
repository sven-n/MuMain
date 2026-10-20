#include <doctest.h>

#include "UI/HUD/MiniMapLayout.h"

using namespace UI::MiniMap;

namespace
{
Screen Reference()
{
    return {640.f, 480.f, 1.f, 1.f, 0.f, 0.f};
}
} // namespace

TEST_CASE("mini map quad puts the hero's map pixel at the window centre [ui][minimap]")
{
    const Quad quad = MapQuad(Reference(), 400.f, 300.f, MapLength, 0.f);
    CHECK(quad.topLeft.x == doctest::Approx(320.f - 400.f));
    CHECK(quad.topLeft.y == doctest::Approx(240.f - 300.f));
    CHECK(quad.topRight.x == doctest::Approx(320.f - 400.f + 800.f));
    CHECK(quad.bottomLeft.y == doctest::Approx(240.f - 300.f + 800.f));

    const CssMatrix m = ElementToQuad(quad, MapLength, MapLength);
    CHECK(m.a * 400.f + m.c * 300.f + m.e == doctest::Approx(320.f));
    CHECK(m.b * 400.f + m.d * 300.f + m.f == doctest::Approx(240.f));
}

TEST_CASE("mini map turns counter-clockwise on screen and keeps the hero at the centre [ui][minimap]")
{
    const Screen wide{1920.f, 1080.f, 3.f, 2.25f, 0.f, 0.f};
    const Quad quad = MapQuad(wide, 250.f, 500.f, MapLength, MapRotation);
    const CssMatrix m = ElementToQuad(quad, MapLength, MapLength);
    CHECK(m.a * 250.f + m.c * 500.f + m.e == doctest::Approx(960.f));
    CHECK(m.b * 250.f + m.d * 500.f + m.f == doctest::Approx(540.f));
    // The texture's +x axis turns up and right on screen (y down): a counter-clockwise turn.
    CHECK(m.a > 0.f);
    CHECK(m.b < 0.f);
    CHECK(m.a == doctest::Approx(3.f * 0.70710678f));
    CHECK(m.d == doctest::Approx(2.25f * 0.70710678f));
}

TEST_CASE("mini map marker sits 25 pixels right of its map point, with the original's hit box [ui][minimap]")
{
    const Marker npc = MarkerQuad(Reference(), 400.f, 300.f, 400.f, 300.f, 15.f, 0.f, MapLength, 0.f, false);
    CHECK(npc.quad.topLeft.x == doctest::Approx(320.f + 25.f - 7.5f));
    CHECK(npc.quad.topLeft.y == doctest::Approx(240.f - 7.5f));
    CHECK(npc.quad.topRight.x == doctest::Approx(320.f + 25.f + 7.5f));
    CHECK(npc.quad.bottomLeft.y == doctest::Approx(240.f + 7.5f));
    CHECK(npc.hitBox[0] == doctest::Approx(337.5f));
    CHECK(npc.hitBox[1] == doctest::Approx(232.5f));
    CHECK(npc.hitBox[2] == doctest::Approx(7.5f));

    const Marker portal = MarkerQuad(Reference(), 400.f, 300.f, 400.f, 300.f, 30.f, 0.f, MapLength, 0.f, true);
    CHECK(portal.hitBox[0] == doctest::Approx(345.f - 15.f - 15.f));
    CHECK(portal.hitBox[1] == doctest::Approx(225.f - 15.f));
    CHECK(portal.hitBox[2] == doctest::Approx(30.f));
}

TEST_CASE("mini map side line turns a quarter around its centre [ui][minimap]")
{
    const Quad quad = RotatedQuad(Reference(), 3.f, 64.f, 35.f, 6.f, -90.f);
    // The texture's width now runs down the window's left edge.
    CHECK(quad.topLeft.x == doctest::Approx(3.f + 3.f));
    CHECK(quad.topLeft.y == doctest::Approx(64.f - 17.5f));
    CHECK(quad.topRight.y == doctest::Approx(64.f + 17.5f));
    CHECK(quad.bottomLeft.x == doctest::Approx(3.f - 3.f));
}
