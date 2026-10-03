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
    SetPosition(REFERENCE_WIDTH - 230 / 2, 100);
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
    SetPosition(REFERENCE_WIDTH - 230 / 2, 100);
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
    y = (float)REFERENCE_HEIGHT - 70.0f;
    g_pRenderText->SetTextColor(128, 128, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 128);

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

    g_pRenderText->RenderText(REFERENCE_WIDTH / 2, static_cast<int>(y), lpszStr, 0, 0, RT3_WRITE_CENTER);
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

void RenderMatchResultTexts(const std::vector<MatchResultText>& texts)
{
    for (const MatchResultText& text : texts)
    {
        if (text.font == MatchResultText::Font::Normal)
            g_pRenderText->SetFont(g_hFont);
        else if (text.font == MatchResultText::Font::Bold)
            g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetTextColor(text.color);
        g_pRenderText->RenderText(text.x, text.y, text.text.c_str(), text.boxWidth, text.boxHeight, text.sort);
    }
}

void CSDevilSquareMatch::RenderMatchResult(void)
{
    g_pRenderText->SetBgColor(0);

    std::vector<MatchResultText> texts;
    CollectMatchResult(texts);
    RenderMatchResultTexts(texts);
}

void CSDevilSquareMatch::CollectMatchResult(std::vector<MatchResultText>& texts) const
{
    int xPos[6] = { m_PosResult.x, };
    xPos[1] = xPos[0] + 15;
    xPos[2] = xPos[1] + 15;
    xPos[3] = xPos[2] + 60;
    xPos[4] = xPos[3] + 50;
    xPos[5] = xPos[4] + 38;

    int yPos = m_PosResult.y + 40;

    wchar_t lpszStr[256] { 0 };
    // The font is the one left set (the original sets none).
    auto add = [&texts](int x, int y, const wchar_t* text, DWORD color, int boxWidth = 0, int boxHeight = 0,
                        int sort = RT3_SORT_LEFT)
    { texts.push_back({text, x, y, boxWidth, boxHeight, sort, MatchResultText::Font::Unchanged, color}); };

    const DWORD white = RGBA(255, 255, 255, 255);
    add(xPos[2], yPos, I18N::Game::Congratulations, white);
    yPos += 16;
    WriteWide(lpszStr, I18N::Game::SYourBraveryIsProvenInDevilSquare, Hero->ID);
    add(xPos[2], yPos, lpszStr, white);
    yPos += 24;

    // The headers pass their width as the box width and RT3_SORT_CENTER as its height: left-aligned.
    const DWORD green = RGBA(0, 255, 0, 255);
    add(xPos[2], yPos, I18N::Game::Rank, green, xPos[3] - xPos[1], RT3_SORT_CENTER);
    add(xPos[3], yPos, I18N::Game::Point, green, xPos[4] - xPos[3], RT3_SORT_CENTER);
    add(xPos[4], yPos, I18N::Game::EXP, green, xPos[5] - xPos[4], RT3_SORT_CENTER);
    add(xPos[5], yPos, I18N::Game::Reward, green, (REFERENCE_WIDTH - 230) / 2 + 210 - xPos[5], RT3_SORT_CENTER);
    yPos += 20;

    int yStartPos = yPos;

    auto addRow = [&](int rank, const MatchResult& result, DWORD color)
    {
        WriteWide(lpszStr, L"%2d", rank);
        add(xPos[1], yPos, lpszStr, color);

        std::fill(std::begin(lpszStr), std::end(lpszStr), L'\0');
        CMultiLanguage::ConvertFromUtf8(lpszStr, reinterpret_cast<const char*>(result.m_lpID), MAX_USERNAME_SIZE);
        add(xPos[2], yPos, lpszStr, color);

        WriteWide(lpszStr, L"%10lu", result.m_iScore);
        add(xPos[3], yPos, lpszStr, color);

        WriteWide(lpszStr, L"%6lu", result.m_dwExp);
        add(xPos[4], yPos, lpszStr, color);

        WriteWide(lpszStr, L"%6lu", result.m_iZen);
        add(xPos[5], yPos, lpszStr, color);
    };

    const DWORD myColor = RGBA(200, 120, 0, 255); // "my result"
    for (int i = 0; i < m_iNumResult; ++i)
    {
        addRow(i + 1, m_MatchResult[i], i == m_iMyResult - 1 ? myColor : RGBA(255, 255, 0, 255));
        yPos += 16;
    }

    // A section for "my result" at a fixed position under the table.
    if (m_iMyResult > 0 && m_iMyResult <= m_iNumResult)
    {
        yPos = yStartPos + 16 * 10;
        add(xPos[0], yPos, I18N::Game::MyInfo, myColor, 230, 0, RT3_SORT_CENTER);
        yPos += 20;
        addRow(m_iMyResult, m_MatchResult[m_iMyResult - 1], myColor);
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