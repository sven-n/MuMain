// Story 7.2.1: Frame Time Instrumentation — ATDD Acceptance Tests
// Flow Code: VS0-QUAL-FRAMETIMER
//
// GREEN PHASE: All tests PASS. mu::MuTimer is implemented in
//              MuMain/src/source/Core/MuTimer.h and MuTimer.cpp.
//
// AC coverage:
//   AC-1       — mu::MuTimer class exists with FrameStart/FrameEnd/GetFrameTimeMs/GetFPS/Reset
//   AC-2       — uses std::chrono::steady_clock exclusively (no Win32 timing APIs)
//   AC-4       — GetFPS() returns rolling average FPS over last N frames (default 60)
//   AC-STD-2   — doctest tests: accuracy, hitch detection, FPS average, reset

#include "doctest.h"
#include <chrono>
#include <thread>

#include "Core/Time/MuTimer.h"

// ---------------------------------------------------------------------------
// AC-1 / AC-STD-2: Frame time accuracy
// Measures a ~50ms sleep via FrameStart()/FrameEnd() and verifies
// GetFrameTimeMs() returns a value in the expected range.
// ---------------------------------------------------------------------------

TEST_CASE("AC-1/AC-STD-2 [7-2-1]: MuTimer measures frame time accurately [core][mu_timer][7-2-1]")
{
    // VS0-QUAL-FRAMETIMER
    mu::MuTimer timer;

    auto start = std::chrono::steady_clock::now();
    timer.FrameStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer.FrameEnd();
    auto elapsed = std::chrono::steady_clock::now() - start;

    // Scheduler delays affect both measurements, so compare against the same
    // steady-clock interval instead of imposing a runner-sensitive upper cap.
    double frameMs = timer.GetFrameTimeMs();
    double elapsedMs = std::chrono::duration<double, std::milli>(elapsed).count();
    REQUIRE(frameMs >= 40.0);
    REQUIRE(frameMs <= elapsedMs);
    REQUIRE(elapsedMs - frameMs < 5.0);
}

// ---------------------------------------------------------------------------
// AC-STD-2 / AC-3: Hitch detection
// Simulate one frame exceeding the 50ms hitch threshold and verify
// GetHitchCount() increments.
// ---------------------------------------------------------------------------

TEST_CASE("AC-STD-2/AC-3 [7-2-1]: MuTimer detects hitches above 50ms [core][mu_timer][7-2-1]")
{
    // VS0-QUAL-FRAMETIMER
    mu::MuTimer timer;

    // Run 3 fast frames (no hitch)
    for (int i = 0; i < 3; ++i)
    {
        timer.FrameStart();
        timer.FrameEnd();
    }

    uint64_t hitchesBefore = timer.GetHitchCount();
    REQUIRE(hitchesBefore == 0);

    // Simulate one slow frame (> 50ms hitch threshold)
    timer.FrameStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    timer.FrameEnd();

    uint64_t hitchesAfter = timer.GetHitchCount();
    REQUIRE(hitchesAfter == 1);
}

// ---------------------------------------------------------------------------
// AC-4 / AC-STD-2: FPS rolling average
// Run 10 rapid FrameStart/FrameEnd cycles and verify GetFPS() is positive.
// ---------------------------------------------------------------------------

TEST_CASE("AC-4/AC-STD-2 [7-2-1]: MuTimer GetFPS returns positive value after frames [core][mu_timer][7-2-1]")
{
    // VS0-QUAL-FRAMETIMER
    mu::MuTimer timer;

    // Run 10 rapid frames (sub-ms, but non-zero duration)
    for (int i = 0; i < 10; ++i)
    {
        timer.FrameStart();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        timer.FrameEnd();
    }

    REQUIRE(timer.GetFPS() > 0.0);
}

// ---------------------------------------------------------------------------
// AC-STD-2 / AC-1: Reset clears all state
// After running several frames, Reset() must zero all stats.
// ---------------------------------------------------------------------------

TEST_CASE("AC-STD-2/AC-1 [7-2-1]: MuTimer Reset clears all state [core][mu_timer][7-2-1]")
{
    // VS0-QUAL-FRAMETIMER
    mu::MuTimer timer;

    // Run a hitch frame to accumulate non-zero stats
    timer.FrameStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    timer.FrameEnd();

    REQUIRE(timer.GetHitchCount() == 1);
    REQUIRE(timer.GetFrameTimeMs() > 0.0);

    // Reset must zero all counters and buffers
    timer.Reset();

    REQUIRE(timer.GetFrameTimeMs() == 0.0);
    REQUIRE(timer.GetFPS() == 0.0);
    REQUIRE(timer.GetHitchCount() == 0);
}
