// Story 6.3.2: Advanced Game Systems Validation [VS1-GAME-VALIDATE-SYSTEMS]
// RED PHASE: Tests define expected logical contracts for quest/pet/PvP/duel validation.
//
// AC coverage (unit-testable subset — see manual test scenarios for full AC validation):
//   AC-1       — Quest system: MAX_QUESTS=200, MAX_QUEST_CONDITION=16, MAX_QUEST_REQUEST=16,
//                TYPE_QUEST/TYPE_QUEST_END enum completeness, QUEST_VIEW enum values,
//                QUEST_CLASS_ACT/QUEST_CLASS_REQUEST/QUEST_ATTRIBUTE struct layouts;
//                CSQuest::QUEST_STATE_MASK=0x03, QUEST_STATES_PER_ENTRY=4, QUEST_STATE_BIT_WIDTH=2,
//                state packing math self-consistency, REQUEST_REWARD_CLASSIFY enum
//                (CSQuest/QuestMng tests gated by MU_GAME_AVAILABLE)
//   AC-2       — Pet system: anonymous pet state enum (PET_FLYING=0..PET_END=7), PET_TYPE enum
//                (NONE=-1, DARK_SPIRIT=0, DARK_HORSE=1, END=2), PET_COMMAND enum (DEFAULT=0..END=4);
//                PET_INFO struct layout, PetObject::ActionType enum (eAction_Stand=0..eAction_End=6),
//                pet type constants PC4_ELF..SKELETON pairwise distinct
//                (PET_INFO/ActionType/constants gated by MU_GAME_AVAILABLE)
//   AC-3       — PvP/Duel: MAX_DUEL_CHANNELS=4, _DUEL_PLAYER_TYPE enum (DUEL_HERO=0, DUEL_ENEMY=1,
//                MAX_DUEL_PLAYERS=2), DUEL_PLAYER_INFO struct layout (index, ID, score, hpRate, sdRate),
//                DUEL_CHANNEL_INFO struct layout (enable, joinable, id1, id2)
//   AC-4       — Duel: MAX_DUEL_CHANNELS consistency with CDuelMgr array contract,
//                CNewBloodCastleSystem/CNewChaosCastleSystem derive from CSBaseMatch
//                (inheritance check gated by MU_GAME_AVAILABLE)
//
// Manual validation (full AC-1..4 on macOS/Linux/Windows with live server):
//   See: _bmad-output/test-scenarios/epic-6/advanced-systems-validation.md
//
// PCC Constraints:
//   - Platform-compatible includes: #ifdef _WIN32 / PlatformTypes.h on non-Win32
//   - No Win32 API calls in test logic — test logic only
//   - Catch2 TEST_CASE / SECTION / REQUIRE / static_assert structure
//   - Allman braces, 4-space indent, 120-column limit
//   - No raw new/delete in test code
//   - #pragma once not used (this is a .cpp file)
//   - #ifdef MU_GAME_AVAILABLE gates tests requiring full game includes (UI/network/pet headers)
//
// Design notes:
//   - mu_define.h: MAX_QUESTS, MAX_QUEST_CONDITION, MAX_QUEST_REQUEST constants
//     — testable standalone without Win32/OpenGL dependencies.
//   - mu_enum.h: TYPE_QUEST, QUEST_VIEW, PET_TYPE, PET_COMMAND, anonymous pet state enum
//     — platform-compatible, standalone.
//   - mu_struct.h: QUEST_CLASS_ACT, QUEST_CLASS_REQUEST, QUEST_ATTRIBUTE structs
//     — platform-compatible with PlatformTypes.h guard.
//   - DuelMgr.h: DUEL_PLAYER_INFO, _DUEL_PLAYER_TYPE, DUEL_CHANNEL_INFO, MAX_DUEL_CHANNELS
//     — standalone after PlatformTypes.h + mu_define.h + mu_struct.h (CHARACTER forward decl).
//   - CSQuest.h, QuestMng.h, w_CharacterInfo.h, w_BasePet.h, w_PetProcess.h:
//     Require full MUGame includes — gated under MU_GAME_AVAILABLE.
//
// MUCommon INTERFACE propagates all src/source/ subdirectory include paths to MuTests.
// No additional target_include_directories entries needed for this test file.

#include <catch2/catch_test_macros.hpp>
#include <cstddef> // offsetof, sizeof

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#endif

#include "mu_define.h" // MAX_QUESTS, MAX_QUEST_CONDITION, MAX_QUEST_REQUEST, MAX_USERNAME_SIZE
#include "mu_enum.h"   // TYPE_QUEST, QUEST_VIEW, PET_TYPE, PET_COMMAND, pet state enum
#include "mu_struct.h" // QUEST_CLASS_ACT, QUEST_CLASS_REQUEST, QUEST_ATTRIBUTE; class CHARACTER;
#include "DuelMgr.h"   // DUEL_PLAYER_INFO, _DUEL_PLAYER_TYPE, DUEL_CHANNEL_INFO, MAX_DUEL_CHANNELS

// Gameplay and rendering headers require full game linkage — gated by compile-time flag.
// To enable: build MuTests with -DMU_GAME_AVAILABLE (requires Win32 + OpenGL + MUGame).
#ifdef MU_GAME_AVAILABLE
#include <type_traits>
#include "CSQuest.h"              // CSQuest: QUEST_STATE_MASK, QUEST_STATES_PER_ENTRY, QUEST_STATE_BIT_WIDTH
#include "QuestMng.h"             // SQuestRequest, SQuestReward, REQUEST_REWARD_CLASSIFY
#include "w_CharacterInfo.h"      // PET_INFO struct (m_dwPetType, m_dwExp1/2, m_wLevel, etc.)
#include "w_BasePet.h"            // PetObject::ActionType enum (eAction_Stand..eAction_End)
#include "w_PetProcess.h"         // PC4_ELF, PC4_TEST, PC4_SATAN, XMAS_RUDOLPH, UNICORN, SKELETON
#include "CSEventMatch.h"         // CSBaseMatch base class for event match systems
#include "NewBloodCastleSystem.h" // CNewBloodCastleSystem : public CSBaseMatch
#include "NewChaosCastleSystem.h" // CNewChaosCastleSystem : public CSBaseMatch
#endif                            // MU_GAME_AVAILABLE

// =============================================================================
// AC-1 [6-3-2]: Quest system constants, struct layouts, and enum integrity
// CSQuest singleton manages quest state via bitfield array (4 states per byte,
// 2-bit encoding). QUEST_ATTRIBUTE holds up to MAX_QUEST_CONDITION acts and
// MAX_QUEST_REQUEST requirements per quest.
// =============================================================================

TEST_CASE("AC-1 [6-3-2]: Quest system constants define correct array dimensions",
          "[quest][constants][6-3-2]")
{
    SECTION("MAX_QUESTS is 200 — total quest slot count in state bitfield array")
    {
        REQUIRE(MAX_QUESTS == 200);
    }

    SECTION("MAX_QUEST_CONDITION is 16 — maximum act conditions per quest entry")
    {
        REQUIRE(MAX_QUEST_CONDITION == 16);
    }

    SECTION("MAX_QUEST_REQUEST is 16 — maximum request requirements per quest entry")
    {
        REQUIRE(MAX_QUEST_REQUEST == 16);
    }
}

TEST_CASE("AC-1 [6-3-2]: Quest type enum covers all quest entry types with no duplicates",
          "[quest][enum][type][6-3-2]")
{
    SECTION("TYPE_QUEST is 0 — standard quest type")
    {
        REQUIRE(TYPE_QUEST == 0);
    }

    SECTION("TYPE_QUEST_END is 4 — sentinel marks end of quest type range")
    {
        REQUIRE(TYPE_QUEST_END == 4);
    }

    SECTION("All 5 quest type enum values (TYPE_QUEST through TYPE_QUEST_END) are pairwise distinct")
    {
        const int types[] = {
            TYPE_QUEST,
            TYPE_DEVIL_SQUARE,
            TYPE_BLOOD_CASTLE,
            TYPE_CURSEDTEMPLE,
            TYPE_QUEST_END,
        };
        constexpr int n = static_cast<int>(sizeof(types) / sizeof(types[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(types[i] != types[j]);
            }
        }
    }
}

TEST_CASE("AC-1 [6-3-2]: Quest view mode enum covers all display states", "[quest][enum][view][6-3-2]")
{
    SECTION("QUEST_VIEW_NONE is 0 — no active quest view")
    {
        REQUIRE(QUEST_VIEW_NONE == 0);
    }

    SECTION("QUEST_VIEW_END is 3 — sentinel marks end of valid view states")
    {
        REQUIRE(QUEST_VIEW_END == 3);
    }

    SECTION("All 4 quest view mode values (QUEST_VIEW_NONE through QUEST_VIEW_END) are pairwise distinct")
    {
        const int views[] = {
            QUEST_VIEW_NONE,
            QUEST_VIEW_NPC,
            QUEST_VIEW_PREVIEW,
            QUEST_VIEW_END,
        };
        constexpr int n = static_cast<int>(sizeof(views) / sizeof(views[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(views[i] != views[j]);
            }
        }
    }
}

TEST_CASE("AC-1 [6-3-2]: QUEST_CLASS_ACT struct has correct field layout", "[quest][struct][act][6-3-2]")
{
    static_assert(sizeof(QUEST_CLASS_ACT) > 0, "QUEST_CLASS_ACT must be non-empty");

    SECTION("QUEST_CLASS_ACT::chLive and byQuestType are single-byte fields (BYTE)")
    {
        REQUIRE(sizeof(QUEST_CLASS_ACT::chLive) == 1u);
        REQUIRE(sizeof(QUEST_CLASS_ACT::byQuestType) == 1u);
    }

    SECTION("QUEST_CLASS_ACT::wItemType is a 2-byte field (WORD)")
    {
        REQUIRE(sizeof(QUEST_CLASS_ACT::wItemType) == 2u);
    }

    SECTION("QUEST_CLASS_ACT::byRequestClass array holds exactly MAX_CLASS=7 entries")
    {
        REQUIRE(sizeof(QUEST_CLASS_ACT::byRequestClass) == static_cast<std::size_t>(MAX_CLASS));
    }

    SECTION("QUEST_CLASS_ACT::shQuestStartText array holds 4 short entries (8 bytes)")
    {
        REQUIRE(sizeof(QUEST_CLASS_ACT::shQuestStartText) == 4u * sizeof(short));
    }

    SECTION("QUEST_CLASS_ACT::byItemSubType, byItemLevel, byItemNum, byRequestType are single-byte fields")
    {
        REQUIRE(sizeof(QUEST_CLASS_ACT::byItemSubType) == 1u);
        REQUIRE(sizeof(QUEST_CLASS_ACT::byItemLevel) == 1u);
        REQUIRE(sizeof(QUEST_CLASS_ACT::byItemNum) == 1u);
        REQUIRE(sizeof(QUEST_CLASS_ACT::byRequestType) == 1u);
    }
}

TEST_CASE("AC-1 [6-3-2]: QUEST_CLASS_REQUEST struct has correct field layout",
          "[quest][struct][request][6-3-2]")
{
    static_assert(sizeof(QUEST_CLASS_REQUEST) > 0, "QUEST_CLASS_REQUEST must be non-empty");

    SECTION("QUEST_CLASS_REQUEST::byLive and byType are single-byte fields (BYTE)")
    {
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::byLive) == 1u);
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::byType) == 1u);
    }

    SECTION("QUEST_CLASS_REQUEST::wCompleteQuestIndex, wLevelMin, wLevelMax, wRequestStrength are 2-byte (WORD)")
    {
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::wCompleteQuestIndex) == 2u);
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::wLevelMin) == 2u);
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::wLevelMax) == 2u);
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::wRequestStrength) == 2u);
    }

    SECTION("QUEST_CLASS_REQUEST::dwZen is a 4-byte field (DWORD)")
    {
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::dwZen) == 4u);
    }

    SECTION("QUEST_CLASS_REQUEST::shErrorText is a 2-byte field (short)")
    {
        REQUIRE(sizeof(QUEST_CLASS_REQUEST::shErrorText) == 2u);
    }
}

TEST_CASE("AC-1 [6-3-2]: QUEST_ATTRIBUTE struct uses correct array sizes and name buffer",
          "[quest][struct][attribute][6-3-2]")
{
    static_assert(sizeof(QUEST_ATTRIBUTE) > 0, "QUEST_ATTRIBUTE must be non-empty");

    SECTION("QUEST_ATTRIBUTE::QuestAct array holds exactly MAX_QUEST_CONDITION=16 entries")
    {
        REQUIRE(sizeof(QUEST_ATTRIBUTE::QuestAct) ==
                static_cast<std::size_t>(MAX_QUEST_CONDITION) * sizeof(QUEST_CLASS_ACT));
    }

    SECTION("QUEST_ATTRIBUTE::QuestRequest array holds exactly MAX_QUEST_REQUEST=16 entries")
    {
        REQUIRE(sizeof(QUEST_ATTRIBUTE::QuestRequest) ==
                static_cast<std::size_t>(MAX_QUEST_REQUEST) * sizeof(QUEST_CLASS_REQUEST));
    }

    SECTION("QUEST_ATTRIBUTE::strQuestName buffer holds 32 wide characters")
    {
        REQUIRE(sizeof(QUEST_ATTRIBUTE::strQuestName) == 32u * sizeof(wchar_t));
    }

    SECTION("QUEST_ATTRIBUTE header fields: shQuestConditionNum, shQuestRequestNum, wNpcType")
    {
        REQUIRE(sizeof(QUEST_ATTRIBUTE::shQuestConditionNum) == 2u);
        REQUIRE(sizeof(QUEST_ATTRIBUTE::shQuestRequestNum) == 2u);
        REQUIRE(sizeof(QUEST_ATTRIBUTE::wNpcType) == 2u);
    }
}

#ifdef MU_GAME_AVAILABLE
TEST_CASE("AC-1 [6-3-2]: CSQuest state packing constants are correct and self-consistent",
          "[quest][constants][packing][6-3-2]")
{
    SECTION("QUEST_STATE_MASK is 0x03 — masks the 2 low bits for one quest state")
    {
        REQUIRE(CSQuest::QUEST_STATE_MASK == static_cast<std::uint8_t>(0x03u));
    }

    SECTION("QUEST_STATES_PER_ENTRY is 4 — four 2-bit quest states fit in one byte")
    {
        REQUIRE(CSQuest::QUEST_STATES_PER_ENTRY == static_cast<std::uint8_t>(4u));
    }

    SECTION("QUEST_STATE_BIT_WIDTH is 2 — each quest state occupies 2 bits")
    {
        REQUIRE(CSQuest::QUEST_STATE_BIT_WIDTH == static_cast<std::uint8_t>(2u));
    }

    SECTION("State packing math: STATES_PER_ENTRY * BIT_WIDTH == 8 bits per byte")
    {
        // 4 states × 2 bits = 8 bits = exactly one byte. No wasted bits.
        const int packed_bits = static_cast<int>(CSQuest::QUEST_STATES_PER_ENTRY) *
                                static_cast<int>(CSQuest::QUEST_STATE_BIT_WIDTH);
        REQUIRE(packed_bits == 8);
    }

    SECTION("QUEST_STATE_MASK covers exactly QUEST_STATE_BIT_WIDTH low bits")
    {
        // 2-bit mask = (1 << 2) - 1 = 0x03
        const std::uint8_t expected_mask =
            static_cast<std::uint8_t>((1u << CSQuest::QUEST_STATE_BIT_WIDTH) - 1u);
        REQUIRE(CSQuest::QUEST_STATE_MASK == expected_mask);
    }
}

TEST_CASE("AC-1 [6-3-2]: REQUEST_REWARD_CLASSIFY enum covers request vs reward classification",
          "[quest][enum][classify][6-3-2]")
{
    SECTION("RRC_NONE is 0 — no classification assigned")
    {
        REQUIRE(static_cast<int>(RRC_NONE) == 0);
    }

    SECTION("RRC_REQUEST is 1 — entry is a quest requirement")
    {
        REQUIRE(static_cast<int>(RRC_REQUEST) == 1);
    }

    SECTION("RRC_REWARD is 2 — entry is a quest reward")
    {
        REQUIRE(static_cast<int>(RRC_REWARD) == 2);
    }

    SECTION("All three REQUEST_REWARD_CLASSIFY values are pairwise distinct")
    {
        REQUIRE(static_cast<int>(RRC_NONE) != static_cast<int>(RRC_REQUEST));
        REQUIRE(static_cast<int>(RRC_NONE) != static_cast<int>(RRC_REWARD));
        REQUIRE(static_cast<int>(RRC_REQUEST) != static_cast<int>(RRC_REWARD));
    }
}
#endif // MU_GAME_AVAILABLE

// =============================================================================
// AC-2 [6-3-2]: Pet system enums and struct validation
// Two pet architectures coexist: CSPetSystem (data/network via PET_INFO) and
// PetProcess/PetObject (rendering/animation via ActionType state machine).
// Pet types use #define constants (PC4_ELF..SKELETON) for rendering identity.
// =============================================================================

TEST_CASE("AC-2 [6-3-2]: Pet state enum covers all locomotion and combat states",
          "[pet][enum][state][6-3-2]")
{
    SECTION("PET_FLYING is 0 — initial/default pet state")
    {
        REQUIRE(PET_FLYING == 0);
    }

    SECTION("PET_END is 7 — sentinel marks end of valid pet states")
    {
        REQUIRE(PET_END == 7);
    }

    SECTION("All 8 pet state values (PET_FLYING through PET_END) are pairwise distinct")
    {
        const int states[] = {
            PET_FLYING,
            PET_FLY,
            PET_ESCAPE,
            PET_STAND,
            PET_STAND_START,
            PET_ATTACK,
            PET_ATTACK_MAGIC,
            PET_END,
        };
        constexpr int n = static_cast<int>(sizeof(states) / sizeof(states[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(states[i] != states[j]);
            }
        }
    }
}

TEST_CASE("AC-2 [6-3-2]: PET_TYPE enum defines correct pet type identifiers", "[pet][enum][type][6-3-2]")
{
    SECTION("PET_TYPE_NONE is -1 — no active pet equipped")
    {
        REQUIRE(static_cast<int>(PET_TYPE_NONE) == -1);
    }

    SECTION("PET_TYPE_DARK_SPIRIT is 0 — first usable pet slot")
    {
        REQUIRE(static_cast<int>(PET_TYPE_DARK_SPIRIT) == 0);
    }

    SECTION("PET_TYPE_DARK_HORSE is 1 — second usable pet slot")
    {
        REQUIRE(static_cast<int>(PET_TYPE_DARK_HORSE) == 1);
    }

    SECTION("PET_TYPE_END is 2 — sentinel marks count of usable pet types (Dark Spirit + Dark Horse)")
    {
        REQUIRE(static_cast<int>(PET_TYPE_END) == 2);
    }
}

TEST_CASE("AC-2 [6-3-2]: PET_COMMAND enum covers all AI command modes with no duplicates",
          "[pet][enum][command][6-3-2]")
{
    SECTION("PET_CMD_DEFAULT is 0 — default autonomous behaviour")
    {
        REQUIRE(static_cast<int>(PET_CMD_DEFAULT) == 0);
    }

    SECTION("PET_CMD_END is 4 — sentinel marks end of valid command range")
    {
        REQUIRE(static_cast<int>(PET_CMD_END) == 4);
    }

    SECTION("All 5 PET_COMMAND values (PET_CMD_DEFAULT through PET_CMD_END) are pairwise distinct")
    {
        const int cmds[] = {
            static_cast<int>(PET_CMD_DEFAULT),
            static_cast<int>(PET_CMD_RANDOM),
            static_cast<int>(PET_CMD_OWNER),
            static_cast<int>(PET_CMD_TARGET),
            static_cast<int>(PET_CMD_END),
        };
        constexpr int n = static_cast<int>(sizeof(cmds) / sizeof(cmds[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(cmds[i] != cmds[j]);
            }
        }
    }
}

#ifdef MU_GAME_AVAILABLE
TEST_CASE("AC-2 [6-3-2]: PET_INFO struct has correct field layout for network data",
          "[pet][struct][info][6-3-2]")
{
    static_assert(sizeof(PET_INFO) > 0, "PET_INFO must be non-empty");

    SECTION("PET_INFO::m_dwPetType, m_dwExp1, m_dwExp2 are 4-byte fields (DWORD)")
    {
        REQUIRE(sizeof(PET_INFO::m_dwPetType) == 4u);
        REQUIRE(sizeof(PET_INFO::m_dwExp1) == 4u);
        REQUIRE(sizeof(PET_INFO::m_dwExp2) == 4u);
    }

    SECTION("PET_INFO::m_wLevel, m_wLife, m_wDamageMin, m_wDamageMax are 2-byte fields (WORD)")
    {
        REQUIRE(sizeof(PET_INFO::m_wLevel) == 2u);
        REQUIRE(sizeof(PET_INFO::m_wLife) == 2u);
        REQUIRE(sizeof(PET_INFO::m_wDamageMin) == 2u);
        REQUIRE(sizeof(PET_INFO::m_wDamageMax) == 2u);
    }

    SECTION("PET_INFO::m_wAttackSpeed, m_wAttackSuccess are 2-byte fields (WORD)")
    {
        REQUIRE(sizeof(PET_INFO::m_wAttackSpeed) == 2u);
        REQUIRE(sizeof(PET_INFO::m_wAttackSuccess) == 2u);
    }
}

TEST_CASE("AC-2 [6-3-2]: PetObject::ActionType enum covers all animation states",
          "[pet][enum][action][6-3-2]")
{
    SECTION("eAction_Stand is 0 — default idle animation state")
    {
        REQUIRE(static_cast<int>(PetObject::ActionType::eAction_Stand) == 0);
    }

    SECTION("eAction_End is 6 — sentinel marks end of valid animation states")
    {
        REQUIRE(static_cast<int>(PetObject::ActionType::eAction_End) == 6);
    }

    SECTION("All 7 ActionType values (eAction_Stand through eAction_End) are pairwise distinct")
    {
        const int actions[] = {
            static_cast<int>(PetObject::ActionType::eAction_Stand),
            static_cast<int>(PetObject::ActionType::eAction_Move),
            static_cast<int>(PetObject::ActionType::eAction_Attack),
            static_cast<int>(PetObject::ActionType::eAction_Skill),
            static_cast<int>(PetObject::ActionType::eAction_Birth),
            static_cast<int>(PetObject::ActionType::eAction_Dead),
            static_cast<int>(PetObject::ActionType::eAction_End),
        };
        constexpr int n = static_cast<int>(sizeof(actions) / sizeof(actions[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(actions[i] != actions[j]);
            }
        }
    }
}

TEST_CASE("AC-2 [6-3-2]: Pet type rendering constants are pairwise distinct",
          "[pet][constants][type][6-3-2]")
{
    SECTION("PC4_ELF starts at 1 — first non-zero pet type rendering constant")
    {
        REQUIRE(PC4_ELF == 1);
    }

    SECTION("SKELETON is 7 — highest standard pet type rendering constant")
    {
        REQUIRE(SKELETON == 7);
    }

    SECTION("All pet type rendering constants (PC4_ELF through SKELETON) are pairwise distinct")
    {
        // PANDA (#define PANDA 5) may not be defined if PJH_ADD_PANDA_PET is not set.
        // Test the six unconditional constants: PC4_ELF, PC4_TEST, PC4_SATAN, XMAS_RUDOLPH, UNICORN, SKELETON.
        const int pet_types[] = {
            PC4_ELF,      // 1
            PC4_TEST,     // 2
            PC4_SATAN,    // 3
            XMAS_RUDOLPH, // 4
            UNICORN,      // 6
            SKELETON,     // 7
        };
        constexpr int n = static_cast<int>(sizeof(pet_types) / sizeof(pet_types[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(pet_types[i] != pet_types[j]);
            }
        }
    }
}
#endif // MU_GAME_AVAILABLE

// =============================================================================
// AC-3 [6-3-2]: PvP targeting and duel channel struct validation
// CDuelMgr manages up to MAX_DUEL_CHANNELS=4 concurrent 1v1 duel arenas.
// DUEL_PLAYER_INFO tracks index, ID, score, HP rate, SD rate per participant.
// DUEL_CHANNEL_INFO tracks enable/joinable state and two player IDs per channel.
// =============================================================================

TEST_CASE("AC-3 [6-3-2]: PvP duel system channel capacity constant is correct",
          "[pvp][constants][6-3-2]")
{
    SECTION("MAX_DUEL_CHANNELS is 4 — supports four concurrent 1v1 duel arenas")
    {
        REQUIRE(MAX_DUEL_CHANNELS == 4);
    }
}

TEST_CASE("AC-3 [6-3-2]: _DUEL_PLAYER_TYPE enum identifies hero and enemy duel participants",
          "[pvp][enum][type][6-3-2]")
{
    SECTION("DUEL_HERO is 0 — index for the local player in a duel")
    {
        REQUIRE(static_cast<int>(DUEL_HERO) == 0);
    }

    SECTION("DUEL_ENEMY is 1 — index for the opposing player in a duel")
    {
        REQUIRE(static_cast<int>(DUEL_ENEMY) == 1);
    }

    SECTION("MAX_DUEL_PLAYERS is 2 — exactly two participants per duel channel")
    {
        REQUIRE(static_cast<int>(MAX_DUEL_PLAYERS) == 2);
    }

    SECTION("DUEL_HERO, DUEL_ENEMY, MAX_DUEL_PLAYERS are pairwise distinct")
    {
        REQUIRE(static_cast<int>(DUEL_HERO) != static_cast<int>(DUEL_ENEMY));
        REQUIRE(static_cast<int>(DUEL_HERO) != static_cast<int>(MAX_DUEL_PLAYERS));
        REQUIRE(static_cast<int>(DUEL_ENEMY) != static_cast<int>(MAX_DUEL_PLAYERS));
    }
}

TEST_CASE("AC-3 [6-3-2]: DUEL_PLAYER_INFO struct has correct field layout for combat tracking",
          "[pvp][struct][player][6-3-2]")
{
    static_assert(sizeof(DUEL_PLAYER_INFO) > 0, "DUEL_PLAYER_INFO must be non-empty");

    SECTION("DUEL_PLAYER_INFO::m_sIndex is a 2-byte short field")
    {
        REQUIRE(sizeof(DUEL_PLAYER_INFO::m_sIndex) == 2u);
    }

    SECTION("DUEL_PLAYER_INFO::m_szID buffer holds MAX_USERNAME_SIZE+1 wide characters")
    {
        constexpr std::size_t expected = (static_cast<std::size_t>(MAX_USERNAME_SIZE) + 1u) * sizeof(wchar_t);
        REQUIRE(sizeof(DUEL_PLAYER_INFO::m_szID) == expected);
    }

    SECTION("DUEL_PLAYER_INFO::m_iScore is a 4-byte int field")
    {
        REQUIRE(sizeof(DUEL_PLAYER_INFO::m_iScore) == 4u);
    }

    SECTION("DUEL_PLAYER_INFO::m_fHPRate and m_fSDRate are 4-byte float fields")
    {
        REQUIRE(sizeof(DUEL_PLAYER_INFO::m_fHPRate) == 4u);
        REQUIRE(sizeof(DUEL_PLAYER_INFO::m_fSDRate) == 4u);
    }
}

TEST_CASE("AC-3 [6-3-2]: DUEL_CHANNEL_INFO struct has correct field layout for arena channels",
          "[pvp][struct][channel][6-3-2]")
{
    static_assert(sizeof(DUEL_CHANNEL_INFO) > 0, "DUEL_CHANNEL_INFO must be non-empty");

    SECTION("DUEL_CHANNEL_INFO::m_bEnable and m_bJoinable are BOOL (4-byte int) fields")
    {
        REQUIRE(sizeof(DUEL_CHANNEL_INFO::m_bEnable) == 4u);
        REQUIRE(sizeof(DUEL_CHANNEL_INFO::m_bJoinable) == 4u);
    }

    SECTION("DUEL_CHANNEL_INFO::m_szID1 buffer holds MAX_USERNAME_SIZE+1 wide characters")
    {
        constexpr std::size_t expected = (static_cast<std::size_t>(MAX_USERNAME_SIZE) + 1u) * sizeof(wchar_t);
        REQUIRE(sizeof(DUEL_CHANNEL_INFO::m_szID1) == expected);
    }

    SECTION("DUEL_CHANNEL_INFO::m_szID2 buffer holds MAX_USERNAME_SIZE+1 wide characters")
    {
        constexpr std::size_t expected = (static_cast<std::size_t>(MAX_USERNAME_SIZE) + 1u) * sizeof(wchar_t);
        REQUIRE(sizeof(DUEL_CHANNEL_INFO::m_szID2) == expected);
    }

    SECTION("DUEL_CHANNEL_INFO::m_szID1 and m_szID2 are the same size — symmetric channel layout")
    {
        REQUIRE(sizeof(DUEL_CHANNEL_INFO::m_szID1) == sizeof(DUEL_CHANNEL_INFO::m_szID2));
    }
}

// =============================================================================
// AC-4 [6-3-2]: Duel invitation/acceptance and event match system validation
// CDuelMgr uses std::array<DUEL_PLAYER_INFO, MAX_DUEL_PLAYERS> and
// std::array<DUEL_CHANNEL_INFO, MAX_DUEL_CHANNELS>. Both event match systems
// (CNewBloodCastleSystem, CNewChaosCastleSystem) derive from CSBaseMatch.
// =============================================================================

TEST_CASE("AC-4 [6-3-2]: MAX_DUEL_CHANNELS matches CDuelMgr channel array contract",
          "[duel][constants][consistency][6-3-2]")
{
    SECTION("MAX_DUEL_CHANNELS is 4 — validates independent channel capacity constant")
    {
        // CDuelMgr allocates MAX_DUEL_CHANNELS independent channel slots.
        // Each channel can host a pair of duellists (via MAX_DUEL_PLAYERS player array).
        REQUIRE(MAX_DUEL_CHANNELS == 4);
    }

    SECTION("MAX_DUEL_PLAYERS is 2 — validates player slots per channel contract")
    {
        // CDuelMgr::m_DuelPlayer is std::array<DUEL_PLAYER_INFO, MAX_DUEL_PLAYERS>.
        // Each duel channel uses exactly 2 player slots (hero + enemy).
        REQUIRE(static_cast<int>(MAX_DUEL_PLAYERS) == 2);
    }

    SECTION("DUEL_HERO and DUEL_ENEMY indices fit within MAX_DUEL_PLAYERS bounds")
    {
        // DUEL_HERO=0 and DUEL_ENEMY=1 must both index within [0, MAX_DUEL_PLAYERS).
        REQUIRE(static_cast<int>(DUEL_HERO) < static_cast<int>(MAX_DUEL_PLAYERS));
        REQUIRE(static_cast<int>(DUEL_ENEMY) < static_cast<int>(MAX_DUEL_PLAYERS));
    }
}

#ifdef MU_GAME_AVAILABLE
TEST_CASE("AC-4 [6-3-2]: Event match systems derive from CSBaseMatch base class",
          "[duel][events][inheritance][6-3-2]")
{
    SECTION("CNewBloodCastleSystem derives from CSBaseMatch — verified at compile time")
    {
        static_assert(std::is_base_of_v<CSBaseMatch, CNewBloodCastleSystem>,
                      "CNewBloodCastleSystem must derive from CSBaseMatch");
    }

    SECTION("CNewChaosCastleSystem derives from CSBaseMatch — verified at compile time")
    {
        static_assert(std::is_base_of_v<CSBaseMatch, CNewChaosCastleSystem>,
                      "CNewChaosCastleSystem must derive from CSBaseMatch");
    }
}
#endif // MU_GAME_AVAILABLE
