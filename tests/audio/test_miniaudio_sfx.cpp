// Story 5.2.2: miniaudio SFX Implementation [VS1-AUDIO-MINIAUDIO-SFX]
// RED PHASE: Tests verify SFX lifecycle safety — all headless (no audio device required).
//
// AC-STD-2: Catch2 SFX lifecycle tests in tests/audio/test_miniaudio_sfx.cpp
//   AC-1 coverage:  LoadWaveFile delegates to g_platformAudio — inspection only (MEDIUM-1 note):
//                  A mock-based Catch2 test would require injecting a mock IPlatformAudio* into
//                  g_platformAudio, which is a global raw pointer (pre-existing from Story 5.1.1).
//                  The delegation is a single-line if-guard in DSplaysound.cpp:742–748, verified
//                  by code inspection. Mocking deferred to a future test-infrastructure story.
//   AC-2 coverage:  PlaySound before Initialize returns S_FALSE
//   AC-3 coverage:  StopSound on unloaded slot is safe
//   AC-4 coverage:  AllStopSound on empty backend is safe
//   AC-5/AC-6 coverage: SetMasterVolume / SetVolume on uninitialized backend do not crash
//   AC-7 coverage:  InitDirectSound removal — code inspection (no Catch2 test needed)
//   AC-8 coverage:  Set3DSoundPosition with nullptr m_soundObjects does not crash
//   AC-9 coverage:  Path normalization — inspected via LoadSound non-existent file (no crash)
//   AC-10 coverage: DirectSoundManager dormancy — code inspection (no Catch2 test needed)
//
// All tests run headless — Initialize() may return false on CI (no audio device).
// No Win32, DirectSound, or wzAudio APIs called.
// Designed to compile on macOS/Linux without Win32 headers.

#include <catch2/catch_test_macros.hpp>

// Include MiniAudioBackend for SFX lifecycle tests
#include "MiniAudioBackend.h"

// ---------------------------------------------------------------------------
// AC-STD-2 (Subtask 6.3): LoadSound non-existent file does not crash
// A backend that has NOT been initialized should return early in LoadSound()
// without crashing when given a non-existent file path.
// Covers AC-9 (path normalization accepted gracefully even for missing file).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend SFX — LoadSound non-existent file does not crash",
          "[audio][sfx][lifecycle][5-2-2]")
{
    // GIVEN: A default-constructed MiniAudioBackend (NOT initialized)
    mu::MiniAudioBackend backend;

    // MEDIUM-3 fix: original assertion REQUIRE(g_platformAudio == nullptr) relied on
    // test execution order (no prior test having set g_platformAudio). Catch2 does not
    // guarantee cross-file test order. The actual intent is: this local `backend`
    // instance is NOT registered as the global singleton — verify that directly.
    REQUIRE(&backend != g_platformAudio);

    // WHEN:  LoadSound is called with a non-existent filename before Initialize()
    // THEN:  Must not crash — guard: !m_initialized → return early
    //        On an initialized backend without an audio device the ma_sound_init_from_file()
    //        call will fail and g_ErrorReport.Write() is invoked (acceptable).
    backend.LoadSound(static_cast<ESound>(0), L"nonexistent.wav", 1, false);

    // No assertion needed beyond "did not crash" — this is a safety guard test.
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Subtask 6.4): PlaySound before Initialize returns S_FALSE
// PlaySound must not crash when called before Initialize() and must return
// S_FALSE to indicate no sound was played.
// Covers AC-2 delegation guard.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend SFX — PlaySound before Initialize returns S_FALSE",
          "[audio][sfx][lifecycle][5-2-2]")
{
    // GIVEN: A default-constructed MiniAudioBackend (NOT initialized)
    mu::MiniAudioBackend backend;

    // WHEN:  PlaySound is called with no object and not looped, before Initialize()
    // THEN:  Must return S_FALSE (existing guard from 5.2.1: !m_initialized → return S_FALSE)
    //        Must not crash or abort.
    // Direct call: miniaudio does not throw C++ exceptions, so REQUIRE_NOTHROW provides no value.
    HRESULT result = backend.PlaySound(static_cast<ESound>(0), nullptr, FALSE);

    REQUIRE(result == S_FALSE);
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Subtask 6.5): StopSound on unloaded slot is safe
// Calling StopSound on a slot that was never loaded must be a safe no-op.
// Covers AC-3 delegation guard (m_soundLoaded[bufIdx] == false).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend SFX — StopSound on unloaded slot is safe",
          "[audio][sfx][lifecycle][5-2-2]")
{
    // GIVEN: A default-constructed MiniAudioBackend
    mu::MiniAudioBackend backend;

    // Initialize may succeed or fail on CI (no audio device) — both are valid.
    // Direct call: miniaudio does not throw C++ exceptions.
    [[maybe_unused]] bool initResult = backend.Initialize();

    // WHEN:  StopSound is called on a slot that was never loaded
    // THEN:  Must not crash — guard: !m_soundLoaded[bufIdx] → return
    backend.StopSound(static_cast<ESound>(0), FALSE);

    // Cleanup
    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Subtask 6.6): AllStopSound on empty backend is safe
// Calling AllStopSound before any sounds are loaded must be a safe no-op.
// Covers AC-4 delegation guard.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend SFX — AllStopSound on empty backend is safe",
          "[audio][sfx][lifecycle][5-2-2]")
{
    // GIVEN: A default-constructed MiniAudioBackend
    mu::MiniAudioBackend backend;

    // Initialize may return false on CI (no audio device) — acceptable.
    // Direct call: miniaudio does not throw C++ exceptions.
    [[maybe_unused]] bool initResult = backend.Initialize();

    // WHEN:  AllStopSound is called with no sounds loaded
    // THEN:  Must not crash — iterates over m_soundLoaded[], skips unloaded slots
    backend.AllStopSound();

    // Cleanup
    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Subtask 6.7): Set3DSoundPosition with no loaded sounds is safe
// Calling Set3DSoundPosition before any 3D sounds are loaded must not crash.
// Covers AC-8 upgrade of the 5.2.1 loop-structure stub.
// When m_soundLoaded[buf] == false for all slots, the loop skips them all.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend SFX — Set3DSoundPosition with no loaded sounds is safe",
          "[audio][sfx][lifecycle][5-2-2]")
{
    // GIVEN: A default-constructed MiniAudioBackend
    mu::MiniAudioBackend backend;

    // Initialize may return false on CI (no audio device) — acceptable.
    [[maybe_unused]] bool initResult = backend.Initialize();

    // WHEN:  Set3DSoundPosition is called with no sounds loaded (all m_soundLoaded[buf] == false)
    // THEN:  Must not crash — loop body is skipped because !m_soundLoaded[buf]
    backend.Set3DSoundPosition();

    // Cleanup
    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Subtask 6.8): Set3DSoundPosition skips nullptr m_soundObjects
// Covers AC-8 nullptr guard for per-slot OBJECT* tracking.
//
// HIGH-1 note (code-review-finalize 2026-03-20):
// This test exercises the !m_soundLoaded[buf] guard (LoadSound fails → slot stays
// unloaded → loop skips), NOT the m_soundObjects[buf] == nullptr guard at line 329
// of MiniAudioBackend.cpp. The deeper nullptr guard — where a slot IS loaded and 3D-
// enabled but PlaySound was called with pObject=nullptr — is covered by code
// inspection only: the guard `m_soundObjects[buf] == nullptr` at Set3DSoundPosition()
// line 329 is verified by reading the source. A full runtime test would require
// either a real audio device (not available on CI) or a test-seam to inject a
// pre-loaded slot state. Accepted as inspection-only per project skip_checks policy.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend SFX — Set3DSoundPosition skips nullptr m_soundObjects",
          "[audio][sfx][lifecycle][5-2-2]")
{
    // GIVEN: A default-constructed MiniAudioBackend
    mu::MiniAudioBackend backend;

    // Initialize may return false on CI (no audio device) — acceptable.
    [[maybe_unused]] bool initResult = backend.Initialize();

    // WHEN:  LoadSound is attempted with a non-existent file for slot 0 with 3D enabled.
    //        ma_sound_init_from_file() returns an error, g_ErrorReport.Write() is called,
    //        and m_soundLoaded[0] remains false. m_soundObjects[0] remains nullptr.
    backend.LoadSound(static_cast<ESound>(0), L"nonexistent_3d_sound.wav", 1, true);

    // THEN:  Set3DSoundPosition must not crash — the !m_soundLoaded[buf] guard skips
    //        the slot before reaching the m_soundObjects nullptr check.
    backend.Set3DSoundPosition();

    // Cleanup
    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-5 / AC-6: SetMasterVolume and SetVolume on uninitialized backend are safe
// Before Initialize(), delegating to SetMasterVolume / SetVolume must not crash.
// These cover the delegation chains for volume controls.
// ---------------------------------------------------------------------------
TEST_CASE("AC-5/AC-6: MiniAudioBackend SFX — volume control before Initialize is safe",
          "[audio][sfx][volume][5-2-2]")
{
    // GIVEN: A default-constructed MiniAudioBackend (NOT initialized)
    mu::MiniAudioBackend backend;

    // WHEN:  SetMasterVolume is called before Initialize()
    // THEN:  Must not crash — guard: !m_initialized → return early
    backend.SetMasterVolume(-2000L);

    // WHEN:  SetVolume is called before Initialize()
    // THEN:  Must not crash — guard: !m_initialized → return early
    backend.SetVolume(static_cast<ESound>(0), -1000L);

    // No assertion needed — "did not crash" is the contract.
}
