// Story 7.8.2: Gameplay Header Cross-Platform Fixes [VS0-QUAL-BUILD-HEADERS]
// RED PHASE: Tests verify that gameplay headers are self-contained and ODR-safe.
//
// AC coverage:
//   AC-1  — mu_enum.h: SKILL_REPLACEMENTS must be declared 'inline' to avoid ODR
//            linker errors when included by multiple translation units.
//            RED: Including mu_enum.h here (alongside test_combat_system_validation.cpp
//                 and test_ui_windows_validation.cpp, which also include mu_enum.h)
//                 causes duplicate-symbol linker error on macOS/Linux until 'inline' is
//                 added. The link failure IS the RED phase failure for AC-1.
//            GREEN: 'inline' added → each TU gets its own copy → linker succeeds.
//   AC-2  — ZzzPath.h: self-contained include (verified at compile/build level via
//            CMake script test_ac2_zzzpath_errorreport_include_7_8_2.cmake).
//   AC-3  — SkillStructs.h: CMultiLanguage properly included (CMake script test).
//   AC-4  — CSItemOption.h: ActionSkillType and ITEM properly included (CMake script test).
//   AC-5  — cmake --build --preset macos-arm64-debug: verified by CI, not unit test.
//   AC-6  — ./ctl check: verified by CI quality gate, not unit test.
//
// Design notes:
//   - mu_enum.h: pure header (enums + const map), platform-independent, no Win32.
//   - SKILL_REPLACEMENTS runtime behavior is testable standalone: map is non-empty,
//     known entries are present, forward (str → base) replacements are consistent.
//   - ZzzPath.h / SkillStructs.h / CSItemOption.h: have Win32 dependencies so they
//     are not included here. Their include correctness is verified by CMake script tests.
//   - No Win32 API calls in this test translation unit.
//   - Framework: Catch2 v3.7.1 — REQUIRE/CHECK macros, TEST_CASE/SECTION structure.
//
// Allman brace style, 4-space indent, 120-column limit (per .clang-format).

#include <catch2/catch_test_macros.hpp>

#ifdef _WIN32
#include "PlatformTypes.h"
#else
#include "PlatformTypes.h"
#endif

#include "mu_enum.h"

// ---------------------------------------------------------------------------
// AC-1: SKILL_REPLACEMENTS is accessible and non-empty
//
// Including mu_enum.h from this TU (alongside test_combat_system_validation.cpp
// and test_ui_windows_validation.cpp) exercises the ODR constraint. On macOS/Linux:
//   RED:   SKILL_REPLACEMENTS defined as 'const' (external linkage) in mu_enum.h →
//          multiple TUs define the same symbol → linker error (duplicate symbol).
//   GREEN: SKILL_REPLACEMENTS defined as 'inline const' → each TU gets a distinct
//          copy → linker succeeds → the tests below execute and pass.
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [7-8-2]: SKILL_REPLACEMENTS map is non-empty and accessible", "[gameplay][headers][7-8-2]")
{
    SECTION("SKILL_REPLACEMENTS contains entries (map is initialized)")
    {
        // GIVEN: mu_enum.h included — this translation unit contributes to ODR test
        // WHEN: accessing SKILL_REPLACEMENTS
        // THEN: map is non-empty (proves it links without ODR error in GREEN phase)
        REQUIRE(!SKILL_REPLACEMENTS.empty());
    }
}

TEST_CASE("AC-1 [7-8-2]: SKILL_REPLACEMENTS contains known str-to-base skill mappings",
          "[gameplay][headers][skill-replacements][7-8-2]")
{
    SECTION("AT_SKILL_POISON_STR maps to AT_SKILL_POISON")
    {
        // GIVEN: The poison str variant should resolve to the base poison skill
        auto it = SKILL_REPLACEMENTS.find(AT_SKILL_POISON_STR);

        // THEN: mapping exists
        REQUIRE(it != SKILL_REPLACEMENTS.end());
        CHECK(it->second == AT_SKILL_POISON);
    }

    SECTION("AT_SKILL_LIGHTNING_STR maps to AT_SKILL_LIGHTNING")
    {
        auto it = SKILL_REPLACEMENTS.find(AT_SKILL_LIGHTNING_STR);
        REQUIRE(it != SKILL_REPLACEMENTS.end());
        CHECK(it->second == AT_SKILL_LIGHTNING);
    }

    SECTION("AT_SKILL_FLAME_STR maps to AT_SKILL_FLAME")
    {
        auto it = SKILL_REPLACEMENTS.find(AT_SKILL_FLAME_STR);
        REQUIRE(it != SKILL_REPLACEMENTS.end());
        CHECK(it->second == AT_SKILL_FLAME);
    }

    SECTION("AT_SKILL_ICE_STR maps to AT_SKILL_ICE")
    {
        auto it = SKILL_REPLACEMENTS.find(AT_SKILL_ICE_STR);
        REQUIRE(it != SKILL_REPLACEMENTS.end());
        CHECK(it->second == AT_SKILL_ICE);
    }

    SECTION("AT_SKILL_FALLING_SLASH_STR maps to AT_SKILL_FALLING_SLASH")
    {
        auto it = SKILL_REPLACEMENTS.find(AT_SKILL_FALLING_SLASH_STR);
        REQUIRE(it != SKILL_REPLACEMENTS.end());
        CHECK(it->second == AT_SKILL_FALLING_SLASH);
    }
}

TEST_CASE("AC-1 [7-8-2]: SKILL_REPLACEMENTS map values are distinct from their keys",
          "[gameplay][headers][skill-replacements][7-8-2]")
{
    SECTION("All entries map str-variant to a different base-skill (no identity mappings)")
    {
        // Every entry in SKILL_REPLACEMENTS should have key != value:
        // the 'str' (strengthened) variant is always different from the base skill.
        for (const auto& [from, to] : SKILL_REPLACEMENTS)
        {
            // Soul barrier proficiency chains str→str, which is key≠value, so this passes.
            // If any entry mapped a skill to itself that would indicate a data error.
            CHECK(from != to);
        }
    }
}
