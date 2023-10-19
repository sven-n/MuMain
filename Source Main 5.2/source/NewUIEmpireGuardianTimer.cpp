// NewUIEmpireGuardianTimer.cpp: implementation of the CNewUIEmpireGuardianTimer class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUIEmpireGuardianTimer.h"

using namespace SEASON3B;

CNewUIEmpireGuardianTimer::CNewUIEmpireGuardianTimer()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_dTime = 600000;
    m_iType = 1;
    m_iDay = EG_MONDAY;//EG_DAY_MAP_LIST::EG_MONDAY;
    m_iZone = 1;
    m_iMonsterCount = 0;
}

CNewUIEmpireGuardianTimer::~CNewUIEmpireGuardianTimer()
{
    Release();
}

bool CNewUIEmpireGuardianTimer::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void CNewUIEmpireGuardianTimer::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIEmpireGuardianTimer::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIEmpireGuardianTimer::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;
    return true;
}

bool CNewUIEmpireGuardianTimer::UpdateKeyEvent()
{
    return true;
}

bool CNewUIEmpireGuardianTimer::Update()
{
    if (!IsVisible())
        return true;

    return true;
}

bool CNewUIEmpireGuardianTimer::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderImage(IMAGE_EMPIREGUARDIAN_TIMER_WINDOW, m_Pos.x, m_Pos.y, float(TIMER_WINDOW_WIDTH), float(TIMER_WINDOW_HEIGHT));

    wchar_t szText[256] = { NULL, };
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    swprintf(szText, GlobalText[2805], m_iDay, m_iZone);
    g_pRenderText->RenderText(m_Pos.x + (TIMER_WINDOW_WIDTH / 2) - 55, m_Pos.y + 13, szText, 110, 0, RT3_SORT_CENTER);

    switch (m_iType)
    {
    case 0:
    case 1:
        g_pRenderText->SetTextColor(10, 200, 10, 255);
        g_pRenderText->RenderText(m_Pos.x + (TIMER_WINDOW_WIDTH / 2) - 55, m_Pos.y + 38, GlobalText[2844], 110, 0, RT3_SORT_CENTER);
        break;
    case 2:
        g_pRenderText->SetTextColor(255, 150, 0, 255);
        swprintf(szText, L"%s (%s)", GlobalText[865], GlobalText[2845]);
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

    swprintf(szText, L"%.2d:%.2d(%d)", iMinute, iSecond % 60, m_iMonsterCount);
    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->RenderText(m_Pos.x + (TIMER_WINDOW_WIDTH / 2) - 55, m_Pos.y + 50, szText, 110, 0, RT3_SORT_CENTER);

    DisableAlphaBlend();

    return true;
}

bool CNewUIEmpireGuardianTimer::BtnProcess()
{
    return false;
}

float CNewUIEmpireGuardianTimer::GetLayerDepth()
{
    return 1.2f;
}

void CNewUIEmpireGuardianTimer::OpenningProcess()
{
}

void CNewUIEmpireGuardianTimer::ClosingProcess()
{
}

void CNewUIEmpireGuardianTimer::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_blood.tga", IMAGE_EMPIREGUARDIAN_TIMER_WINDOW, GL_LINEAR);
}

void CNewUIEmpireGuardianTimer::UnloadImages()
{
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_TIMER_WINDOW);
}