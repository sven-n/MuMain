// Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
// RED PHASE: Tests verify IPlatformAudio interface purity, g_platformAudio default state,
// MiniAudioBackend default construction, and graceful failure when no audio device is present.
//
// Designed to compile on macOS/Linux without Win32 or DirectSound APIs.
// All tests use Catch2 REQUIRE/CHECK macros and TEST_CASE/SECTION structure.
// No mocking framework used — pure construction/interface checks only.

#include <catch2/catch_test_macros.hpp>
#include <type_traits>

// Include IPlatformAudio interface for abstractness check and g_platformAudio declaration
#include "IPlatformAudio.h"

// Include MiniAudioBackend for construction and initialization tests
#include "MiniAudioBackend.h"

// ---------------------------------------------------------------------------
// AC-1 / AC-2: IPlatformAudio interface is a pure virtual class
// Verifies the interface design contract that no concrete instances can be
// created directly from IPlatformAudio, enforcing the abstraction layer.
// ---------------------------------------------------------------------------
TEST_CASE("AC-1: IPlatformAudio interface is pure virtual", "[audio][interface][5-1-1]")
{
    SECTION("IPlatformAudio is an abstract class (std::is_abstract_v)")
    {
        // GIVEN: The IPlatformAudio interface definition
        // WHEN:  We check its abstract status at compile time
        // THEN:  It must be a pure abstract class — no direct instantiation allowed
        static_assert(std::is_abstract_v<mu::IPlatformAudio>,
                      "IPlatformAudio must be a pure abstract class (all methods = 0)");
        REQUIRE(std::is_abstract_v<mu::IPlatformAudio>);
    }

    SECTION("MiniAudioBackend is NOT abstract — it is the concrete implementation")
    {
        // GIVEN: The MiniAudioBackend concrete class
        // WHEN:  We check its abstract status
        // THEN:  It must NOT be abstract — it overrides all pure virtual methods
        static_assert(!std::is_abstract_v<mu::MiniAudioBackend>,
                      "MiniAudioBackend must be concrete (overrides all pure virtual methods)");
        REQUIRE_FALSE(std::is_abstract_v<mu::MiniAudioBackend>);
    }

    SECTION("MiniAudioBackend is a subclass of IPlatformAudio")
    {
        // GIVEN: The class hierarchy
        // WHEN:  We check the base class relationship
        // THEN:  MiniAudioBackend must derive from IPlatformAudio
        static_assert(std::is_base_of_v<mu::IPlatformAudio, mu::MiniAudioBackend>,
                      "MiniAudioBackend must inherit from IPlatformAudio");
        REQUIRE(std::is_base_of_v<mu::IPlatformAudio, mu::MiniAudioBackend>);
    }
}

// ---------------------------------------------------------------------------
// AC-4: g_platformAudio is nullptr at startup
// Verifies the global pointer default state before any game initialization.
// Story 5.2.1 will set this pointer; this test confirms the safe default.
// ---------------------------------------------------------------------------
TEST_CASE("AC-4: g_platformAudio is nullptr at startup", "[audio][global][5-1-1]")
{
    // GIVEN: No game initialization has occurred
    // WHEN:  We check the g_platformAudio global pointer
    // THEN:  It must be nullptr (defined in MiniAudioBackend.cpp as nullptr)
    REQUIRE(g_platformAudio == nullptr);
}

// ---------------------------------------------------------------------------
// AC-2 / AC-5 / AC-6: MiniAudioBackend default construction
// Verifies the backend can be stack-allocated without crashing or throwing.
// IsEndMusic() must return true for a freshly constructed (non-initialized) backend.
// ---------------------------------------------------------------------------
TEST_CASE("AC-2: MiniAudioBackend default-constructs cleanly", "[audio][backend][5-1-1]")
{
    SECTION("Stack-allocated MiniAudioBackend does not crash or throw on construction")
    {
        // GIVEN: No audio device or engine initialized
        // WHEN:  A MiniAudioBackend is stack-allocated
        // THEN:  No crash, no exception, default state is clean
        REQUIRE_NOTHROW([]() {
            mu::MiniAudioBackend backend;
            (void)backend;
        }());
    }

    SECTION("Freshly constructed backend reports music as not playing")
    {
        // GIVEN: A default-constructed MiniAudioBackend (not initialized)
        mu::MiniAudioBackend backend;

        // WHEN:  IsEndMusic() is called before Initialize()
        // THEN:  Returns true — no music is playing (matches wzAudioGetStreamOffsetRange() == 100)
        REQUIRE(backend.IsEndMusic());
    }

    SECTION("Freshly constructed backend reports music position as 0")
    {
        // GIVEN: A default-constructed MiniAudioBackend (not initialized)
        mu::MiniAudioBackend backend;

        // WHEN:  GetMusicPosition() is called before Initialize()
        // THEN:  Returns 0 — no stream cursor to report
        REQUIRE(backend.GetMusicPosition() == 0);
    }
}

// ---------------------------------------------------------------------------
// AC-6: MiniAudioBackend::Initialize() fails gracefully without audio device
// On CI runners (headless, no audio hardware), ma_engine_init() will fail.
// The backend MUST return false without crashing — not throw, not abort.
// This test uses CHECK (not REQUIRE) so execution continues regardless of
// whether Initialize() succeeds or fails (device availability varies by host).
// ---------------------------------------------------------------------------
TEST_CASE("AC-6: MiniAudioBackend Initialize fails gracefully without audio device",
          "[audio][backend][initialize][5-1-1]")
{
    SECTION("Initialize() does not crash regardless of audio device availability")
    {
        // GIVEN: A default-constructed MiniAudioBackend
        mu::MiniAudioBackend backend;

        // WHEN:  Initialize() is called (may fail on CI with no audio device)
        // THEN:  Must not throw or abort — graceful failure is correct behavior
        bool result = false;
        REQUIRE_NOTHROW([&]() { result = backend.Initialize(); }());

        // NOTE: result may be true (audio device present) or false (no device, CI)
        // Both outcomes are valid. We only assert no crash/exception.
        // If init succeeded, we must also shut down cleanly.
        if (result)
        {
            REQUIRE_NOTHROW([&]() { backend.Shutdown(); }());
        }
    }

    SECTION("Initialize() returns bool (false on failure, true on success)")
    {
        // GIVEN: A default-constructed MiniAudioBackend
        mu::MiniAudioBackend backend;

        // WHEN:  Initialize() is called
        // THEN:  Return value is a valid bool (not void, not HRESULT)
        //        On CI, we expect false; on dev machines with audio, possibly true.
        REQUIRE_NOTHROW([&]() {
            [[maybe_unused]] bool initResult = backend.Initialize();
            // If init succeeded, shut down to avoid resource leak
            if (initResult)
            {
                backend.Shutdown();
            }
        }());
    }
}

// ---------------------------------------------------------------------------
// AC-6: Shutdown() is safe to call without a prior Initialize()
// Verifies that Shutdown() is a no-op when the backend was never initialized.
// ---------------------------------------------------------------------------
TEST_CASE("AC-6: MiniAudioBackend Shutdown is safe without prior Initialize",
          "[audio][backend][shutdown][5-1-1]")
{
    // GIVEN: A default-constructed backend that was never initialized
    mu::MiniAudioBackend backend;

    // WHEN:  Shutdown() is called without a prior Initialize()
    // THEN:  Must not crash, abort, or throw
    REQUIRE_NOTHROW([&]() { backend.Shutdown(); }());
}

// ---------------------------------------------------------------------------
// AC-2: All IPlatformAudio methods are safe on an uninitialized backend
// LOW-2 (code-review-finalize 2026-03-19): Added to cover no-op guard paths
// for PlaySound, StopSound, AllStopSound, SetVolume, SetMasterVolume, PlayMusic,
// StopMusic, LoadSound, and Set3DSoundPosition on an uninitialized backend.
// All calls must be safe (no crash, no exception) regardless of state.
// This also detects HIGH-1 regressions if pObject were dereferenced when null.
// ---------------------------------------------------------------------------
TEST_CASE("AC-2: All IPlatformAudio methods are safe to call on uninitialized backend",
          "[audio][backend][safety][5-1-1]")
{
    // GIVEN: A default-constructed (never initialized) MiniAudioBackend
    mu::MiniAudioBackend backend;

    // WHEN/THEN: All 13 methods must be callable without crash or exception.
    // PlaySound with nullptr pObject — guards against HIGH-1 null-deref regression.
    REQUIRE_NOTHROW([&]() {
        backend.LoadSound(static_cast<ESound>(0), L"nonexistent.wav", 1, false);
    }());

    REQUIRE_NOTHROW([&]() {
        backend.PlaySound(static_cast<ESound>(0), nullptr, false);
    }());

    REQUIRE_NOTHROW([&]() {
        backend.StopSound(static_cast<ESound>(0), false);
    }());

    REQUIRE_NOTHROW([&]() {
        backend.AllStopSound();
    }());

    REQUIRE_NOTHROW([&]() {
        backend.Set3DSoundPosition();
    }());

    REQUIRE_NOTHROW([&]() {
        backend.SetVolume(static_cast<ESound>(0), 0L);
    }());

    REQUIRE_NOTHROW([&]() {
        backend.SetMasterVolume(0L);
    }());

    REQUIRE_NOTHROW([&]() {
        backend.PlayMusic("nonexistent.mp3", false);
    }());

    REQUIRE_NOTHROW([&]() {
        backend.StopMusic(nullptr, false);
    }());

    REQUIRE_NOTHROW([&]() {
        backend.StopMusic("nonexistent.mp3", true);
    }());

    // These return values — verify they don't crash and return safe defaults.
    bool endMusic = true;
    REQUIRE_NOTHROW([&]() { endMusic = backend.IsEndMusic(); }());
    REQUIRE(endMusic); // Uninitialized backend: music is "ended" (not playing)

    int position = -1;
    REQUIRE_NOTHROW([&]() { position = backend.GetMusicPosition(); }());
    REQUIRE(position == 0); // Uninitialized backend: position is 0
}

// ---------------------------------------------------------------------------
// AC-STD-1: Namespace compliance
// Verifies that the mu:: namespace is used correctly and the interface is
// accessible via the correct fully-qualified name.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-1: mu namespace compliance", "[audio][namespace][5-1-1]")
{
    SECTION("IPlatformAudio is in the mu:: namespace")
    {
        // GIVEN: The interface declaration in IPlatformAudio.h
        // WHEN:  We use the fully-qualified name
        // THEN:  It compiles and resolves correctly
        static_assert(std::is_abstract_v<mu::IPlatformAudio>,
                      "mu::IPlatformAudio must be accessible via fully-qualified name");
        REQUIRE(std::is_abstract_v<mu::IPlatformAudio>);
    }

    SECTION("MiniAudioBackend is in the mu:: namespace")
    {
        // GIVEN: The backend class declaration in MiniAudioBackend.h
        // WHEN:  We use the fully-qualified name
        // THEN:  It compiles and resolves correctly
        static_assert(!std::is_abstract_v<mu::MiniAudioBackend>,
                      "mu::MiniAudioBackend must be accessible via fully-qualified name");
        REQUIRE_FALSE(std::is_abstract_v<mu::MiniAudioBackend>);
    }
}
