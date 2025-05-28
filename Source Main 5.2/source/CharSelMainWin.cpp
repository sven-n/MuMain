//*****************************************************************************
// File: CharSelMainWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharSelMainWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "UIGuildInfo.h"
#include "ZzzOpenData.h"
#include "ZzzOpenglUtil.h"
#include "ServerListManager.h"




CCharSelMainWin::CCharSelMainWin()
{
}

CCharSelMainWin::~CCharSelMainWin()
{
}

void CCharSelMainWin::Create()
{
    m_asprBack[CSMW_SPR_DECO].Create(189, 103, BITMAP_LOG_IN + 2);
    m_asprBack[CSMW_SPR_INFO].Create(
        CInput::Instance().GetScreenWidth() - 266, 21);
    m_asprBack[CSMW_SPR_INFO].SetColor(0, 0, 0);
    m_asprBack[CSMW_SPR_INFO].SetAlpha(143);

    m_aBtn[CSMW_BTN_CREATE].Create(54, 30, BITMAP_LOG_IN + 3, 4, 2, 1, 3);
    m_aBtn[CSMW_BTN_MENU].Create(54, 30, BITMAP_LOG_IN + 4, 3, 2, 1);
    m_aBtn[CSMW_BTN_CONNECT].Create(54, 30, BITMAP_LOG_IN + 5, 4, 2, 1, 3);
    m_aBtn[CSMW_BTN_DELETE].Create(54, 30, BITMAP_LOG_IN + 6, 4, 2, 1, 3);

    CWin::Create(
        m_aBtn[0].GetWidth() * CSMW_BTN_MAX + m_asprBack[CSMW_SPR_INFO].GetWidth() + 6,
        m_aBtn[0].GetHeight(), -2);

    for (int i = 0; i < CSMW_BTN_MAX; ++i)
        CWin::RegisterButton(&m_aBtn[i]);

    m_bAccountBlockItem = false;

    for (int i = 0; i < 5; ++i)
    {
        if (CharactersClient[i].Object.Live)
        {
            if (CharactersClient[i].CtlCode & CTLCODE_10ACCOUNT_BLOCKITEM)
            {
                m_bAccountBlockItem = true;
                break;
            }
        }
    }
}

void CCharSelMainWin::PreRelease()
{
    for (int i = 0; i < CSMW_SPR_MAX; ++i)
        m_asprBack[i].Release();
}

void CCharSelMainWin::SetPosition(int nXCoord, int nYCoord)
{
    CWin::SetPosition(nXCoord, nYCoord);

    int nBtnWidth = m_aBtn[0].GetWidth();

    m_aBtn[CSMW_BTN_CREATE].SetPosition(nXCoord, nYCoord);
    m_aBtn[CSMW_BTN_MENU].SetPosition(nXCoord + nBtnWidth + 1, nYCoord);
    m_asprBack[CSMW_SPR_INFO].SetPosition(m_aBtn[CSMW_BTN_MENU].GetXPos() + nBtnWidth + 2, nYCoord + 5);

    int nWinRPosX = nXCoord + CWin::GetWidth();
    m_asprBack[CSMW_SPR_DECO].SetPosition(nWinRPosX - (m_asprBack[CSMW_SPR_DECO].GetWidth() - 22), nYCoord - 59);
    m_aBtn[CSMW_BTN_DELETE].SetPosition(nWinRPosX - nBtnWidth, nYCoord);
    m_aBtn[CSMW_BTN_CONNECT].SetPosition(nWinRPosX - (nBtnWidth * 2 + 1), nYCoord);
}

void CCharSelMainWin::Show(bool bShow)
{
    CWin::Show(bShow);

    int i;
    for (i = 0; i < CSMW_SPR_MAX; ++i)
        m_asprBack[i].Show(bShow);
    for (i = 0; i < CSMW_BTN_MAX; ++i)
        m_aBtn[i].Show(bShow);
}

bool CCharSelMainWin::CursorInWin(int nArea)
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

void CCharSelMainWin::UpdateDisplay()
{
    m_aBtn[CSMW_BTN_CREATE].SetEnable(false);
    int i = 0;

    for (i = 0; i < 5; ++i)
    {
        if (!CharactersClient[i].Object.Live)
        {
            m_aBtn[CSMW_BTN_CREATE].SetEnable(true);
            break;
        }
    }

    if (SelectedHero > -1)
    {
        m_aBtn[CSMW_BTN_CONNECT].SetEnable(true);
        m_aBtn[CSMW_BTN_DELETE].SetEnable(true);
    }
    else
    {
        m_aBtn[CSMW_BTN_CONNECT].SetEnable(false);
        m_aBtn[CSMW_BTN_DELETE].SetEnable(false);
    }

    bool bNobodyCharacter = true;

    for (int i = 0; i < 5; ++i)
    {
        if (CharactersClient[i].Object.Live == true)
        {
            bNobodyCharacter = false;
            break;
        }
    }

    if (bNobodyCharacter == true)
    {
        CUIMng& rUIMng = CUIMng::Instance();
        rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
    }
}

void CCharSelMainWin::UpdateWhileActive(double dDeltaTick)
{
    if (m_aBtn[CSMW_BTN_CONNECT].IsClick())
        ::StartGame();
    else if (m_aBtn[CSMW_BTN_MENU].IsClick())
    {
        CUIMng& rUIMng = CUIMng::Instance();
        rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
        rUIMng.SetSysMenuWinShow(true);
    }
    else if (m_aBtn[CSMW_BTN_CREATE].IsClick())
    {
        CUIMng& rUIMng = CUIMng::Instance();
        rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
    }
    else if (m_aBtn[CSMW_BTN_DELETE].IsClick())
        DeleteCharacter();
}

void CCharSelMainWin::RenderControls()
{
    for (int i = 0; i < CSMW_SPR_MAX; ++i)
        m_asprBack[i].Render();

    ::EnableAlphaTest();
    ::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    if (m_bAccountBlockItem)
    {
        g_pRenderText->SetTextColor(0, 0, 0, 255);
        g_pRenderText->SetBgColor(255, 255, 0, 128);
        g_pRenderText->RenderText(320, 330, GlobalText[436], 0, 0, RT3_WRITE_CENTER);
        g_pRenderText->RenderText(320, 348, GlobalText[437], 0, 0, RT3_WRITE_CENTER);
    }
    CWin::RenderButtons();
}

void CCharSelMainWin::DeleteCharacter()
{
    if (CharactersClient[SelectedHero].GuildStatus != G_NONE)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_GUILDWARNING);
    else if (CharactersClient[SelectedHero].CtlCode & (CTLCODE_02BLOCKITEM | CTLCODE_10ACCOUNT_BLOCKITEM))
        CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_ID_BLOCK);
    else
        CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_CONFIRM);
}