// NewUIDuelWatchUserListWindow.cpp: implementation of the CNewUIDuelWatchUserListWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIDuelWatchUserListWindow.h"
#include "NewUISystem.h"
#include "DuelMgr.h"

using namespace SEASON3B;

CNewUIDuelWatchUserListWindow::CNewUIDuelWatchUserListWindow()
{
}

CNewUIDuelWatchUserListWindow::~CNewUIDuelWatchUserListWindow()
{
    Release();
}

bool CNewUIDuelWatchUserListWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_DUELWATCH_USERLIST, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void CNewUIDuelWatchUserListWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIDuelWatchUserListWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIDuelWatchUserListWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    POINT ptSize = { 57, 17 };
    POINT ptOrigin = { m_Pos.x, m_Pos.y - (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount() };

    if (CheckMouseIn(ptOrigin.x, ptOrigin.y, ptSize.x, (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount() + 10))
        return false;

    return true;
}

bool CNewUIDuelWatchUserListWindow::UpdateKeyEvent()
{
    return true;
}

bool CNewUIDuelWatchUserListWindow::Update()
{
    // 	if(IsVisible())
    // 	{
    // 	}
    return true;
}

bool CNewUIDuelWatchUserListWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    

    SIZE TextSize;

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), L"Q", 1, &TextSize);
    float fFontHeight = TextSize.cy / g_fScreenRate_y;

    POINT ptSize = { 57, 17 };
    POINT ptOrigin = { m_Pos.x, m_Pos.y - (ptSize.y + 1) * (long)g_DuelMgr.GetDuelWatchUserCount() + (ptSize.y - (long)fFontHeight) / 2 + 1 };

    for (int i = 0; i < g_DuelMgr.GetDuelWatchUserCount(); ++i)
    {
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, g_DuelMgr.GetDuelWatchUser(i), ptSize.x, 0, RT3_SORT_CENTER);
        ptOrigin.y += ptSize.y + 1;
    }

    DisableAlphaBlend();

    return true;
}

void CNewUIDuelWatchUserListWindow::OpeningProcess()
{
}

void CNewUIDuelWatchUserListWindow::ClosingProcess()
{
}

float CNewUIDuelWatchUserListWindow::GetLayerDepth()
{
    return 5.0f;
}

void CNewUIDuelWatchUserListWindow::LoadImages()
{
    LoadBitmap(L"Interface\\Pk_box.tga", IMAGE_DUELWATCH_USERLIST_BOX, GL_LINEAR);
}

void CNewUIDuelWatchUserListWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_DUELWATCH_USERLIST_BOX);
}

void CNewUIDuelWatchUserListWindow::RenderFrame()
{
    POINT ptSize = { 57, 17 };

    int i;
    glColor4f(0.f, 0.f, 0.f, 0.8f);
    for (i = 0; i < g_DuelMgr.GetDuelWatchUserCount(); ++i)
    {
        RenderColor(m_Pos.x, m_Pos.y - (ptSize.y + 1) * (i + 1), ptSize.x, ptSize.y);
    }
    EndRenderColor();

    for (i = 0; i < g_DuelMgr.GetDuelWatchUserCount(); ++i)
    {
        RenderImage(IMAGE_DUELWATCH_USERLIST_BOX, m_Pos.x, m_Pos.y - (ptSize.y + 1) * (i + 1), ptSize.x, ptSize.y);
    }
}

bool CNewUIDuelWatchUserListWindow::BtnProcess()
{
    return false;
}