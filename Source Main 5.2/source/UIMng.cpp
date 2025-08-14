//*****************************************************************************
// File: UIMng.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UIMng.h"
#include "Input.h"
#include "Sprite.h"
#include "GaugeBar.h"
#include "ZzzOpenglUtil.h"
#include "Zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"

#include "UIControls.h"
#include "ServerListManager.h"

#define	DOCK_EXTENT		10

//#define	UIM_TS_BG_BLACK		0
#define	UIM_TS_BACK0		0
#define	UIM_TS_BACK1		1
#define	UIM_TS_121518		3
#define UIM_TS_BACK2		5
#define UIM_TS_BACK3		6
#define UIM_TS_BACK4		7
#define UIM_TS_BACK5		8
#define UIM_TS_BACK6		9
#define UIM_TS_BACK7		10
#define UIM_TS_BACK8		11
#define UIM_TS_BACK9		12
#define	UIM_TS_MAX			13

CUIMng::CUIMng()
{
    m_asprTitle = NULL;
    m_pgbLoding = NULL;
    m_pLoadingScene = NULL;
}

CUIMng::~CUIMng()
{
}

CUIMng& CUIMng::Instance()
{
    static CUIMng s_UIMng;
    return s_UIMng;
}

void CUIMng::CreateTitleSceneUI()
{
    ReleaseTitleSceneUI();

    CInput& rInput = CInput::Instance();
    float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
    float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

    m_asprTitle = new CSprite[UIM_TS_MAX];

    float _fScaleXTemp = (float)rInput.GetScreenWidth() / 1280.0f;
    float _fScaleYTemp = (float)rInput.GetScreenHeight() / 1024.0f;

    m_asprTitle[UIM_TS_BACK0].Create(400, 69, BITMAP_TITLE, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
    m_asprTitle[UIM_TS_BACK0].SetPosition(0, 0);

    m_asprTitle[UIM_TS_BACK1].Create(400, 69, BITMAP_TITLE + 1, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
    m_asprTitle[UIM_TS_BACK1].SetPosition(400, 0);

    m_asprTitle[UIM_TS_BACK2].Create(400, 100, BITMAP_TITLE + 6, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
    m_asprTitle[UIM_TS_BACK2].SetPosition(0, 500);

    m_asprTitle[UIM_TS_BACK3].Create(400, 100, BITMAP_TITLE + 7, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
    m_asprTitle[UIM_TS_BACK3].SetPosition(400, 500);

    m_asprTitle[UIM_TS_BACK4].Create(512, 512, BITMAP_TITLE + 8, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK4].SetPosition(0, 119);

    m_asprTitle[UIM_TS_BACK5].Create(512, 512, BITMAP_TITLE + 9, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK5].SetPosition(512, 119);

    m_asprTitle[UIM_TS_BACK6].Create(256, 512, BITMAP_TITLE + 10, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK6].SetPosition(1024, 119);

    m_asprTitle[UIM_TS_BACK7].Create(512, 223, BITMAP_TITLE + 11, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK7].SetPosition(0, 512 + 119);

    m_asprTitle[UIM_TS_BACK8].Create(512, 223, BITMAP_TITLE + 12, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK8].SetPosition(512, 512 + 119);

    m_asprTitle[UIM_TS_BACK9].Create(256, 223, BITMAP_TITLE + 13, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK9].SetPosition(1024, 512 + 119);

  
    m_asprTitle[UIM_TS_121518].Create(256, 206, BITMAP_TITLE + 3, 0, NULL, 0, 0,
        false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
    m_asprTitle[UIM_TS_121518].SetPosition(544, 60);

    m_pgbLoding = new CGaugeBar;

    RECT rc = { 0, 0, 656, 15 };
    m_pgbLoding->Create(4, 15, BITMAP_TITLE + 5, &rc, 0, 0, -1, true, fScaleX, fScaleY);

    m_pgbLoding->SetPosition(72, 540);
    for (int i = 0; i < UIM_TS_MAX; ++i)
    {
        m_asprTitle[i].Show();
    }
    m_pgbLoding->Show();
    m_asprTitle[UIM_TS_121518].Show(false);
    m_nScene = UIM_SCENE_TITLE;
}

void CUIMng::ReleaseTitleSceneUI()
{
    SAFE_DELETE_ARRAY(m_asprTitle);
    SAFE_DELETE(m_pgbLoding);

    m_nScene = UIM_SCENE_NONE;
}

void CUIMng::RenderTitleSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal)
{
    ::BeginOpengl();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ::BeginBitmap();

    for (int i = 0; i < UIM_TS_MAX; ++i)
    {
        if (i == 2)
            continue;
        m_asprTitle[i].Render();
    }

    m_pgbLoding->SetValue(dwNow, dwTotal);
    m_pgbLoding->Render();

    ::EndBitmap();
    ::EndOpengl();
    ::glFlush();
    ::SwapBuffers(hDC);
}

void CUIMng::Create()
{
    m_bCursorOnUI = false;
    m_bBlockCharMove = false;
    m_bWinActive = false;
    m_nScene = UIM_SCENE_NONE;

    return;
}

void CUIMng::RemoveWinList()
{
    CWin* pWin;
    while (m_WinList.GetCount())
    {
        pWin = (CWin*)m_WinList.RemoveHead();
        pWin->Release();
    }
}

void CUIMng::Release()
{
    RemoveWinList();

    m_CharInfoBalloonMng.Release();

    m_nScene = UIM_SCENE_NONE;
}

void CUIMng::CreateLoginScene()
{
    RemoveWinList();

    m_CharInfoBalloonMng.Release();

    CInput& rInput = CInput::Instance();

    m_MsgWin.Create();
    m_WinList.AddHead(&m_MsgWin);
    m_MsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2,
        (rInput.GetScreenHeight() - 113) / 2);

    m_SysMenuWin.Create();
    m_WinList.AddHead(&m_SysMenuWin);

    m_OptionWin.Create();
    m_WinList.AddHead(&m_OptionWin);

    m_LoginMainWin.Create();
    m_WinList.AddHead(&m_LoginMainWin);

    int nBaseY = int(567.0f / 600.0f * (float)rInput.GetScreenHeight());
    m_LoginMainWin.SetPosition(30, nBaseY - m_LoginMainWin.GetHeight() - 11);

    m_ServerSelWin.Create();
    m_WinList.AddHead(&m_ServerSelWin);
    m_ServerSelWin.SetPosition(
        (rInput.GetScreenWidth() - m_ServerSelWin.GetWidth()) / 2,
        (rInput.GetScreenHeight() - m_ServerSelWin.GetHeight()) / 2);

    m_LoginWin.Create();
    m_WinList.AddHead(&m_LoginWin);
    m_LoginWin.SetPosition(
        (rInput.GetScreenWidth() - m_LoginWin.GetWidth()) / 2,
        (rInput.GetScreenHeight() - m_LoginWin.GetHeight()) * 2 / 3);

    m_CreditWin.Create();
    m_WinList.AddHead(&m_CreditWin);

    m_bSysMenuWinShow = false;
    m_nScene = UIM_SCENE_LOGIN;
}

void CUIMng::CreateCharacterScene()
{
    RemoveWinList();

    m_CharInfoBalloonMng.Create();

    CInput& rInput = CInput::Instance();

    m_MsgWin.Create();
    m_WinList.AddHead(&m_MsgWin);
    m_MsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2,
        (rInput.GetScreenHeight() - 113) / 2);

    m_ServerMsgWin.Create();
    m_WinList.AddHead(&m_ServerMsgWin);
    int nBaseY = int(31.0f / 600.0f * (float)rInput.GetScreenHeight());
    m_ServerMsgWin.SetPosition(10, nBaseY + 10);

    m_SysMenuWin.Create();
    m_WinList.AddHead(&m_SysMenuWin);

    m_OptionWin.Create();
    m_WinList.AddHead(&m_OptionWin);

    m_CharSelMainWin.Create();
    m_WinList.AddHead(&m_CharSelMainWin);
    nBaseY = int(567.0f / 600.0f * (float)rInput.GetScreenHeight());
    m_CharSelMainWin.SetPosition(22, nBaseY - m_CharSelMainWin.GetHeight() - 11);

    m_CharMakeWin.Create();
    m_WinList.AddHead(&m_CharMakeWin);

    m_CharMakeWin.SetPosition((rInput.GetScreenWidth() - 454) / 2,
        (rInput.GetScreenHeight() - 406) / 2);

    m_CharSelMainWin.UpdateDisplay();
    m_CharInfoBalloonMng.UpdateDisplay();

    ShowWin(&m_CharSelMainWin);

    m_bSysMenuWinShow = false;
    m_nScene = UIM_SCENE_CHARACTER;
}

void CUIMng::CreateMainScene()
{
    RemoveWinList();

    m_CharInfoBalloonMng.Release();

    m_nScene = UIM_SCENE_MAIN;
}

CWin* CUIMng::SetActiveWin(CWin* pWin)
{
    CWin* pBeforeActWin = (CWin*)m_WinList.GetHead();

    if (pBeforeActWin == NULL)
        return NULL;

    if (pBeforeActWin->IsActive())
        pBeforeActWin->Active(FALSE);
    else
        pBeforeActWin = NULL;

    if (pWin->IsShow())
    {
        if (!m_WinList.RemoveAt(m_WinList.Find(pWin)))
            return NULL;

        m_bWinActive = true;
        m_WinList.AddHead(pWin);
    }

    return pBeforeActWin;
}

void CUIMng::ShowWin(CWin* pWin)
{
    pWin->Show(TRUE);
    SetActiveWin(pWin);
}

void CUIMng::HideWin(CWin* pWin)
{
    if (!m_WinList.RemoveAt(m_WinList.Find(pWin)))
        return;

    pWin->Show(FALSE);
    pWin->Active(FALSE);
    m_WinList.AddTail(pWin);

    pWin = (CWin*)m_WinList.GetHead();
    if (pWin->IsShow())
        m_bWinActive = true;
}

void CUIMng::CheckDockWin()
{
    NODE* position = m_WinList.GetHeadPosition();
    if (NULL == position)
        return;

    CWin* pMovWin = (CWin*)m_WinList.GetNext(position);

    if (pMovWin->GetState() != WS_MOVE)
        return;

    pMovWin->SetDocking(false);

    RECT rcMovWin = { pMovWin->GetTempXPos(), pMovWin->GetTempYPos(),
        pMovWin->GetTempXPos() + pMovWin->GetWidth(),
        pMovWin->GetTempYPos() + pMovWin->GetHeight() };

    RECT rcDock[4] =
    {
        { rcMovWin.left - DOCK_EXTENT, rcMovWin.top - DOCK_EXTENT,
            rcMovWin.left + DOCK_EXTENT, rcMovWin.top + DOCK_EXTENT },
        { rcMovWin.right - DOCK_EXTENT, rcMovWin.top - DOCK_EXTENT,
            rcMovWin.right + DOCK_EXTENT, rcMovWin.top + DOCK_EXTENT },
        { rcMovWin.left - DOCK_EXTENT, rcMovWin.bottom - DOCK_EXTENT,
            rcMovWin.left + DOCK_EXTENT, rcMovWin.bottom + DOCK_EXTENT },
        { rcMovWin.right - DOCK_EXTENT, rcMovWin.bottom - DOCK_EXTENT,
            rcMovWin.right + DOCK_EXTENT, rcMovWin.bottom + DOCK_EXTENT }
    };

    CInput& rInput = CInput::Instance();

    POINT pt[4] = { { 0, 0 }, { rInput.GetScreenWidth(), 0 },
        { 0, rInput.GetScreenHeight() },
        { rInput.GetScreenWidth(), rInput.GetScreenHeight() } };

    if (::PtInRect(&rcDock[0], pt[0]))
    {
        pMovWin->SetPosition(pt[0].x, pt[0].y);
        pMovWin->SetDocking(true);
    }
    else if (::PtInRect(&rcDock[1], pt[1]))
    {
        pMovWin->SetPosition(pt[1].x - pMovWin->GetWidth(), pt[1].y);
        pMovWin->SetDocking(true);
    }
    else if (::PtInRect(&rcDock[2], pt[2]))
    {
        pMovWin->SetPosition(pt[2].x, pt[2].y - pMovWin->GetHeight());
        pMovWin->SetDocking(true);
    }
    else if (::PtInRect(&rcDock[3], pt[3]))
    {
        pMovWin->SetPosition(pt[3].x - pMovWin->GetWidth(),
            pt[3].y - pMovWin->GetHeight());
        pMovWin->SetDocking(true);
    }
    else if (rcDock[0].top < 0 && rcDock[0].bottom > 0)
    {
        pMovWin->SetPosition(rcMovWin.left, 0);
        pMovWin->SetDocking(true);
    }
    else if (rcDock[2].top < pt[2].y && rcDock[2].bottom > pt[2].y)
    {
        pMovWin->SetPosition(rcMovWin.left, pt[2].y - pMovWin->GetHeight());
        pMovWin->SetDocking(true);
    }
    else if (rcDock[0].left < 0 && rcDock[0].right > 0)
    {
        pMovWin->SetPosition(0, rcMovWin.top);
        pMovWin->SetDocking(true);
    }
    else if (rcDock[1].left < pt[1].x && rcDock[1].right > pt[1].x)
    {
        pMovWin->SetPosition(pt[1].x - pMovWin->GetWidth(), rcMovWin.top);
        pMovWin->SetDocking(true);
    }

    BOOL bEdgeDocking = FALSE;
    int i, j, nXCoord, nYCoord;
    CWin* pWin;

    while (position)
    {
        pWin = (CWin*)m_WinList.GetNext(position);
        if (!pWin->IsShow())
            continue;

        pt[0].x = pWin->GetXPos();
        pt[0].y = pWin->GetYPos();
        pt[1].x = pWin->GetXPos() + pWin->GetWidth();
        pt[1].y = pt[0].y;
        pt[2].x = pt[0].x;
        pt[2].y = pWin->GetYPos() + pWin->GetHeight();
        pt[3].x = pt[1].x;
        pt[3].y = pt[2].y;

        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 4; j++)
            {
                if (i != j && ::PtInRect(&rcDock[i], pt[j]))
                {
                    bEdgeDocking = TRUE;
                    goto DOCKING;
                }
            }
        }

        if (pt[0].x < rcDock[1].left && pt[1].x > rcDock[0].right)
        {
            nXCoord = rcMovWin.left;
            if (pt[2].y > rcDock[0].top && pt[2].y < rcDock[0].bottom)
            {
                if (SetDockWinPosition(pMovWin, nXCoord, pt[2].y))
                    continue;
            }
            else if (pt[0].y > rcDock[2].top && pt[0].y < rcDock[2].bottom)
            {
                if (SetDockWinPosition(pMovWin,
                    nXCoord, pt[0].y - pMovWin->GetHeight()))
                    continue;
            }
        }
        else if (pt[0].y < rcDock[2].top && pt[2].y > rcDock[0].bottom)
        {
            nYCoord = rcMovWin.top;
            if (pt[1].x > rcDock[0].left && pt[1].x < rcDock[0].right)
            {
                if (SetDockWinPosition(pMovWin, pt[1].x, nYCoord))
                    continue;
            }
            else if (pt[0].x > rcDock[1].left && pt[0].x < rcDock[1].right)
            {
                if (SetDockWinPosition(pMovWin,
                    pt[0].x - pMovWin->GetWidth(), nYCoord))
                    continue;
            }
        }
    }

DOCKING:
    if (bEdgeDocking)
    {
        switch (j)
        {
        case 0:
            switch (i)
            {
            case 1:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos();
                break;
            case 2:
                nXCoord = pWin->GetXPos();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
                break;
            case 3:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
            }
            break;

        case 1:
            switch (i)
            {
            case 0:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos();
                break;
            case 2:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
                break;
            case 3:
                nXCoord = pWin->GetXPos() + pWin->GetWidth()
                    - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
            }
            break;

        case 2:
            switch (i)
            {
            case 0:
                nXCoord = pWin->GetXPos();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 1:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 3:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight()
                    - pMovWin->GetHeight();
            }
            break;

        case 3:
            switch (i)
            {
            case 0:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 1:
                nXCoord = pWin->GetXPos() + pWin->GetWidth()
                    - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 2:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight()
                    - pMovWin->GetHeight();
            }
        }
        SetDockWinPosition(pMovWin, nXCoord, nYCoord);
    }
}

bool CUIMng::SetDockWinPosition(CWin* pMoveWin, int nDockX, int nDockY)
{
    CInput& rInput = CInput::Instance();
    RECT rcDummy;
    RECT rcScreen = { 0, 0, rInput.GetScreenWidth(), rInput.GetScreenHeight() };
    RECT rcMoveWin = { nDockX, nDockY,
        nDockX + pMoveWin->GetWidth(), nDockY + pMoveWin->GetHeight() };

    if (::IntersectRect(&rcDummy, &rcScreen, &rcMoveWin))
    {
        pMoveWin->SetPosition(nDockX, nDockY);
        pMoveWin->SetDocking(true);
        return true;
    }

    return false;
}

void CUIMng::Update(double dDeltaTick)
{
    if (UIM_SCENE_NONE == m_nScene || m_WinList.IsEmpty())
        return;

    if (m_bWinActive)
    {
        CWin* pWin = (CWin*)m_WinList.GetHead();
        if (pWin->IsShow())
        {
            pWin->Active(true);
            m_bWinActive = false;
        }
    }

    CInput& rInput = CInput::Instance();
    if (rInput.IsKeyDown(VK_ESCAPE))
    {
        m_bSysMenuWinShow = !m_bSysMenuWinShow;
    }

    CWin* pWin;
    NODE* position;

    m_bCursorOnUI = false;

    if (rInput.IsLBtnDn())
    {
        bool bWinClick = false;
        position = m_WinList.GetHeadPosition();
        while (position)
        {
            pWin = (CWin*)m_WinList.GetNext(position);

            if (pWin->CursorInWin(WA_ALL))
            {
                SetActiveWin(pWin);
                bWinClick = true;
                break;
            }
        }

        if (!bWinClick)
        {
            pWin = (CWin*)m_WinList.GetHead();
            pWin->Active(false);
        }
    }
    else if (rInput.IsLBtnUp())
    {
        m_bBlockCharMove = false;
    }
    int nlist = m_WinList.GetCount();
    CWin** apTempWin = new (CWin * [nlist]);

    position = m_WinList.GetHeadPosition();
    for (int i = 0; i < nlist; ++i)
    {
        apTempWin[i] = (CWin*)m_WinList.GetNext(position);
        apTempWin[i]->ActiveBtns(false);
    }

    position = m_WinList.GetHeadPosition();
    while (position)
    {
        pWin = (CWin*)m_WinList.GetNext(position);
        if (pWin->CursorInWin(WA_ALL))
        {
            pWin->ActiveBtns(true);
            break;
        }
    }

    for (int i = 0; i < nlist; ++i)
    {
        apTempWin[i]->Update(dDeltaTick);
    }

    SAFE_DELETE_ARRAY(apTempWin);

    //	CheckKey();
    CheckDockWin();

    position = m_WinList.GetHeadPosition();
    while (position)
    {
        pWin = (CWin*)m_WinList.GetNext(position);

        switch (pWin->GetState())
        {
        case WS_ETC:
            m_bCursorOnUI = true;
            break;

        case WS_MOVE:
            //			eCursorActType = CURSOR_M;
            m_bCursorOnUI = true;
            break;

        case WS_EXTEND_UP:
            //			eCursorActType = CURSOR_V;
            m_bCursorOnUI = true;
            break;

        case WS_EXTEND_DN:
            //			eCursorActType = CURSOR_V;
            m_bCursorOnUI = true;
            break;
        }

        if (m_bCursorOnUI)
            break;

        if (pWin->CursorInWin(WA_ALL))
        {
            m_bCursorOnUI = true;
            break;
        }
    }
}

void CUIMng::Render()
{
    if (UIM_SCENE_NONE == m_nScene)
        return;

    m_CharInfoBalloonMng.Render();

    CWin* pWin;
    NODE* position = m_WinList.GetTailPosition();
    while (position)
    {
        pWin = (CWin*)m_WinList.GetPrev(position);
        pWin->Render();
    }
}

void CUIMng::PopUpMsgWin(int nMsgCode, wchar_t* pszMsg)
{
    if (UIM_SCENE_NONE == m_nScene || UIM_SCENE_TITLE == m_nScene || UIM_SCENE_LOADING == m_nScene)
        return;

    if (UIM_SCENE_MAIN == m_nScene)	return;

    m_MsgWin.PopUp(nMsgCode, pszMsg);
}

void CUIMng::AddServerMsg(wchar_t* pszMsg)
{
    if (UIM_SCENE_CHARACTER != m_nScene)
        return;

    m_ServerMsgWin.AddMsg(pszMsg);
}