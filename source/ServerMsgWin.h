//*****************************************************************************
// File: ServerMsgWin.h
//
// Desc: interface for the CServerMsgWin class.
//		 서버 메시지 창 클래스.(캐릭터 선택씬에서 사용)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
#define AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinEx.h"

#define SMW_MSG_LINE_MAX	5	// 메시지 줄 수.
#define SMW_MSG_ROW_MAX		83	// 한줄에 최대 글자(byte)수 + 1

class CServerMsgWin : public CWinEx
{
protected:
	char	m_aszMsg[SMW_MSG_LINE_MAX][SMW_MSG_ROW_MAX];// 메시지 문자열.
	int		m_nMsgLine;									// 메시지 줄 수.

public:
	CServerMsgWin();
	virtual ~CServerMsgWin();

	void Create();
	bool CursorInWin(int nArea);

	void AddMsg(char* pszMsg);

protected:
	void RenderControls();
};

#endif // !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
