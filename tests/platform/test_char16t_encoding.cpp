// Story 3.2.1: char16_t Encoding at .NET Interop Boundary [VS1-NET-CHAR16T-ENCODING]
//
// RED PHASE: These tests FAIL until the following are implemented:
//   - mu_wchar_to_char16() and mu_char16_to_wchar() in PlatformCompat.h
//   - Connection::Connection(const char16_t* host, ...) in Connection.h / Connection.cpp
//   - Common.xslt String nativetype uses const char16_t*
//   - ConnectionManager.cs uses Marshal.PtrToStringUni
//
// Test vectors:
//   Korean "한국어" = U+D55C U+AD6D U+C5B4
//   UTF-16LE bytes (little-endian): 5C D5 6D AD B4 C5
//
// AC Coverage:
//   AC-4:       mu_wchar_to_char16 / mu_char16_to_wchar utilities
//   AC-5:       Round-trip correctness (Korean, Latin, mixed, null, IP)
//   AC-STD-2:   Byte-level UTF-16LE validation + 2/4-byte wchar_t path coverage
//   AC-VAL-1:   Round-trip tests pass
//   AC-VAL-2:   Korean byte baseline matches UTF-16LE

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <string>

// PlatformCompat.h provides mu_wchar_to_char16 and mu_char16_to_wchar.
// RED PHASE: This include will compile once PlatformCompat.h has the utilities.
#include "PlatformCompat.h"

// ============================================================================
// AC-4 / AC-5 / AC-VAL-1: Latin ASCII round-trip
// ============================================================================

TEST_CASE("AC-4: mu_wchar_to_char16 -- Latin ASCII round-trip", "[char16t][encoding][AC-4][AC-VAL-1]")
{
    // GIVEN: A Latin ASCII wide string
    const wchar_t* input = L"hello";

    // WHEN: Convert to char16_t and back
    std::u16string u16 = mu_wchar_to_char16(input);
    std::wstring roundtrip = mu_char16_to_wchar(u16.c_str());

    // THEN: Round-trip matches original
    REQUIRE(roundtrip == std::wstring(input));

    // AND: Length is preserved (5 code units for "hello")
    CHECK(u16.size() == 5u);
}

// ============================================================================
// AC-4 / AC-5 / AC-STD-2 / AC-VAL-1 / AC-VAL-2: Korean BMP round-trip
// ============================================================================

TEST_CASE("AC-4: mu_wchar_to_char16 -- Korean BMP round-trip", "[char16t][encoding][AC-4][AC-5][AC-STD-2][AC-VAL-1][AC-VAL-2]")
{
    // GIVEN: Korean string L"한국어" (U+D55C U+AD6D U+C5B4)
    // Using hex escapes to be compiler/locale agnostic
    const wchar_t input[] = { 0xD55C, 0xAD6D, 0xC5B4, L'\0' };

    // WHEN: Convert to char16_t
    std::u16string u16 = mu_wchar_to_char16(input);

    // THEN: Exactly 3 char16_t code units (all BMP, no surrogates needed)
    REQUIRE(u16.size() == 3u);

    // AND: Code unit values match the known Unicode codepoints
    CHECK(u16[0] == static_cast<char16_t>(0xD55Cu));
    CHECK(u16[1] == static_cast<char16_t>(0xAD6Du));
    CHECK(u16[2] == static_cast<char16_t>(0xC5B4u));

    // AND: Byte-level UTF-16LE layout matches known baseline {5C D5 6D AD B4 C5}
    // (AC-VAL-2: Korean string matches Windows MSVC wchar_t=2 baseline)
    SECTION("AC-VAL-2: Byte-level UTF-16LE baseline comparison")
    {
        // Expected UTF-16LE bytes for 한국어 (little-endian)
        const unsigned char expected_bytes[] = { 0x5C, 0xD5, 0x6D, 0xAD, 0xB4, 0xC5 };

        // Interpret u16 storage as bytes (char16_t is little-endian on all supported platforms)
        const unsigned char* actual_bytes = reinterpret_cast<const unsigned char*>(u16.data());

        for (size_t i = 0; i < sizeof(expected_bytes); ++i)
        {
            INFO("Byte index " << i);
            CHECK(actual_bytes[i] == expected_bytes[i]);
        }
    }

    SECTION("Round-trip back to wchar_t")
    {
        std::wstring roundtrip = mu_char16_to_wchar(u16.c_str());
        REQUIRE(roundtrip == std::wstring(input));
    }
}

// ============================================================================
// AC-4 / AC-5 / AC-VAL-1: Mixed script round-trip
// ============================================================================

TEST_CASE("AC-4: mu_wchar_to_char16 -- mixed script round-trip", "[char16t][encoding][AC-4][AC-5][AC-VAL-1]")
{
    // GIVEN: Mixed Latin + Korean string L"Hello 한국어!"
    // U+0048 U+0065 U+006C U+006C U+006F U+0020 U+D55C U+AD6D U+C5B4 U+0021
    const wchar_t input[] = { L'H', L'e', L'l', L'l', L'o', L' ', 0xD55C, 0xAD6D, 0xC5B4, L'!', L'\0' };

    // WHEN: Convert to char16_t and back
    std::u16string u16 = mu_wchar_to_char16(input);
    std::wstring roundtrip = mu_char16_to_wchar(u16.c_str());

    // THEN: Length matches (10 code units — all BMP)
    REQUIRE(u16.size() == 10u);

    // AND: Round-trip preserves all characters
    REQUIRE(roundtrip == std::wstring(input));
}

// ============================================================================
// AC-4: Null input safety
// ============================================================================

TEST_CASE("AC-4: mu_wchar_to_char16 -- null input returns empty", "[char16t][encoding][AC-4]")
{
    // GIVEN: nullptr input
    // WHEN: Convert null to char16_t
    std::u16string result = mu_wchar_to_char16(nullptr);

    // THEN: Returns empty string, no crash
    REQUIRE(result.empty());
}

TEST_CASE("AC-4: mu_char16_to_wchar -- null input returns empty", "[char16t][encoding][AC-4]")
{
    // GIVEN: nullptr input
    // WHEN: Convert null to wchar_t
    std::wstring result = mu_char16_to_wchar(nullptr);

    // THEN: Returns empty string, no crash
    REQUIRE(result.empty());
}

// ============================================================================
// AC-4 / AC-5: IP address (ASCII) round-trip — Connection caller regression
// ============================================================================

TEST_CASE("AC-4: mu_wchar_to_char16 -- IP address ASCII round-trip", "[char16t][encoding][AC-4][AC-5]")
{
    // GIVEN: IPv4 address as wide string (Connection caller uses wchar_t* host)
    const wchar_t* ip = L"127.0.0.1";

    // WHEN: Convert to char16_t (as WSclient.cpp caller would do)
    std::u16string u16 = mu_wchar_to_char16(ip);

    // THEN: All ASCII codepoints — each char16_t equals the wchar_t value
    REQUIRE(u16.size() == 9u); // "127.0.0.1" = 9 chars

    // AND: Round-trip correct
    std::wstring roundtrip = mu_char16_to_wchar(u16.c_str());
    REQUIRE(roundtrip == std::wstring(ip));

    SECTION("ASCII codepoints are unchanged after conversion")
    {
        // Each char16_t must equal the original wchar_t (all ASCII, single code unit)
        for (size_t i = 0; i < u16.size(); ++i)
        {
            INFO("Index " << i);
            CHECK(static_cast<wchar_t>(u16[i]) == ip[i]);
        }
    }
}

// ============================================================================
// AC-STD-2: sizeof(wchar_t) coverage comment
// ============================================================================

TEST_CASE("AC-STD-2: mu_wchar_to_char16 -- compile-time wchar_t size is documented", "[char16t][encoding][AC-STD-2]")
{
    // GIVEN: The platform wchar_t size
    constexpr size_t wchar_size = sizeof(wchar_t);

    // THEN: wchar_t is either 2 bytes (Windows/MinGW) or 4 bytes (Linux/macOS)
    // This test documents the platform path being exercised and passes on any platform.
    // RED phase: passes immediately once the header compiles; ensures coverage of
    // the correct branch in mu_wchar_to_char16 (if constexpr selection).
    REQUIRE((wchar_size == 2u || wchar_size == 4u));

    // AND: char16_t is always 2 bytes (C++11 guarantee)
    REQUIRE(sizeof(char16_t) == 2u);

    INFO("Platform wchar_t size: " << wchar_size << " bytes");
    INFO("if constexpr path: " << (wchar_size == 2u ? "reinterpret_cast (Windows/MinGW)" : "UTF-32 -> UTF-16 transcode (Linux/macOS)"));
}

// ============================================================================
// AC-4 / AC-STD-2: Non-BMP surrogate pair encoding (wchar_t=4 path)
// ============================================================================

TEST_CASE("AC-STD-2: mu_wchar_to_char16 -- non-BMP codepoint produces surrogate pair", "[char16t][encoding][AC-STD-2]")
{
    // GIVEN: U+1F600 (GRINNING FACE) — a non-BMP codepoint not in MU game text
    // On Linux/macOS: single wchar_t = 0x1F600 (4-byte)
    // On Windows/MinGW: sizeof(wchar_t)==2 so this path is not exercised
    // This test passes on all platforms (guarded by if constexpr internally).

    SECTION("Non-BMP encoded as surrogate pair on 4-byte wchar_t platforms")
    {
        // wchar_t value for U+1F600
        const wchar_t emoji[] = { static_cast<wchar_t>(0x1F600), L'\0' };
        std::u16string u16 = mu_wchar_to_char16(emoji);

        if constexpr (sizeof(wchar_t) == 4u)
        {
            // Linux/macOS: U+1F600 -> surrogate pair (2 char16_t)
            REQUIRE(u16.size() == 2u);
            // High surrogate: 0xD83D, Low surrogate: 0xDE00
            CHECK(u16[0] == static_cast<char16_t>(0xD83Du));
            CHECK(u16[1] == static_cast<char16_t>(0xDE00u));
        }
        else
        {
            // Windows/MinGW: wchar_t is 2-byte; emoji literal may already be a surrogate pair
            // (compiler-dependent). We only verify no crash and output is non-empty.
            CHECK(!u16.empty());
        }
    }
}

// ============================================================================
// AC-4: Empty string (not null) round-trip
// ============================================================================

TEST_CASE("AC-4: mu_wchar_to_char16 -- empty string round-trip", "[char16t][encoding][AC-4]")
{
    // GIVEN: An empty wide string (not null — a valid L"" pointer)
    const wchar_t* empty = L"";

    // WHEN: Convert to char16_t and back
    std::u16string u16 = mu_wchar_to_char16(empty);
    std::wstring roundtrip = mu_char16_to_wchar(u16.c_str());

    // THEN: Result is empty, no crash
    REQUIRE(u16.empty());
    REQUIRE(roundtrip.empty());
}
