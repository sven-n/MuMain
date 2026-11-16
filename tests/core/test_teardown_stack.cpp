#include <doctest.h>

#include "Core/Utilities/TeardownStack.h"

#include <string>

TEST_CASE("A teardown stack undoes what was set up, newest first, once")
{
    std::string undone;
    Core::TeardownStack teardown;
    teardown.Push([&] { undone += "log "; });
    teardown.Push([&] { undone += "window "; });
    teardown.Push([&] { undone += "renderer "; });
    // A step that fails pushes nothing, so only the three above are undone.

    teardown.Unwind();
    CHECK(undone == "renderer window log ");

    teardown.Unwind();
    CHECK(undone == "renderer window log ");
}

