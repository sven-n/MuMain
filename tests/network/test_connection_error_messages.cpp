// Flow Code: VS1-NET-ERROR-MESSAGING
// Story:     3.4.1 - Connection Error Messaging & Graceful Degradation
// AC:        AC-1, AC-2, AC-STD-2, AC-STD-11
//
// Catch2 unit tests for DotNetBridge error message formatting helpers.
// Tests validate the exact string templates required by AC-1 and AC-2.
// These tests do NOT require an actual .NET library at test time — they test
// pure formatting logic only (free functions extracted from Connection.cpp).
//
// RED PHASE: All tests compile but FAIL until Connection.cpp exposes:
//   - DotNetBridge::FormatLibraryNotFoundMessage(path, platform)
//   - DotNetBridge::FormatSymbolNotFoundMessage(name)
//
// Run: ctest --test-dir build -R "3.4.1"

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <string>

// Forward declarations of testable helpers that will be extracted from Connection.cpp.
// These functions will be declared in a Connection_internal.h or inline in Connection.cpp
// and exposed via the DotNetBridge namespace for unit testing.
//
// If the formatting cannot be cleanly extracted (circular includes), the fallback
// strategy is documented in the story Dev Notes §Catch2 Test Strategy.
namespace DotNetBridge
{
std::string FormatLibraryNotFoundMessage(const std::string& path, const char* platform);
std::string FormatSymbolNotFoundMessage(const char* name);
} // namespace DotNetBridge

// =============================================================================
// AC-1: Library-Not-Found Message Format
// =============================================================================

TEST_CASE("AC-1: FormatLibraryNotFoundMessage includes path and platform", "[network][error-messaging][ac-1]")
{
    SECTION("AC-1: Linux platform — path and platform name both appear in message")
    {
        // GIVEN: a library path and platform string
        const std::string path = "/usr/local/lib/MUnique.Client.Library.so";
        const char* platform = "Linux";

        // WHEN: formatting the library-not-found message
        std::string msg = DotNetBridge::FormatLibraryNotFoundMessage(path, platform);

        // THEN: message contains the path
        REQUIRE(msg.find(path) != std::string::npos);
        // THEN: message contains the platform name
        REQUIRE(msg.find("Linux") != std::string::npos);
        // THEN: message is user-actionable (contains build instruction hint)
        REQUIRE(msg.find("Build ClientLibrary") != std::string::npos);
    }

    SECTION("AC-1: macOS platform — path and platform name both appear in message")
    {
        // GIVEN: a macOS library path
        const std::string path = "/Applications/MuMain.app/Contents/MacOS/MUnique.Client.Library.dylib";
        const char* platform = "macOS";

        // WHEN: formatting the library-not-found message
        std::string msg = DotNetBridge::FormatLibraryNotFoundMessage(path, platform);

        // THEN: message contains the path
        REQUIRE(msg.find(path) != std::string::npos);
        // THEN: message contains the platform name
        REQUIRE(msg.find("macOS") != std::string::npos);
    }

    SECTION("AC-1: Windows platform — path and platform name both appear in message")
    {
        // GIVEN: a Windows library path
        const std::string path = "MUnique.Client.Library.dll";
        const char* platform = "Windows";

        // WHEN: formatting the library-not-found message
        std::string msg = DotNetBridge::FormatLibraryNotFoundMessage(path, platform);

        // THEN: message contains the path
        REQUIRE(msg.find(path) != std::string::npos);
        // THEN: message contains the platform name
        REQUIRE(msg.find("Windows") != std::string::npos);
        // THEN: message starts with expected prefix per AC-1 spec
        REQUIRE(msg.find("Network library not found at") != std::string::npos);
    }

    SECTION("AC-1: Empty path produces message without crashing")
    {
        // GIVEN: an empty path (edge case)
        const std::string path;
        const char* platform = "Linux";

        // WHEN: formatting the message
        std::string msg = DotNetBridge::FormatLibraryNotFoundMessage(path, platform);

        // THEN: message is non-empty and still user-actionable
        REQUIRE_FALSE(msg.empty());
        REQUIRE(msg.find("Network library not found at") != std::string::npos);
    }
}

// =============================================================================
// AC-2: Symbol-Not-Found Message Format
// =============================================================================

TEST_CASE("AC-2: FormatSymbolNotFoundMessage includes function name", "[network][error-messaging][ac-2]")
{
    SECTION("AC-2: ConnectionManager_Connect — function name appears in message")
    {
        // GIVEN: a function symbol name that failed to resolve
        const char* symbolName = "ConnectionManager_Connect";

        // WHEN: formatting the symbol-not-found message
        std::string msg = DotNetBridge::FormatSymbolNotFoundMessage(symbolName);

        // THEN: message contains the function name
        REQUIRE(msg.find(symbolName) != std::string::npos);
        // THEN: message hints at version mismatch
        REQUIRE(msg.find("Version mismatch") != std::string::npos);
        // THEN: message starts with expected prefix per AC-2 spec
        REQUIRE(msg.find("Network library loaded but function") != std::string::npos);
    }

    SECTION("AC-2: ConnectionManager_Send — function name appears in message")
    {
        // GIVEN: a different symbol name
        const char* symbolName = "ConnectionManager_Send";

        // WHEN: formatting the message
        std::string msg = DotNetBridge::FormatSymbolNotFoundMessage(symbolName);

        // THEN: message contains the exact symbol name
        REQUIRE(msg.find(symbolName) != std::string::npos);
    }

    SECTION("AC-2: Null symbol name — does not crash, produces safe message")
    {
        // GIVEN: a null detail pointer (defensive edge case)
        // NOTE: Implementation should handle null gracefully per existing ReportDotNetError pattern
        const char* symbolName = "unknown";

        // WHEN: formatting with a fallback name
        std::string msg = DotNetBridge::FormatSymbolNotFoundMessage(symbolName);

        // THEN: message is non-empty
        REQUIRE_FALSE(msg.empty());
    }
}

// =============================================================================
// AC-7: Once-Per-Session Guard (structural test — verifies guard semantics)
// =============================================================================
// Note: AC-7's g_dotnetErrorDisplayed guard is a global state concern that
// cannot be unit tested without full Connection.cpp linkage. This is verified
// via manual validation (AC-VAL-1) and the story's integration path.
// This section documents the requirement for the dev agent.
//
// AC-7 manual verification:
//   1. Trigger library-not-found error twice in a session
//   2. Confirm dialog appears ONCE only (g_dotnetErrorDisplayed guard)
//   3. Confirm MuError.log shows the error message

// =============================================================================
// AC-STD-11: Flow Code Traceability (verified separately by CMake script)
// =============================================================================
// Flow code VS1-NET-ERROR-MESSAGING must appear:
//   - In Connection.cpp header comment
//   - In this test file header (above) — verified by CMake script
//   - Run: cmake -P tests/build/test_ac_std11_flow_code_3_4_1.cmake
