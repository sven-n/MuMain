//*****************************************************************************
// File: ServerMsgWin.h
//*****************************************************************************

#if !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
#define AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_

#pragma once

#include "WinEx.h"

#define SMW_MSG_LINE_MAX	5
#define SMW_MSG_ROW_MAX		83

class CServerMsgWin : public CWinEx
{
protected:
    wchar_t	m_aszMsg[SMW_MSG_LINE_MAX][SMW_MSG_ROW_MAX];
    int		m_nMsgLine;

public:
    CServerMsgWin();
    virtual ~CServerMsgWin();

    void Create();
    bool CursorInWin(int nArea);

    void AddMsg(wchar_t* pszMsg);

protected:
    void RenderControls();
};

#endif // !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
