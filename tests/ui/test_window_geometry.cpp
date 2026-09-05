#include "App/stdafx.h"
#include "UI/Core/WindowGeometry.h"

#include "doctest.h"

using mu::ui::window::WindowGeometry;

TEST_CASE("WindowGeometry defaults to an empty rect at the origin")
{
    const WindowGeometry geometry;

    CHECK(geometry.Position().x == 0);
    CHECK(geometry.Position().y == 0);
    CHECK(geometry.Size().cx == 0);
    CHECK(geometry.Size().cy == 0);
    CHECK_FALSE(geometry.Contains(0, 0));
}

static void CheckSameBounds(const WindowGeometry& geometry)
{
    CHECK(geometry.Position().x == 10);
    CHECK(geometry.Position().y == 20);
    CHECK(geometry.Size().cx == 100);
    CHECK(geometry.Size().cy == 40);

    const RECT bounds = geometry.Bounds();
    CHECK(bounds.left == 10);
    CHECK(bounds.top == 20);
    CHECK(bounds.right == 110);
    CHECK(bounds.bottom == 60);
}

TEST_CASE("WindowGeometry construction and SetBounds agree on the resulting rect")
{
    const WindowGeometry constructed(10, 20, 100, 40);
    CheckSameBounds(constructed);

    WindowGeometry viaSetters;
    viaSetters.SetBounds(10, 20, 100, 40);
    CheckSameBounds(viaSetters);
}

TEST_CASE("WindowGeometry Contains is half-open, matching CheckMouseIn's own convention")
{
    WindowGeometry geometry(10, 20, 100, 40);

    // Inside.
    CHECK(geometry.Contains(10, 20));   // top-left corner: inclusive
    CHECK(geometry.Contains(60, 40));   // interior

    // Left/top edges are inclusive.
    CHECK_FALSE(geometry.Contains(9, 20));
    CHECK_FALSE(geometry.Contains(10, 19));

    // Right/bottom edges are exclusive.
    CHECK_FALSE(geometry.Contains(110, 40));
    CHECK_FALSE(geometry.Contains(60, 60));

    // Well outside.
    CHECK_FALSE(geometry.Contains(0, 0));
    CHECK_FALSE(geometry.Contains(200, 200));
}

TEST_CASE("WindowGeometry SetPosition and SetSize update independently")
{
    WindowGeometry geometry(0, 0, 10, 10);

    geometry.SetPosition(5, 5);
    CHECK(geometry.Position().x == 5);
    CHECK(geometry.Position().y == 5);
    CHECK(geometry.Size().cx == 10);
    CHECK(geometry.Size().cy == 10);

    geometry.SetSize(20, 30);
    CHECK(geometry.Position().x == 5);
    CHECK(geometry.Position().y == 5);
    CHECK(geometry.Size().cx == 20);
    CHECK(geometry.Size().cy == 30);
}
