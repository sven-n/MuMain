// Story 5.2.1: miniaudio BGM Implementation [VS1-AUDIO-MINIAUDIO-BGM]
// GREEN PHASE: Tests verify BGM lifecycle semantics — construct, initialize, play,
// stop, position query — all headless (no audio device required).
//
// AC-STD-2: Catch2 BGM lifecycle test in tests/audio/test_miniaudio_bgm.cpp
//   - IsEndMusic() == true before play (no stream loaded)
//   - StopMusic on unloaded stream does not crash
//   - PlayMusic with non-existent file does not crash; IsEndMusic() == true
//   - GetMusicPosition() returns 0 before play
//
// All tests run headless — Initialize() may return false on CI (no audio device), which
// is acceptable. No Win32, DirectSound, or wzAudio APIs called.
// Designed to compile on macOS/Linux without Win32 headers.
//
// MEDIUM-1 fix (code-review-finalize 2026-03-19): REQUIRE_NOTHROW on non-throwing C++
// code provides zero test value — miniaudio does not use C++ exceptions. Direct calls
// replace the lambda-wrapped REQUIRE_NOTHROW pattern. Both results from Initialize()
// (true/false) remain acceptable — CI headless is expected to return false.

#include <catch2/catch_test_macros.hpp>

// Include MiniAudioBackend for BGM lifecycle tests
#include "MiniAudioBackend.h"

// ---------------------------------------------------------------------------
// AC-STD-2 (Task 5.3): IsEndMusic() returns true before play
// A freshly constructed backend has no loaded music stream.
// IsEndMusic() must report "ended" (== true) since nothing is playing.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend BGM lifecycle — IsEndMusic before play",
          "[audio][bgm][lifecycle][5-2-1]")
{
    // GIVEN: A default-constructed MiniAudioBackend (NOT initialized)
    mu::MiniAudioBackend backend;

    // WHEN:  IsEndMusic() is called before Initialize()
    // THEN:  Returns true — no music stream is loaded, nothing is playing
    REQUIRE(backend.IsEndMusic());
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Task 5.4): StopMusic on an unloaded stream does not crash
// Before any track is played, calling StopMusic must be a safe no-op.
// Initialize() is attempted (may fail on CI); either result is acceptable.
// After StopMusic(nullptr, TRUE), IsEndMusic() must still return true.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend BGM lifecycle — StopMusic on unloaded stream",
          "[audio][bgm][lifecycle][5-2-1]")
{
    // GIVEN: A default-constructed MiniAudioBackend
    mu::MiniAudioBackend backend;

    // Initialize may succeed or fail (CI headless) — both are valid.
    // Direct call: miniaudio does not throw C++ exceptions, so REQUIRE_NOTHROW
    // would provide no test value. (MEDIUM-1 fix, code-review-finalize 2026-03-19)
    [[maybe_unused]] bool initResult = backend.Initialize();

    // WHEN:  StopMusic is called with no track loaded (nullptr name, hard stop)
    // THEN:  Must not crash or abort — StopMusic guards !m_initialized and !m_musicLoaded
    backend.StopMusic(nullptr, TRUE);

    // THEN:  IsEndMusic() still returns true — no stream was ever loaded
    REQUIRE(backend.IsEndMusic());

    // Cleanup: safe to call Shutdown regardless of init result
    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Task 5.5): PlayMusic with non-existent file does not crash
// If the file cannot be opened, the backend logs the error and returns early.
// IsEndMusic() must remain true (no stream was loaded).
// Initialize() may fail on CI; that is acceptable — PlayMusic on an
// uninitialized backend must also be safe (guard: !m_initialized return early).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend BGM lifecycle — PlayMusic non-existent file returns without crash",
          "[audio][bgm][lifecycle][5-2-1]")
{
    // GIVEN: A default-constructed MiniAudioBackend
    mu::MiniAudioBackend backend;

    // Initialize may return false on CI (no audio device) — acceptable.
    // Direct call: miniaudio does not throw C++ exceptions. (MEDIUM-1 fix, code-review-finalize 2026-03-19)
    [[maybe_unused]] bool initResult = backend.Initialize();

    // WHEN:  PlayMusic is called with a file that does not exist
    // THEN:  Must not crash — will log error and return early
    //        (MiniAudioBackend::PlayMusic guards: !m_initialized → return;
    //         ma_sound_init_from_file() failure → g_ErrorReport.Write() + return)
    backend.PlayMusic("nonexistent_track.mp3", TRUE);

    // THEN:  IsEndMusic() returns true — no stream was successfully loaded.
    // NOTE:  When Initialize() returns false (CI headless), PlayMusic() hits the
    //        !m_initialized guard and returns before ma_sound_init_from_file() is
    //        called — the file-not-found error path is only exercised on developer
    //        workstations with audio hardware. Both branches produce IsEndMusic()==true
    //        but for different reasons: !m_initialized guard vs. stream-init failure.
    //        (MEDIUM-NEW-2 fix, code-review-finalize 2026-03-19)
    CHECK(backend.IsEndMusic());

    // Cleanup
    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-STD-2 (Task 5.6): GetMusicPosition() returns 0 before play
// Without a loaded stream, position must be 0 (no cursor to report).
// This mirrors the wzAudioGetStreamOffsetRange() == 0 behavior before any
// stream is opened.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2: MiniAudioBackend BGM — GetMusicPosition before play returns 0",
          "[audio][bgm][lifecycle][5-2-1]")
{
    // GIVEN: A default-constructed MiniAudioBackend (NOT initialized)
    mu::MiniAudioBackend backend;

    // WHEN:  GetMusicPosition() is called without Initialize() or PlayMusic()
    // THEN:  Returns 0 — no stream cursor to report
    REQUIRE(backend.GetMusicPosition() == 0);
}
