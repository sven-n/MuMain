//*****************************************************************************
// File: WinEx.cpp
//*****************************************************************************

#include "stdafx.h"
#include "WinEx.h"

#include "Input.h"
#include "Button.h"
#include "UsefulDef.h"

#define	WE_CENTER_SPR_POS		3

CWinEx::CWinEx()
{
}

CWinEx::~CWinEx()
{
    Release();
}

void CWinEx::Create(SImgInfo* aImgInfo, int nBgSideMin, int nBgSideMax)
{
    Release();

    CWin::m_psprBg = new CSprite[WE_BG_MAX];

    CWin::m_psprBg[WE_BG_CENTER].Create(aImgInfo, 0, 0, true);
    CWin::m_psprBg[WE_BG_TOP].Create(aImgInfo + 1);
    CWin::m_psprBg[WE_BG_BOTTOM].Create(aImgInfo + 2);
    CWin::m_psprBg[WE_BG_LEFT].Create(aImgInfo + 3, 0, 0, true);
    CWin::m_psprBg[WE_BG_RIGHT].Create(aImgInfo + 4, 0, 0, true);

    CWin::m_psprBg[WE_BG_CENTER].SetSize(CWin::m_psprBg[WE_BG_TOP].GetWidth() - WE_CENTER_SPR_POS * 2, 0, X);

    CWin::m_ptPos.x = CWin::m_ptPos.y = 0;
    CWin::m_ptHeld = CWin::m_ptTemp = CWin::m_ptPos;
    CWin::m_bDocking = CWin::m_bActive = CWin::m_bShow = false;
    CWin::m_nState = WS_NORMAL;
    CWin::m_Size.cx = CWin::m_psprBg[WE_BG_TOP].GetWidth();
    CWin::m_Size.cy = CWin::m_psprBg[WE_BG_TOP].GetHeight()
        + CWin::m_psprBg[WE_BG_BOTTOM].GetHeight()
        + CWin::m_psprBg[WE_BG_LEFT].GetHeight();

    m_nBgSideNow = m_nBgSideMin = nBgSideMin;
    m_nBgSideMax = nBgSideMax;
}

void CWinEx::Release()
{
    PreRelease();
    CButton* pBtn;
    while (CWin::m_BtnList.GetCount())
    {
        pBtn = (CButton*)CWin::m_BtnList.RemoveHead();
        pBtn->Release();
    }

    SAFE_DELETE_ARRAY(CWin::m_psprBg);
}

void CWinEx::SetPosition(int nXCoord, int nYCoord)
{
    CWin::m_psprBg[WE_BG_TOP].SetPosition(nXCoord, nYCoord);

    CWin::m_psprBg[WE_BG_CENTER].SetPosition(nXCoord + WE_CENTER_SPR_POS,
        nYCoord + WE_CENTER_SPR_POS);

    CWin::m_psprBg[WE_BG_LEFT].SetPosition(nXCoord,
        nYCoord + CWin::m_psprBg[WE_BG_TOP].GetHeight());

    CWin::m_psprBg[WE_BG_RIGHT].SetPosition(
        nXCoord + CWin::m_psprBg[WE_BG_TOP].GetWidth()
        - CWin::m_psprBg[WE_BG_RIGHT].GetWidth(),
        CWin::m_psprBg[WE_BG_LEFT].GetYPos());

    CWin::m_psprBg[WE_BG_BOTTOM].SetPosition(nXCoord,
        CWin::m_psprBg[WE_BG_LEFT].GetYPos()
        + CWin::m_psprBg[WE_BG_LEFT].GetHeight());

    m_ptPos.x = nXCoord;
    m_ptPos.y = nYCoord;
}

int CWinEx::SetLine(int nLine)
{
    nLine = LIMIT(nLine, m_nBgSideMin, m_nBgSideMax);

    if (m_nBgSideNow == nLine)
        return m_nBgSideNow;

    int nOldLine = m_nBgSideNow;
    m_nBgSideNow = nLine;

    int nBgSideHeight
        = CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideNow;

    CWin::m_psprBg[WE_BG_LEFT].SetSize(0, nBgSideHeight, Y);
    CWin::m_psprBg[WE_BG_RIGHT].SetSize(0, nBgSideHeight, Y);

    CWin::m_psprBg[WE_BG_BOTTOM].SetPosition(0,
        CWin::m_psprBg[WE_BG_LEFT].GetYPos()
        + CWin::m_psprBg[WE_BG_LEFT].GetHeight(), Y);

    CWin::m_Size.cy = CWin::m_psprBg[WE_BG_TOP].GetHeight()
        + CWin::m_psprBg[WE_BG_BOTTOM].GetHeight() + nBgSideHeight;

    CWin::m_psprBg[WE_BG_CENTER].SetSize(0,
        CWin::m_Size.cy - WE_CENTER_SPR_POS * 2, Y);

    return nOldLine;
}

void CWinEx::SetSize(int nHeight)
{
    int nLine = (nHeight - CWin::m_psprBg[WE_BG_TOP].GetHeight()
        - CWin::m_psprBg[WE_BG_BOTTOM].GetHeight())
        / CWin::m_psprBg[WE_BG_LEFT].GetTexHeight();

    SetLine(nLine);
}

bool CWinEx::CursorInWin(int nArea)
{
    if (!CWin::m_bShow)
        return false;

    CInput& rInput = CInput::Instance();
    RECT rc = { 0, 0, 0, 0 };

    switch (nArea)
    {
    case WA_EXTEND_DN:
        ::SetRect(&rc, CWin::m_ptPos.x, CWin::m_ptPos.y + CWin::m_Size.cy - 5,
            CWin::m_ptPos.x + CWin::m_Size.cx,
            CWin::m_ptPos.y + CWin::m_Size.cy);
        if (::PtInRect(&rc, rInput.GetCursorPos()))
            return true;
        break;

    case WA_EXTEND_UP:
        ::SetRect(&rc, CWin::m_ptPos.x, CWin::m_ptPos.y,
            CWin::m_ptPos.x + CWin::m_Size.cx, CWin::m_ptPos.y + 4);
        if (::PtInRect(&rc, rInput.GetCursorPos()))
            return true;
        break;
    }

    return CWin::CursorInWin(nArea);
}

void CWinEx::Show(bool bShow)
{
    for (int i = 0; i < WE_BG_MAX; ++i)
        CWin::m_psprBg[i].Show(bShow);

    CWin::m_bShow = bShow;
    if (!CWin::m_bShow)
        CWin::m_bActive = false;
}

void CWinEx::CheckAdditionalState()
{
    CInput& rInput = CInput::Instance();

    if (rInput.IsLBtnDn())
    {
        if (CursorInWin(WA_EXTEND_UP))
        {
            m_nBasisY = CWin::m_ptPos.y
                + CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideNow;
            CWin::m_nState = WS_EXTEND_UP;
        }

        if (CursorInWin(WA_EXTEND_DN))
        {
            m_nBasisY = CWin::m_ptPos.y + CWin::m_psprBg[WE_BG_TOP].GetHeight()
                + CWin::m_psprBg[WE_BG_BOTTOM].GetHeight();
            CWin::m_nState = WS_EXTEND_DN;
        }
    }

    int nBgSideHeight;
    switch (CWin::m_nState)
    {
    case WS_EXTEND_UP:
        nBgSideHeight = m_nBasisY - rInput.GetCursorY();
        if (nBgSideHeight
            < CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideMin)
            SetLine(m_nBgSideMin);
        else
            SetLine(nBgSideHeight / CWin::m_psprBg[WE_BG_LEFT].GetTexHeight()
                + 1);

        SetPosition(CWin::m_ptPos.x, m_nBasisY
            - CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideNow);

        break;

    case WS_EXTEND_DN:
        nBgSideHeight = rInput.GetCursorY() - m_nBasisY;
        if (nBgSideHeight
            < CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideMin)
            SetLine(m_nBgSideMin);
        else
            SetLine(nBgSideHeight / CWin::m_psprBg[WE_BG_LEFT].GetTexHeight()
                + 1);

        break;
    }
}

void CWinEx::Render()
{
    if (CWin::m_bShow)
    {
        for (int i = 0; i < WE_BG_MAX; ++i)
            CWin::m_psprBg[i].Render();

        RenderControls();
    }
}