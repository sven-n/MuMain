//////////////////////////////////////////////////////////////////////////
//  CSEventMatch.cpp
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Scenes/SceneCore.h"
#include "Engine/AI/ZzzAI.h"
#include "CSEventMatch.h"
#include "I18N/All.h"

#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Core/WindowSystem.h"
#include "Render/Text/CUIRenderText.h"

#include <algorithm>
#include <chrono>
#include <cwchar>
#include <iterator>

extern int g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];

namespace
{
constexpr std::chrono::seconds kMatchCountdownDuration{30};

template <std::size_t N>
void ClearWideBuffer(wchar_t (&buffer)[N])
{
    std::fill(std::begin(buffer), std::end(buffer), L'\0');
}

template <std::size_t N, typename... Args>
void WriteWide(wchar_t (&buffer)[N], const wchar_t* format, Args... args)
{
    if (format == nullptr)
    {
        buffer[0] = L'\0';
        return;
    }

    std::swprintf(buffer, static_cast<std::size_t>(N), format, args...);
}

template <std::size_t N, typename... Args>
void AppendWide(wchar_t (&buffer)[N], const wchar_t* format, Args... args)
{
    if (format == nullptr)
    {
        return;
    }

    const std::size_t currentLength = std::wcslen(buffer);
    if (currentLength >= N)
    {
        return;
    }

    std::swprintf(buffer + currentLength, static_cast<std::size_t>(N - currentLength), format, args...);
}
} // namespace

void CSBaseMatch::clearMatchInfo(void)
{
    m_byMatchType = 0;
    m_iMatchTimeMax = -1;
    m_iMatchTime = -1;
    m_iMaxKillMonster = -1;
    m_iKillMonster = -1;
}

bool CSBaseMatch::getEqualMonster(int addV)
{
    if (m_iKillMonster <= (m_iMaxKillMonster + addV)) return  true;

    return false;
}

void CSBaseMatch::StartMatchCountDown(int iType)
{
    if (m_iMatchCountDownType >= TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && m_iMatchCountDownType <= TYPE_MATCH_DOPPELGANGER_CLOSE)
    {
        if (!(iType >= TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && iType <= TYPE_MATCH_DOPPELGANGER_CLOSE) && iType != TYPE_MATCH_NONE)
        {
            return;
        }
    }
    m_iMatchCountDownType = static_cast<MATCH_TYPE>(iType);
    m_matchCountDownStart = MatchClock::now();
}
void CSBaseMatch::SetMatchInfo(const std::uint8_t byType, const int iMaxTime, const int iTime, const int iMaxMonster, const int iKillMonster)
{
    m_byMatchType = byType;
    m_iMatchTimeMax = iMaxTime;
    m_iMatchTime = iTime;
    m_iMaxKillMonster = iMaxMonster;
    m_iKillMonster = iKillMonster;
}

std::wstring CSBaseMatch::CountdownText(void)
{
    if (m_iMatchCountDownType <= TYPE_MATCH_NONE || m_iMatchCountDownType >= TYPE_MATCH_END)
    {
        return {};
    }

    const auto now = MatchClock::now();
    const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - m_matchCountDownStart);
    if (elapsedSeconds >= kMatchCountdownDuration)
    {
        m_iMatchCountDownType = TYPE_MATCH_NONE;
        return {};
    }

    const int remainingSeconds = static_cast<int>((kMatchCountdownDuration - elapsedSeconds).count());
    wchar_t lpszStr[256]{0};

    if (m_iMatchCountDownType >= TYPE_MATCH_CASTLE_ENTER_CLOSE && m_iMatchCountDownType <= TYPE_MATCH_CASTLE_END)
    {
        const int textNum = 824 + m_iMatchCountDownType - TYPE_MATCH_CASTLE_ENTER_CLOSE;
        WriteWide(lpszStr, I18N::Game::Lookup(textNum), I18N::Game::BloodCastle, remainingSeconds);
    }
    else if (m_iMatchCountDownType >= TYPE_MATCH_CHAOS_ENTER_START && m_iMatchCountDownType <= TYPE_MATCH_CHAOS_END)
    {
        int textNum = 824 + m_iMatchCountDownType - TYPE_MATCH_CHAOS_ENTER_START;
        if (textNum == 825)
        {
            textNum = 828;
        }
        WriteWide(lpszStr, I18N::Game::Lookup(textNum), I18N::Game::ChaosCastle, remainingSeconds);
    }
    else if (m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_ENTER_CLOSE
        || m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_GAME_START)
    {
        int textNum = (m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_GAME_START) ? 2386 : 2384;
        WriteWide(lpszStr, I18N::Game::Lookup(textNum), remainingSeconds);
    }
    else if (m_iMatchCountDownType >= TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && m_iMatchCountDownType <= TYPE_MATCH_DOPPELGANGER_CLOSE)
    {
        const int textNum = 2860 + m_iMatchCountDownType - TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE;
        WriteWide(lpszStr, I18N::Game::Lookup(textNum), remainingSeconds);
    }
    else
    {
        const int textNum = 640 + m_iMatchCountDownType - TYPE_MATCH_DEVIL_ENTER_START;
        WriteWide(lpszStr, I18N::Game::Lookup(textNum), remainingSeconds);
    }

    return lpszStr;
}

void CSDevilSquareMatch::SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success)
{
    if (iNumDevilRank >= 200)
    {
        return;
    }

    m_iNumResult = iNumDevilRank;
    m_iMyResult = iMyRank;

    memcpy(m_MatchResult, pMatchResult, m_iNumResult * sizeof(MatchResult));

    mu::ui::window::TMsgBoxLayoutContainer<mu::ui::window::CDevilSquareRankMsgBoxLayout> msgBoxLayout;
    mu::ui::window::CreateMessageBox(msgBoxLayout);
}

void CSDevilSquareMatch::RenderMatchTimes(void)
{
    return;
}

void CSDevilSquareMatch::SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data)
{
    return;
}

void CSDevilSquareMatch::CollectMatchResult(std::vector<MatchResultLine>& lines) const
{
    wchar_t lpszStr[256] { 0 };
    // The font is the one left set (the original sets none).
    auto cell = [](const wchar_t* text, DWORD color, int boxWidth = 0, int boxHeight = 0)
    { return MatchResultCell{text, boxWidth, boxHeight, MatchResultCell::Font::Unchanged, color}; };

    const DWORD white = RGBA(255, 255, 255, 255);
    lines.push_back({"message", {cell(I18N::Game::Congratulations, white)}});
    WriteWide(lpszStr, I18N::Game::SYourBraveryIsProvenInDevilSquare, Hero->ID);
    lines.push_back({"message", {cell(lpszStr, white)}});

    // The headers pass their column's width as the box width and RT3_SORT_CENTER as its height,
    // which shrinks them.
    const DWORD green = RGBA(0, 255, 0, 255);
    lines.push_back({"header",
                     {cell(I18N::Game::Rank, green, 75, RT3_SORT_CENTER), cell(I18N::Game::Point, green, 50, RT3_SORT_CENTER),
                      cell(I18N::Game::EXP, green, 38, RT3_SORT_CENTER),
                      cell(I18N::Game::Reward, green, 32, RT3_SORT_CENTER)}});

    auto row = [&](const char* role, int rank, const MatchResult& result, DWORD color)
    {
        MatchResultLine line{role, {}};
        WriteWide(lpszStr, L"%2d", rank);
        line.cells.push_back(cell(lpszStr, color));

        std::fill(std::begin(lpszStr), std::end(lpszStr), L'\0');
        CMultiLanguage::ConvertFromUtf8(lpszStr, reinterpret_cast<const char*>(result.m_lpID), MAX_USERNAME_SIZE);
        line.cells.push_back(cell(lpszStr, color));

        WriteWide(lpszStr, L"%10lu", result.m_iScore);
        line.cells.push_back(cell(lpszStr, color));

        WriteWide(lpszStr, L"%6lu", result.m_dwExp);
        line.cells.push_back(cell(lpszStr, color));

        WriteWide(lpszStr, L"%6lu", result.m_iZen);
        line.cells.push_back(cell(lpszStr, color));
        lines.push_back(std::move(line));
    };

    const DWORD myColor = RGBA(200, 120, 0, 255); // "my result"
    for (int i = 0; i < m_iNumResult; ++i)
        row("row", i + 1, m_MatchResult[i], i == m_iMyResult - 1 ? myColor : RGBA(255, 255, 0, 255));

    // A section for "my result" at a fixed place under the table.
    if (m_iMyResult > 0 && m_iMyResult <= m_iNumResult)
    {
        lines.push_back({"my-info", {cell(I18N::Game::MyInfo, myColor, 230, 0)}});
        row("my-row", m_iMyResult, m_MatchResult[m_iMyResult - 1], myColor);
    }
}

void CCursedTempleMatch::SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data)
{
    return;
}

void CCursedTempleMatch::SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success)
{
    return;
}

void CCursedTempleMatch::RenderMatchTimes()
{
    return;
}
