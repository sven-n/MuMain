// Story 6.3.1: Social Systems Validation [VS1-GAME-VALIDATE-SOCIAL]
// RED PHASE: Tests define expected logical contracts for chat/party/guild validation.
//
// AC coverage (unit-testable subset — see manual test scenarios for full AC validation):
//   AC-1       — Chat system: MESSAGE_TYPE enum completeness (10 values + TYPE_UNKNOWN sentinel),
//                INPUT_MESSAGE_TYPE channel count, MAX_CHAT_SIZE=90, MAX_CHAT_BUFFER_SIZE=60,
//                MAX_NUMBER_OF_LINES=200, PCHATING/PCHATING_KEY packet struct field presence
//   AC-2       — Party system: MAX_PARTYS=5, PARTY_t struct field layout
//                (Name, Number, Map, x, y, currHP, maxHP, stepHP, index)
//   AC-3       — Guild info panel: GuildConstants::GUILD_NAME_LENGTH=8,
//                GuildConstants::GUILD_MARK_SIZE=64, GuildConstants::Capacity::MAX_CAPACITY=80,
//                GuildTab enum (INFO/MEMBERS/UNION), GuildInfoButton enum (7 values incl. END),
//                RelationshipType enum values, GUILD_LIST_t/MARK_t struct field presence
//   AC-4       — Player names/guild tags: CHARACTER guild fields (GuildStatus, GuildType,
//                GuildRelationShip, GuildMarkIndex), GuildConstants::Colors ARGB values,
//                MAX_MARKS=2000, MARK_t guild name buffer covers GUILD_NAME_LENGTH
//   AC-5       — Chat encoding: MAX_CHAT_SIZE alignment across packet structs,
//                PCHATING ChatText size, MAX_USERNAME_SIZE cross-system consistency
//
// Manual validation (full AC-1..5 on macOS/Linux/Windows with live server):
//   See: _bmad-output/test-scenarios/epic-6/social-systems-validation.md
//
// PCC Constraints:
//   - Platform-compatible includes: #ifdef _WIN32 / PlatformTypes.h on non-Win32
//   - No Win32 API calls in test logic — test logic only
//   - Catch2 TEST_CASE / SECTION / REQUIRE structure
//   - Allman braces, 4-space indent, 120-column limit
//   - No raw new/delete in test code
//   - #pragma once not used (this is a .cpp file)
//   - #ifdef MU_GAME_AVAILABLE gates tests requiring full game includes (UI/network headers)
//
// Design notes:
//   - mu_define.h: MAX_CHAT_SIZE, MAX_PARTYS, MAX_MARKS, MAX_USERNAME_SIZE constants
//     — testable standalone without Win32/OpenGL dependencies.
//   - mu_struct.h: PARTY_t, GUILD_LIST_t, MARK_t struct layouts
//     — platform-compatible with PlatformTypes.h guard.
//   - GuildConstants.h: Centralized guild constants and enums — standalone, no game loop dep.
//   - NewUIChatLogWindow.h, NewUIChatInputBox.h, WSclient.h:
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

#include "mu_define.h"      // MAX_CHAT_SIZE, MAX_PARTYS, MAX_MARKS, MAX_USERNAME_SIZE
#include "mu_struct.h"      // PARTY_t, GUILD_LIST_t, MARK_t structs
#include "GuildConstants.h" // GuildConstants namespace: GUILD_NAME_LENGTH, GUILD_MARK_SIZE, enums

// UI and network headers require full game linkage — gated by compile-time flag.
// To enable: build MuTests with -DMU_GAME_AVAILABLE (requires Win32 + OpenGL + MUGame).
#ifdef MU_GAME_AVAILABLE
#include "NewUIChatLogWindow.h" // MESSAGE_TYPE enum, MAX_CHAT_BUFFER_SIZE, MAX_NUMBER_OF_LINES
#include "NewUIChatInputBox.h"  // INPUT_MESSAGE_TYPE, INPUT_MESSAGE_TYPE_COUNT
#include "WSclient.h"           // PCHATING, PCHATING_KEY packet structs
#endif                          // MU_GAME_AVAILABLE

// =============================================================================
// AC-1 [6-3-1]: Chat system constants, enum completeness, packet struct layout
// The chat system (CNewUIChatLogWindow/CNewUIChatInputBox) routes messages via
// MESSAGE_TYPE channels. MAX_CHAT_SIZE=90 governs all chat field buffers.
// PCHATING and PCHATING_KEY are the wire-format structs sent to the server.
// =============================================================================

TEST_CASE("AC-1 [6-3-1]: Chat constants define correct buffer dimensions", "[chat][constants][6-3-1]")
{
    SECTION("MAX_CHAT_SIZE is 90 — governs chat field and packet buffer sizes")
    {
        REQUIRE(MAX_CHAT_SIZE == 90);
    }

#ifdef MU_GAME_AVAILABLE
    SECTION("MAX_CHAT_BUFFER_SIZE is 60 — chat log window circular buffer capacity")
    {
        REQUIRE(CNewUIChatLogWindow::MAX_CHAT_BUFFER_SIZE == 60);
    }

    SECTION("MAX_NUMBER_OF_LINES is 200 — maximum rendered chat lines")
    {
        REQUIRE(CNewUIChatLogWindow::MAX_NUMBER_OF_LINES == 200);
    }
#endif // MU_GAME_AVAILABLE
}

#ifdef MU_GAME_AVAILABLE
TEST_CASE("AC-1 [6-3-1]: MESSAGE_TYPE enum covers all chat channels with no duplicates",
          "[chat][enum][completeness][6-3-1]")
{
    SECTION("TYPE_ALL_MESSAGE starts at 0")
    {
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_ALL_MESSAGE) == 0);
    }

    SECTION("NUMBER_OF_TYPES is 10 — covers all named channels before sentinel")
    {
        REQUIRE(static_cast<int>(MESSAGE_TYPE::NUMBER_OF_TYPES) == 10);
    }

    SECTION("TYPE_UNKNOWN sentinel is 0xFFFFFFFF — distinct from all valid channels")
    {
        REQUIRE(static_cast<unsigned int>(MESSAGE_TYPE::TYPE_UNKNOWN) == 0xFFFFFFFFu);
    }

    SECTION("All named MESSAGE_TYPE values are pairwise distinct")
    {
        const int types[] = {
            static_cast<int>(MESSAGE_TYPE::TYPE_ALL_MESSAGE),     static_cast<int>(MESSAGE_TYPE::TYPE_CHAT_MESSAGE),
            static_cast<int>(MESSAGE_TYPE::TYPE_WHISPER_MESSAGE), static_cast<int>(MESSAGE_TYPE::TYPE_SYSTEM_MESSAGE),
            static_cast<int>(MESSAGE_TYPE::TYPE_ERROR_MESSAGE),   static_cast<int>(MESSAGE_TYPE::TYPE_PARTY_MESSAGE),
            static_cast<int>(MESSAGE_TYPE::TYPE_GUILD_MESSAGE),   static_cast<int>(MESSAGE_TYPE::TYPE_UNION_MESSAGE),
            static_cast<int>(MESSAGE_TYPE::TYPE_GENS_MESSAGE),    static_cast<int>(MESSAGE_TYPE::TYPE_GM_MESSAGE),
        };
        constexpr int n = static_cast<int>(sizeof(types) / sizeof(types[0]));
        // Verify pairwise uniqueness across all 10 named values (45 pairs)
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(types[i] != types[j]);
            }
        }
    }

    SECTION("Channel type ordering: chat(1) < whisper(2) < system(3) < party(5) < guild(6)")
    {
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_CHAT_MESSAGE) == 1);
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_WHISPER_MESSAGE) == 2);
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_SYSTEM_MESSAGE) == 3);
        // TYPE_ERROR_MESSAGE=4 sits between system(3) and party(5)
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_ERROR_MESSAGE) == 4);
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_PARTY_MESSAGE) == 5);
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_GUILD_MESSAGE) == 6);
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_UNION_MESSAGE) == 7);
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_GENS_MESSAGE) == 8);
        REQUIRE(static_cast<int>(MESSAGE_TYPE::TYPE_GM_MESSAGE) == 9);
    }
}

TEST_CASE("AC-1 [6-3-1]: INPUT_MESSAGE_TYPE covers all input channels", "[chat][input][enum][6-3-1]")
{
    SECTION("INPUT_MESSAGE_TYPE_COUNT is 4 — normal/party/guild/gens channels")
    {
        REQUIRE(CNewUIChatInputBox::INPUT_MESSAGE_TYPE_COUNT == 4);
    }

    SECTION("INPUT_NOTHING sentinel is -1 — distinct from all valid channel indices")
    {
        REQUIRE(static_cast<int>(INPUT_MESSAGE_TYPE::INPUT_NOTHING) == -1);
    }
}

TEST_CASE("AC-1 [6-3-1]: PCHATING packet struct has correct ChatText buffer size", "[chat][packet][struct][6-3-1]")
{
    SECTION("PCHATING ChatText field is MAX_CHAT_SIZE bytes (90)")
    {
        REQUIRE(sizeof(PCHATING::ChatText) == static_cast<std::size_t>(MAX_CHAT_SIZE));
    }
}

TEST_CASE("AC-1 [6-3-1]: PCHATING_KEY packet struct has correct ChatText buffer size", "[chat][packet][struct][6-3-1]")
{
    SECTION("PCHATING_KEY ChatText field is MAX_CHAT_SIZE bytes (90)")
    {
        REQUIRE(sizeof(PCHATING_KEY::ChatText) == static_cast<std::size_t>(MAX_CHAT_SIZE));
    }
}
#endif // MU_GAME_AVAILABLE

// =============================================================================
// AC-2 [6-3-1]: Party system constants and PARTY_t struct field layout
// CPartyManager (singleton) manages a party of up to MAX_PARTYS=5 members.
// PARTY_t carries name, map location, and HP bar data for each member.
// =============================================================================

TEST_CASE("AC-2 [6-3-1]: Party constants define correct capacity and struct layout",
          "[party][constants][struct][6-3-1]")
{
    SECTION("MAX_PARTYS is 5 — maximum party member count")
    {
        REQUIRE(MAX_PARTYS == 5);
    }

    SECTION("PARTY_t::Name buffer holds MAX_USERNAME_SIZE+1 wchar_t characters")
    {
        // Name is declared as wchar_t Name[MAX_USERNAME_SIZE + 1]
        constexpr std::size_t expected = (static_cast<std::size_t>(MAX_USERNAME_SIZE) + 1) * sizeof(wchar_t);
        REQUIRE(sizeof(PARTY_t::Name) == expected);
    }

    SECTION("PARTY_t::Number, Map, x, y, stepHP are single-byte fields (BYTE)")
    {
        REQUIRE(sizeof(PARTY_t::Number) == 1u);
        REQUIRE(sizeof(PARTY_t::Map) == 1u);
        REQUIRE(sizeof(PARTY_t::x) == 1u);
        REQUIRE(sizeof(PARTY_t::y) == 1u);
        REQUIRE(sizeof(PARTY_t::stepHP) == 1u);
    }

    SECTION("PARTY_t::currHP, maxHP, index are 4-byte int fields")
    {
        REQUIRE(sizeof(PARTY_t::currHP) == 4u);
        REQUIRE(sizeof(PARTY_t::maxHP) == 4u);
        REQUIRE(sizeof(PARTY_t::index) == 4u);
    }

    SECTION("PARTY_t struct is non-empty — carries all AC-specified member fields")
    {
        static_assert(sizeof(PARTY_t) > 0, "PARTY_t must be a non-empty struct");
    }
}

// =============================================================================
// AC-3 [6-3-1]: Guild info panel constants, enums, and struct layouts
// CNewUIGuildInfoWindow renders three tabs (GuildTab: Info/Members/Union).
// GuildConstants centralizes all guild sizing and capacity constants.
// =============================================================================

TEST_CASE("AC-3 [6-3-1]: GuildConstants define correct guild name and mark dimensions", "[guild][constants][6-3-1]")
{
    SECTION("GUILD_NAME_LENGTH is 8 — maximum guild name length in characters")
    {
        REQUIRE(GuildConstants::GUILD_NAME_LENGTH == 8);
    }

    SECTION("GUILD_NAME_BUFFER_SIZE is GUILD_NAME_LENGTH + 1 = 9 — null-terminated buffer")
    {
        REQUIRE(GuildConstants::GUILD_NAME_BUFFER_SIZE == GuildConstants::GUILD_NAME_LENGTH + 1);
        REQUIRE(GuildConstants::GUILD_NAME_BUFFER_SIZE == 9);
    }

    SECTION("GUILD_MARK_SIZE is 64 — 8x8 pixel bitmap stored as 64 bytes (byte-per-pixel)")
    {
        REQUIRE(GuildConstants::GUILD_MARK_SIZE == 64);
    }

    SECTION("GUILD_MARK_PIXELS is 8 — marks are 8x8 pixel grids")
    {
        REQUIRE(GuildConstants::GUILD_MARK_PIXELS == 8);
        // Each pixel stored as 1 byte (8-bit palette index).
        // 8x8 grid = 64 pixels × 1 byte = GUILD_MARK_SIZE (64 bytes).
        REQUIRE(GuildConstants::GUILD_MARK_PIXELS * GuildConstants::GUILD_MARK_PIXELS ==
                GuildConstants::GUILD_MARK_SIZE);
    }

    SECTION("Capacity::MAX_CAPACITY is 80 — maximum guild member count")
    {
        REQUIRE(GuildConstants::Capacity::MAX_CAPACITY == 80);
    }
}

TEST_CASE("AC-3 [6-3-1]: GuildTab enum covers Info/Members/Union tabs with correct values", "[guild][enum][tab][6-3-1]")
{
    SECTION("GuildTab::INFO is 0 — default tab")
    {
        REQUIRE(static_cast<int>(GuildConstants::GuildTab::INFO) == 0);
    }

    SECTION("GuildTab::MEMBERS is 1")
    {
        REQUIRE(static_cast<int>(GuildConstants::GuildTab::MEMBERS) == 1);
    }

    SECTION("GuildTab::UNION is 2")
    {
        REQUIRE(static_cast<int>(GuildConstants::GuildTab::UNION) == 2);
    }

    SECTION("GuildTab values are pairwise distinct")
    {
        const int tabs[] = {
            static_cast<int>(GuildConstants::GuildTab::INFO),
            static_cast<int>(GuildConstants::GuildTab::MEMBERS),
            static_cast<int>(GuildConstants::GuildTab::UNION),
        };
        REQUIRE(tabs[0] != tabs[1]);
        REQUIRE(tabs[0] != tabs[2]);
        REQUIRE(tabs[1] != tabs[2]);
    }
}

TEST_CASE("AC-3 [6-3-1]: GuildInfoButton enum covers all 7 button values including END sentinel",
          "[guild][enum][button][6-3-1]")
{
    SECTION("GuildInfoButton starts at GUILD_OUT=0")
    {
        REQUIRE(static_cast<int>(GuildConstants::GuildInfoButton::GUILD_OUT) == 0);
    }

    SECTION("GuildInfoButton::END is 6 — correct count for array-sizing sentinel")
    {
        REQUIRE(static_cast<int>(GuildConstants::GuildInfoButton::END) == 6);
    }

    SECTION("All 7 GuildInfoButton values (GUILD_OUT through END) are pairwise distinct")
    {
        const int buttons[] = {
            static_cast<int>(GuildConstants::GuildInfoButton::GUILD_OUT),
            static_cast<int>(GuildConstants::GuildInfoButton::GET_POSITION),
            static_cast<int>(GuildConstants::GuildInfoButton::FREE_POSITION),
            static_cast<int>(GuildConstants::GuildInfoButton::GET_OUT),
            static_cast<int>(GuildConstants::GuildInfoButton::UNION_CREATE),
            static_cast<int>(GuildConstants::GuildInfoButton::UNION_OUT),
            static_cast<int>(GuildConstants::GuildInfoButton::END),
        };
        constexpr int n = static_cast<int>(sizeof(buttons) / sizeof(buttons[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(buttons[i] != buttons[j]);
            }
        }
    }
}

TEST_CASE("AC-3 [6-3-1]: RelationshipType enum covers all alliance/rivalry flag bits",
          "[guild][enum][relationship][6-3-1]")
{
    SECTION("RelationshipType::NONE is 0x00 — no active relationship")
    {
        REQUIRE(static_cast<BYTE>(GuildConstants::RelationshipType::NONE) == 0x00u);
    }

    SECTION("RelationshipType::UNION is 0x01")
    {
        REQUIRE(static_cast<BYTE>(GuildConstants::RelationshipType::UNION) == 0x01u);
    }

    SECTION("RelationshipType::RIVAL is 0x02")
    {
        REQUIRE(static_cast<BYTE>(GuildConstants::RelationshipType::RIVAL) == 0x02u);
    }

    SECTION("RelationshipType::UNION_MASTER is 0x04")
    {
        REQUIRE(static_cast<BYTE>(GuildConstants::RelationshipType::UNION_MASTER) == 0x04u);
    }

    SECTION("RelationshipType::RIVAL_UNION is 0x08")
    {
        REQUIRE(static_cast<BYTE>(GuildConstants::RelationshipType::RIVAL_UNION) == 0x08u);
    }

    SECTION("RelationshipType values are power-of-two bit flags — no aliasing between non-NONE values")
    {
        const BYTE flags[] = {
            static_cast<BYTE>(GuildConstants::RelationshipType::UNION),
            static_cast<BYTE>(GuildConstants::RelationshipType::RIVAL),
            static_cast<BYTE>(GuildConstants::RelationshipType::UNION_MASTER),
            static_cast<BYTE>(GuildConstants::RelationshipType::RIVAL_UNION),
        };
        constexpr int n = static_cast<int>(sizeof(flags) / sizeof(flags[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                // Bit flags must not overlap (AND == 0)
                REQUIRE((flags[i] & flags[j]) == 0u);
            }
        }
    }
}

TEST_CASE("AC-3 [6-3-1]: GUILD_LIST_t struct has correct field layout", "[guild][struct][6-3-1]")
{
    SECTION("GUILD_LIST_t is a non-empty struct")
    {
        static_assert(sizeof(GUILD_LIST_t) > 0, "GUILD_LIST_t must be non-empty");
    }

    SECTION("GUILD_LIST_t::Name buffer covers MAX_USERNAME_SIZE+1 wide characters")
    {
        constexpr std::size_t expected = (static_cast<std::size_t>(MAX_USERNAME_SIZE) + 1) * sizeof(wchar_t);
        REQUIRE(sizeof(GUILD_LIST_t::Name) == expected);
    }

    SECTION("GUILD_LIST_t::Number, Server, GuildStatus are single-byte fields")
    {
        REQUIRE(sizeof(GUILD_LIST_t::Number) == 1u);
        REQUIRE(sizeof(GUILD_LIST_t::Server) == 1u);
        REQUIRE(sizeof(GUILD_LIST_t::GuildStatus) == 1u);
    }
}

TEST_CASE("AC-3 [6-3-1]: MARK_t struct has correct field layout for guild mark storage", "[guild][struct][mark][6-3-1]")
{
    SECTION("MARK_t is a non-empty struct")
    {
        static_assert(sizeof(MARK_t) > 0, "MARK_t must be non-empty");
    }

    SECTION("MARK_t::GuildName buffer is GUILD_NAME_BUFFER_SIZE wide characters")
    {
        constexpr std::size_t expected =
            static_cast<std::size_t>(GuildConstants::GUILD_NAME_BUFFER_SIZE) * sizeof(wchar_t);
        REQUIRE(sizeof(MARK_t::GuildName) == expected);
    }

    SECTION("MARK_t::UnionName buffer is GUILD_NAME_BUFFER_SIZE wide characters")
    {
        constexpr std::size_t expected =
            static_cast<std::size_t>(GuildConstants::GUILD_NAME_BUFFER_SIZE) * sizeof(wchar_t);
        REQUIRE(sizeof(MARK_t::UnionName) == expected);
    }

    SECTION("MARK_t::Mark array is GUILD_MARK_SIZE bytes (64)")
    {
        REQUIRE(sizeof(MARK_t::Mark) == static_cast<std::size_t>(GuildConstants::GUILD_MARK_SIZE));
    }

    SECTION("MARK_t::Key is a 4-byte int field")
    {
        REQUIRE(sizeof(MARK_t::Key) == 4u);
    }
}

// =============================================================================
// AC-4 [6-3-1]: Character guild fields and rendering constants
// CNewUINameWindow renders guild marks above characters using CHARACTER struct
// guild fields. GuildConstants::Colors defines ARGB values for relationship tiers.
// =============================================================================

TEST_CASE("AC-4 [6-3-1]: MAX_MARKS constant defines guild mark array capacity", "[guild][marks][constants][6-3-1]")
{
    SECTION("MAX_MARKS is 2000 — maximum concurrent guild marks in GuildMark[] global array")
    {
        REQUIRE(MAX_MARKS == 2000);
    }

    SECTION("MAX_MARKS exceeds Capacity::MAX_CAPACITY — mark cache larger than single guild")
    {
        // The mark cache must accommodate far more guilds than one guild's capacity
        REQUIRE(MAX_MARKS > GuildConstants::Capacity::MAX_CAPACITY);
    }
}

TEST_CASE("AC-4 [6-3-1]: GuildConstants::Colors ARGB values are correct for all tiers",
          "[guild][colors][rendering][6-3-1]")
{
    SECTION("YELLOW color is 0xFFC8FF64 — used for allied guild names")
    {
        REQUIRE(GuildConstants::Colors::YELLOW == 0xFFC8FF64u);
    }

    SECTION("WHITE color is 0xFFFFFFFF — used for neutral guild names")
    {
        REQUIRE(GuildConstants::Colors::WHITE == 0xFFFFFFFFu);
    }

    SECTION("GRAY color is 0xFF999999 — used for rival guild names")
    {
        REQUIRE(GuildConstants::Colors::GRAY == 0xFF999999u);
    }

    SECTION("All three guild colors are pairwise distinct ARGB values")
    {
        REQUIRE(GuildConstants::Colors::YELLOW != GuildConstants::Colors::WHITE);
        REQUIRE(GuildConstants::Colors::YELLOW != GuildConstants::Colors::GRAY);
        REQUIRE(GuildConstants::Colors::WHITE != GuildConstants::Colors::GRAY);
    }

    SECTION("All guild colors have full opacity (alpha = 0xFF)")
    {
        // Top byte of ARGB is alpha channel
        REQUIRE((GuildConstants::Colors::YELLOW >> 24u) == 0xFFu);
        REQUIRE((GuildConstants::Colors::WHITE >> 24u) == 0xFFu);
        REQUIRE((GuildConstants::Colors::GRAY >> 24u) == 0xFFu);
    }
}

TEST_CASE("AC-4 [6-3-1]: MARK_t guild name buffer is sufficient for GUILD_NAME_LENGTH characters",
          "[guild][marks][buffer][6-3-1]")
{
    SECTION("MARK_t::GuildName can hold at least GUILD_NAME_LENGTH+1 wide characters")
    {
        const std::size_t buffer_chars = sizeof(MARK_t::GuildName) / sizeof(wchar_t);
        REQUIRE(buffer_chars >= static_cast<std::size_t>(GuildConstants::GUILD_NAME_LENGTH) + 1);
    }

    SECTION("MARK_t::UnionName can hold at least GUILD_NAME_LENGTH+1 wide characters")
    {
        const std::size_t buffer_chars = sizeof(MARK_t::UnionName) / sizeof(wchar_t);
        REQUIRE(buffer_chars >= static_cast<std::size_t>(GuildConstants::GUILD_NAME_LENGTH) + 1);
    }
}

// =============================================================================
// AC-5 [6-3-1]: Chat encoding — char16_t alignment and constant consistency
// The .NET AOT bridge transmits chat via char16_t (SendPublicChatMessage /
// SendWhisperMessage). MAX_CHAT_SIZE must be consistent across all structs.
// MAX_USERNAME_SIZE is shared by chat ID fields, party names, and guild lists.
// =============================================================================

TEST_CASE("AC-5 [6-3-1]: Chat encoding constants are consistent across all subsystems",
          "[chat][encoding][constants][6-3-1]")
{
    SECTION("MAX_USERNAME_SIZE is 10 — shared by chat ID, party name, guild list name buffers")
    {
        REQUIRE(MAX_USERNAME_SIZE == 10);
    }

    SECTION("MAX_CHAT_SIZE is 90 — consistent architecture constraint for all chat buffers")
    {
        // Intentional AC traceability: also verified in AC-1. AC-5 re-checks cross-system consistency.
        REQUIRE(MAX_CHAT_SIZE == 90);
    }

    SECTION("PARTY_t name buffer uses MAX_USERNAME_SIZE — same as chat ID field")
    {
        // Intentional AC traceability: also verified in AC-2. AC-5 re-checks cross-system consistency.
        const std::size_t party_name_chars = sizeof(PARTY_t::Name) / sizeof(wchar_t);
        REQUIRE(party_name_chars == static_cast<std::size_t>(MAX_USERNAME_SIZE) + 1);
    }

    SECTION("GUILD_LIST_t name buffer uses MAX_USERNAME_SIZE — consistent with chat system")
    {
        // Intentional AC traceability: also verified in AC-3. AC-5 re-checks cross-system consistency.
        const std::size_t guild_name_chars = sizeof(GUILD_LIST_t::Name) / sizeof(wchar_t);
        REQUIRE(guild_name_chars == static_cast<std::size_t>(MAX_USERNAME_SIZE) + 1);
    }

    SECTION("GUILD_NAME_LENGTH(8) fits within MAX_USERNAME_SIZE(10) — guild names fit chat ID field")
    {
        // Guild names (8 chars) are shorter than username fields (10 chars),
        // which is architecturally correct — guild tags appear alongside player names
        REQUIRE(GuildConstants::GUILD_NAME_LENGTH <= MAX_USERNAME_SIZE);
    }
}

#ifdef MU_GAME_AVAILABLE
TEST_CASE("AC-5 [6-3-1]: PCHATING struct ChatText matches MAX_CHAT_SIZE for char16_t alignment",
          "[chat][encoding][packet][6-3-1]")
{
    SECTION("PCHATING::ChatText is exactly MAX_CHAT_SIZE bytes — aligned for .NET AOT char16_t bridge")
    {
        // The .NET bridge uses char16_t for SendPublicChatMessage/SendWhisperMessage.
        // MAX_CHAT_SIZE=90 bytes = 45 char16_t code units. The PCHATING ChatText is a
        // char[] of MAX_CHAT_SIZE bytes, matching the char16_t payload size.
        REQUIRE(sizeof(PCHATING::ChatText) == static_cast<std::size_t>(MAX_CHAT_SIZE));
    }

    SECTION("PCHATING_KEY::ChatText is exactly MAX_CHAT_SIZE bytes — same encoding alignment")
    {
        REQUIRE(sizeof(PCHATING_KEY::ChatText) == static_cast<std::size_t>(MAX_CHAT_SIZE));
    }

    SECTION("MAX_CHAT_SIZE is even — compatible with char16_t (2-byte) code units")
    {
        // char16_t requires 2-byte alignment. An even MAX_CHAT_SIZE means the buffer
        // can hold an exact number of char16_t code units without partial-unit waste.
        REQUIRE(MAX_CHAT_SIZE % 2 == 0);
    }
}
#endif // MU_GAME_AVAILABLE
