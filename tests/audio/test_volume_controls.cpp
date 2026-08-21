// Story 5.4.1: Volume Controls & Audio State Management [VS1-AUDIO-VOLUME-CONTROLS]
// RED PHASE: Tests verify SetBGMVolume/SetSFXVolume/GetBGMVolume/GetSFXVolume on
// MiniAudioBackend — default values, clamping behaviour, get/set round-trip, and
// safety on an uninitialized backend.  No audio device, Win32, or DirectSound API
// is required.  All tests compile and run on macOS/Linux.
//
// Tests FAIL until Tasks 1-2 in story 5.4.1 are implemented:
//   Task 1: Add SetBGMVolume/SetSFXVolume/GetBGMVolume/GetSFXVolume to IPlatformAudio
//   Task 2: Implement those methods + m_bgmVolume/m_sfxVolume in MiniAudioBackend
//
// Framework: Catch2 v3.7.1 — REQUIRE/CHECK macros, TEST_CASE/SECTION structure.
// No mocking framework — pure construction and state-inspection tests only.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// Include IPlatformAudio for interface compliance checks
#include "IPlatformAudio.h"

// Include MiniAudioBackend — concrete implementation under test
#include "MiniAudioBackend.h"

using Catch::Matchers::WithinAbs;

// ---------------------------------------------------------------------------
// AC-1: IPlatformAudio declares the four new volume methods
// The interface must expose SetBGMVolume/SetSFXVolume/GetBGMVolume/GetSFXVolume
// as pure virtual.  Verified by checking that MiniAudioBackend (concrete) has
// overrides — i.e., compiling with a call resolves at link time.
// ---------------------------------------------------------------------------
TEST_CASE("AC-1 [5-4-1]: IPlatformAudio volume methods exist on MiniAudioBackend",
          "[audio][volume][interface][5-4-1]")
{
    SECTION("MiniAudioBackend exposes GetBGMVolume() returning float")
    {
        // GIVEN: A default-constructed MiniAudioBackend
        mu::MiniAudioBackend backend;

        // WHEN:  GetBGMVolume() is called via the concrete type
        // THEN:  Must compile and return a valid float (default = 1.0f)
        float vol = backend.GetBGMVolume();
        REQUIRE(vol >= 0.0f);
        REQUIRE(vol <= 1.0f);
    }

    SECTION("MiniAudioBackend exposes GetSFXVolume() returning float")
    {
        // GIVEN: A default-constructed MiniAudioBackend
        mu::MiniAudioBackend backend;

        // WHEN:  GetSFXVolume() is called via the concrete type
        // THEN:  Must compile and return a valid float (default = 1.0f)
        float vol = backend.GetSFXVolume();
        REQUIRE(vol >= 0.0f);
        REQUIRE(vol <= 1.0f);
    }

    SECTION("IPlatformAudio pointer resolves volume methods via virtual dispatch")
    {
        // GIVEN: A MiniAudioBackend accessed through the interface pointer
        mu::MiniAudioBackend backendImpl;
        mu::IPlatformAudio* audio = &backendImpl;

        // WHEN:  Volume methods are called through the base pointer
        // THEN:  Virtual dispatch resolves to MiniAudioBackend overrides — no crash
        REQUIRE_NOTHROW([&]() {
            audio->SetBGMVolume(0.5f);
            audio->SetSFXVolume(0.5f);
            [[maybe_unused]] float bgm = audio->GetBGMVolume();
            [[maybe_unused]] float sfx = audio->GetSFXVolume();
        }());
    }
}

// ---------------------------------------------------------------------------
// AC-2: MiniAudioBackend default BGM volume is 1.0f
// Per AC-STD-2: "default volumes are 1.0f"
// ---------------------------------------------------------------------------
TEST_CASE("AC-2/AC-STD-2 [5-4-1]: MiniAudioBackend default BGM volume is 1.0f",
          "[audio][volume][default][5-4-1]")
{
    // GIVEN: A freshly constructed MiniAudioBackend (not initialized)
    mu::MiniAudioBackend backend;

    // WHEN:  GetBGMVolume() is called before any Set call
    // THEN:  Must return 1.0f (full volume default per story spec)
    REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(1.0f, 0.001f));
}

// ---------------------------------------------------------------------------
// AC-2 (continued): Default SFX volume is 1.0f
// ---------------------------------------------------------------------------
TEST_CASE("AC-3/AC-STD-2 [5-4-1]: MiniAudioBackend default SFX volume is 1.0f",
          "[audio][volume][default][5-4-1]")
{
    // GIVEN: A freshly constructed MiniAudioBackend (not initialized)
    mu::MiniAudioBackend backend;

    // WHEN:  GetSFXVolume() is called before any Set call
    // THEN:  Must return 1.0f (full volume default per story spec)
    REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(1.0f, 0.001f));
}

// ---------------------------------------------------------------------------
// AC-2: SetBGMVolume clamps values below 0.0f to 0.0f
// AC-STD-2: "values < 0.0 clamp to 0.0"
// ---------------------------------------------------------------------------
TEST_CASE("AC-2/AC-STD-2 [5-4-1]: SetBGMVolume clamps to valid range [0.0, 1.0]",
          "[audio][volume][clamp][5-4-1]")
{
    mu::MiniAudioBackend backend;

    SECTION("Value below 0.0f clamps to 0.0f")
    {
        // GIVEN: An uninitialized backend with default volume 1.0f
        // WHEN:  SetBGMVolume is called with a negative value
        backend.SetBGMVolume(-0.5f);

        // THEN:  GetBGMVolume() returns 0.0f (clamped)
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.0f, 0.001f));
    }

    SECTION("Value above 1.0f clamps to 1.0f")
    {
        // GIVEN: An uninitialized backend
        // WHEN:  SetBGMVolume is called with a value greater than 1.0
        backend.SetBGMVolume(2.0f);

        // THEN:  GetBGMVolume() returns 1.0f (clamped)
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(1.0f, 0.001f));
    }

    SECTION("Valid mid-range value is stored as-is")
    {
        // GIVEN: An uninitialized backend
        // WHEN:  SetBGMVolume is called with 0.5f
        backend.SetBGMVolume(0.5f);

        // THEN:  GetBGMVolume() returns 0.5f (no clamping applied)
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.5f, 0.001f));
    }

    SECTION("Exact boundary value 0.0f is stored as-is")
    {
        backend.SetBGMVolume(0.0f);
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.0f, 0.001f));
    }

    SECTION("Exact boundary value 1.0f is stored as-is")
    {
        backend.SetBGMVolume(1.0f);
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(1.0f, 0.001f));
    }
}

// ---------------------------------------------------------------------------
// AC-3: SetSFXVolume clamps values to [0.0, 1.0]
// AC-STD-2: "values < 0.0 clamp to 0.0, values > 1.0 clamp to 1.0"
// ---------------------------------------------------------------------------
TEST_CASE("AC-3/AC-STD-2 [5-4-1]: SetSFXVolume clamps to valid range [0.0, 1.0]",
          "[audio][volume][clamp][5-4-1]")
{
    mu::MiniAudioBackend backend;

    SECTION("Value below 0.0f clamps to 0.0f")
    {
        backend.SetSFXVolume(-1.5f);
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(0.0f, 0.001f));
    }

    SECTION("Value above 1.0f clamps to 1.0f")
    {
        backend.SetSFXVolume(3.7f);
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(1.0f, 0.001f));
    }

    SECTION("Valid mid-range value is stored as-is")
    {
        backend.SetSFXVolume(0.75f);
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(0.75f, 0.001f));
    }

    SECTION("Exact boundary value 0.0f is stored as-is")
    {
        backend.SetSFXVolume(0.0f);
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(0.0f, 0.001f));
    }

    SECTION("Exact boundary value 1.0f is stored as-is")
    {
        backend.SetSFXVolume(1.0f);
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(1.0f, 0.001f));
    }
}

// ---------------------------------------------------------------------------
// AC-2 / AC-3: Get/Set round-trip — getter returns the clamped set value
// AC-STD-2: "getters return the clamped value"
// ---------------------------------------------------------------------------
TEST_CASE("AC-2/AC-3/AC-STD-2 [5-4-1]: GetBGMVolume/GetSFXVolume return the clamped set value",
          "[audio][volume][roundtrip][5-4-1]")
{
    mu::MiniAudioBackend backend;

    SECTION("BGM volume round-trip at 0.3f")
    {
        backend.SetBGMVolume(0.3f);
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.3f, 0.001f));
    }

    SECTION("SFX volume round-trip at 0.8f")
    {
        backend.SetSFXVolume(0.8f);
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(0.8f, 0.001f));
    }

    SECTION("BGM and SFX volumes are independent — setting one does not affect the other")
    {
        // GIVEN: Distinct BGM and SFX volumes are set
        backend.SetBGMVolume(0.2f);
        backend.SetSFXVolume(0.9f);

        // THEN:  Each getter returns its own stored value, not the other's
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.2f, 0.001f));
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(0.9f, 0.001f));
    }

    SECTION("Multiple consecutive set calls — last write wins for BGM")
    {
        backend.SetBGMVolume(0.1f);
        backend.SetBGMVolume(0.6f);
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.6f, 0.001f));
    }

    SECTION("Multiple consecutive set calls — last write wins for SFX")
    {
        backend.SetSFXVolume(0.9f);
        backend.SetSFXVolume(0.4f);
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(0.4f, 0.001f));
    }
}

// ---------------------------------------------------------------------------
// AC-2 / AC-3 / AC-9: Volume set/get on uninitialized backend does not crash
// AC-STD-2: "SetBGMVolume/SetSFXVolume on uninitialized backend does not crash.
//             Volumes are stored even without init."
// AC-9: g_platformAudio guards exist — but the backend itself must survive uninitialized calls.
// ---------------------------------------------------------------------------
TEST_CASE("AC-2/AC-3/AC-9/AC-STD-2 [5-4-1]: Volume controls on uninitialized backend are safe",
          "[audio][volume][safety][5-4-1]")
{
    // GIVEN: A default-constructed backend that has never been Initialize()d
    mu::MiniAudioBackend backend;

    SECTION("SetBGMVolume does not crash on uninitialized backend")
    {
        // WHEN:  SetBGMVolume is called with a valid value before Initialize()
        // THEN:  No crash.  The value is stored (m_bgmVolume) but no ma_sound_set_volume is called.
        REQUIRE_NOTHROW([&]() { backend.SetBGMVolume(0.5f); }());

        // AND:  GetBGMVolume returns the stored value
        REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.5f, 0.001f));
    }

    SECTION("SetSFXVolume does not crash on uninitialized backend")
    {
        // WHEN:  SetSFXVolume is called before Initialize()
        // THEN:  No crash.  Volume is stored; no per-slot ma_sound_set_volume is called.
        REQUIRE_NOTHROW([&]() { backend.SetSFXVolume(0.5f); }());

        // AND:  GetSFXVolume returns the stored value
        REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(0.5f, 0.001f));
    }

    SECTION("GetBGMVolume does not crash on uninitialized backend")
    {
        REQUIRE_NOTHROW([&]() {
            [[maybe_unused]] float v = backend.GetBGMVolume();
        }());
    }

    SECTION("GetSFXVolume does not crash on uninitialized backend")
    {
        REQUIRE_NOTHROW([&]() {
            [[maybe_unused]] float v = backend.GetSFXVolume();
        }());
    }
}

// ---------------------------------------------------------------------------
// AC-2: SetBGMVolume with clamped value + uninitialized — stored correctly
// Validates that clamping works even when m_initialized == false.
// ---------------------------------------------------------------------------
TEST_CASE("AC-2 [5-4-1]: SetBGMVolume clamps on uninitialized backend",
          "[audio][volume][clamp][safety][5-4-1]")
{
    mu::MiniAudioBackend backend;

    // WHEN:  A clamping call is made on an uninitialized backend
    backend.SetBGMVolume(-99.0f);

    // THEN:  Value is clamped to 0.0f and stored (no ma_sound call since not initialized)
    REQUIRE_THAT(backend.GetBGMVolume(), WithinAbs(0.0f, 0.001f));
}

// ---------------------------------------------------------------------------
// AC-3: SetSFXVolume with clamped value + uninitialized — stored correctly
// ---------------------------------------------------------------------------
TEST_CASE("AC-3 [5-4-1]: SetSFXVolume clamps on uninitialized backend",
          "[audio][volume][clamp][safety][5-4-1]")
{
    mu::MiniAudioBackend backend;

    backend.SetSFXVolume(999.0f);
    REQUIRE_THAT(backend.GetSFXVolume(), WithinAbs(1.0f, 0.001f));
}

// ---------------------------------------------------------------------------
// AC-STD-1 / AC-1: nodiscard attribute on getters — compile-time contract
// GetBGMVolume() and GetSFXVolume() must be [[nodiscard]].
// If the attribute is missing, the compiler will not warn and the test still
// passes, but the static_assert documents intent.  The code review step will
// verify [[nodiscard]] in the header.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-1/AC-1 [5-4-1]: Volume getters are accessible via mu:: namespace",
          "[audio][volume][namespace][5-4-1]")
{
    SECTION("SetBGMVolume is callable via mu::MiniAudioBackend")
    {
        mu::MiniAudioBackend backend;
        REQUIRE_NOTHROW([&]() { backend.SetBGMVolume(0.5f); }());
    }

    SECTION("SetSFXVolume is callable via mu::MiniAudioBackend")
    {
        mu::MiniAudioBackend backend;
        REQUIRE_NOTHROW([&]() { backend.SetSFXVolume(0.5f); }());
    }

    SECTION("GetBGMVolume is callable via mu::IPlatformAudio pointer")
    {
        mu::MiniAudioBackend backendImpl;
        mu::IPlatformAudio* audio = &backendImpl;
        audio->SetBGMVolume(0.4f);
        REQUIRE_THAT(audio->GetBGMVolume(), WithinAbs(0.4f, 0.001f));
    }

    SECTION("GetSFXVolume is callable via mu::IPlatformAudio pointer")
    {
        mu::MiniAudioBackend backendImpl;
        mu::IPlatformAudio* audio = &backendImpl;
        audio->SetSFXVolume(0.6f);
        REQUIRE_THAT(audio->GetSFXVolume(), WithinAbs(0.6f, 0.001f));
    }
}
