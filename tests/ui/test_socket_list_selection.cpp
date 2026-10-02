#include "doctest.h"
#include "UI/Inventory/SocketListSelection.h"

#include <array>

using UI::Inventory::SocketListSelection;

TEST_CASE("Socket selection survives unchanged refreshes and repeated clicks")
{
    SocketListSelection selection;
    const std::array options{SocketListSelection::Option{255, 0}, SocketListSelection::Option{4, 2}};
    CHECK(selection.Update(1, options));
    CHECK(selection.Selected() == -1);
    REQUIRE(selection.Select(1));
    CHECK_FALSE(selection.Update(1, options));
    CHECK(selection.Selected() == 1);
    CHECK(selection.Select(1));
    CHECK(selection.Selected() == 1);
    CHECK_FALSE(selection.Select(-1));
    CHECK_FALSE(selection.Select(2));
    CHECK(selection.Selected() == 1);
}

TEST_CASE("Socket changes and recipe switches invalidate selection")
{
    SocketListSelection selection;
    std::array options{SocketListSelection::Option{255, 0}, SocketListSelection::Option{4, 2}};
    selection.Update(1, options);
    selection.Select(1);
    options[1].sphereLevel = 3;
    CHECK(selection.Update(1, options));
    CHECK(selection.Selected() == -1);
    selection.Select(0);
    CHECK(selection.Update(2, options));
    CHECK(selection.Selected() == -1);
    selection.Select(1);
    CHECK(selection.Update(2, {}));
    CHECK(selection.Selected() == -1);
    CHECK_FALSE(selection.Select(0));
}

TEST_CASE("Replacing an item clears selection even with identical socket options")
{
    SocketListSelection selection;
    const std::array options{SocketListSelection::Option{255, 0}};
    selection.Update(1, options);
    selection.Select(0);
    selection.ClearSelection();
    CHECK_FALSE(selection.Update(1, options));
    CHECK(selection.Selected() == -1);
}
