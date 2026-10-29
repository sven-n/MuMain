#include "App/stdafx.h"

#include <doctest.h>

#include <cstdint>
#include <string>

#include "I18N/All.h"
#include "GameLogic/Events/CSEventMatch.h"
#include "Network/Server/ServerMessageTexts.h"
#include "Network/Server/SocketSystem.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/CursedTempleSystem.h"
#include "UI/Events/KanturuEvent.h"
#include "World/GameMaps/GMCrywolf1st.h"
#include "LegacyTextComparison.h"

namespace
{
// Every value a BYTE packet field can hold.
constexpr int kByteValueCount = 256;

// WSclient.cpp's ReceiveServerCommand(), command 1: Lookup(830 + Cmd2 - 20) from 20 on, else
// Lookup(650 + Cmd2). The Devil Square NPC lines are numbers 0 to 12, the Blood Castle ones 20 to
// 46 (OpenMU's BloodCastleContext sends 23, 24 and 46).
constexpr int kLegacyLastDevilSquareNpcLine = 12;
constexpr int kLegacyFirstBloodCastleNpcLine = 20;
constexpr int kLegacyLastBloodCastleNpcLine = 46;

const wchar_t* LegacyEventNpcLine(int cmd2)
{
    if (cmd2 >= 20)
    {
        return I18N::Game::Lookup(830 + cmd2 - 20);
    }
    return I18N::Game::Lookup(650 + cmd2);
}

bool IsEventNpcLineNumber(int cmd2)
{
    return cmd2 <= kLegacyLastDevilSquareNpcLine ||
           (cmd2 >= kLegacyFirstBloodCastleNpcLine && cmd2 <= kLegacyLastBloodCastleNpcLine);
}

// Commands 3 and 4: Lookup(710 + Cmd2), the Golden Archer's lines up to the Rena hints, and
// Lookup(725 + Cmd2), the three Rena hints.
constexpr int kLegacyGoldenArcherLineCount = 15;
constexpr int kLegacyRenaLineCount = 3;

// KanturuEvent.cpp's CreateMessageBox(): Lookup(2170 + result) for the results without a check of
// their own; the block's texts for POPUP_NONE to POPUP_NOT_MUNSTONE (OpenMU sends 0 and 1).
constexpr int kLegacyEntryResultTextCount = 3;

// CSEventMatch.cpp's CountdownText() text id for a countdown type between TYPE_MATCH_NONE and
// TYPE_MATCH_END.
int LegacyCountdownTextNum(int type)
{
    if (type >= TYPE_MATCH_CASTLE_ENTER_CLOSE && type <= TYPE_MATCH_CASTLE_END)
    {
        return 824 + type - TYPE_MATCH_CASTLE_ENTER_CLOSE;
    }
    if (type >= TYPE_MATCH_CHAOS_ENTER_START && type <= TYPE_MATCH_CHAOS_END)
    {
        int textNum = 824 + type - TYPE_MATCH_CHAOS_ENTER_START;
        if (textNum == 825)
        {
            textNum = 828;
        }
        return textNum;
    }
    if (type == TYPE_MATCH_CURSEDTEMPLE_ENTER_CLOSE || type == TYPE_MATCH_CURSEDTEMPLE_GAME_START)
    {
        return (type == TYPE_MATCH_CURSEDTEMPLE_GAME_START) ? 2386 : 2384;
    }
    if (type >= TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && type <= TYPE_MATCH_DOPPELGANGER_CLOSE)
    {
        return 2860 + type - TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE;
    }
    return 640 + type - TYPE_MATCH_DEVIL_ENTER_START;
}

// The countdown types with a line: every type CountdownText() looked a text up for, except the two
// after the server's Blood Castle "Congratulations" (TYPE_MATCH_CASTLE_END + 1), which no server
// sends and whose texts were unrelated.
bool HasCountdownLine(int type)
{
    const int castleCongratulations = TYPE_MATCH_CASTLE_END + 1;
    if (type <= TYPE_MATCH_NONE || type >= TYPE_MATCH_END)
    {
        return false;
    }
    return type <= castleCongratulations || type >= TYPE_MATCH_CHAOS_ENTER_START;
}

// The countdown types the client can be given: the server's BYTE plus one (ReceiveDevilSquareCountDown),
// or that sum cast back to a BYTE on the Doppelganger maps.
constexpr int kLastCountdownType = kByteValueCount;

// SocketSystem.cpp: Lookup(2640 + category - 1) for a socket option category from the socket item
// data file; probed well past both ends of the category range.
constexpr int kSocketCategoryProbe = 256;

// CursedTempleSystem.cpp's SyncSkill(): Lookup(2379 + (skill - AT_SKILL_CURSED_TEMPLE_PRODECTION)).
constexpr int kLegacyFirstTempleSkillText = 2379;
constexpr int kTempleSkillProbeEnd = 1024;

// GMCrywolf1st.cpp's AdvanceNoticeTexts(): Lookup(1957 + line + 4 * page) for page -1 to 3, with
// 1966 and 1967 printed through mu_swprintf.
constexpr int kFirstNoticePage = -1;
constexpr int kNoticePageCount = 4;
constexpr int kNoticeLinesPerPage = 4;

std::wstring LegacyNoticeText(int page, int line)
{
    const int nText = 1957 + line + 4 * page;
    if (1966 == nText || 1967 == nText)
    {
        wchar_t szText[256];
        mu_swprintf(szText, I18N::Game::Lookup(nText));
        return szText;
    }
    return I18N::Game::Lookup(nText);
}
} // namespace

TEST_CASE("server command 1 NPC lines match the legacy ids in their blocks, empty outside them [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int number = 0; number < kByteValueCount; ++number)
        {
            const wchar_t* actual = Network::ServerMessages::GetEventNpcLine(static_cast<std::uint8_t>(number));
            const wchar_t* expected = IsEventNpcLineNumber(number) ? LegacyEventNpcLine(number) : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << number);
        }
    });
}

TEST_CASE("server command 3 Golden Archer lines match the legacy ids, empty past them [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int number = 0; number < kByteValueCount; ++number)
        {
            const wchar_t* actual = Network::ServerMessages::GetGoldenArcherLine(static_cast<std::uint8_t>(number));
            const wchar_t* expected = number < kLegacyGoldenArcherLineCount ? I18N::Game::Lookup(710 + number) : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << number);
        }
    });
}

TEST_CASE("server command 4 Rena hints match the legacy ids, empty past them [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int number = 0; number < kByteValueCount; ++number)
        {
            const wchar_t* actual = Network::ServerMessages::GetRenaLine(static_cast<std::uint8_t>(number));
            const wchar_t* expected = number < kLegacyRenaLineCount ? I18N::Game::Lookup(725 + number) : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << number);
        }
    });
}

TEST_CASE("Kanturu entry result texts match the legacy ids, empty for other results [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int result = 0; result < kByteValueCount; ++result)
        {
            const wchar_t* actual = mu::ui::window::CKanturu2ndEnterNpc::EntryResultText(result);
            const wchar_t* expected = result < kLegacyEntryResultTextCount ? I18N::Game::Lookup(2170 + result) : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << result);
        }
    });
}

TEST_CASE("event countdown formats match the legacy ids for the countdown types, empty otherwise [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int type = 0; type <= kLastCountdownType; ++type)
        {
            const wchar_t* actual = GetMatchCountdownFormat(type);
            const wchar_t* expected = HasCountdownLine(type) ? I18N::Game::Lookup(LegacyCountdownTextNum(type)) : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << type);
        }
    });
}

TEST_CASE("socket category names match the legacy ids, empty for other categories [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int category = -kSocketCategoryProbe; category <= kSocketCategoryProbe; ++category)
        {
            const wchar_t* actual = SEASON4A::CSocketItemMgr::GetSocketCategoryName(category);
            const bool isCategory = category >= SEASON4A::SOC_IMPROVE_ATTACK &&
                                    category < SEASON4A::MAX_SOCKET_OPTION_CATEGORY;
            const wchar_t* expected = isCategory ? I18N::Game::Lookup(2640 + category - 1) : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << category);
        }
    });
}

TEST_CASE("Illusion Temple skill descriptions match the legacy ids, empty for other skills [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int skill = 0; skill < kTempleSkillProbeEnd; ++skill)
        {
            const wchar_t* actual = mu::ui::window::CCursedTempleSystem::SkillDescription(skill);
            const bool isTempleSkill =
                skill >= AT_SKILL_CURSED_TEMPLE_PRODECTION && skill <= AT_SKILL_CURSED_TEMPLE_SUBLIMATION;
            const wchar_t* expected = isTempleSkill
                ? I18N::Game::Lookup(kLegacyFirstTempleSkillText + (skill - AT_SKILL_CURSED_TEMPLE_PRODECTION))
                : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << skill);
        }
    });
}

TEST_CASE("Crywolf notice board lines match the legacy ids on every page [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int page = kFirstNoticePage; page < kNoticePageCount; ++page)
        {
            for (int line = 0; line < kNoticeLinesPerPage; ++line)
            {
                const std::wstring actual = M34CryWolf1st::NoticeText(page, line);
                const std::wstring expected = LegacyNoticeText(page, line);
                CHECK_MESSAGE(LegacyTextComparison::SameText(actual.c_str(), expected.c_str()),
                              locale << " " << page << " " << line);
            }
        }

        CHECK_MESSAGE(M34CryWolf1st::NoticeText(kFirstNoticePage - 1, 0).empty(), locale);
        CHECK_MESSAGE(M34CryWolf1st::NoticeText(kNoticePageCount, 0).empty(), locale);
        CHECK_MESSAGE(M34CryWolf1st::NoticeText(0, -1).empty(), locale);
        CHECK_MESSAGE(M34CryWolf1st::NoticeText(0, kNoticeLinesPerPage).empty(), locale);
    });
}
