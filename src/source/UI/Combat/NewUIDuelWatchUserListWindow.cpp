// NewUIDuelWatchUserListWindow.cpp: implementation of the CDuelWatchUserListWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Combat/NewUIDuelWatchUserListWindow.h"
#include "UI/Core/NewUISystem.h"
#include "GameLogic/Combat/DuelMgr.h"

using namespace SEASON3B;
using namespace mu::ui::window;

// cppcheck-suppress uninitMemberVar
CDuelWatchUserListWindow::CDuelWatchUserListWindow() {}

CDuelWatchUserListWindow::~CDuelWatchUserListWindow()
{
    Release();
}

bool CDuelWatchUserListWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DUELWATCH_USERLIST, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void CDuelWatchUserListWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CDuelWatchUserListWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CDuelWatchUserListWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    POINT ptSize = {57, 17};
    POINT ptOrigin = {m_Pos.x, m_Pos.y - (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount()};

    if (CheckMouseIn(ptOrigin.x, ptOrigin.y, ptSize.x, (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount() + 10))
        return false;

    return true;
}

bool CDuelWatchUserListWindow::UpdateKeyEvent()
{
    return true;
}

bool CDuelWatchUserListWindow::Update()
{
    // 	if(IsVisible())
    // 	{
    // 	}
    return true;
}

bool CDuelWatchUserListWindow::Render()
{
    EnableAlphaTest();

    RenderFrame();

    g_pRenderText->SetFont(g_hFont);
    const float fFontHeight = static_cast<float>(g_pRenderText->MeasureText(L"Q", 1).cy);

    POINT ptSize = {57, 17};
    POINT ptOrigin = {
        static_cast<LONG>(m_Pos.x),
        static_cast<LONG>(m_Pos.y - (ptSize.y + 1) * static_cast<long>(g_DuelMgr.GetDuelWatchUserCount()) +
                          (ptSize.y - static_cast<long>(fFontHeight)) / 2 + 1)
    };

    for (int i = 0; i < g_DuelMgr.GetDuelWatchUserCount(); ++i)
    {
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, g_DuelMgr.GetDuelWatchUser(i), ptSize.x, 0, RT3_SORT_CENTER);
        ptOrigin.y += ptSize.y + 1;
    }

    DisableAlphaBlend();

    return true;
}

void CDuelWatchUserListWindow::OpeningProcess() {}

void CDuelWatchUserListWindow::ClosingProcess() {}

float CDuelWatchUserListWindow::GetLayerDepth()
{
    return 5.0f;
}

void CDuelWatchUserListWindow::LoadImages()
{
    LoadBitmap(L"Interface\\Pk_box.tga", IMAGE_DUELWATCH_USERLIST_BOX, GL_LINEAR);
}

void CDuelWatchUserListWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_DUELWATCH_USERLIST_BOX);
}

void CDuelWatchUserListWindow::RenderFrame()
{
    POINT ptSize = {57, 17};

    int i;
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

bool CDuelWatchUserListWindow::BtnProcess()
{
    return false;
}
