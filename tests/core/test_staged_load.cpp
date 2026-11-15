#include <doctest.h>

#include "Core/Utilities/StagedLoad.h"

#include <chrono>
#include <string>

using namespace std::chrono_literals;

TEST_CASE("A staged load runs one step per pass when the budget is spent, in order")
{
    std::string ran;
    Core::StagedLoad load({[&] { ran += "a"; }, [&] { ran += "b"; }, [&] { ran += "c"; }});
    CHECK(load.Total() == 3);

    CHECK_FALSE(load.Pump(0ms));
    CHECK(ran == "a");
    CHECK(load.Done() == 1);
    CHECK_FALSE(load.Pump(0ms));
    CHECK(load.Pump(0ms));
    CHECK(ran == "abc");

    // Done stays done.
    CHECK(load.Pump(0ms));
    CHECK(ran == "abc");
}

TEST_CASE("A staged load runs every step in one pass when the budget allows")
{
    int ran = 0;
    Core::StagedLoad load({[&] { ++ran; }, [&] { ++ran; }, [&] { ++ran; }});
    CHECK(load.Pump(1h));
    CHECK(ran == 3);
    CHECK(load.Done() == load.Total());
}
