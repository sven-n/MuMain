// Story 7.10.1: Logging Migration Build-Time Regression Tests
// Flow Code: VS0-CORE-MIGRATE-LOGGING
//
// Purpose: Verify that old logging infrastructure is fully deleted after migration.
//          Uses __has_include to check for the *presence* of headers that must be
//          ABSENT in GREEN phase — if found, the test FAILs with a clear message.
//
// RED PHASE:  AC-9 tests FAIL because ErrorReport.h and muConsoleDebug.h still exist.
//             AC-1 test SKIPS because spdlog FetchContent is not yet added.
//
// GREEN PHASE: All tests PASS after migration is complete:
//             - ErrorReport.h / muConsoleDebug.h are deleted
//             - spdlog is linked via FetchContent
//
// Framework:  Catch2 v3.7.1 (FetchContent), CTest runner
// Location:   MuMain/tests/core/test_logging_migration_7_10_1.cpp
//
// AC Coverage:
//   AC-1  — spdlog FetchContent integration (build compiles, header available)
//   AC-9  — Old logging headers deleted (ErrorReport.h, muConsoleDebug.h gone)

#include <catch2/catch_test_macros.hpp>

// ---------------------------------------------------------------------------
// spdlog availability (AC-1) — only defined after FetchContent adds spdlog
// ---------------------------------------------------------------------------
#if __has_include(<spdlog/spdlog.h>)
#include <spdlog/spdlog.h>
#define SPDLOG_INTEGRATED
#endif

// ---------------------------------------------------------------------------
// AC-1 [7-10-1]: spdlog is integrated via FetchContent
//
// RED:   SKIP — spdlog FetchContent not yet added to src/CMakeLists.txt
// GREEN: PASS — spdlog major version is 1 (v1.15.3 as specified in story)
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [7-10-1]: spdlog is integrated via FetchContent and version is 1.x",
          "[core][mu_logger][7-10-1][build]")
{
#ifndef SPDLOG_INTEGRATED
    SKIP(); // RED phase — spdlog FetchContent not yet added
#else
    // GIVEN: spdlog headers are available
    // THEN: Version major is 1 (v1.15.3 as required by story)
    REQUIRE(SPDLOG_VER_MAJOR == 1);

    // THEN: Version is at least 1.15.x
    REQUIRE(SPDLOG_VER_MINOR >= 15);
#endif
}

// ---------------------------------------------------------------------------
// AC-9 [7-10-1]: ErrorReport.h must NOT exist after migration
//
// RED:   FAIL — ErrorReport.h still exists (migration not complete)
// GREEN: PASS — ErrorReport.h has been deleted
//
// Note: __has_include resolves using the same include paths as normal #include.
//       This will find ErrorReport.h if the Core/ directory is on the include path.
// ---------------------------------------------------------------------------

TEST_CASE("AC-9 [7-10-1]: ErrorReport.h is deleted \xe2\x80\x94 CErrorReport removed from codebase",
          "[core][mu_logger][7-10-1][regression]")
{
#if __has_include("ErrorReport.h")
    // Header still exists — old logging infrastructure not yet deleted
    FAIL(
        "RED PHASE: ErrorReport.h still exists. "
        "Complete Task 7.1 (delete ErrorReport.h/cpp) before marking AC-9 done.");
#else
    // ErrorReport.h is gone — AC-9 satisfied for CErrorReport deletion
    SUCCEED();
#endif
}

// ---------------------------------------------------------------------------
// AC-9 [7-10-1]: muConsoleDebug.h must NOT exist after migration
//
// RED:   FAIL — muConsoleDebug.h still exists (migration not complete)
// GREEN: PASS — muConsoleDebug.h has been deleted
// ---------------------------------------------------------------------------

TEST_CASE("AC-9 [7-10-1]: muConsoleDebug.h is deleted \xe2\x80\x94 CmuConsoleDebug removed",
          "[core][mu_logger][7-10-1][regression]")
{
#if __has_include("muConsoleDebug.h")
    FAIL(
        "RED PHASE: muConsoleDebug.h still exists. "
        "Complete Task 7.2 (delete muConsoleDebug.h/cpp) before marking AC-9 done.");
#else
    SUCCEED();
#endif
}

// ---------------------------------------------------------------------------
// AC-9 [7-10-1]: MuLogger.h MUST exist after migration
//
// RED:   FAIL — MuLogger.h has not been created yet
// GREEN: PASS — MuLogger.h exists and is the new logging facade
// ---------------------------------------------------------------------------

TEST_CASE("AC-9/AC-2 [7-10-1]: MuLogger.h exists \xe2\x80\x94 new logging facade is in place",
          "[core][mu_logger][7-10-1][regression]")
{
#if __has_include("MuLogger.h")
    SUCCEED(); // MuLogger.h exists — new facade is in place
#else
    FAIL(
        "RED PHASE: MuLogger.h does not exist yet. "
        "Complete Task 2.1 (create Core/MuLogger.h) to satisfy AC-2 and AC-9.");
#endif
}
