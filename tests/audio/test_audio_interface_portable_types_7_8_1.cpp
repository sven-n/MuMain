// Story 7.8.1: Audio Interface Win32 Type Cleanup [VS0-QUAL-BUILD-AUDIO]
// RED PHASE: Tests verify that IPlatformAudio and MiniAudioBackend use portable C++ types
// instead of Win32 types (HRESULT, BOOL, OBJECT*).
//
// Primary indicator test (AC-2):
//   PlaySound() must return bool (sizeof == 1), NOT HRESULT (= long, sizeof == 4 or 8).
//   In RED phase: PlaySound returns HRESULT (long), sizeof(result) > 1 → TEST FAILS.
//   In GREEN phase: PlaySound returns bool, sizeof(result) == 1 → TEST PASSES.
//
// Secondary tests verify parameter and interface continuity after type replacement.
//
// Tests compile on macOS/Linux/Windows (MinGW CI) in BOTH phases:
//   - nullptr is compatible with both OBJECT* and void*
//   - false is compatible with both BOOL=int and bool
//   - auto deduces return type; sizeof works on any type
//
// No audio device required. No Win32 APIs used in this test translation unit.
// Framework: Catch2 v3.7.1 — REQUIRE/CHECK macros, TEST_CASE/SECTION structure.

#include <catch2/catch_test_macros.hpp>
#include <type_traits>

#include "IPlatformAudio.h"
#include "MiniAudioBackend.h"

// ---------------------------------------------------------------------------
// AC-2 PRIMARY: PlaySound return type must be bool (not HRESULT)
//
// sizeof check distinguishes HRESULT (= long: 4 bytes on Windows, 8 bytes on 64-bit
// macOS/Linux) from bool (1 byte on all platforms).
//
// RED PHASE FAILURE: auto deduces HRESULT (long); sizeof(long) != sizeof(bool)
// GREEN PHASE PASS:  auto deduces bool;   sizeof(bool) == sizeof(bool) == 1
// ---------------------------------------------------------------------------
TEST_CASE("AC-2: IPlatformAudio::PlaySound return type is bool (not HRESULT)",
          "[audio][interface][portable-types][7-8-1]")
{
    SECTION("PlaySound return type size equals sizeof(bool) = 1")
    {
        // GIVEN: An uninitialized MiniAudioBackend (no audio device required)
        mu::MiniAudioBackend backend;

        // WHEN: PlaySound is called — nullptr and false are compatible with both
        //       old (OBJECT*, BOOL) and new (void*, bool) signatures
        auto result = backend.PlaySound(static_cast<ESound>(0), nullptr, false);

        // THEN: sizeof(result) == sizeof(bool) == 1
        // RED PHASE: sizeof(result) = sizeof(HRESULT) = sizeof(long) > 1 → FAILS
        // GREEN PHASE: sizeof(result) = sizeof(bool) = 1 → PASSES
        REQUIRE(sizeof(result) == sizeof(bool));
    }
}

// ---------------------------------------------------------------------------
// AC-2/AC-3: Interface purity and backend concreteness unchanged after type fix
// These tests must remain GREEN through the refactor — they verify no regression
// in the abstraction hierarchy established by Story 5.1.1.
// ---------------------------------------------------------------------------
TEST_CASE("AC-2/AC-3: IPlatformAudio remains abstract after Win32 type removal",
          "[audio][interface][portable-types][7-8-1]")
{
    SECTION("IPlatformAudio is still pure abstract")
    {
        // GIVEN/WHEN/THEN: std::is_abstract_v is a compile-time property
        // Must remain true regardless of whether methods return HRESULT or bool
        static_assert(std::is_abstract_v<mu::IPlatformAudio>,
                      "IPlatformAudio must remain pure abstract after Win32 type removal");
        REQUIRE(std::is_abstract_v<mu::IPlatformAudio>);
    }

    SECTION("MiniAudioBackend remains concrete — overrides all pure virtual methods")
    {
        // GIVEN/WHEN/THEN: After updating override signatures to match the portable
        // interface, MiniAudioBackend must still override ALL pure virtual methods.
        // RED PHASE: passes (overrides exist, just with wrong types)
        // GREEN PHASE: passes (overrides exist with correct portable types)
        // If an override is accidentally removed, this fails at compile time.
        static_assert(!std::is_abstract_v<mu::MiniAudioBackend>,
                      "MiniAudioBackend must remain concrete (all overrides present) after type fix");
        REQUIRE_FALSE(std::is_abstract_v<mu::MiniAudioBackend>);
    }

    SECTION("MiniAudioBackend still derives from IPlatformAudio")
    {
        static_assert(std::is_base_of_v<mu::IPlatformAudio, mu::MiniAudioBackend>,
                      "MiniAudioBackend must still inherit from IPlatformAudio after type fix");
        REQUIRE(std::is_base_of_v<mu::IPlatformAudio, mu::MiniAudioBackend>);
    }
}

// ---------------------------------------------------------------------------
// AC-3: All IPlatformAudio method call sites compile with new portable signatures
// Tests call every affected method to ensure compatibility of call-site arguments
// (nullptr, false, 0L) with both old Win32 types and new portable types.
// None of these assertions check return values — they only verify no crash.
// ---------------------------------------------------------------------------
TEST_CASE("AC-4: Affected method call sites compile and do not crash with portable arguments",
          "[audio][interface][call-sites][7-8-1]")
{
    // GIVEN: An uninitialized MiniAudioBackend
    mu::MiniAudioBackend backend;

    SECTION("PlaySound: nullptr and false arguments compatible with both OBJECT*/BOOL and void*/bool")
    {
        // WHEN/THEN: Must not crash. Return value intentionally ignored.
        REQUIRE_NOTHROW([&]() {
            [[maybe_unused]] auto r = backend.PlaySound(static_cast<ESound>(0), nullptr, false);
        }());
    }

    SECTION("StopSound: false argument compatible with both BOOL=int and bool")
    {
        // WHEN/THEN: Must not crash.
        REQUIRE_NOTHROW([&]() {
            backend.StopSound(static_cast<ESound>(0), false);
        }());
    }

    SECTION("PlayMusic: false argument compatible with both BOOL=int and bool")
    {
        // WHEN/THEN: Must not crash.
        REQUIRE_NOTHROW([&]() {
            backend.PlayMusic("nonexistent.mp3", false);
        }());
    }

    SECTION("StopMusic: false argument compatible with both BOOL=int and bool")
    {
        // WHEN/THEN: Must not crash.
        REQUIRE_NOTHROW([&]() {
            backend.StopMusic("nonexistent.mp3", false);
        }());
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2: Headers compile on this platform without Win32 API dependencies
// This test exists solely to verify the translation unit compiles successfully
// — if IPlatformAudio.h or MiniAudioBackend.h have unresolved Win32 type
// dependencies that break compilation, this entire test file fails to build.
//
// On macOS/Linux: types like HRESULT, BOOL, OBJECT* come from PlatformTypes.h
// stubs. After the fix, these types are no longer referenced in the interface,
// so no PlatformTypes.h stubs are needed for the audio interface headers.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: Audio interface headers compile on this platform",
          "[audio][interface][compilation][7-8-1]")
{
    SECTION("IPlatformAudio.h includes and compiles successfully")
    {
        // GIVEN/WHEN/THEN: If this test file compiled, both headers are includable.
        REQUIRE(true);
    }
}
