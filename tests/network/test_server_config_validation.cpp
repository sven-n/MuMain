// Flow Code: VS1-NET-CONFIG-SERVER
// Story:     3.4.2 - Server Connection Configuration
// AC:        AC-4, AC-5, AC-STD-2, AC-STD-11
//
// Catch2 unit tests for GameConfig validation helpers.
// Tests validate boundary and edge-case behaviour required by AC-4 and AC-5.
// These tests do NOT require config.ini at test time — they call the free
// functions ValidateServerPort() and ValidateServerIP() directly.
//
// RED PHASE: All tests compile but FAIL until the following are created:
//   - Core/GameConfigValidation.h  — declares ValidateServerPort / ValidateServerIP
//   - Core/GameConfigValidation.cpp — defines them (auto-discovered by MU_CORE_SOURCES glob)
//
// Run: ctest --test-dir build -R "3.4.2"

#include <catch2/catch_test_macros.hpp>
#include <string>

#include "GameConfigValidation.h"

// =============================================================================
// AC-4: ValidateServerPort — boundary and invalid-value behaviour
// =============================================================================

TEST_CASE("AC-4: ValidateServerPort rejects invalid port values", "[network][server-config][ac-4]")
{
    SECTION("AC-4: Port 0 is invalid — returns default 44405")
    {
        // GIVEN: port value of 0 (at or below valid range)
        int value = 0;
        int defaultPort = 44405;

        // WHEN: validating the port
        int result = GameConfig::ValidateServerPort(value, defaultPort);

        // THEN: default is substituted
        REQUIRE(result == 44405);
    }

    SECTION("AC-4: Port 65536 is invalid — returns default 44405")
    {
        // GIVEN: port value above valid range
        int value = 65536;
        int defaultPort = 44405;

        // WHEN: validating the port
        int result = GameConfig::ValidateServerPort(value, defaultPort);

        // THEN: default is substituted
        REQUIRE(result == 44405);
    }

    SECTION("AC-4: Port 65535 is valid — returns 65535")
    {
        // GIVEN: maximum valid port
        int value = 65535;
        int defaultPort = 44405;

        // WHEN: validating the port
        int result = GameConfig::ValidateServerPort(value, defaultPort);

        // THEN: the value is preserved
        REQUIRE(result == 65535);
    }

    SECTION("AC-4: Port 44405 (normal OpenMU default) is valid — returns 44405")
    {
        // GIVEN: the OpenMU default port
        int value = 44405;
        int defaultPort = 44405;

        // WHEN: validating the port
        int result = GameConfig::ValidateServerPort(value, defaultPort);

        // THEN: the value is preserved
        REQUIRE(result == 44405);
    }

    SECTION("AC-4: Negative port is invalid — returns default")
    {
        // GIVEN: a negative port value
        int value = -1;
        int defaultPort = 44405;

        // WHEN: validating the port
        int result = GameConfig::ValidateServerPort(value, defaultPort);

        // THEN: default is substituted
        REQUIRE(result == 44405);
    }
}

// =============================================================================
// AC-5: ValidateServerIP — empty / whitespace-only / valid-value behaviour
// =============================================================================

TEST_CASE("AC-5: ValidateServerIP rejects empty and whitespace-only values", "[network][server-config][ac-5]")
{
    SECTION("AC-5: Empty string — returns default 'localhost'")
    {
        // GIVEN: an empty ServerIP string
        std::wstring value = L"";
        std::wstring defaultIP = L"localhost";

        // WHEN: validating the IP
        std::wstring result = GameConfig::ValidateServerIP(value, defaultIP);

        // THEN: default is substituted
        REQUIRE(result == L"localhost");
    }

    SECTION("AC-5: Whitespace-only string — returns default 'localhost'")
    {
        // GIVEN: a whitespace-only ServerIP string
        std::wstring value = L"   ";
        std::wstring defaultIP = L"localhost";

        // WHEN: validating the IP
        std::wstring result = GameConfig::ValidateServerIP(value, defaultIP);

        // THEN: default is substituted (whitespace trimmed → empty)
        REQUIRE(result == L"localhost");
    }

    SECTION("AC-5: Mixed whitespace (tabs and spaces) — returns default")
    {
        // GIVEN: a tab+space-only string
        std::wstring value = L"\t  \t";
        std::wstring defaultIP = L"localhost";

        // WHEN: validating the IP
        std::wstring result = GameConfig::ValidateServerIP(value, defaultIP);

        // THEN: default is substituted
        REQUIRE(result == L"localhost");
    }

    SECTION("AC-5: Valid IP address — returns trimmed value unchanged")
    {
        // GIVEN: a valid IPv4 address
        std::wstring value = L"192.168.1.1";
        std::wstring defaultIP = L"localhost";

        // WHEN: validating the IP
        std::wstring result = GameConfig::ValidateServerIP(value, defaultIP);

        // THEN: the value is preserved
        REQUIRE(result == L"192.168.1.1");
    }

    SECTION("AC-5: Hostname with surrounding whitespace — returns trimmed hostname")
    {
        // GIVEN: a hostname with leading/trailing whitespace
        std::wstring value = L"  game.server.example.com  ";
        std::wstring defaultIP = L"localhost";

        // WHEN: validating the IP
        std::wstring result = GameConfig::ValidateServerIP(value, defaultIP);

        // THEN: whitespace is trimmed and value is preserved
        REQUIRE(result == L"game.server.example.com");
    }

    SECTION("AC-5: 'localhost' value — returns 'localhost' unchanged")
    {
        // GIVEN: the literal default hostname
        std::wstring value = L"localhost";
        std::wstring defaultIP = L"localhost";

        // WHEN: validating the IP
        std::wstring result = GameConfig::ValidateServerIP(value, defaultIP);

        // THEN: the value is preserved
        REQUIRE(result == L"localhost");
    }
}

// =============================================================================
// AC-STD-11: Flow Code Traceability (verified separately by CMake script)
// =============================================================================
// Flow code VS1-NET-CONFIG-SERVER must appear:
//   - In GameConfig.cpp header comment (first 1000 chars)
//   - In this test file header (above) — verified by CMake script
//   - Run: cmake -P tests/build/test_ac_std11_flow_code_3_4_2.cmake
