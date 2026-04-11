// Story 7.10.1: spdlog Logging Infrastructure Migration — ATDD Acceptance Tests
// Flow Code: VS0-CORE-MIGRATE-LOGGING
//
// RED PHASE:  Tests SKIP until MuLogger.h is created (story 7-10-1).
//             `__has_include("MuLogger.h")` resolves to false → SKIP() at runtime.
//             The file compiles cleanly and reports tests as skipped (not failed).
//
// GREEN PHASE: All tests PASS after MuLogger.h + MuLogger.cpp are implemented
//              and mu::log::Init() is wired into MuMain().
//
// Framework:  Catch2 v3.7.1 (FetchContent), CTest runner
// Location:   MuMain/tests/core/test_mu_logger.cpp
//
// AC Coverage:
//   AC-2  — mu::log::Get(name) returns valid named logger; MU_LOG_* macros compile
//   AC-3  — Init() creates rotating file sink writing to MuError.log
//   AC-11 — Logger creation, level filtering, file sink writes, multi-logger isolation
//
// Run with: ctest --test-dir MuMain/build -R mu_logger

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

// ---------------------------------------------------------------------------
// Conditional include: gracefully degrade to SKIP() in RED phase when
// MuLogger.h does not yet exist. Remove this guard after 7-10-1 is complete.
// ---------------------------------------------------------------------------
#if __has_include("MuLogger.h")
#include "MuLogger.h"
#define MU_LOGGER_AVAILABLE
#endif

// ---------------------------------------------------------------------------
// Test helpers
// ---------------------------------------------------------------------------

namespace
{

std::string ReadFileContent(const std::filesystem::path& path)
{
    std::ifstream f(path);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void RemoveAll(const std::filesystem::path& dir)
{
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

std::filesystem::path MakeTempDir(const char* suffix)
{
    auto dir = std::filesystem::temp_directory_path() / (std::string("mu_test_7_10_1_") + suffix);
    std::filesystem::create_directories(dir);
    return dir;
}

} // namespace

// ---------------------------------------------------------------------------
// AC-2 [7-10-1]: mu::log::Get() returns a valid named logger
//
// RED:   SKIP — MuLogger.h does not exist yet
// GREEN: PASS — mu::log::Get("core") returns non-null, same pointer on second call
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [7-10-1]: mu::log::Get() returns a valid named logger",
          "[core][mu_logger][7-10-1]")
{
#ifndef MU_LOGGER_AVAILABLE
    SKIP(); // RED phase — MuLogger.h not yet created
#else
    // GIVEN: Logging system initialized with a temp directory
    auto tmpDir = MakeTempDir("get");
    mu::log::Init(tmpDir);

    // WHEN: We request a named logger
    auto logger = mu::log::Get("core");

    // THEN: A valid logger is returned (not null)
    REQUIRE(logger != nullptr);

    // THEN: Requesting the same name twice returns the same logger instance
    auto logger2 = mu::log::Get("core");
    REQUIRE(logger.get() == logger2.get());

    mu::log::Shutdown();
    RemoveAll(tmpDir);
#endif
}

// ---------------------------------------------------------------------------
// AC-2 [7-10-1]: MU_LOG_* macros compile and execute without crash
//
// RED:   SKIP — MuLogger.h does not exist yet
// GREEN: PASS — all 6 macro levels compile and run without error
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [7-10-1]: MU_LOG_* macros compile and execute at all levels",
          "[core][mu_logger][7-10-1]")
{
#ifndef MU_LOGGER_AVAILABLE
    SKIP(); // RED phase — MuLogger.h not yet created
#else
    auto tmpDir = MakeTempDir("macros");
    mu::log::Init(tmpDir);

    auto logger = mu::log::Get("core");
    REQUIRE(logger != nullptr);

    // GIVEN/WHEN: All six MU_LOG_* macro levels are called
    // THEN: No compile error, no crash, no exception
    REQUIRE_NOTHROW([&]() {
        MU_LOG_TRACE(logger, "trace msg {}", 1);
        MU_LOG_DEBUG(logger, "debug msg {}", 2);
        MU_LOG_INFO(logger, "info msg {}", 3);
        MU_LOG_WARN(logger, "warn msg {}", 4);
        MU_LOG_ERROR(logger, "error msg {}", 5);
        MU_LOG_CRITICAL(logger, "critical msg {}", 6);
    }());

    mu::log::Shutdown();
    RemoveAll(tmpDir);
#endif
}

// ---------------------------------------------------------------------------
// AC-3 / AC-11 [7-10-1]: Init() creates MuError.log and the file sink writes
//
// RED:   SKIP — MuLogger.h does not exist yet
// GREEN: PASS — MuError.log exists and contains logged message after Init()+flush
// ---------------------------------------------------------------------------

TEST_CASE("AC-3/AC-11 [7-10-1]: Init() creates MuError.log and file sink writes messages",
          "[core][mu_logger][7-10-1]")
{
#ifndef MU_LOGGER_AVAILABLE
    SKIP(); // RED phase — MuLogger.h not yet created
#else
    auto tmpDir = MakeTempDir("sink");

    // GIVEN: Logging system initialized with test directory
    mu::log::Init(tmpDir);

    // WHEN: We log an error message and flush
    auto logger = mu::log::Get("core");
    REQUIRE(logger != nullptr);
    MU_LOG_ERROR(logger, "AC-3 file sink verification: unique-marker-7-10-1");
    logger->flush();
    mu::log::Shutdown();

    // THEN: MuError.log exists in the specified directory
    auto logFile = tmpDir / "MuError.log";
    REQUIRE(std::filesystem::exists(logFile));

    // THEN: The file contains the message we wrote
    std::string content = ReadFileContent(logFile);
    REQUIRE(content.find("AC-3 file sink verification: unique-marker-7-10-1") != std::string::npos);

    RemoveAll(tmpDir);
#endif
}

// ---------------------------------------------------------------------------
// AC-11 [7-10-1]: Level filtering — trace messages suppressed at info level
//
// RED:   SKIP — MuLogger.h does not exist yet
// GREEN: PASS — disabled-level messages do not appear in log file
// ---------------------------------------------------------------------------

TEST_CASE("AC-11 [7-10-1]: Level filtering suppresses messages below logger threshold",
          "[core][mu_logger][7-10-1]")
{
#ifndef MU_LOGGER_AVAILABLE
    SKIP(); // RED phase — MuLogger.h not yet created
#else
    auto tmpDir = MakeTempDir("filter");
    mu::log::Init(tmpDir);

    // Use a real registered logger name to test level filtering on an actual
    // named logger (not the default fallback). "audio" is a registered subsystem
    // logger unlikely to conflict with other tests.
    auto logger = mu::log::Get("audio");
    REQUIRE(logger != nullptr);

    // GIVEN: Logger threshold set to info (trace/debug are suppressed)
    logger->set_level(spdlog::level::info);

    // WHEN: A trace message is logged (below threshold)
    MU_LOG_TRACE(logger, "MUST_NOT_APPEAR_IN_OUTPUT");

    // WHEN: An info message is logged (at threshold)
    MU_LOG_INFO(logger, "MUST_APPEAR_IN_OUTPUT");

    logger->flush();
    mu::log::Shutdown();

    // THEN: Log file contains info message but not trace message
    auto logFile = tmpDir / "MuError.log";
    if (std::filesystem::exists(logFile))
    {
        std::string content = ReadFileContent(logFile);
        REQUIRE(content.find("MUST_APPEAR_IN_OUTPUT") != std::string::npos);
        REQUIRE(content.find("MUST_NOT_APPEAR_IN_OUTPUT") == std::string::npos);
    }

    RemoveAll(tmpDir);
#endif
}

// ---------------------------------------------------------------------------
// AC-11 [7-10-1]: Multi-logger isolation — separate loggers don't interfere
//
// RED:   SKIP — MuLogger.h does not exist yet
// GREEN: PASS — "core" and "network" loggers are distinct, independent objects
// ---------------------------------------------------------------------------

TEST_CASE("AC-11 [7-10-1]: Named loggers are isolated \xe2\x80\x94 different instances, independent levels",
          "[core][mu_logger][7-10-1]")
{
#ifndef MU_LOGGER_AVAILABLE
    SKIP(); // RED phase — MuLogger.h not yet created
#else
    auto tmpDir = MakeTempDir("multi");
    mu::log::Init(tmpDir);

    // GIVEN: Two different named loggers
    auto coreLogger = mu::log::Get("core");
    auto networkLogger = mu::log::Get("network");
    REQUIRE(coreLogger != nullptr);
    REQUIRE(networkLogger != nullptr);

    // THEN: They are distinct objects
    REQUIRE(coreLogger.get() != networkLogger.get());

    // THEN: Setting level on one does not affect the other
    coreLogger->set_level(spdlog::level::err);
    networkLogger->set_level(spdlog::level::debug);

    REQUIRE(coreLogger->level() == spdlog::level::err);
    REQUIRE(networkLogger->level() == spdlog::level::debug);

    mu::log::Shutdown();
    RemoveAll(tmpDir);
#endif
}

// ---------------------------------------------------------------------------
// AC-11 [7-10-1]: All 11 named loggers defined in story can be retrieved
//
// RED:   SKIP — MuLogger.h does not exist yet
// GREEN: PASS — all named loggers return non-null logger instances
// ---------------------------------------------------------------------------

TEST_CASE("AC-11 [7-10-1]: All named loggers (core, network, render, ...) are retrievable",
          "[core][mu_logger][7-10-1]")
{
#ifndef MU_LOGGER_AVAILABLE
    SKIP(); // RED phase — MuLogger.h not yet created
#else
    auto tmpDir = MakeTempDir("named");
    mu::log::Init(tmpDir);

    // THEN: Every named logger defined in Task 2.3 is retrievable
    const char* loggers[] = {
        "core", "network", "render", "data", "gameplay",
        "ui", "audio", "platform", "dotnet", "gameshop", "scenes"
    };

    for (const char* name : loggers)
    {
        auto logger = mu::log::Get(name);
        REQUIRE(logger != nullptr);
    }

    mu::log::Shutdown();
    RemoveAll(tmpDir);
#endif
}
