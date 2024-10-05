// NewUIDuelWatchMainFrameWindow.cpp: implementation of the CNewUIDuelWatchMainFrameWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIDuelWatchMainFrameWindow.h"
#include "NewUISystem.h"
#include "DuelMgr.h"


using namespace SEASON3B;

CNewUIDuelWatchMainFrameWindow::CNewUIDuelWatchMainFrameWindow()
{
    m_pNewUIMng = NULL;

    m_bHasHPReceived = FALSE;
    m_fPrevHPRate1 = 0;
    m_fPrevHPRate2 = 0;
    m_fPrevSDRate1 = 0;
    m_fPrevSDRate2 = 0;
    m_fLastHPRate1 = 0;
    m_fLastHPRate2 = 0;
    m_fLastSDRate1 = 0;
    m_fLastSDRate2 = 0;
    m_fReceivedHPRate1 = 0;
    m_fReceivedHPRate2 = 0;
    m_fReceivedSDRate1 = 0;
    m_fReceivedSDRate2 = 0;
}

CNewUIDuelWatchMainFrameWindow::~CNewUIDuelWatchMainFrameWindow()
{
    Release();
}

bool CNewUIDuelWatchMainFrameWindow::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_DUELWATCH_MAINFRAME, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, ITEMHOTKEYNUMBER_CAMERA_Z_ORDER);

    LoadImages();

    m_BtnExit.SetPos(640 - 36, 480 - 29);
    m_BtnExit.ChangeButtonImgState(true, IMAGE_INVENTORY_EXIT_BTN, false);
    m_BtnExit.ChangeButtonInfo(640 - 36, 480 - 29, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[2702], true);

    Show(false);

    return true;
}

void CNewUIDuelWatchMainFrameWindow::Release()
{
    UnloadImages();

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool CNewUIDuelWatchMainFrameWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;
    return true;
}

bool CNewUIDuelWatchMainFrameWindow::UpdateKeyEvent()
{
    return true;
}

bool CNewUIDuelWatchMainFrameWindow::Update()
{
    return true;
}

bool CNewUIDuelWatchMainFrameWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    m_BtnExit.Render();

    if (g_DuelMgr.GetCurrentChannel() == -1)
        return true;

    POINT ptOrigin = { 0, (long)(480.f - 51.f) };

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(ptOrigin.x + 320 - 80, ptOrigin.y + 36, g_DuelMgr.GetDuelPlayerID(DUEL_HERO), 55, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(ptOrigin.x + 320 + 25, ptOrigin.y + 36, g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY), 55, 0, RT3_SORT_CENTER);

    int i;
    for (i = 0; i < g_DuelMgr.GetScore(DUEL_HERO); ++i)
    {
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SCORE, 57 + 17 * i, 460, 16.f, 17.f);
    }
    for (i = 0; i < g_DuelMgr.GetScore(DUEL_ENEMY); ++i)
    {
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SCORE, 640 - 74 - 17 * i, 460, 16.f, 17.f);
    }

    if (m_bHasHPReceived == FALSE || g_DuelMgr.GetFighterRegenerated())
    {
        m_bHasHPReceived = TRUE;
        g_DuelMgr.SetFighterRegenerated(FALSE);
        m_fPrevHPRate1 = m_fLastHPRate1 = m_fReceivedHPRate1 = g_DuelMgr.GetHP(DUEL_HERO);
        m_fPrevHPRate2 = m_fLastHPRate2 = m_fReceivedHPRate2 = g_DuelMgr.GetHP(DUEL_ENEMY);
        m_fPrevSDRate1 = m_fLastSDRate1 = m_fReceivedSDRate1 = g_DuelMgr.GetSD(DUEL_HERO);
        m_fPrevSDRate2 = m_fLastSDRate2 = m_fReceivedSDRate2 = g_DuelMgr.GetSD(DUEL_ENEMY);
    }

    if (m_fLastHPRate1 != g_DuelMgr.GetHP(DUEL_HERO) || m_fLastHPRate2 != g_DuelMgr.GetHP(DUEL_ENEMY) || m_fLastSDRate1 != g_DuelMgr.GetSD(DUEL_HERO) || m_fLastSDRate2 != g_DuelMgr.GetSD(DUEL_ENEMY))
    {
        m_fLastHPRate1 = g_DuelMgr.GetHP(DUEL_HERO);
        m_fLastHPRate2 = g_DuelMgr.GetHP(DUEL_ENEMY);
        m_fLastSDRate1 = g_DuelMgr.GetSD(DUEL_HERO);
        m_fLastSDRate2 = g_DuelMgr.GetSD(DUEL_ENEMY);
        m_fReceivedHPRate1 = m_fPrevHPRate1;
        m_fReceivedHPRate2 = m_fPrevHPRate2;
        m_fReceivedSDRate1 = m_fPrevSDRate1;
        m_fReceivedSDRate2 = m_fPrevSDRate2;
    }

    int iDamageGap = int(absf(m_fReceivedHPRate1 - g_DuelMgr.GetHP(DUEL_HERO)) * 5.0f) + 2;
    float fHPRatePerPixel = 1.0f / 236.f * iDamageGap;

    float fHPRate = g_DuelMgr.GetHP(DUEL_HERO);
    if (m_fPrevHPRate1 > fHPRate + fHPRatePerPixel)
    {
        m_fPrevHPRate1 -= fHPRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE_FX, 60 + 236.f * (1.f - m_fPrevHPRate1), 440, 236.f * m_fPrevHPRate1, 7.f, 235.f / 256.f * m_fPrevHPRate1, 0, -235.f / 256.f * m_fPrevHPRate1, 6.f / 8.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE, 60 + 236.f * (1.f - fHPRate), 440, 236.f * fHPRate, 7.f, 235.f / 256.f * fHPRate, 0, -235.f / 256.f * fHPRate, 6.f / 8.f);
    }
    else if (m_fPrevHPRate1 < fHPRate - fHPRatePerPixel)
    {
        m_fPrevHPRate1 += fHPRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE_FX, 60 + 236.f * (1.f - fHPRate), 440, 236.f * fHPRate, 7.f, 235.f / 256.f * fHPRate, 0, -235.f / 256.f * fHPRate, 6.f / 8.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE, 60 + 236.f * (1.f - m_fPrevHPRate1), 440, 236.f * m_fPrevHPRate1, 7.f, 235.f / 256.f * m_fPrevHPRate1, 0, -235.f / 256.f * m_fPrevHPRate1, 6.f / 8.f);
    }
    else
    {
        m_fPrevHPRate1 = fHPRate;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE, 60 + 236.f * (1.f - fHPRate), 440, 236.f * fHPRate, 7.f, 235.f / 256.f * fHPRate, 0, -235.f / 256.f * fHPRate, 6.f / 8.f);
    }

    iDamageGap = int(absf(m_fReceivedHPRate2 - g_DuelMgr.GetHP(DUEL_ENEMY)) * 5.0f) + 2;
    fHPRatePerPixel = 1.0f / 236.f * iDamageGap;

    fHPRate = g_DuelMgr.GetHP(DUEL_ENEMY);
    if (m_fPrevHPRate2 > fHPRate + fHPRatePerPixel)
    {
        m_fPrevHPRate2 -= fHPRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE_FX, 580 - 236, 440, 236.f * m_fPrevHPRate2, 7.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE, 580 - 236, 440, 236.f * fHPRate, 7.f);
    }
    else if (m_fPrevHPRate2 < fHPRate - fHPRatePerPixel)
    {
        m_fPrevHPRate2 += fHPRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE_FX, 580 - 236, 440, 236.f * fHPRate, 7.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE, 580 - 236, 440, 236.f * m_fPrevHPRate2, 7.f);
    }
    else
    {
        m_fPrevHPRate2 = fHPRate;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE, 580 - 236, 440, 236.f * fHPRate, 7.f);
    }

    iDamageGap = int(absf(m_fReceivedSDRate1 - g_DuelMgr.GetSD(DUEL_HERO)) * 5.0f) + 2;
    float fSDRatePerPixel = 1.0f / 154.f * iDamageGap;

    float fSDRate = g_DuelMgr.GetSD(DUEL_HERO);
    if (m_fPrevSDRate1 > fSDRate + fSDRatePerPixel)
    {
        m_fPrevSDRate1 -= fSDRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE_FX, 142 + 154.f * (1.f - m_fPrevSDRate1), 450, 154.f * m_fPrevSDRate1, 4.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE, 142 + 154.f * (1.f - fSDRate), 450, 154.f * fSDRate, 4.f);
    }
    else if (m_fPrevSDRate1 < fSDRate - fSDRatePerPixel)
    {
        m_fPrevSDRate1 += fSDRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE_FX, 142 + 154.f * (1.f - fSDRate), 450, 154.f * fSDRate, 4.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE, 142 + 154.f * (1.f - m_fPrevSDRate1), 450, 154.f * m_fPrevSDRate1, 4.f);
    }
    else
    {
        m_fPrevSDRate1 = fSDRate;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE, 142 + 154.f * (1.f - fSDRate), 450, 154.f * fSDRate, 4.f);
    }

    iDamageGap = int(absf(m_fReceivedSDRate2 - g_DuelMgr.GetSD(DUEL_HERO)) * 5.0f) + 2;
    fSDRatePerPixel = 1.0f / 154.f * iDamageGap;

    fSDRate = g_DuelMgr.GetSD(DUEL_ENEMY);
    if (m_fPrevSDRate2 > fSDRate + fSDRatePerPixel)
    {
        m_fPrevSDRate2 -= fSDRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE_FX, 344, 450, 154.f * m_fPrevSDRate2, 4.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE, 344, 450, 154.f * fSDRate, 4.f);
    }
    else if (m_fPrevSDRate2 < fSDRate - fSDRatePerPixel)
    {
        m_fPrevSDRate2 += fSDRatePerPixel;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE_FX, 344, 450, 154.f * fSDRate, 4.f);
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE, 344, 450, 154.f * m_fPrevSDRate2, 4.f);
    }
    else
    {
        m_fPrevSDRate2 = fSDRate;
        RenderImage(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE, 344, 450, 154.f * fSDRate, 4.f);
    }

    DisableAlphaBlend();

    return true;
}

void CNewUIDuelWatchMainFrameWindow::Render3D()
{
}

bool CNewUIDuelWatchMainFrameWindow::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

void CNewUIDuelWatchMainFrameWindow::OpeningProcess()
{
    m_bHasHPReceived = FALSE;
}

void CNewUIDuelWatchMainFrameWindow::ClosingProcess()
{
    m_bHasHPReceived = FALSE;
}

float CNewUIDuelWatchMainFrameWindow::GetLayerDepth()
{
    return 5.0f;
}

void CNewUIDuelWatchMainFrameWindow::LoadImages()
{
    LoadBitmap(L"Interface\\menu_pk_01.jpg", IMAGE_DUELWATCH_MAINFRAME_BACK1, GL_LINEAR);
    LoadBitmap(L"Interface\\menu_pk_02.jpg", IMAGE_DUELWATCH_MAINFRAME_BACK2, GL_LINEAR);
    LoadBitmap(L"Interface\\menu_pk_03.jpg", IMAGE_DUELWATCH_MAINFRAME_BACK3, GL_LINEAR);
    LoadBitmap(L"Interface\\menu_pk_bt02.tga", IMAGE_DUELWATCH_MAINFRAME_SCORE, GL_LINEAR);
    LoadBitmap(L"Interface\\menu_pk_hp03(bar2).jpg", IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE, GL_LINEAR);
    LoadBitmap(L"Interface\\menu_pk_sd03(bar2).jpg", IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE, GL_LINEAR);
    LoadBitmap(L"Interface\\menu_pk_hp06(bar).jpg", IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE_FX, GL_LINEAR);
    LoadBitmap(L"Interface\\menu_pk_sd05(bar).jpg", IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE_FX, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_INVENTORY_EXIT_BTN, GL_LINEAR);
}

void CNewUIDuelWatchMainFrameWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_BACK1);
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_BACK2);
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_BACK3);
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_SCORE);
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE);
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE);
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE_FX);
    DeleteBitmap(IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE_FX);
    DeleteBitmap(IMAGE_INVENTORY_EXIT_BTN);
}

void CNewUIDuelWatchMainFrameWindow::RenderFrame()
{
    float width, height;
    float x, y;

    width = 256.f; height = 51.f;
    x = 0.f; y = 480.f - height;
    SEASON3B::RenderImage(IMAGE_DUELWATCH_MAINFRAME_BACK1, x, y, width, height);
    width = 128.f;
    x = 256.f;
    SEASON3B::RenderImage(IMAGE_DUELWATCH_MAINFRAME_BACK2, x, y, width, height);
    width = 256.f;
    x = 256.f + 128.f;
    SEASON3B::RenderImage(IMAGE_DUELWATCH_MAINFRAME_BACK3, x, y, width, height);
}

bool CNewUIDuelWatchMainFrameWindow::BtnProcess()
{
    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        if (g_DuelMgr.GetCurrentChannel() >= 0)
        {
            SocketClient->ToGameServer()->SendDuelChannelQuitRequest();//g_DuelMgr.GetCurrentChannel());
        }
        return true;
    }

    return false;
}