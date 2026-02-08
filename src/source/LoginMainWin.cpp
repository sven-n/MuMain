//*****************************************************************************
// File: LoginMainWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "LoginMainWin.h"

#include "Input.h"
#include "UIMng.h"
#include "WSclient.h"

//=============================================================================
// Global Variables
//=============================================================================


//=============================================================================
// Constructor / Destructor
//=============================================================================

CLoginMainWin::CLoginMainWin()
{
}

CLoginMainWin::~CLoginMainWin()
{
}

//=============================================================================
// Public Methods
//=============================================================================

void CLoginMainWin::Create()
{
    for (int i = 0; i <= LMW_BTN_CREDIT; ++i)
        m_aBtn[i].Create(54, 30, BITMAP_LOG_IN + 4 + i, 3, 2, 1);

    CWin::Create(
        CInput::Instance().GetScreenWidth() - 30 * 2,
        m_aBtn[0].GetHeight(),
        -2
    );

    for (int i = 0; i < LMW_BTN_MAX; ++i)
        CWin::RegisterButton(&m_aBtn[i]);

    m_sprDeco.Create(189, 103, BITMAP_LOG_IN + 6, 0, nullptr, 105, 59);
}

void CLoginMainWin::PreRelease()
{
    m_sprDeco.Release();
}

void CLoginMainWin::SetPosition(int nXCoord, int nYCoord)
{
    CWin::SetPosition(nXCoord, nYCoord);

    m_aBtn[LMW_BTN_MENU].SetPosition(nXCoord, nYCoord);

    m_aBtn[LMW_BTN_CREDIT].SetPosition(
        nXCoord + CWin::GetWidth() - m_aBtn[LMW_BTN_CREDIT].GetWidth(),
        nYCoord
    );

    m_sprDeco.SetPosition(
        m_aBtn[LMW_BTN_CREDIT].GetXPos(),
        m_aBtn[LMW_BTN_CREDIT].GetYPos()
    );
}

void CLoginMainWin::Show(bool bShow)
{
    CWin::Show(bShow);

    for (int i = 0; i < LMW_BTN_MAX; ++i)
        m_aBtn[i].Show(bShow);

    m_sprDeco.Show(bShow);
}

bool CLoginMainWin::CursorInWin(int nArea)
{
    if (!CWin::m_bShow)
        return false;

    switch (nArea)
    {
    case WA_MOVE:
        return false;
    }

    return CWin::CursorInWin(nArea);
}

void CLoginMainWin::UpdateWhileActive(double dDeltaTick)
{
    CUIMng& rUIMng = CUIMng::Instance();

    if (m_aBtn[LMW_BTN_MENU].IsClick())
    {
        rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
        rUIMng.SetSysMenuWinShow(true);
    }
    else if (m_aBtn[LMW_BTN_CREDIT].IsClick())
    {
        SocketClient->ToConnectServer()->SendServerListRequest();

        rUIMng.ShowWin(&rUIMng.m_CreditWin);

        ::StopMp3(MUSIC_MAIN_THEME);
        ::PlayMp3(MUSIC_MUTHEME);
    }
}

void CLoginMainWin::RenderControls()
{
    m_sprDeco.Render();
    CWin::RenderButtons();
}
