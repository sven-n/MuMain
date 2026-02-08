//////////////////////////////////////////////////////////////////////////
//  CSEventMatch.cpp
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzLodTerrain.h"
#include "Scenes/SceneCore.h"
#include "ZzzAI.h"
#include "CSEventMatch.h"

#include "NewUICustomMessageBox.h"
#include "NewUISystem.h"

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
    SetPosition(640 - 230 / 2, 100);
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
    SetPosition(640 - 230 / 2, 100);
}

void CSBaseMatch::RenderTime(void)
{
    float x, y;

    if (m_iMatchCountDownType <= TYPE_MATCH_NONE || m_iMatchCountDownType >= TYPE_MATCH_END)
    {
        return;
    }

    const auto now = MatchClock::now();
    const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - m_matchCountDownStart);
    if (elapsedSeconds >= kMatchCountdownDuration)
    {
        m_iMatchCountDownType = TYPE_MATCH_NONE;
        return;
    }

    DisableAlphaBlend();
    EnableAlphaTest(false);

    x = 10.0f;
    y = 480.0f - 70.0f;
    g_pRenderText->SetTextColor(128, 128, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 128);

    const int remainingSeconds = static_cast<int>((kMatchCountdownDuration - elapsedSeconds).count());
    wchar_t lpszStr[256]{0};

    if (m_iMatchCountDownType >= TYPE_MATCH_CASTLE_ENTER_CLOSE && m_iMatchCountDownType <= TYPE_MATCH_CASTLE_END)
    {
        const int textNum = 824 + m_iMatchCountDownType - TYPE_MATCH_CASTLE_ENTER_CLOSE;
        WriteWide(lpszStr, GlobalText[textNum], GlobalText[1146], remainingSeconds);
    }
    else if (m_iMatchCountDownType >= TYPE_MATCH_CHAOS_ENTER_START && m_iMatchCountDownType <= TYPE_MATCH_CHAOS_END)
    {
        int textNum = 824 + m_iMatchCountDownType - TYPE_MATCH_CHAOS_ENTER_START;
        if (textNum == 825)
        {
            textNum = 828;
        }
        WriteWide(lpszStr, GlobalText[textNum], GlobalText[1147], remainingSeconds);
    }
    else if (m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_ENTER_CLOSE
        || m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_GAME_START)
    {
        int textNum = (m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_GAME_START) ? 2386 : 2384;
        WriteWide(lpszStr, GlobalText[textNum], remainingSeconds);
    }
    else if (m_iMatchCountDownType >= TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && m_iMatchCountDownType <= TYPE_MATCH_DOPPELGANGER_CLOSE)
    {
        const int textNum = 2860 + m_iMatchCountDownType - TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE;
        WriteWide(lpszStr, GlobalText[textNum], remainingSeconds);
    }
    else
    {
        const int textNum = 640 + m_iMatchCountDownType - TYPE_MATCH_DEVIL_ENTER_START;
        WriteWide(lpszStr, GlobalText[textNum], remainingSeconds);
    }

    g_pRenderText->RenderText(640 / 2, static_cast<int>(y), lpszStr, 0, 0, RT3_WRITE_CENTER);
}

void CSBaseMatch::renderOnlyTime(float x, float y, int MatchTime)
{
    wchar_t lpszStr[256]{0};
    const int iMinute = MatchTime / 60;
    const int iSecondTime = MatchTime - (iMinute * 60);

    WriteWide(lpszStr, L" %.2d :", iMinute);

    if (iSecondTime >= 0)
    {
        AppendWide(lpszStr, L" %.2d", iSecondTime);
    }

    if (iMinute < 5)
    {
        g_pRenderText->SetTextColor(255, 32, 32, 255);
    }
    if (iMinute < 15)
    {
        AppendWide(lpszStr, L": %.2d", static_cast<int>(WorldTime) % 60);
    }
    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->RenderText(static_cast<int>(x), static_cast<int>(y), lpszStr, 0, 0, RT3_WRITE_CENTER);
}

void CSBaseMatch::SetPosition(int ix, int iy)
{
    m_PosResult.x = ix;
    m_PosResult.y = iy;
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

    SEASON3B::TMsgBoxLayoutContainer<SEASON3B::CDevilSquareRankMsgBoxLayout> msgBoxLayout;
    SEASON3B::CreateMessageBox(msgBoxLayout);
}

void CSDevilSquareMatch::RenderMatchTimes(void)
{
    return;
}

void CSDevilSquareMatch::SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data)
{
    return;
}

void CSDevilSquareMatch::RenderMatchResult(void)
{
    int xPos[6] = { m_PosResult.x, };
    xPos[1] = xPos[0] + 15;
    xPos[2] = xPos[1] + 15;
    xPos[3] = xPos[2] + 60;
    xPos[4] = xPos[3] + 50;
    xPos[5] = xPos[4] + 38;

    int yPos = m_PosResult.y + 40;

    g_pRenderText->SetBgColor(0);

    wchar_t lpszStr[256] { 0 };

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(xPos[2], yPos, GlobalText[647]);
    yPos += 16;
    WriteWide(lpszStr, GlobalText[648], Hero->ID);
    g_pRenderText->RenderText((xPos[2]), yPos, lpszStr);
    yPos += 24;

    g_pRenderText->SetTextColor(0, 255, 0, 255);
    g_pRenderText->RenderText(xPos[2], yPos, GlobalText[680], xPos[3] - xPos[1], RT3_SORT_CENTER);
    g_pRenderText->RenderText(xPos[3], yPos, GlobalText[682], xPos[4] - xPos[3], RT3_SORT_CENTER);
    g_pRenderText->RenderText(xPos[4], yPos, GlobalText[683], xPos[5] - xPos[4], RT3_SORT_CENTER);
    g_pRenderText->RenderText(xPos[5], yPos, GlobalText[684], (640 - 230) / 2 + 210 - xPos[5], RT3_SORT_CENTER);
    yPos += 20;

    int yStartPos = yPos;

    for (int i = 0; i < m_iNumResult; ++i)
    {
        MatchResult* pResult = &m_MatchResult[i];

        // Highlight "my result" with a different color
        if (i == m_iMyResult - 1)
        {
            g_pRenderText->SetTextColor(200, 120, 0, 255); // Special color for "my result"
        }
        else
        {
            g_pRenderText->SetTextColor(255, 255, 0, 255); // Normal color for others
        }

        // Render rank number (index + 1 for display)
        WriteWide(lpszStr, L"%2d", i + 1);
        g_pRenderText->RenderText(xPos[1], yPos, lpszStr);

        // Render player ID
        std::fill(std::begin(lpszStr), std::end(lpszStr), L'\0');
        CMultiLanguage::ConvertFromUtf8(lpszStr, reinterpret_cast<char*>(pResult->m_lpID), MAX_USERNAME_SIZE);
        g_pRenderText->RenderText(xPos[2], yPos, lpszStr);

        // Render score
        WriteWide(lpszStr, L"%10lu", pResult->m_iScore);
        g_pRenderText->RenderText(xPos[3], yPos, lpszStr);

        // Render experience
        WriteWide(lpszStr, L"%6lu", pResult->m_dwExp);
        g_pRenderText->RenderText(xPos[4], yPos, lpszStr);

        // Render Zen
        WriteWide(lpszStr, L"%6lu", pResult->m_iZen);
        g_pRenderText->RenderText(xPos[5], yPos, lpszStr);

        // Increment yPos for the next row
        yPos += 16;
    }

    // Render a special section for "my result"
    if (m_iMyResult > 0 && m_iMyResult <= m_iNumResult)
    {
        int myIndex = m_iMyResult - 1; // Convert to zero-based index
        MatchResult* myResult = &m_MatchResult[myIndex];

        yPos = yStartPos + 16 * 10; // Fixed position for the special section
        g_pRenderText->SetTextColor(200, 120, 0, 255); // Special color

        // "My Ranking" label
        g_pRenderText->RenderText(xPos[0], yPos, GlobalText[685], 230, 0, RT3_SORT_CENTER);
        yPos += 20;

        // Render my rank
        WriteWide(lpszStr, L"%2d", m_iMyResult);
        g_pRenderText->RenderText(xPos[1], yPos, lpszStr);

        // Render my ID
        std::fill(std::begin(lpszStr), std::end(lpszStr), L'\0');
        CMultiLanguage::ConvertFromUtf8(lpszStr, reinterpret_cast<char*>(myResult->m_lpID), MAX_USERNAME_SIZE);
        g_pRenderText->RenderText(xPos[2], yPos, lpszStr);

        // Render my score
        WriteWide(lpszStr, L"%10lu", myResult->m_iScore);
        g_pRenderText->RenderText(xPos[3], yPos, lpszStr);

        // Render my experience
        WriteWide(lpszStr, L"%6lu", myResult->m_dwExp);
        g_pRenderText->RenderText(xPos[4], yPos, lpszStr);

        // Render my Zen
        WriteWide(lpszStr, L"%6lu", myResult->m_iZen);
        g_pRenderText->RenderText(xPos[5], yPos, lpszStr);
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

void CCursedTempleMatch::RenderMatchResult()
{
    return;
}