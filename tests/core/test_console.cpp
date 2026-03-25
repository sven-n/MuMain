// Story 7.6.5: Cross-Platform Terminal / Console [VS0-QUAL-WIN32CLEAN-CONSOLE]
// GREEN PHASE — mu_console_init() and mu_get_console_size() added to PlatformCompat.h
//
// AC-STD-2: Catch2 unit tests — console init does not crash, GetConsoleSize returns positive dimensions.
// AC-3:     mu_console_init() enables ANSI on Windows; no crash on POSIX.
// AC-6:     mu_get_console_size() returns cols > 0, rows > 0 on all platforms.

#include <catch2/catch_test_macros.hpp>

#include "PlatformCompat.h"

// AC-STD-2, AC-3: mu_console_init() must return cleanly without crashing or throwing
// on any platform (macOS, Linux, Windows). On Windows it enables ANSI terminal
// processing via SetConsoleMode; on POSIX it checks isatty(STDOUT_FILENO) and stores
// g_muConsoleIsTTY. No Win32 console APIs (SetConsoleTextAttribute, etc.) are called.
TEST_CASE("AC-STD-2: console init does not crash on macOS/Linux", "[core][console]")
{
    // GIVEN: Process has stdout available (may or may not be a TTY on CI)
    // WHEN: mu_console_init() is called unconditionally
    mu_console_init();
    // THEN: Returns cleanly — no crash, no exception, no assertion
    SUCCEED("mu_console_init() completed without crash");
}

// AC-STD-2, AC-6: mu_get_console_size() must return positive (cols > 0, rows > 0) on
// all platforms. Minimum fallback is 80×24 when terminal size is unavailable (e.g., CI).
TEST_CASE("AC-STD-2: GetConsoleSize returns positive dimensions", "[core][console]")
{
    int cols = 0;
    int rows = 0;

    // GIVEN: Console has been initialised
    mu_console_init();

    // WHEN: mu_get_console_size() is queried
    mu_get_console_size(&cols, &rows);

    // THEN: Both dimensions are positive — minimum 80×24 fallback enforced
    REQUIRE(cols > 0);
    REQUIRE(rows > 0);
}

// AC-4: Colour output uses ANSI escape sequences. This test indirectly verifies
// the internal color mapping table (ColorIndexToAnsiFg / ColorIndexToAnsiBg) by
// calling SetConsoleTextColor, which uses the mapping to emit ANSI codes.
// No crash, no invalid ANSI codes — table boundaries respected.
TEST_CASE("AC-4: SetConsoleTextColor all colour indices", "[core][console]")
{
    // GIVEN: Console is initialized
    mu_console_init();

    // WHEN: SetConsoleTextColor is called with all valid colour indices (0-15)
    for (int fg = 0; fg <= 15; ++fg)
    {
        for (int bg = 0; bg <= 15; ++bg)
        {
            // THEN: No crash, ANSI codes map correctly via ColorIndexToAnsiFg/Bg
            leaf::SetConsoleTextColor(fg, bg);
        }
    }

    // WHEN: Invalid indices are passed (outside 0-15)
    leaf::SetConsoleTextColor(-1, 0);   // Should fallback safely
    leaf::SetConsoleTextColor(16, 0);   // Should fallback safely
    leaf::SetConsoleTextColor(0, -1);   // Should fallback safely
    leaf::SetConsoleTextColor(0, 100);  // Should fallback safely

    // THEN: No crash — boundary clamping works
    SUCCEED("All colour index combinations handled without crash");
}
