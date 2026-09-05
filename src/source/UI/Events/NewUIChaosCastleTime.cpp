// NewUIChaosCastleTime.cpp: implementation of the CChaosCastleTime class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "World/MapInfra/MapManager.h"
#include "UI/Events/NewUIChaosCastleTime.h"
#include "UI/Core/NewUISystem.h"
#include "GameLogic/Events/MatchEvent.h"
#include "I18N/All.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CChaosCastleTime::CChaosCastleTime()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iTime = 0;
    m_iTimeState = CC_TIME_STATE_NORMAL;
    m_iMaxKillMonster = MAX_KILL_MONSTER;
    m_iKilledMonster = 0;
}

CChaosCastleTime::~CChaosCastleTime()
{
    Release();
}

bool CChaosCastleTime::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CHAOSCASTLE_TIME, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void CChaosCastleTime::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CChaosCastleTime::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CChaosCastleTime::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, CHAOSCASTLE_TIME_WINDOW_WIDTH, CHAOSCASTLE_TIME_WINDOW_HEIGHT))
        return false;

    return true;
}

bool CChaosCastleTime::UpdateKeyEvent()
{
    return true;
}

bool CChaosCastleTime::Update()
{
    if (!IsVisible())
        return true;

    if (gMapManager.InChaosCastle() == false)
    {
        Show(false);
    }

    return true;
}

bool CChaosCastleTime::Render()
{
    EnableAlphaTest();

    wchar_t szText[256] = {};

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 150, 0, 255);

    RenderImage(IMAGE_CHAOSCASTLE_TIME_WINDOW, m_Pos.x, m_Pos.y,
        float(CHAOSCASTLE_TIME_WINDOW_WIDTH), float(CHAOSCASTLE_TIME_WINDOW_HEIGHT));

    if (m_iMaxKillMonster != MAX_KILL_MONSTER)
    {
        mu_swprintf(szText, I18N::Game::CharacterDD, m_iKilledMonster, m_iMaxKillMonster);
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, szText, CHAOSCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
    }

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 38, I18N::Game::TimeLeft, CHAOSCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    if (m_iTimeState == CC_TIME_STATE_IMMINENCE)
        g_pRenderText->SetTextColor(255, 32, 32, 255);

    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 50, m_szTime, CHAOSCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    DisableAlphaBlend();

    return true;
}

bool CChaosCastleTime::BtnProcess()
{
    return false;
}

float CChaosCastleTime::GetLayerDepth()
{
    return 1.3f;
}

void CChaosCastleTime::OpenningProcess()
{
}

void CChaosCastleTime::ClosingProcess()
{
}

void CChaosCastleTime::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_blood.tga", IMAGE_CHAOSCASTLE_TIME_WINDOW, GL_LINEAR);
}

void CChaosCastleTime::UnloadImages()
{
    DeleteBitmap(IMAGE_CHAOSCASTLE_TIME_WINDOW);
}

void CChaosCastleTime::SetTime(int iTime)
{
    m_iTime = iTime;

    int iMinute = m_iTime / 60;
    mu_swprintf(m_szTime, L" %.2d:%.2d:%.2d", iMinute, m_iTime % 60, (int)WorldTime % 60);

    if (iMinute < 5)
    {
        m_iTimeState = CC_TIME_STATE_IMMINENCE;
    }
    else
    {
        m_iTimeState = CC_TIME_STATE_NORMAL;
    }
}

void CChaosCastleTime::SetKillMonsterStatue(int iKilled, int iMaxKill)
{
    m_iKilledMonster = iKilled;
    m_iMaxKillMonster = iMaxKill;
}
