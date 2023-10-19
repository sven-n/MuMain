// NewUIBloodCastleTime.cpp: implementation of the CNewUIPartyInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapManager.h"
#include "NewUIBloodCastleTime.h"
#include "NewUISystem.h"
#include "MatchEvent.h"

using namespace SEASON3B;
using namespace matchEvent;

CNewUIBloodCastle::CNewUIBloodCastle()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iTime = 0;
    m_iTimeState = BC_TIME_STATE_NORMAL;
    m_iMaxKillMonster = MAX_KILL_MONSTER;
    m_iKilledMonster = 0;
}

CNewUIBloodCastle::~CNewUIBloodCastle()
{
    Release();
}

bool CNewUIBloodCastle::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_BLOODCASTLE_TIME, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void CNewUIBloodCastle::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIBloodCastle::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIBloodCastle::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, BLOODCASTLE_TIME_WINDOW_WIDTH, BLOODCASTLE_TIME_WINDOW_HEIGHT))
        return false;

    return true;
}

bool CNewUIBloodCastle::UpdateKeyEvent()
{
    return true;
}

bool CNewUIBloodCastle::Update()
{
    if (!IsVisible())
        return true;

    if ((g_csMatchInfo == NULL) || (gMapManager.InBloodCastle() == false))
    {
        Show(false);
    }

    return true;
}

bool CNewUIBloodCastle::Render()
{
    if (g_csMatchInfo == NULL)
    {
        Show(false);
        return true;
    }

    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    wchar_t szText[256] = { NULL, };

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 150, 0, 255);

    RenderImage(IMAGE_BLOODCASTLE_TIME_WINDOW, m_Pos.x, m_Pos.y,
        float(BLOODCASTLE_TIME_WINDOW_WIDTH), float(BLOODCASTLE_TIME_WINDOW_HEIGHT));

    if (m_iMaxKillMonster != MAX_KILL_MONSTER)
    {
        if (g_csMatchInfo->GetMatchType() == 5)
        {
            swprintf(szText, GlobalText[866], m_iKilledMonster, m_iMaxKillMonster);
        }
        else
        {
            swprintf(szText, GlobalText[864], m_iKilledMonster, m_iMaxKillMonster);
        }
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, szText, BLOODCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
    }

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 38, GlobalText[865], BLOODCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    if (m_iTimeState == BC_TIME_STATE_IMMINENCE)
        g_pRenderText->SetTextColor(255, 32, 32, 255);

    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 50, m_szTime, BLOODCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    DisableAlphaBlend();

    return true;
}

bool CNewUIBloodCastle::BtnProcess()
{
    return false;
}

float CNewUIBloodCastle::GetLayerDepth()
{
    return 1.2f;
}

void CNewUIBloodCastle::OpenningProcess()
{
}

void CNewUIBloodCastle::ClosingProcess()
{
}

void CNewUIBloodCastle::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_blood.tga", IMAGE_BLOODCASTLE_TIME_WINDOW, GL_LINEAR);
}

void CNewUIBloodCastle::UnloadImages()
{
    DeleteBitmap(IMAGE_BLOODCASTLE_TIME_WINDOW);
}

void CNewUIBloodCastle::SetTime(int iTime)
{
    m_iTime = iTime;

    int iMinute = m_iTime / 60;
    swprintf(m_szTime, L" %.2d:%.2d:%.2d", iMinute, m_iTime % 60, (int)WorldTime % 60);

    if (iMinute < 5)
    {
        m_iTimeState = BC_TIME_STATE_IMMINENCE;
    }
    else
    {
        m_iTimeState = BC_TIME_STATE_NORMAL;
    }
}

void CNewUIBloodCastle::SetKillMonsterStatue(int iKilled, int iMaxKill)
{
    m_iKilledMonster = iKilled;
    m_iMaxKillMonster = iMaxKill;
}