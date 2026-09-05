// NewUIEmpireGuardianTimer.cpp: implementation of the CEmpireGuardianTimer class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UI/Core/NewUISystem.h"
#include "UI/NPCs/NewUIEmpireGuardianTimer.h"
#include "I18N/All.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CEmpireGuardianTimer::CEmpireGuardianTimer()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_dTime = 600000;
    m_iType = 1;
    m_iDay = EG_MONDAY;//EG_DAY_MAP_LIST::EG_MONDAY;
    m_iZone = 1;
    m_iMonsterCount = 0;
}

CEmpireGuardianTimer::~CEmpireGuardianTimer()
{
    Release();
}

bool CEmpireGuardianTimer::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_EMPIREGUARDIAN_TIMER, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void CEmpireGuardianTimer::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CEmpireGuardianTimer::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CEmpireGuardianTimer::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;
    return true;
}

bool CEmpireGuardianTimer::UpdateKeyEvent()
{
    return true;
}

bool CEmpireGuardianTimer::Update()
{
    if (!IsVisible())
        return true;

    return true;
}

bool CEmpireGuardianTimer::Render()
{
    EnableAlphaTest();

    RenderImage(IMAGE_EMPIREGUARDIAN_TIMER_WINDOW, m_Pos.x, m_Pos.y, float(TIMER_WINDOW_WIDTH), float(TIMER_WINDOW_HEIGHT));

    wchar_t szText[256] = {};
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    mu_swprintf(szText, I18N::Game::RoundDZoneD, m_iDay, m_iZone);
    g_pRenderText->RenderText(m_Pos.x + (TIMER_WINDOW_WIDTH / 2) - 55, m_Pos.y + 13, szText, 110, 0, RT3_SORT_CENTER);

    switch (m_iType)
    {
    case 0:
    case 1:
        g_pRenderText->SetTextColor(10, 200, 10, 255);
        g_pRenderText->RenderText(m_Pos.x + (TIMER_WINDOW_WIDTH / 2) - 55, m_Pos.y + 38, I18N::Game::StandbyTime, 110, 0, RT3_SORT_CENTER);
        break;
    case 2:
        g_pRenderText->SetTextColor(255, 150, 0, 255);
        mu_swprintf(szText, L"%ls (%ls)", I18N::Game::TimeLeft, I18N::Game::RemainingMonsters);
        g_pRenderText->RenderText(m_Pos.x + (TIMER_WINDOW_WIDTH / 2) - 55, m_Pos.y + 38, szText, 110, 0, RT3_SORT_CENTER);
        break;
    }

    int iSecond = m_dTime / 1000;
    int iMinute = iSecond / 60;

    if (2 < iMinute)
    {
        g_pRenderText->SetTextColor(255, 150, 0, 255);
    }
    else if (0 < iMinute && iMinute <= 2)
    {
        g_pRenderText->SetTextColor(255, 70, 0, 255);
    }
    else if (iMinute == 0)
    {
        g_pRenderText->SetTextColor(255, 0, 0, 255);
    }

    mu_swprintf(szText, L"%.2d:%.2d(%d)", iMinute, iSecond % 60, m_iMonsterCount);
    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->RenderText(m_Pos.x + (TIMER_WINDOW_WIDTH / 2) - 55, m_Pos.y + 50, szText, 110, 0, RT3_SORT_CENTER);

    DisableAlphaBlend();

    return true;
}

bool CEmpireGuardianTimer::BtnProcess()
{
    return false;
}

float CEmpireGuardianTimer::GetLayerDepth()
{
    return 1.2f;
}

void CEmpireGuardianTimer::OpenningProcess()
{
}

void CEmpireGuardianTimer::ClosingProcess()
{
}

void CEmpireGuardianTimer::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_blood.tga", IMAGE_EMPIREGUARDIAN_TIMER_WINDOW, GL_LINEAR);
}

void CEmpireGuardianTimer::UnloadImages()
{
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_TIMER_WINDOW);
}
