
#include "stdafx.h"
#include "I18N/All.h"

#include "NewBloodCastleSystem.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Core/WindowSystem.h"
#include "Audio/DSPlaySound.h"
#include "CSChaosCastle.h"
#include "World/MapInfra/MapManager.h"
#include "Render/Text/CUIRenderText.h"

using namespace SEASON3B;
using namespace mu::ui::window;

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
    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CBloodCastleResultMsgBoxLayout));
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
                if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_BLOODCASTLE_TIME))
                {
                    g_pNewUISystem->Show(mu::ui::window::INTERFACE_BLOODCASTLE_TIME);
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
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_BLOODCASTLE_TIME))
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_BLOODCASTLE_TIME);
        }
    }
}

void CNewBloodCastleSystem::RenderMatchResult(void)
{
    EnableAlphaTest();
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    std::vector<MatchResultText> texts;
    CollectMatchResult(texts);
    RenderMatchResultTexts(texts);

    DisableAlphaBlend();
}

void CNewBloodCastleSystem::CollectMatchResult(std::vector<MatchResultText>& texts) const
{
    int x = REFERENCE_WIDTH / 2;
    int yPos = m_PosResult.y + 40;
    wchar_t lpszStr[256] = {};

    auto add = [&texts, x](int y, const wchar_t* text, MatchResultText::Font font, DWORD color)
    { texts.push_back({text, x, y, 0, 0, RT3_WRITE_CENTER, font, color}); };

    const DWORD green = RGBA(128, 255, 128, 255);
    if (m_iNumResult)
    {
        add(yPos, I18N::Game::CompletedTheBloodCastleQuest, MatchResultText::Font::Normal, green);
        yPos += 16;
        add(yPos, I18N::Game::CongratulationsYouHaveSuccessfully, MatchResultText::Font::Normal, green);
    }
    else
    {
        add(yPos, I18N::Game::ToCompleteTheBloodCastleQuest, MatchResultText::Font::Normal, green);
        yPos += 16;
        add(yPos, I18N::Game::UnfortunatelyYouHaveFailed, MatchResultText::Font::Normal, green);
    }

    yPos += 30;

    const MatchResult* pResult = &m_MatchResult[0];

    mu_swprintf(lpszStr, I18N::Game::RewardedExpD, pResult->m_dwExp);
    add(yPos, lpszStr, MatchResultText::Font::Bold, RGBA(210, 255, 210, 255));
    yPos += 24;

    if (m_iNumResult)
    {
        mu_swprintf(lpszStr, I18N::Game::RewardedZenD, pResult->m_iZen);
        add(yPos, lpszStr, MatchResultText::Font::Bold, RGBA(255, 210, 210, 255));
        yPos += 24;
    }

    mu_swprintf(lpszStr, I18N::Game::BloodCastlePointD, pResult->m_iScore);
    add(yPos, lpszStr, MatchResultText::Font::Bold, RGBA(210, 210, 255, 255));
}
