// Story 6.1.1: Authentication & Character Management Validation [VS1-GAME-VALIDATE-AUTH]
// RED PHASE: Tests define expected logical contracts for auth/character validation.
//
// AC coverage (unit-testable subset — see manual test scenarios for full AC validation):
//   AC-1       — Login scene initializes to known clean state (SceneInitializationState)
//   AC-3       — Character list: exactly 5 character slots per account (MAX_CHARACTERS_PER_ACCOUNT)
//   AC-4       — 5 base character classes exist with correct enum values
//   AC-5       — Character selection: valid index bounds enforced (CharacterSelectionState)
//   AC-6       — Logout/switch: ClearSelection returns to NO_SELECTION state
//   AC-STD-2   — Catch2 test suite exercises scene state logic without live server
//
// Manual validation (AC-1 visual, AC-2 text input, AC-5 world entry, AC-VAL-1..6):
//   See: _bmad-output/test-scenarios/epic-6/auth-character-validation.md
//
// PCC Constraints:
//   - Platform-compatible includes: #ifdef _WIN32 / PlatformTypes.h on non-Win32
//   - No Win32 API calls in test logic — test logic only
//   - Catch2 TEST_CASE / SECTION / REQUIRE structure
//   - Allman brace style, 4-space indent, 120-column limit
//   - No raw new/delete — stack allocation only
//   - #pragma once not used (this is a .cpp file)
//
// Design notes:
//   - SceneCommon.h includes mu_define.h for MAX_CHARACTERS_PER_ACCOUNT. Tests validate
//     the LOGICAL CONTRACT of CharacterSelectionState and SceneInitializationState by
//     testing their behaviours against their documented interface, compiled as part of
//     MUGame when MU_SCENE_TESTS_ENABLED is defined.
//   - mu_define.h and mu_enum.h are lightweight headers includable without stdafx.h.
//   - CLASS_TYPE is a BYTE-based enum: include PlatformTypes.h on non-Win32 first.
//   - AC-2 (SDL3 text input path) is already covered by test_platform_text_input.cpp.

#include <catch2/catch_test_macros.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#endif

#include "mu_define.h" // MAX_CHARACTERS_PER_ACCOUNT = 5
#include "mu_enum.h"   // CLASS_TYPE enum values

// =============================================================================
// AC-4 [6-1-1]: 5 base character classes exist with correct enum values
// Tests that CLASS_TYPE defines exactly the 5 Season 5.2 base classes:
//   Dark Wizard (CLASS_WIZARD), Dark Knight (CLASS_KNIGHT), Fairy Elf (CLASS_ELF),
//   Magic Gladiator (CLASS_DARK), Dark Lord (CLASS_DARK_LORD)
// =============================================================================

TEST_CASE("AC-4 [6-1-1]: 5 base character classes defined in CLASS_TYPE enum", "[scenes][auth][character][6-1-1]")
{
    SECTION("Dark Wizard is CLASS_WIZARD = 0 (first class)")
    {
        REQUIRE(static_cast<int>(CLASS_WIZARD) == 0);
        REQUIRE(CLASS_START == CLASS_WIZARD);
    }

    SECTION("Dark Knight is CLASS_KNIGHT = 1")
    {
        REQUIRE(static_cast<int>(CLASS_KNIGHT) == 1);
    }

    SECTION("Fairy Elf is CLASS_ELF = 2")
    {
        REQUIRE(static_cast<int>(CLASS_ELF) == 2);
    }

    SECTION("Magic Gladiator is CLASS_DARK = 3")
    {
        REQUIRE(static_cast<int>(CLASS_DARK) == 3);
    }

    SECTION("Dark Lord is CLASS_DARK_LORD = 4")
    {
        REQUIRE(static_cast<int>(CLASS_DARK_LORD) == 4);
    }

    SECTION("CLASS_UNDEFINED sentinel is 0xFF (invalid class marker)")
    {
        REQUIRE(static_cast<int>(CLASS_UNDEFINED) == 0xFF);
    }

    SECTION("5 base classes occupy contiguous range [CLASS_WIZARD, CLASS_DARK_LORD]")
    {
        int baseClassCount = static_cast<int>(CLASS_DARK_LORD) - static_cast<int>(CLASS_WIZARD) + 1;
        REQUIRE(baseClassCount == 5);
    }
}

// =============================================================================
// AC-3 [6-1-1]: Character slots per account matches game specification (5 slots)
// Tests that MAX_CHARACTERS_PER_ACCOUNT == 5 (Season 5.2 allows 5 characters).
// This constant governs CharacterSelectionState bounds and char list display.
// =============================================================================

TEST_CASE("AC-3 [6-1-1]: Character account supports exactly 5 character slots", "[scenes][auth][character][6-1-1]")
{
    REQUIRE(MAX_CHARACTERS_PER_ACCOUNT == 5);
}

// =============================================================================
// AC-1, AC-5, AC-6 [6-1-1]: Scene state logic contracts (CharacterSelectionState,
//   SceneInitializationState) — compiled via MUGame linkage when available.
//
// NOTE: These tests require linking against MUGame (for SceneCommon.cpp globals).
// They are guarded by MU_SCENE_TESTS_ENABLED, set when MUGame is linked.
// Without MUGame, the SKIP macro documents the required behavior.
// =============================================================================

#ifdef MU_SCENE_TESTS_ENABLED

#include "SceneCommon.h"  // CharacterSelectionState, SceneInitializationState
#include "SceneManager.h" // FrameTimingState

// ---------------------------------------------------------------------------
// AC-1 [6-1-1]: Login scene initialization state starts clean
// SceneInitializationState must have all flags false on fresh construction.
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [6-1-1]: SceneInitializationState starts with all flags false", "[scenes][auth][login][6-1-1]")
{
    SceneInitializationState state;

    SECTION("InitLogIn is false on construction")
    {
        REQUIRE(state.GetInitLogIn() == false);
    }

    SECTION("InitLoading is false on construction")
    {
        REQUIRE(state.GetInitLoading() == false);
    }

    SECTION("InitCharacterScene is false on construction")
    {
        REQUIRE(state.GetInitCharacterScene() == false);
    }

    SECTION("InitMainScene is false on construction")
    {
        REQUIRE(state.GetInitMainScene() == false);
    }

    SECTION("EnableMainRender is false on construction")
    {
        REQUIRE(state.GetEnableMainRender() == false);
    }
}

// ---------------------------------------------------------------------------
// AC-1 [6-1-1]: ResetAll clears all scene initialization flags
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [6-1-1]: SceneInitializationState ResetAll clears all flags", "[scenes][auth][login][6-1-1]")
{
    SceneInitializationState state;

    // Set all flags via setters
    state.SetInitLogIn(true);
    state.SetInitLoading(true);
    state.SetInitCharacterScene(true);
    state.SetInitMainScene(true);
    state.SetEnableMainRender(true);

    state.ResetAll();

    REQUIRE(state.GetInitLogIn() == false);
    REQUIRE(state.GetInitLoading() == false);
    REQUIRE(state.GetInitCharacterScene() == false);
    REQUIRE(state.GetInitMainScene() == false);
    REQUIRE(state.GetEnableMainRender() == false);
}

// ---------------------------------------------------------------------------
// AC-6 [6-1-1]: ResetForDisconnect resets login/char/main but preserves loading
// This covers the logout → disconnect → re-login flow.
// ---------------------------------------------------------------------------

TEST_CASE("AC-6 [6-1-1]: SceneInitializationState ResetForDisconnect preserves loading flag",
          "[scenes][auth][logout][6-1-1]")
{
    SceneInitializationState state;

    state.SetInitLogIn(true);
    state.SetInitLoading(true);
    state.SetInitCharacterScene(true);
    state.SetInitMainScene(true);
    state.SetEnableMainRender(true);

    state.ResetForDisconnect();

    // Loading must be preserved (not reset on disconnect)
    REQUIRE(state.GetInitLoading() == true);

    // All other flags must be cleared
    REQUIRE(state.GetInitLogIn() == false);
    REQUIRE(state.GetInitCharacterScene() == false);
    REQUIRE(state.GetInitMainScene() == false);
    REQUIRE(state.GetEnableMainRender() == false);
}

// ---------------------------------------------------------------------------
// AC-3 [6-1-1]: CharacterSelectionState starts with no selection
// ---------------------------------------------------------------------------

TEST_CASE("AC-3 [6-1-1]: CharacterSelectionState has no selection on construction", "[scenes][auth][character][6-1-1]")
{
    CharacterSelectionState state;

    REQUIRE(state.HasSelection() == false);
    REQUIRE(state.GetSelectedIndex() == CharacterSelectionState::NO_SELECTION);
}

// ---------------------------------------------------------------------------
// AC-5 [6-1-1]: CharacterSelectionState accepts valid character slot indices
// Valid indices are [0, MAX_CHARACTERS_PER_ACCOUNT).
// ---------------------------------------------------------------------------

TEST_CASE("AC-5 [6-1-1]: CharacterSelectionState accepts valid character slots", "[scenes][auth][character][6-1-1]")
{
    CharacterSelectionState state;

    SECTION("Select first character slot (index 0)")
    {
        state.SelectCharacter(0);
        REQUIRE(state.HasSelection() == true);
        REQUIRE(state.GetSelectedIndex() == 0);
    }

    SECTION("Select last character slot (index MAX_CHARACTERS_PER_ACCOUNT - 1)")
    {
        state.SelectCharacter(MAX_CHARACTERS_PER_ACCOUNT - 1);
        REQUIRE(state.HasSelection() == true);
        REQUIRE(state.GetSelectedIndex() == MAX_CHARACTERS_PER_ACCOUNT - 1);
    }

    SECTION("Out-of-bounds index is rejected (index < 0)")
    {
        state.SelectCharacter(2); // establish valid selection first
        state.SelectCharacter(-1);
        REQUIRE(state.HasSelection() == true);
        REQUIRE(state.GetSelectedIndex() == 2); // preserved
    }

    SECTION("Out-of-bounds index is rejected (index >= MAX_CHARACTERS_PER_ACCOUNT)")
    {
        state.SelectCharacter(2); // establish valid selection first
        state.SelectCharacter(MAX_CHARACTERS_PER_ACCOUNT);
        REQUIRE(state.HasSelection() == true);
        REQUIRE(state.GetSelectedIndex() == 2); // preserved
    }

    SECTION("Invalid selection does not clear existing valid selection")
    {
        state.SelectCharacter(2);
        state.SelectCharacter(-1);
        REQUIRE(state.HasSelection() == true);
        REQUIRE(state.GetSelectedIndex() == 2);
    }
}

// ---------------------------------------------------------------------------
// AC-6 [6-1-1]: CharacterSelectionState ClearSelection returns to NO_SELECTION
// This simulates the logout / character switch flow.
// ---------------------------------------------------------------------------

TEST_CASE("AC-6 [6-1-1]: CharacterSelectionState ClearSelection returns to NO_SELECTION",
          "[scenes][auth][logout][6-1-1]")
{
    CharacterSelectionState state;

    state.SelectCharacter(2);
    REQUIRE(state.HasSelection() == true);

    state.ClearSelection();

    REQUIRE(state.HasSelection() == false);
    REQUIRE(state.GetSelectedIndex() == CharacterSelectionState::NO_SELECTION);
}

// ---------------------------------------------------------------------------
// AC-5 [6-1-1]: FrameTimingState — scene transitions render when frame is due
// SceneManager uses FrameTimingState to drive the game loop. On cross-platform
// builds, the render tick must advance correctly for scene transitions to work.
// ---------------------------------------------------------------------------

// AC-5: Frame timing controls scene transition readiness.
// ShouldRenderNextFrame() == false during frame limiting (e.g., login -> character select).
// ShouldRenderNextFrame() == true when frame is due for rendering.
TEST_CASE("AC-5 [6-1-1]: FrameTimingState ShouldRenderNextFrame controls scene loop", "[scenes][auth][timing][6-1-1]")
{
    FrameTimingState timing;

    SECTION("Uncapped mode: always renders next frame (msPerFrame <= 0)")
    {
        timing.SetTargetFps(-1);
        timing.UpdateCurrentTime(0.0);
        REQUIRE(timing.ShouldRenderNextFrame() == true);
    }

    SECTION("Capped mode: renders when elapsed time >= msPerFrame")
    {
        timing.SetTargetFps(60.0); // 60 FPS = 16.67 ms/frame
        timing.UpdateCurrentTime(0.0);
        timing.MarkFrameRendered();

        // Advance time by exactly 1 frame worth
        timing.UpdateCurrentTime(1000.0 / 60.0 + 1.0);
        REQUIRE(timing.ShouldRenderNextFrame() == true);
    }

    SECTION("Capped mode: does NOT render before frame time elapsed")
    {
        timing.SetTargetFps(60.0);
        timing.UpdateCurrentTime(0.0);
        timing.MarkFrameRendered();

        // Advance by less than 1 frame
        timing.UpdateCurrentTime(5.0); // 5ms < 16.67ms
        REQUIRE(timing.ShouldRenderNextFrame() == false);
    }
}

#else // MU_SCENE_TESTS_ENABLED not defined

// ---------------------------------------------------------------------------
// Placeholder tests documenting AC-1/5/6 scene state contracts.
// These are compiled as SKIP when MUGame is not linked (macOS quality gates,
// standalone MuTests build without MUGame).
//
// To enable: add -DMU_SCENE_TESTS_ENABLED and link MuTests against MUGame.
// See story 6-1-1 CMakeLists.txt comment for enablement path.
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [6-1-1]: SceneInitializationState logic — requires MUGame linkage", "[scenes][auth][login][6-1-1]")
{
    SKIP("MU_SCENE_TESTS_ENABLED not defined: SceneCommon.h needs MUGame. "
         "Set -DMU_SCENE_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("AC-5/AC-6 [6-1-1]: CharacterSelectionState logic — requires MUGame linkage",
          "[scenes][auth][character][6-1-1]")
{
    SKIP("MU_SCENE_TESTS_ENABLED not defined: SceneCommon.h needs MUGame. "
         "Set -DMU_SCENE_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

#endif // MU_SCENE_TESTS_ENABLED
