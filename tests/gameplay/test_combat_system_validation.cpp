// Story 6.2.1: Combat System Validation [VS1-GAME-VALIDATE-COMBAT]
// RED PHASE: Tests define expected logical contracts for combat system validation.
//
// AC coverage (unit-testable subset — see manual test scenarios for full AC validation):
//   AC-1       — Melee attacks: ActionSkillType values for melee attack skills
//                (AT_SKILL_FALLING_SLASH, AT_SKILL_LUNGE, AT_SKILL_UPPERCUT, AT_SKILL_CYCLONE, AT_SKILL_SLASH)
//   AC-2       — Skill system: SKILL_ATTRIBUTE struct constants (MAX_CLASS, MAX_DUTY_CLASS, MAX_SKILL_NAME),
//                DemendConditionInfo requirement checking, ActionSkillType magic skill enumeration
//   AC-3       — Monster death/loot: MonsterSkillType enum base values and skill range
//   AC-4       — Player death/respawn: AT_SKILL_UNDEFINED initial state, AT_SKILL_MASTER_END range
//   AC-5       — Health/mana: MAX_SKILLS capacity, SKILL_ATTRIBUTE Mana/Damage field read-write
//   AC-6       — Combat audio: SOUND_BRANDISH_SWORD01..04 values, SOUND_ATTACK_MELEE_HIT range,
//                SOUND_MONSTER range (210-450),
//                melee hit sound range, non-overlapping sound region validation
//
// Manual validation (full AC-1..6 on macOS/Linux/Windows with live server):
//   See: _bmad-output/test-scenarios/epic-6/combat-system-validation.md
//
// PCC Constraints:
//   - Platform-compatible includes: #ifdef _WIN32 / PlatformTypes.h on non-Win32
//   - No Win32 API calls in test logic — test logic only
//   - Catch2 TEST_CASE / SECTION / REQUIRE structure
//   - Allman brace style, 4-space indent, 120-column limit
//   - No raw new/delete in test code
//   - #pragma once not used (this is a .cpp file)
//
// Design notes:
//   - ActionSkillType: Pure enum in mu_enum.h — testable standalone.
//   - MonsterSkillType: Pure enum in mu_enum.h — testable standalone.
//   - SKILL_ATTRIBUTE: Struct in Data/Skills/SkillStructs.h — platform-compatible, testable standalone.
//   - DemendConditionInfo: Struct in SkillManager.h — pure logic (operator<= on WORD/int fields), testable standalone.
//   - ESound/SOUND_BRANDISH_SWORD*: Enum and macros in Audio/DSPlaySound.h — testable standalone.
//   - Script_Skill/CSkillManager runtime: gated by MU_COMBAT_TESTS_ENABLED (requires MUGame linkage).
//
// MUCommon INTERFACE propagates all src/source/ subdirectory include paths to MuTests.
// No additional target_include_directories entries needed for this test file.

#include <catch2/catch_test_macros.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#endif

// Forward declarations for types referenced only in function-signature parameters of included headers.
// These are not used in test logic — only required to allow header compilation.
class OBJECT;
class CHARACTER;

#include "mu_define.h" // MAX_SKILLS, DWORD, WORD, BYTE, game constants
#include "mu_enum.h"   // ActionSkillType, MonsterSkillType

#include "CSItemOption.h" // ITEM_SET_TYPE, ITEM_SET_OPTION, SET_OPTION, MAX_SET_OPTION, MASTERY_OPTION
#include "DSPlaySound.h"  // ESound enum, SOUND_BRANDISH_SWORD*, SOUND_MONSTER* macros
#include "SkillManager.h" // DemendConditionInfo struct
#include "SkillStructs.h" // SKILL_ATTRIBUTE, SKILL_ATTRIBUTE_FILE, MAX_CLASS, MAX_DUTY_CLASS, MAX_SKILL_NAME

// =============================================================================
// AC-1 [6-2-1]: ActionSkillType — melee attack skill identifiers
// SetPlayerAttack() and AttackStage() use ActionSkillType to identify which attack
// animation to play. These enum values must be stable across platforms for combat
// animations to render identically on macOS/Linux/Windows.
// =============================================================================

TEST_CASE("AC-1 [6-2-1]: ActionSkillType defines correct IDs for melee attack skills", "[combat][skills][melee][6-2-1]")
{
    SECTION("AT_SKILL_UNDEFINED is 0 (no-skill / initial state sentinel)")
    {
        REQUIRE(static_cast<int>(AT_SKILL_UNDEFINED) == 0);
    }

    SECTION("AT_SKILL_FALLING_SLASH is 19 (Dark Knight melee skill)")
    {
        REQUIRE(static_cast<int>(AT_SKILL_FALLING_SLASH) == 19);
    }

    SECTION("AT_SKILL_LUNGE is 20")
    {
        REQUIRE(static_cast<int>(AT_SKILL_LUNGE) == 20);
    }

    SECTION("AT_SKILL_UPPERCUT is 21")
    {
        REQUIRE(static_cast<int>(AT_SKILL_UPPERCUT) == 21);
    }

    SECTION("AT_SKILL_CYCLONE is 22")
    {
        REQUIRE(static_cast<int>(AT_SKILL_CYCLONE) == 22);
    }

    SECTION("AT_SKILL_SLASH is 23")
    {
        REQUIRE(static_cast<int>(AT_SKILL_SLASH) == 23);
    }
}

TEST_CASE("AC-1 [6-2-1]: ActionSkillType melee skill IDs are all distinct", "[combat][skills][melee][6-2-1]")
{
    // All 10 pairs among the 5 melee skill values must be unique to ensure
    // correct attack animation dispatch in SetPlayerAttack() / AttackStage().
    const int kFallingSlash = static_cast<int>(AT_SKILL_FALLING_SLASH);
    const int kLunge = static_cast<int>(AT_SKILL_LUNGE);
    const int kUppercut = static_cast<int>(AT_SKILL_UPPERCUT);
    const int kCyclone = static_cast<int>(AT_SKILL_CYCLONE);
    const int kSlash = static_cast<int>(AT_SKILL_SLASH);

    REQUIRE(kFallingSlash != kLunge);
    REQUIRE(kFallingSlash != kUppercut);
    REQUIRE(kFallingSlash != kCyclone);
    REQUIRE(kFallingSlash != kSlash);
    REQUIRE(kLunge != kUppercut);
    REQUIRE(kLunge != kCyclone);
    REQUIRE(kLunge != kSlash);
    REQUIRE(kUppercut != kCyclone);
    REQUIRE(kUppercut != kSlash);
    REQUIRE(kCyclone != kSlash);
}

// =============================================================================
// AC-2 [6-2-1]: Skill system — SKILL_ATTRIBUTE struct layout and DemendConditionInfo
// SKILL_ATTRIBUTE stores all per-skill data loaded from the BMD file.
// DemendConditionInfo encodes the stat prerequisites for skill activation via operator<=.
// These must have stable layouts across platforms for correct skill data parsing and gating.
// =============================================================================

TEST_CASE("AC-2 [6-2-1]: Skill attribute struct constants define correct array dimensions",
          "[combat][skills][data][6-2-1]")
{
    SECTION("MAX_CLASS is 7 (number of character classes with class-specific skill requirements)")
    {
        REQUIRE(MAX_CLASS == 7);
    }

    SECTION("MAX_DUTY_CLASS is 3 (duty class array dimension in SKILL_ATTRIBUTE)")
    {
        REQUIRE(MAX_DUTY_CLASS == 3);
    }

    SECTION("MAX_SKILL_NAME is 50 (name buffer character capacity in file and runtime structs)")
    {
        REQUIRE(MAX_SKILL_NAME == 50);
    }
}

TEST_CASE("AC-2 [6-2-1]: SKILL_ATTRIBUTE_FILE Name buffer has correct byte size for BMD parsing",
          "[combat][skills][data][6-2-1]")
{
    // SKILL_ATTRIBUTE_FILE is used for reading current BMD skill data files.
    // Its char Name[MAX_SKILL_NAME] must be exactly MAX_SKILL_NAME bytes for correct binary parsing.
    REQUIRE(sizeof(SKILL_ATTRIBUTE_FILE::Name) == static_cast<size_t>(MAX_SKILL_NAME) * sizeof(char));
}

TEST_CASE("AC-2 [6-2-1]: SKILL_ATTRIBUTE runtime Name buffer has wide-character size", "[combat][skills][data][6-2-1]")
{
    // SKILL_ATTRIBUTE is the in-memory runtime struct.
    // Its wchar_t Name[MAX_SKILL_NAME] must be MAX_SKILL_NAME * sizeof(wchar_t) bytes.
    REQUIRE(sizeof(SKILL_ATTRIBUTE::Name) == static_cast<size_t>(MAX_SKILL_NAME) * sizeof(wchar_t));
}

TEST_CASE("AC-2 [6-2-1]: DemendConditionInfo default-constructs with all stat requirements at zero",
          "[combat][skills][requirements][6-2-1]")
{
    DemendConditionInfo info;

    SECTION("SkillType initialised to 0 (undefined skill type)")
    {
        REQUIRE(info.SkillType == 0);
    }

    SECTION("SkillLevel initialised to 0 (no minimum level requirement)")
    {
        REQUIRE(info.SkillLevel == 0);
    }

    SECTION("SkillStrength initialised to 0")
    {
        REQUIRE(info.SkillStrength == 0);
    }

    SECTION("SkillDexterity initialised to 0")
    {
        REQUIRE(info.SkillDexterity == 0);
    }

    SECTION("SkillVitality initialised to 0")
    {
        REQUIRE(info.SkillVitality == 0);
    }

    SECTION("SkillEnergy initialised to 0")
    {
        REQUIRE(info.SkillEnergy == 0);
    }

    SECTION("SkillCharisma initialised to 0")
    {
        REQUIRE(info.SkillCharisma == 0);
    }
}

TEST_CASE("AC-2 [6-2-1]: DemendConditionInfo operator<= validates all stat thresholds",
          "[combat][skills][requirements][6-2-1]")
{
    // heroStats represents the hero's current stat values.
    // A skill requirement (basicInfo) is met when basicInfo <= heroStats.
    DemendConditionInfo heroStats;
    heroStats.SkillLevel = 10;
    heroStats.SkillStrength = 100;
    heroStats.SkillDexterity = 80;
    heroStats.SkillVitality = 50;
    heroStats.SkillEnergy = 200;
    heroStats.SkillCharisma = 30;

    SECTION("Requirements met when hero has exactly the required stats")
    {
        DemendConditionInfo meetsAll;
        meetsAll.SkillLevel = 10;
        meetsAll.SkillStrength = 100;
        meetsAll.SkillDexterity = 80;
        meetsAll.SkillVitality = 50;
        meetsAll.SkillEnergy = 200;
        meetsAll.SkillCharisma = 30;
        REQUIRE(meetsAll <= heroStats);
    }

    SECTION("Requirements met when hero exceeds all required stats")
    {
        DemendConditionInfo lower;
        lower.SkillLevel = 5;
        lower.SkillStrength = 50;
        lower.SkillDexterity = 40;
        lower.SkillVitality = 25;
        lower.SkillEnergy = 100;
        lower.SkillCharisma = 15;
        REQUIRE(lower <= heroStats);
    }

    SECTION("Requirements not met when one stat (strength) exceeds hero's value by 1")
    {
        DemendConditionInfo tooHigh;
        tooHigh.SkillLevel = 10;
        tooHigh.SkillStrength = 101; // one point over hero's 100
        tooHigh.SkillDexterity = 80;
        tooHigh.SkillVitality = 50;
        tooHigh.SkillEnergy = 200;
        tooHigh.SkillCharisma = 30;
        REQUIRE_FALSE(tooHigh <= heroStats);
    }

    SECTION("Requirements not met when one stat (energy) exceeds hero's value by 1")
    {
        DemendConditionInfo highEnergy;
        highEnergy.SkillLevel = 10;
        highEnergy.SkillStrength = 100;
        highEnergy.SkillDexterity = 80;
        highEnergy.SkillVitality = 50;
        highEnergy.SkillEnergy = 201; // one point over hero's 200
        highEnergy.SkillCharisma = 30;
        REQUIRE_FALSE(highEnergy <= heroStats);
    }

    SECTION("Requirements not met when one stat (level) exceeds hero's value by 1")
    {
        DemendConditionInfo highLevel;
        highLevel.SkillLevel = 11;  // one point over hero's 10
        highLevel.SkillStrength = 100;
        highLevel.SkillDexterity = 80;
        highLevel.SkillVitality = 50;
        highLevel.SkillEnergy = 200;
        highLevel.SkillCharisma = 30;
        REQUIRE_FALSE(highLevel <= heroStats);
    }

    SECTION("Requirements not met when one stat (dexterity) exceeds hero's value by 1")
    {
        DemendConditionInfo highDex;
        highDex.SkillLevel = 10;
        highDex.SkillStrength = 100;
        highDex.SkillDexterity = 81;  // one point over hero's 80
        highDex.SkillVitality = 50;
        highDex.SkillEnergy = 200;
        highDex.SkillCharisma = 30;
        REQUIRE_FALSE(highDex <= heroStats);
    }

    SECTION("Requirements not met when one stat (vitality) exceeds hero's value by 1")
    {
        DemendConditionInfo highVit;
        highVit.SkillLevel = 10;
        highVit.SkillStrength = 100;
        highVit.SkillDexterity = 80;
        highVit.SkillVitality = 51;  // one point over hero's 50
        highVit.SkillEnergy = 200;
        highVit.SkillCharisma = 30;
        REQUIRE_FALSE(highVit <= heroStats);
    }

    SECTION("Requirements not met when one stat (charisma) exceeds hero's value by 1")
    {
        DemendConditionInfo highChar;
        highChar.SkillLevel = 10;
        highChar.SkillStrength = 100;
        highChar.SkillDexterity = 80;
        highChar.SkillVitality = 50;
        highChar.SkillEnergy = 200;
        highChar.SkillCharisma = 31;  // one point over hero's 30
        REQUIRE_FALSE(highChar <= heroStats);
    }

    SECTION("Zero requirements (default-constructed) are always met by any hero")
    {
        DemendConditionInfo noReqs;
        REQUIRE(noReqs <= heroStats);
    }
}

TEST_CASE("AC-2 [6-2-1]: ActionSkillType magic and support skill values", "[combat][skills][magic][6-2-1]")
{
    SECTION("AT_SKILL_POISON is 1 (first magic skill)")
    {
        REQUIRE(static_cast<int>(AT_SKILL_POISON) == 1);
    }

    SECTION("AT_SKILL_METEO is 2")
    {
        REQUIRE(static_cast<int>(AT_SKILL_METEO) == 2);
    }

    SECTION("AT_SKILL_LIGHTNING is 3")
    {
        REQUIRE(static_cast<int>(AT_SKILL_LIGHTNING) == 3);
    }

    SECTION("AT_SKILL_FIREBALL is 4")
    {
        REQUIRE(static_cast<int>(AT_SKILL_FIREBALL) == 4);
    }

    SECTION("AT_SKILL_SOUL_BARRIER is 16 (defensive magic skill)")
    {
        REQUIRE(static_cast<int>(AT_SKILL_SOUL_BARRIER) == 16);
    }

    SECTION("AT_SKILL_HEALING is 26 (Elf healing skill)")
    {
        REQUIRE(static_cast<int>(AT_SKILL_HEALING) == 26);
    }
}

// =============================================================================
// AC-3 [6-2-1]: Monster skill system — MonsterSkillType enum for monster behavior
// ZzzAI.h uses MonsterSkillType to dispatch monster special attacks, death behaviors,
// and loot drop triggers. These values must be stable for correct AI dispatch across
// platforms.
// =============================================================================

TEST_CASE("AC-3 [6-2-1]: MonsterSkillType defines correct base values for monster behavior",
          "[combat][monsters][skills][6-2-1]")
{
    SECTION("ATMON_SKILL_BIGIN is 0 (sentinel / begin value)")
    {
        REQUIRE(static_cast<int>(ATMON_SKILL_BIGIN) == 0);
    }

    SECTION("ATMON_SKILL_THUNDER is 1")
    {
        REQUIRE(static_cast<int>(ATMON_SKILL_THUNDER) == 1);
    }

    SECTION("ATMON_SKILL_WIND is 2")
    {
        REQUIRE(static_cast<int>(ATMON_SKILL_WIND) == 2);
    }

    SECTION("ATMON_SKILL_NORMAL is 18 (standard monster attack — most common behavior)")
    {
        REQUIRE(static_cast<int>(ATMON_SKILL_NORMAL) == 18);
    }

    SECTION("ATMON_SKILL_SUMMON is 20 (boss summon mechanic)")
    {
        REQUIRE(static_cast<int>(ATMON_SKILL_SUMMON) == 20);
    }
}

TEST_CASE("AC-3 [6-2-1]: MonsterSkillType basic skill values are all distinct", "[combat][monsters][skills][6-2-1]")
{
    // Basic monster skill types must have unique values for correct AI dispatch.
    // All 10 pairwise combinations among the 5 monster skill types must be unique.
    const int kBigin = static_cast<int>(ATMON_SKILL_BIGIN);
    const int kThunder = static_cast<int>(ATMON_SKILL_THUNDER);
    const int kWind = static_cast<int>(ATMON_SKILL_WIND);
    const int kNormal = static_cast<int>(ATMON_SKILL_NORMAL);
    const int kSummon = static_cast<int>(ATMON_SKILL_SUMMON);

    REQUIRE(kBigin != kThunder);
    REQUIRE(kBigin != kWind);
    REQUIRE(kBigin != kNormal);
    REQUIRE(kBigin != kSummon);
    REQUIRE(kThunder != kWind);
    REQUIRE(kThunder != kNormal);
    REQUIRE(kThunder != kSummon);
    REQUIRE(kWind != kNormal);
    REQUIRE(kWind != kSummon);
    REQUIRE(kNormal != kSummon);
}

// =============================================================================
// AC-4 [6-2-1]: Player death and respawn — ActionSkillType skill range validation
// AT_SKILL_UNDEFINED marks the "no active skill" state used in death/respawn.
// AT_SKILL_MASTER_END defines the upper bound for the master skill indexing arrays
// declared as CHARACTER::MasterSkillInfo[AT_SKILL_MASTER_END + 1].
// =============================================================================

TEST_CASE("AC-4 [6-2-1]: AT_SKILL_UNDEFINED is 0 — initial and post-death state sentinel",
          "[combat][death][respawn][6-2-1]")
{
    // AT_SKILL_UNDEFINED == 0 is the sentinel used when no skill is active.
    // After player death and respawn, the active skill must be reset to AT_SKILL_UNDEFINED
    // to clear any in-progress attack or effect.
    REQUIRE(static_cast<int>(AT_SKILL_UNDEFINED) == 0);
}

TEST_CASE("AC-4 [6-2-1]: AT_SKILL_MASTER_END defines master skill index upper bound at 608",
          "[combat][death][respawn][skills][6-2-1]")
{
    // AT_SKILL_MASTER_END == 608: all master skill arrays are sized [AT_SKILL_MASTER_END + 1].
    // This must be stable for CSkillTreeInfo MasterSkillInfo[AT_SKILL_MASTER_END + 1] in CHARACTER
    // struct (mu_struct.h) to not overrun its bounds.
    REQUIRE(static_cast<int>(AT_SKILL_MASTER_END) == 608);
}

TEST_CASE("AC-4 [6-2-1]: AT_SKILL_MASTER_END is within MAX_SKILLS skill array capacity",
          "[combat][death][skills][6-2-1]")
{
    // AT_SKILL_MASTER_END must be strictly less than MAX_SKILLS so the per-character
    // skill arrays (ClassSkill, Skill, SkillDelay, SkillLevel — all sized MAX_SKILLS)
    // are not overrun by master skill index lookups.
    REQUIRE(static_cast<int>(AT_SKILL_MASTER_END) < MAX_SKILLS);
}

// =============================================================================
// AC-5 [6-2-1]: Health/mana bar data — MAX_SKILLS and SKILL_ATTRIBUTE capacity
// SKILL_ATTRIBUTE includes Mana (cost) and Damage fields read by the HUD to display
// skill tooltip info. MAX_SKILLS bounds all per-character skill arrays in CHARACTER.
// =============================================================================

TEST_CASE("AC-5 [6-2-1]: MAX_SKILLS defines per-character skill array capacity as 650",
          "[combat][health][mana][skills][6-2-1]")
{
    // MAX_SKILLS == 650: bounds of all per-character skill arrays
    // (CHARACTER::ClassSkill, Skill, SkillDelay, SkillLevel in mu_struct.h).
    REQUIRE(MAX_SKILLS == 650);
}

TEST_CASE("AC-5 [6-2-1]: SKILL_ATTRIBUTE Mana and Damage fields are independently addressable",
          "[combat][health][mana][skills][6-2-1]")
{
    // Mana (skill activation cost) and Damage (base damage) are WORD fields in SKILL_ATTRIBUTE.
    // The HUD reads Mana to display skill mana cost and Damage for damage tooltip.
    SKILL_ATTRIBUTE attr = {};
    attr.Mana = 50;
    attr.Damage = 100;

    SECTION("Mana field stores and retrieves value correctly")
    {
        REQUIRE(attr.Mana == 50);
    }

    SECTION("Damage field stores and retrieves value correctly")
    {
        REQUIRE(attr.Damage == 100);
    }

    SECTION("Mana and Damage fields are independent (distinct memory locations)")
    {
        // Prove non-aliasing: write to Mana, verify Damage unchanged
        attr.Mana = 42;
        attr.Damage = 42;
        attr.Mana = 99;
        REQUIRE(attr.Damage == 42); // Damage must remain unchanged after Mana write
    }
}

TEST_CASE("AC-5 [6-2-1]: SKILL_ATTRIBUTE RequireClass array has MAX_CLASS entries", "[combat][skills][class][6-2-1]")
{
    // RequireClass[MAX_CLASS] determines which of the 7 character classes can use a skill.
    // This drives skill slot eligibility checks in the HUD (greyed out for ineligible classes).
    SKILL_ATTRIBUTE attr = {};
    REQUIRE(sizeof(attr.RequireClass) == static_cast<size_t>(MAX_CLASS) * sizeof(BYTE));
}

// =============================================================================
// AC-6 [6-2-1]: Combat audio — ESound enum values for sword and monster sounds
// SOUND_BRANDISH_SWORD01..04 play on melee attacks.
// SOUND_ATTACK_MELEE_HIT1..5 play on successful hits.
// SOUND_MONSTER range (macro: 210-450) covers all monster ambient, attack, and death sounds.
// These values must be correct for DSPlaySound to map sound IDs to the right audio files.
// =============================================================================

TEST_CASE("AC-6 [6-2-1]: SOUND_BRANDISH_SWORD enum values for melee attack swing sounds",
          "[combat][audio][melee][6-2-1]")
{
    SECTION("SOUND_BRANDISH_SWORD01 is 60")
    {
        REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD01) == 60);
    }

    SECTION("SOUND_BRANDISH_SWORD02 is 61")
    {
        REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD02) == 61);
    }

    SECTION("SOUND_BRANDISH_SWORD03 is 62")
    {
        REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD03) == 62);
    }

    SECTION("SOUND_BRANDISH_SWORD04 is 63")
    {
        REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD04) == 63);
    }

    SECTION("SOUND_BRANDISH_SWORD01..04 are four consecutive IDs")
    {
        REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD02) == static_cast<int>(SOUND_BRANDISH_SWORD01) + 1);
        REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD03) == static_cast<int>(SOUND_BRANDISH_SWORD02) + 1);
        REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD04) == static_cast<int>(SOUND_BRANDISH_SWORD03) + 1);
    }
}

TEST_CASE("AC-6 [6-2-1]: SOUND_ATTACK_MELEE_HIT sounds form a range of 5 consecutive IDs",
          "[combat][audio][melee][6-2-1]")
{
    // SOUND_ATTACK_MELEE_HIT1..5 are the five melee hit sound variants played on successful attacks.
    SECTION("SOUND_ATTACK_MELEE_HIT1 is 70")
    {
        REQUIRE(static_cast<int>(SOUND_ATTACK_MELEE_HIT1) == 70);
    }

    SECTION("SOUND_ATTACK_MELEE_HIT5 is 74")
    {
        REQUIRE(static_cast<int>(SOUND_ATTACK_MELEE_HIT5) == 74);
    }

    SECTION("Melee hit sounds span exactly 5 IDs (HIT1 to HIT5 inclusive)")
    {
        const int first = static_cast<int>(SOUND_ATTACK_MELEE_HIT1);
        const int last = static_cast<int>(SOUND_ATTACK_MELEE_HIT5);
        REQUIRE(last - first == 4);
    }
}

TEST_CASE("AC-6 [6-2-1]: SOUND_MONSTER macro range covers IDs 210 to 450", "[combat][audio][monsters][6-2-1]")
{
    // SOUND_MONSTER and SOUND_MONSTER_END are preprocessor macros defining the range of
    // sound indices reserved for monster ambient, attack, and death sounds.
    SECTION("SOUND_MONSTER starts at 210")
    {
        REQUIRE(SOUND_MONSTER == 210);
    }

    SECTION("SOUND_MONSTER_END is 450")
    {
        REQUIRE(SOUND_MONSTER_END == 450);
    }

    SECTION("Monster sound range spans 240 indices (210 to 450 exclusive, 240 slots)")
    {
        REQUIRE(SOUND_MONSTER_END - SOUND_MONSTER == 240);
    }
}

TEST_CASE("AC-6 [6-2-1]: SOUND_MONSTER_BULL1 is the first monster enum entry matching SOUND_MONSTER",
          "[combat][audio][monsters][6-2-1]")
{
    // SOUND_MONSTER_BULL1 == 210: the first monster enum value must equal the SOUND_MONSTER
    // macro so range membership checks (sound >= SOUND_MONSTER) correctly classify it.
    REQUIRE(static_cast<int>(SOUND_MONSTER_BULL1) == SOUND_MONSTER);
}

TEST_CASE("AC-6 [6-2-1]: Combat sound ranges are mutually non-overlapping", "[combat][audio][6-2-1]")
{
    // Sword swing sounds (60-63) must not overlap with hit sounds (70-74).
    REQUIRE(static_cast<int>(SOUND_BRANDISH_SWORD04) < static_cast<int>(SOUND_ATTACK_MELEE_HIT1));

    // Melee hit sounds (70-74) must not overlap with monster sounds (210+).
    REQUIRE(static_cast<int>(SOUND_ATTACK_MELEE_HIT5) < SOUND_MONSTER);
}

// =============================================================================
// Task 2.4 [6-2-1]: Buff system data structures — eBuffState, eBuffClass enums
// w_BuffStateSystem manages buffs/debuffs with value control, time tracking.
// eBuffState defines all buff/debuff types; eBuffClass categorises them.
// These enum values must be stable for correct buff application across platforms.
// =============================================================================

TEST_CASE("Task-2.4 [6-2-1]: eBuffState sentinel and combat-relevant buff values", "[combat][buffs][enum][6-2-1]")
{
    SECTION("eBuffNone is 0 (no buff / initial state)")
    {
        REQUIRE(static_cast<int>(eBuffNone) == 0);
    }

    SECTION("eBuff_Attack is 1 (attack power buff)")
    {
        REQUIRE(static_cast<int>(eBuff_Attack) == 1);
    }

    SECTION("eBuff_Defense is 2 (defense buff)")
    {
        REQUIRE(static_cast<int>(eBuff_Defense) == 2);
    }

    SECTION("eBuff_Berserker is 81 (rage mode combat buff)")
    {
        REQUIRE(static_cast<int>(eBuff_Berserker) == 81);
    }

    SECTION("eBuff_Count is 206 (total buff enum count)")
    {
        REQUIRE(static_cast<int>(eBuff_Count) == 206);
    }
}

TEST_CASE("Task-2.4 [6-2-1]: eBuffState debuff sentinel values for combat effects", "[combat][buffs][debuff][6-2-1]")
{
    SECTION("eDeBuff_Poison is the first debuff entry (follows buff entries)")
    {
        // eDeBuff_Poison comes after the buff entries in the enum, starting the debuff block.
        REQUIRE(static_cast<int>(eDeBuff_Poison) > static_cast<int>(eBuff_SecretPotion5));
    }

    SECTION("eDeBuff_Freeze follows eDeBuff_Poison")
    {
        REQUIRE(static_cast<int>(eDeBuff_Freeze) == static_cast<int>(eDeBuff_Poison) + 1);
    }

    SECTION("eDeBuff_Stun is within the debuff range")
    {
        REQUIRE(static_cast<int>(eDeBuff_Stun) > static_cast<int>(eDeBuff_Poison));
        REQUIRE(static_cast<int>(eDeBuff_Stun) < static_cast<int>(eBuff_Count));
    }

    SECTION("eDeBuff_Sleep is 72 (crowd control debuff)")
    {
        REQUIRE(static_cast<int>(eDeBuff_Sleep) == 72);
    }
}

TEST_CASE("Task-2.4 [6-2-1]: eBuffClass categorises buffs and debuffs", "[combat][buffs][class][6-2-1]")
{
    SECTION("eBuffClass_Buff is 0 (positive effects)")
    {
        REQUIRE(static_cast<int>(eBuffClass_Buff) == 0);
    }

    SECTION("eBuffClass_DeBuff is 1 (negative effects)")
    {
        REQUIRE(static_cast<int>(eBuffClass_DeBuff) == 1);
    }

    SECTION("eBuffClass_Count is 2 (exactly two categories)")
    {
        REQUIRE(static_cast<int>(eBuffClass_Count) == 2);
    }
}

// =============================================================================
// Task 2.5 [6-2-1]: Item combat attributes — CSItemOption constants and structs
// CSItemOption manages ancient set bonuses that affect combat damage.
// ITEM_SET_TYPE maps items to their set membership.
// ITEM_SET_OPTION stores the option bonuses activated by set completion.
// These constants/structs must be stable for correct set bonus calculation.
// =============================================================================

TEST_CASE("Task-2.5 [6-2-1]: Item set system constants define correct capacities", "[combat][items][constants][6-2-1]")
{
    SECTION("MAX_SET_OPTION is 64 (maximum number of ancient sets)")
    {
        REQUIRE(MAX_SET_OPTION == 64);
    }

    SECTION("MASTERY_OPTION is 24 (mastery option that increases a specific skill)")
    {
        REQUIRE(MASTERY_OPTION == 24);
    }

    SECTION("MAX_EQUIPPED_SETS is 5 (max concurrent equipped sets)")
    {
        REQUIRE(MAX_EQUIPPED_SETS == 5);
    }

    SECTION("MAX_EQUIPMENT_INDEX is 12 (equipment slot count)")
    {
        REQUIRE(MAX_EQUIPMENT_INDEX == 12);
    }

    SECTION("MAX_ITEM is 8192 (MAX_ITEM_TYPE * MAX_ITEM_INDEX)")
    {
        REQUIRE(MAX_ITEM == 8192);
    }
}

TEST_CASE("Task-2.5 [6-2-1]: ITEM_SET_TYPE struct has correct array dimensions", "[combat][items][struct][6-2-1]")
{
    ITEM_SET_TYPE setType = {};

    SECTION("byOption array has MAX_ITEM_SETS_PER_ITEM (2) entries")
    {
        REQUIRE(setType.byOption.size() == MAX_ITEM_SETS_PER_ITEM);
    }

    SECTION("byMixItemLevel array has MAX_ITEM_SETS_PER_ITEM (2) entries")
    {
        REQUIRE(setType.byMixItemLevel.size() == MAX_ITEM_SETS_PER_ITEM);
    }

    SECTION("Default-constructed arrays are zero-initialised")
    {
        REQUIRE(setType.byOption[0] == 0);
        REQUIRE(setType.byOption[1] == 0);
        REQUIRE(setType.byMixItemLevel[0] == 0);
        REQUIRE(setType.byMixItemLevel[1] == 0);
    }
}

TEST_CASE("Task-2.5 [6-2-1]: ITEM_SET_OPTION struct has correct nested array dimensions",
          "[combat][items][struct][6-2-1]")
{
    ITEM_SET_OPTION setOption = {};

    SECTION("byStandardOption has MAX_ITEM_SET_STANDARD_OPTION_COUNT (6) rows")
    {
        REQUIRE(setOption.byStandardOption.size() == MAX_ITEM_SET_STANDARD_OPTION_COUNT);
    }

    SECTION("Each byStandardOption row has MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT (2) columns")
    {
        REQUIRE(setOption.byStandardOption[0].size() == MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT);
    }

    SECTION("byFullOption has MAX_ITEM_SET_FULL_OPTION_COUNT (5) entries")
    {
        REQUIRE(setOption.byFullOption.size() == MAX_ITEM_SET_FULL_OPTION_COUNT);
    }

    SECTION("byRequireClass has MAX_CLASS (7) entries")
    {
        REQUIRE(setOption.byRequireClass.size() == static_cast<size_t>(MAX_CLASS));
    }

    SECTION("bySetItemCount default-constructs to 0")
    {
        REQUIRE(setOption.bySetItemCount == 0);
    }
}

TEST_CASE("Task-2.5 [6-2-1]: SET_OPTION struct fields are independently addressable", "[combat][items][struct][6-2-1]")
{
    SET_OPTION opt = {};
    opt.IsActive = true;
    opt.IsFullOption = false;
    opt.IsExtOption = true;
    opt.FulfillsClassRequirement = true;
    opt.OptionNumber = 42;
    opt.Value = 150;

    SECTION("IsActive field stores and retrieves correctly")
    {
        REQUIRE(opt.IsActive == true);
    }

    SECTION("IsFullOption field stores and retrieves correctly")
    {
        REQUIRE(opt.IsFullOption == false);
    }

    SECTION("IsExtOption field stores and retrieves correctly")
    {
        REQUIRE(opt.IsExtOption == true);
    }

    SECTION("FulfillsClassRequirement field stores and retrieves correctly")
    {
        REQUIRE(opt.FulfillsClassRequirement == true);
    }

    SECTION("OptionNumber field stores and retrieves correctly")
    {
        REQUIRE(opt.OptionNumber == 42);
    }

    SECTION("Value field stores and retrieves correctly")
    {
        REQUIRE(opt.Value == 150);
    }
}

// =============================================================================
// AC-1..5 [6-2-1]: Tests requiring MUGame linkage (CSkillManager, Script_Skill, combat state)
// These tests need non-inline implementations or heavy-dependency types from MUGame:
//   - CSkillManager::GetSkillInformation() — reads g_SkillAttribute data (MUGame data segment)
//   - CSkillManager::CheckSkillDelay()     — accesses CHARACTER::SkillDelay array (MUGame)
//   - Script_Skill struct                  — in mu_struct.h (pulls CHARACTER/OBJECT from MUGame)
//   - SetPlayerAttack(), AttackStage()     — non-inline in ZzzCharacter.cpp (MUGame target)
//
// Enable by defining MU_COMBAT_TESTS_ENABLED and linking MuTests against MUGame.
// =============================================================================

#ifdef MU_COMBAT_TESTS_ENABLED
#error "MU_COMBAT_TESTS_ENABLED is defined but MUGame-linked combat tests for story 6-2-1 are \
not yet implemented. Define this flag only when ready to add real test implementations for \
CSkillManager runtime behavior, Script_Skill struct layout, and ZzzCharacter attack state machine."
#else

TEST_CASE("AC-1 [6-2-1]: CSkillManager::GetSkillInformation runtime lookup — requires MUGame linkage",
          "[combat][skills][manager][6-2-1]")
{
    SKIP("MU_COMBAT_TESTS_ENABLED not defined: CSkillManager::GetSkillInformation reads "
         "g_SkillAttribute (MUGame data segment). "
         "Set -DMU_COMBAT_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("AC-2 [6-2-1]: CSkillManager::CheckSkillDelay activation gating — requires MUGame linkage",
          "[combat][skills][delay][6-2-1]")
{
    SKIP("MU_COMBAT_TESTS_ENABLED not defined: CSkillManager::CheckSkillDelay accesses "
         "CHARACTER::SkillDelay array (MUGame). "
         "Set -DMU_COMBAT_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("AC-3 [6-2-1]: Script_Skill array capacity (MAX_MONSTERSKILL_NUM slots) — requires MUGame linkage",
          "[combat][monsters][struct][6-2-1]")
{
    SKIP("MU_COMBAT_TESTS_ENABLED not defined: Script_Skill is defined in mu_struct.h which "
         "includes w_ObjectInfo.h and w_CharacterInfo.h (MUGame). "
         "Set -DMU_COMBAT_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("AC-1 [6-2-1]: SetPlayerAttack and AttackStage state transitions — requires MUGame linkage",
          "[combat][attack][animation][6-2-1]")
{
    SKIP("MU_COMBAT_TESTS_ENABLED not defined: SetPlayerAttack() and AttackStage() are "
         "non-inline methods in ZzzCharacter.cpp (MUGame target). "
         "Set -DMU_COMBAT_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("Task-2.4 [6-2-1]: w_BuffStateSystem RegisterBuff/UnRegisterBuff runtime — requires MUGame linkage",
          "[combat][buffs][runtime][6-2-1]")
{
    SKIP("MU_COMBAT_TESTS_ENABLED not defined: w_BuffStateSystem uses WindowMessageHandler "
         "and SmartPointer (MUGame). BuffScriptLoader loads buff definitions at runtime. "
         "Set -DMU_COMBAT_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("Task-2.5 [6-2-1]: GetAttackDamage min/max calculation — requires MUGame linkage",
          "[combat][items][damage][6-2-1]")
{
    SKIP("MU_COMBAT_TESTS_ENABLED not defined: GetAttackDamage() in ZzzInventory.h "
         "reads CHARACTER equipment state and calculates min/max weapon damage (MUGame). "
         "Set -DMU_COMBAT_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

#endif // MU_COMBAT_TESTS_ENABLED
