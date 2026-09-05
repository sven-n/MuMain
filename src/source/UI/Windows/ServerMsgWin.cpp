//*****************************************************************************
// File: ServerMsgWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "ServerMsgWin.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Core/SceneUICoordinator.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/UsefulDef.h"

#define WE_CENTER_SPR_POS 3

CServerMsgWin g_ServerMsgWin;

// cppcheck-suppress uninitMemberVar
CServerMsgWin::CServerMsgWin() {}

CServerMsgWin::~CServerMsgWin()
{
    Release();
}

void CServerMsgWin::Create()
{
    Release();

    // Same 5-part composite background CWinEx::Create() built (WinEx.cpp), ported directly since
    // this window no longer goes through CWinEx.
    SImgInfo aiiDescBg[BG_MAX] = {{BITMAP_LOG_IN + 11, 0, 0, 4, 4},
                                  {BITMAP_LOG_IN + 12, 0, 0, 512, 6},
                                  {BITMAP_LOG_IN + 12, 0, 6, 512, 6},
                                  {BITMAP_LOG_IN + 13, 0, 0, 3, 4},
                                  {BITMAP_LOG_IN + 13, 3, 0, 3, 4}};

    m_aSprBg[BG_CENTER].Create(&aiiDescBg[0], 0, 0, true);
    m_aSprBg[BG_TOP].Create(&aiiDescBg[1]);
    m_aSprBg[BG_BOTTOM].Create(&aiiDescBg[2]);
    m_aSprBg[BG_LEFT].Create(&aiiDescBg[3], 0, 0, true);
    m_aSprBg[BG_RIGHT].Create(&aiiDescBg[4], 0, 0, true);

    m_aSprBg[BG_CENTER].SetSize(m_aSprBg[BG_TOP].GetWidth() - WE_CENTER_SPR_POS * 2, 0, X);

    m_ptPos.x = m_ptPos.y = 0;
    m_nBgSideNow = 1; // was CWinEx::Create(aiiDescBg, /*nBgSideMin=*/1, ...)

    ::memset(m_aszMsg, 0, sizeof(wchar_t) * SMW_MSG_LINE_MAX * SMW_MSG_ROW_MAX);
    m_nMsgLine = 0;

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(SEASON3B::INTERFACE_SERVER_MESSAGE, this);
    Show(false);
}

void CServerMsgWin::Release()
{
    for (auto& sprite : m_aSprBg)
        sprite.Release();
}

void CServerMsgWin::SetPosition(int nXCoord, int nYCoord)
{
    m_aSprBg[BG_TOP].SetPosition(nXCoord, nYCoord);
    m_aSprBg[BG_CENTER].SetPosition(nXCoord + WE_CENTER_SPR_POS, nYCoord + WE_CENTER_SPR_POS);
    m_aSprBg[BG_LEFT].SetPosition(nXCoord, nYCoord + m_aSprBg[BG_TOP].GetHeight());
    m_aSprBg[BG_RIGHT].SetPosition(nXCoord + m_aSprBg[BG_TOP].GetWidth() - m_aSprBg[BG_RIGHT].GetWidth(),
                                   m_aSprBg[BG_LEFT].GetYPos());
    m_aSprBg[BG_BOTTOM].SetPosition(nXCoord, m_aSprBg[BG_LEFT].GetYPos() + m_aSprBg[BG_LEFT].GetHeight());

    m_ptPos.x = nXCoord;
    m_ptPos.y = nYCoord;
}

int CServerMsgWin::SetLine(int nLine)
{
    nLine = LIMIT(nLine, 1, SMW_MSG_LINE_MAX * 5);

    if (m_nBgSideNow == nLine)
        return m_nBgSideNow;

    int nOldLine = m_nBgSideNow;
    m_nBgSideNow = nLine;

    int nBgSideHeight = m_aSprBg[BG_LEFT].GetTexHeight() * m_nBgSideNow;

    m_aSprBg[BG_LEFT].SetSize(0, nBgSideHeight, Y);
    m_aSprBg[BG_RIGHT].SetSize(0, nBgSideHeight, Y);

    m_aSprBg[BG_BOTTOM].SetPosition(0, m_aSprBg[BG_LEFT].GetYPos() + m_aSprBg[BG_LEFT].GetHeight(), Y);

    const int nHeight = m_aSprBg[BG_TOP].GetHeight() + m_aSprBg[BG_BOTTOM].GetHeight() + nBgSideHeight;
    m_aSprBg[BG_CENTER].SetSize(0, nHeight - WE_CENTER_SPR_POS * 2, Y);

    return nOldLine;
}

void CServerMsgWin::Show(bool bShow)
{
    SEASON3B::CNewUIObj::Show(bShow);
    for (auto& sprite : m_aSprBg)
        sprite.Show(bShow);
}

void CServerMsgWin::AddMsg(wchar_t* pszMsg)
{
    if (++m_nMsgLine > SMW_MSG_LINE_MAX)
    {
        m_nMsgLine = SMW_MSG_LINE_MAX;
        for (int i = 0; i < SMW_MSG_LINE_MAX - 1; ++i)
            ::wcscpy(m_aszMsg[i], m_aszMsg[i + 1]);
    }
    else
        SetLine(m_nMsgLine * 5);

    wcscpy(m_aszMsg[m_nMsgLine - 1], pszMsg);

    Show(true);
}

bool CServerMsgWin::Render()
{
    for (auto& sprite : m_aSprBg)
        sprite.Render();

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    // LayoutMode::Legacy (UILayoutPolicy.cpp) keeps the active transform an identity while this
    // renders, so these real-pixel coordinates need no rescaling -- was CWin::GetXPos()/GetYPos()
    // pre-divided by g_fScreenRate_x/y to cancel out whatever the ambient transform happened to be
    // during the old CWin::Render() pass; that cancellation trick is unnecessary now.
    for (int i = 0; i < m_nMsgLine; ++i)
        g_pRenderText->RenderText(m_ptPos.x + 11, m_ptPos.y + 12 + i * 20, m_aszMsg[i]);

    return true;
}
