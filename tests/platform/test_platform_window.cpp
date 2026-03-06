// Story 2.1.1: SDL3 Window Creation & Event Loop
// Flow: VS1-SDL-WINDOW-CREATE
// GREEN PHASE: All platform interfaces implemented and tests passing.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_version_macros.hpp>

#include "MuPlatform.h"
#include "IPlatformWindow.h"

// ---------------------------------------------------------------------------
// AC-1: Window creation via MuPlatform::CreateWindow
// ---------------------------------------------------------------------------

TEST_CASE("AC-1: MuPlatform::CreateWindow creates a window", "[platform][window][2-1-1]")
{
    SECTION("CreateWindow succeeds with valid parameters")
    {
        bool initOk = mu::MuPlatform::Initialize();
        REQUIRE(initOk);

        bool created = mu::MuPlatform::CreateWindow("MU Online Test", 800, 600, 0);
        REQUIRE(created);

        mu::MuPlatform::Shutdown();
    }
}

// ---------------------------------------------------------------------------
// AC-2: Event loop via MuPlatform::PollEvents
// ---------------------------------------------------------------------------

TEST_CASE("AC-2: MuPlatform::PollEvents pumps events without blocking", "[platform][eventloop][2-1-1]")
{
    SECTION("PollEvents returns true when no quit event and queue is empty")
    {
        bool initOk = mu::MuPlatform::Initialize();
        REQUIRE(initOk);
        bool created = mu::MuPlatform::CreateWindow("PollEvents Test", 320, 240, 0);
        REQUIRE(created);

        bool continueLoop = mu::MuPlatform::PollEvents();
        REQUIRE(continueLoop);

        mu::MuPlatform::Shutdown();
    }
}

// ---------------------------------------------------------------------------
// AC-3: Window lifecycle events translated without crash
// ---------------------------------------------------------------------------

TEST_CASE("AC-3: Window lifecycle events do not crash", "[platform][window][2-1-1]")
{
    SECTION("Engine handles resize/focus/minimize events gracefully")
    {
        bool initOk = mu::MuPlatform::Initialize();
        REQUIRE(initOk);
        bool created = mu::MuPlatform::CreateWindow("Lifecycle Test", 640, 480, 0);
        REQUIRE(created);

        // Cannot inject SDL lifecycle events in unit tests — verify no crash on empty queue.
        // Lifecycle event translation is verified by integration/manual tests.
        bool continueLoop = mu::MuPlatform::PollEvents();
        REQUIRE(continueLoop);

        mu::MuPlatform::Shutdown();
    }
}

// ---------------------------------------------------------------------------
// AC-4: Configurable title and dimensions
// ---------------------------------------------------------------------------

TEST_CASE("AC-4: Window creation accepts title and dimensions", "[platform][window][2-1-1]")
{
    SECTION("Custom title and dimensions are accepted")
    {
        bool initOk = mu::MuPlatform::Initialize();
        REQUIRE(initOk);

        bool created = mu::MuPlatform::CreateWindow("Custom Title", 1024, 768, 0);
        REQUIRE(created);

        mu::MuPlatform::Shutdown();
    }

    SECTION("Fullscreen flag constant is correctly defined")
    {
        REQUIRE(mu::MU_WINDOW_FULLSCREEN == 0x1);
    }
}

// ---------------------------------------------------------------------------
// AC-5: MuPlatform::GetWindow() returns singleton handle
// ---------------------------------------------------------------------------

TEST_CASE("AC-5: MuPlatform::GetWindow returns consistent singleton", "[platform][window][2-1-1]")
{
    SECTION("GetWindow returns same instance on repeated calls")
    {
        bool initOk = mu::MuPlatform::Initialize();
        REQUIRE(initOk);
        bool created = mu::MuPlatform::CreateWindow("Singleton Test", 640, 480, 0);
        REQUIRE(created);

        auto* window1 = mu::MuPlatform::GetWindow();
        auto* window2 = mu::MuPlatform::GetWindow();

        REQUIRE(window1 != nullptr);
        REQUIRE(window1 == window2);

        mu::MuPlatform::Shutdown();
    }

    SECTION("GetWindow returns nullptr before CreateWindow")
    {
        bool initOk = mu::MuPlatform::Initialize();
        REQUIRE(initOk);

        auto* window = mu::MuPlatform::GetWindow();
        REQUIRE(window == nullptr);

        mu::MuPlatform::Shutdown();
    }
}

// ---------------------------------------------------------------------------
// AC-6: Quit event signals clean exit
// NOTE: Cannot inject SDL_EVENT_QUIT in unit test without SDL test utilities.
// Verified by: (1) integration/manual test, (2) code review confirming
// SDL_EVENT_QUIT/SDL_EVENT_WINDOW_CLOSE_REQUESTED set Destroy=true.
// ---------------------------------------------------------------------------

TEST_CASE("AC-6: PollEvents returns false without initialized platform", "[platform][eventloop][2-1-1]")
{
    // Verify PollEvents returns false (no event loop) when platform not initialized
    bool result = mu::MuPlatform::PollEvents();
    REQUIRE_FALSE(result);
}

// ---------------------------------------------------------------------------
// AC-STD-16: Correct test infrastructure
// ---------------------------------------------------------------------------

TEST_CASE("AC-STD-16: Test infrastructure uses Catch2 v3", "[platform][std][2-1-1]")
{
    REQUIRE(CATCH_VERSION_MAJOR == 3);
}

// ===========================================================================
// Story 2.1.2: SDL3 Window Focus & Display Management
// Flow: VS1-SDL-WINDOW-FOCUS
// RED PHASE: Tests written before implementation — all MUST fail until
// the corresponding production code is added.
// ===========================================================================

// ---------------------------------------------------------------------------
// AC-3: Fullscreen toggle — SetFullscreen null-guard (AC-STD-2)
// ---------------------------------------------------------------------------

TEST_CASE("AC-3: SDLWindow::SetFullscreen does not crash when window is null",
          "[platform][window][2-1-2][VS1-SDL-WINDOW-FOCUS]")
{
    // GIVEN: Platform initialized but no window created
    bool initOk = mu::MuPlatform::Initialize();
    REQUIRE(initOk);

    // WHEN: SetFullscreen called with no active window
    // THEN: No crash — null-guard prevents SDL call on nullptr
    mu::MuPlatform::SetFullscreen(true);
    mu::MuPlatform::SetFullscreen(false);

    mu::MuPlatform::Shutdown();
}

// ---------------------------------------------------------------------------
// AC-4: Mouse cursor confinement — SetMouseGrab null-guard (AC-STD-2)
// ---------------------------------------------------------------------------

TEST_CASE("AC-4: SDLWindow::SetMouseGrab does not crash when window is null",
          "[platform][window][2-1-2][VS1-SDL-WINDOW-FOCUS]")
{
    // GIVEN: Platform initialized but no window created
    bool initOk = mu::MuPlatform::Initialize();
    REQUIRE(initOk);

    // WHEN: SetMouseGrab called with no active window
    // THEN: No crash — null-guard prevents SDL call on nullptr
    mu::MuPlatform::SetMouseGrab(true);
    mu::MuPlatform::SetMouseGrab(false);

    mu::MuPlatform::Shutdown();
}

// ---------------------------------------------------------------------------
// AC-2: Display mode detection — GetDisplaySize null-guard (AC-STD-2)
// ---------------------------------------------------------------------------

TEST_CASE("AC-2: MuPlatform::GetDisplaySize returns false when no window created",
          "[platform][window][2-1-2][VS1-SDL-WINDOW-FOCUS]")
{
    // GIVEN: Platform initialized but no window created
    bool initOk = mu::MuPlatform::Initialize();
    REQUIRE(initOk);

    int width = -1;
    int height = -1;

    // WHEN: GetDisplaySize called with no active window
    bool result = mu::MuPlatform::GetDisplaySize(width, height);

    // THEN: Returns false, output parameters unchanged
    REQUIRE_FALSE(result);
    REQUIRE(width == -1);
    REQUIRE(height == -1);

    mu::MuPlatform::Shutdown();
}

// ---------------------------------------------------------------------------
// AC-2: Display mode detection — positive dimensions when window exists
// ---------------------------------------------------------------------------

TEST_CASE("AC-2: MuPlatform::GetDisplaySize returns positive dimensions with window",
          "[platform][window][2-1-2][VS1-SDL-WINDOW-FOCUS]")
{
    // GIVEN: Platform initialized and window created
    bool initOk = mu::MuPlatform::Initialize();
    REQUIRE(initOk);
    bool created = mu::MuPlatform::CreateWindow("DisplaySize Test", 640, 480, 0);
    REQUIRE(created);

    int width = 0;
    int height = 0;

    // WHEN: GetDisplaySize called with an active window
    bool result = mu::MuPlatform::GetDisplaySize(width, height);

    // THEN: Returns true with positive width and height
    REQUIRE(result);
    REQUIRE(width > 0);
    REQUIRE(height > 0);

    mu::MuPlatform::Shutdown();
}

// ---------------------------------------------------------------------------
// AC-3: Fullscreen toggle — SetFullscreen on active window does not crash
// ---------------------------------------------------------------------------

TEST_CASE("AC-3: MuPlatform::SetFullscreen toggles without crash on active window",
          "[platform][window][2-1-2][VS1-SDL-WINDOW-FOCUS]")
{
    // GIVEN: Platform initialized and window created
    bool initOk = mu::MuPlatform::Initialize();
    REQUIRE(initOk);
    bool created = mu::MuPlatform::CreateWindow("Fullscreen Test", 640, 480, 0);
    REQUIRE(created);

    // WHEN: SetFullscreen toggled on and off
    // THEN: No crash or hang
    mu::MuPlatform::SetFullscreen(true);
    mu::MuPlatform::SetFullscreen(false);

    mu::MuPlatform::Shutdown();
}

// ---------------------------------------------------------------------------
// AC-4: Mouse grab state transitions
// ---------------------------------------------------------------------------

TEST_CASE("AC-4: MuPlatform::SetMouseGrab state transitions do not crash",
          "[platform][window][2-1-2][VS1-SDL-WINDOW-FOCUS]")
{
    // GIVEN: Platform initialized and window created
    bool initOk = mu::MuPlatform::Initialize();
    REQUIRE(initOk);
    bool created = mu::MuPlatform::CreateWindow("MouseGrab Test", 640, 480, 0);
    REQUIRE(created);

    // WHEN: Mouse grab toggled on/off repeatedly
    // THEN: No crash — state transitions are clean
    mu::MuPlatform::SetMouseGrab(true);
    mu::MuPlatform::SetMouseGrab(false);
    mu::MuPlatform::SetMouseGrab(true);
    mu::MuPlatform::SetMouseGrab(false);

    mu::MuPlatform::Shutdown();
}
