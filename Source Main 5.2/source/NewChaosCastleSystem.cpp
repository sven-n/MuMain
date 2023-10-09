// NewUIChaosCastleSystem.cpp: implementation of the CNewUIChaosCastleSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

using namespace SEASON3B;

#include "NewChaosCastleSystem.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "NewUICustomMessageBox.h"
#include "MapManager.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"

extern int g_iChatInputType;
extern int g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];
extern  int g_iActionObjectType;
extern  int g_iActionWorld;
extern  int g_iActionTime;
extern  float g_fActionObjectVelocity;

CNewChaosCastleSystem::CNewChaosCastleSystem()
{
    int iChaosCastleLimitArea1[16] = { 23, 75, 44, 76, 43, 77, 44, 108, 23, 107, 42, 108, 23, 77, 24, 106 };
    int iChaosCastleLimitArea2[16] = { 25, 77, 42, 78, 41, 79, 42, 106, 25, 105, 40, 106, 25, 79, 26, 104 };
    int iChaosCastleLimitArea3[16] = { 27, 79, 40, 80, 39, 81, 40, 104, 27, 103, 38, 104, 27, 81, 28, 102 };
    memcpy(m_iChaosCastleLimitArea1, iChaosCastleLimitArea1, sizeof(int) * 16);
    memcpy(m_iChaosCastleLimitArea2, iChaosCastleLimitArea2, sizeof(int) * 16);
    memcpy(m_iChaosCastleLimitArea3, iChaosCastleLimitArea3, sizeof(int) * 16);

    m_byCurrCastleLevel = 255;
    m_bActionMatch = true;
}

CNewChaosCastleSystem::~CNewChaosCastleSystem()
{
}

void CNewChaosCastleSystem::SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success)
{
    if (iNumDevilRank != 254)
    {
        return;
    }

    m_iNumResult = Success;

    memcpy(m_MatchResult, pMatchResult, sizeof(MatchResult));

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosCastleResultMsgBoxLayout));
}

void CNewChaosCastleSystem::SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data)
{
    switch (data->m_byPlayState)
    {
    case 5:
        m_bActionMatch = true;
        m_byCurrCastleLevel = 255;

        SetAllAction(PLAYER_RUSH1);
        StopBuffer(SOUND_CHAOS_ENVIR, true);
        PlayBuffer(SOUND_CHAOSCASTLE, NULL, true);
        break;
    case 6:
        SetMatchInfo(data->m_byPlayState + 1, 15 * 60, data->m_wRemainSec, data->m_wMaxKillMonster, data->m_wCurKillMonster);
        break;
    case 7:
        clearMatchInfo();
        StopBuffer(SOUND_CHAOSCASTLE, true);
        if (gMapManager.InChaosCastle() == true)
        {
            PlayBuffer(SOUND_CHAOS_ENVIR, NULL, true);
            PlayBuffer(SOUND_CHAOS_END);
        }
        break;
    case 8:
        if (m_byCurrCastleLevel == 0)
        {
            m_byCurrCastleLevel = 1;
            SetActionObject(gMapManager.WorldActive, 1, 40, 1);
            AddTerrainAttributeRange(23, 75, 22, 2, TW_NOGROUND, 1);
            AddTerrainAttributeRange(43, 77, 2, 32, TW_NOGROUND, 1);
            AddTerrainAttributeRange(23, 107, 20, 2, TW_NOGROUND, 1);
            AddTerrainAttributeRange(23, 77, 2, 30, TW_NOGROUND, 1);

            PlayBuffer(SOUND_CHAOS_FALLING_STONE);
        }
        break;
    case 9:
        if (m_byCurrCastleLevel == 3)
        {
            m_byCurrCastleLevel = 4;
            SetActionObject(gMapManager.WorldActive, 1, 40, 1);
            AddTerrainAttributeRange(25, 77, 18, 2, TW_NOGROUND, 1);
            AddTerrainAttributeRange(41, 79, 2, 28, TW_NOGROUND, 1);
            AddTerrainAttributeRange(25, 105, 16, 2, TW_NOGROUND, 1);
            AddTerrainAttributeRange(25, 79, 2, 26, TW_NOGROUND, 1);

            PlayBuffer(SOUND_CHAOS_FALLING_STONE);
        }
        break;
    case 10:
        if (m_byCurrCastleLevel == 6)
        {
            m_byCurrCastleLevel = 7;
            SetActionObject(gMapManager.WorldActive, 1, 40, 1);
            AddTerrainAttributeRange(27, 79, 14, 2, TW_NOGROUND, 1);
            AddTerrainAttributeRange(39, 81, 2, 24, TW_NOGROUND, 1);
            AddTerrainAttributeRange(27, 103, 12, 2, TW_NOGROUND, 1);
            AddTerrainAttributeRange(27, 81, 2, 22, TW_NOGROUND, 1);

            PlayBuffer(SOUND_CHAOS_FALLING_STONE);
        }
        break;
    }
}

void CNewChaosCastleSystem::RenderMatchTimes(void)
{
    if (m_byMatchType > 0 && gMapManager.InChaosCastle())
    {
        switch (m_byMatchType)
        {
        case 6:
        case 7:
        case 8:
            if (m_iMatchTime > 0)
            {
                if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSCASTLE_TIME))
                {
                    g_pNewUISystem->HideAll();
                    g_pNewUISystem->Show(SEASON3B::INTERFACE_CHAOSCASTLE_TIME);
                }

                g_pChaosCastleTime->SetTime(m_iMatchTime);
                g_pChaosCastleTime->SetKillMonsterStatue(m_iKillMonster, m_iMaxKillMonster);

                if (m_iKillMonster <= 46 && m_iKillMonster > 40)
                {
                    m_byCurrCastleLevel = 0;
                }
                else if (m_iKillMonster <= 36 && m_iKillMonster > 30)
                {
                    m_byCurrCastleLevel = 3;
                }
                else if (m_iKillMonster <= 26 && m_iKillMonster > 20)
                {
                    m_byCurrCastleLevel = 6;
                }
            }
            break;

        default:
            break;
        }
    }
    else
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSCASTLE_TIME))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHAOSCASTLE_TIME);
        }
    }
}

void CNewChaosCastleSystem::RenderMatchResult(void)
{
    int x = 640 / 2;
    int yPos = m_PosResult.y + 40;
    wchar_t lpszStr[256] = { NULL, };

    EnableAlphaTest();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(128, 255, 128, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    if (m_iNumResult)
    {
        g_pRenderText->RenderText(x, yPos, GlobalText[1151], 0, 0, RT3_WRITE_CENTER);
        yPos += 16;
        swprintf(lpszStr, L"%s %s", GlobalText[1152], GlobalText[858]);
        g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);
    }
    else
    {
        swprintf(lpszStr, L"%s %s", GlobalText[1152], GlobalText[860]);
        g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);
        yPos += 16;
        g_pRenderText->RenderText(x, yPos, GlobalText[1153], 0, 0, RT3_WRITE_CENTER);
    }
    yPos += 30;

    MatchResult* pResult = &m_MatchResult[0];

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(210, 255, 210, 255);

    swprintf(lpszStr, GlobalText[861], pResult->m_dwExp);
    g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); yPos += 20;

    swprintf(lpszStr, GlobalText[1162], pResult->m_iScore);
    g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); yPos += 20;

    swprintf(lpszStr, GlobalText[1163], pResult->m_iZen);
    g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); yPos += 24;

    DisableAlphaBlend();
}