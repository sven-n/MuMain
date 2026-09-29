//////////////////////////////////////////////////////////////////////
// NewBloodCastleSystem.cpp: implementation of the CNewBloodCastleSystem class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I18N/All.h"

#include "NewBloodCastleSystem.h"
#include "UI/NewUI/Dialogs/NewUICustomMessageBox.h"
#include "UI/NewUI/NewUISystem.h"
#include "Audio/DSPlaySound.h"
#include "CSChaosCastle.h"
#include "World/MapInfra/MapManager.h"
#include "Core/Text/TextLineWrap.h"

using namespace SEASON3B;

namespace
{
// Layout of the result box, relative to its top edge.
constexpr int ResultTextTop = 40;
constexpr int ResultTextWidth = 210;
constexpr int ResultLineHeight = 16;
constexpr int ResultSectionGap = 14;
// The result box is sized for this many lines; each further line makes it taller.
constexpr int ResultFittingLines = 2;
} // namespace

CNewBloodCastleSystem::CNewBloodCastleSystem()
{
}

CNewBloodCastleSystem::~CNewBloodCastleSystem()
{
}

void CNewBloodCastleSystem::SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success)
{
    if (iNumDevilRank != 255)
    {
        return;
    }

    m_iNumResult = Success;
    memcpy(m_MatchResult, pMatchResult, sizeof(MatchResult));
    WrapResultText();
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CBloodCastleResultMsgBoxLayout));
}

void CNewBloodCastleSystem::SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data)
{
    switch (data->m_byPlayState)
    {
    case 0:
        SetAllAction(PLAYER_RUSH1);
        PlayBuffer(SOUND_BLOODCASTLE, NULL, true);

    case 1:
        SetMatchInfo(data->m_byPlayState + 1, 15 * 60, data->m_wRemainSec, data->m_wMaxKillMonster, data->m_wCurKillMonster);

        if (data->m_wIndex != 65535 && data->m_byItemType != 255 && data->m_byItemType != 0)
        {
            WORD Key = data->m_wIndex;
            Key &= 0x7FFF;

            int  index = HangerBloodCastleQuestItem(Key);
            CHARACTER* c = &CharactersClient[index];

            if (c != NULL)
            {
                c->EtcPart = data->m_byItemType;
            }
        }
        break;

    case 2:
        clearMatchInfo();
        StopBuffer(SOUND_BLOODCASTLE, true);

        break;

    case 3:
        SetActionObject(gMapManager.WorldActive, 36, 20, 1.f);
        break;
    case 4:
        SetMatchInfo(data->m_byPlayState + 1, 15 * 60, data->m_wRemainSec, data->m_wMaxKillMonster, data->m_wCurKillMonster);

        if (data->m_wIndex != 65535 && data->m_byItemType != 255 && data->m_byItemType != 0)
        {
            WORD Key = data->m_wIndex;
            Key &= 0x7FFF;

            int  index = HangerBloodCastleQuestItem(Key);
            CHARACTER* c = &CharactersClient[index];

            if (c != NULL)
            {
                c->EtcPart = data->m_byItemType;
            }
        }
        break;
    }
}

void CNewBloodCastleSystem::RenderMatchTimes(void)
{
    if (m_byMatchType > 0 && gMapManager.InBloodCastle() == true)
    {
        switch (m_byMatchType)
        {
        case 0:
        case 1:
        case 2:
        case 5:
            if (m_iMatchTime > 0)
            {
                if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE_TIME))
                {
                    g_pNewUISystem->Show(SEASON3B::INTERFACE_BLOODCASTLE_TIME);
                }

                g_pBloodCastle->SetTime(m_iMatchTime);
                g_pBloodCastle->SetKillMonsterStatue(m_iKillMonster, m_iMaxKillMonster);
            }
            break;

        default:
            break;
        }
    }
    else
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE_TIME))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_BLOODCASTLE_TIME);
        }
    }
}

// One sentence per outcome, wrapped to the width of the result box once when the
// result arrives, so every language keeps its own word order across the lines.
void CNewBloodCastleSystem::WrapResultText()
{
    g_pRenderText->SetFont(g_hFont);
    m_ResultLines =
        WrapTextToWidth(m_iNumResult ? I18N::Game::BloodCastleQuestCompleted : I18N::Game::BloodCastleQuestFailed,
                        ResultTextWidth, [](const wchar_t* text, size_t length)
                        { return g_pRenderText->MeasureText(text, static_cast<int>(length)).cx; });
}

int CNewBloodCastleSystem::GetResultExtraHeight() const
{
    const int extraLines = static_cast<int>(m_ResultLines.size()) - ResultFittingLines;
    return extraLines > 0 ? extraLines * ResultLineHeight : 0;
}

void CNewBloodCastleSystem::RenderMatchResult(void)
{
    int x = REFERENCE_WIDTH / 2;
    int yPos = m_PosResult.y + ResultTextTop;

    EnableAlphaTest();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(128, 255, 128, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    wchar_t lpszStr[256] = {};

    for (const auto& line : m_ResultLines)
    {
        g_pRenderText->RenderText(x, yPos, line.c_str(), 0, 0, RT3_WRITE_CENTER);
        yPos += ResultLineHeight;
    }

    yPos += ResultSectionGap;

    MatchResult* pResult = &m_MatchResult[0];

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(210, 255, 210, 255);
    mu_swprintf(lpszStr, I18N::Game::RewardedExpD, pResult->m_dwExp);
    g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);
    yPos += 24;

    if (m_iNumResult)
    {
        g_pRenderText->SetTextColor(255, 210, 210, 255);
        mu_swprintf(lpszStr, I18N::Game::RewardedZenD, pResult->m_iZen);
        g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);
        yPos += 24;
    }

    g_pRenderText->SetTextColor(210, 210, 255, 255);
    mu_swprintf(lpszStr, I18N::Game::BloodCastlePointD, pResult->m_iScore);
    g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);

    DisableAlphaBlend();
}