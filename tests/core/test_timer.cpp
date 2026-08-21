#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <thread>
#include "Timer.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("CTimer reports elapsed time in milliseconds", "[core][timer]")
{
    CTimer timer;

    SECTION("newly created timer reports near-zero elapsed time")
    {
        double elapsed = timer.GetTimeElapsed();
        REQUIRE_THAT(elapsed, WithinAbs(0.0, 5.0));
    }

    SECTION("elapsed time increases after a short sleep")
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double elapsed = timer.GetTimeElapsed();
        REQUIRE(elapsed >= 40.0);
    }

    SECTION("ResetTimer resets elapsed but not absolute time")
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double absBeforeReset = timer.GetAbsTime();
        timer.ResetTimer();
        double elapsedAfterReset = timer.GetTimeElapsed();
        double absAfterReset = timer.GetAbsTime();

        REQUIRE_THAT(elapsedAfterReset, WithinAbs(0.0, 5.0));
        REQUIRE(absAfterReset >= absBeforeReset);
    }
}

TEST_CASE("CTimer2 delay-based timer", "[core][timer]")
{
    CTimer2 timer;

    SECTION("SetTimer stores delay and GetDelay returns it")
    {
        timer.SetTimer(500);
        REQUIRE(timer.GetDelay() == 500);
    }

    SECTION("zero delay means IsTime is immediately true")
    {
        timer.SetTimer(0);
        timer.UpdateTime();
        REQUIRE(timer.IsTime());
    }

    SECTION("IsTime is false before delay elapses")
    {
        timer.SetTimer(1000);
        timer.UpdateTime();
        REQUIRE_FALSE(timer.IsTime());
    }
}
