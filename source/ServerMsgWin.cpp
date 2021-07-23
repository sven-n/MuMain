//*****************************************************************************
// File: ServerMsgWin.cpp
//
// Desc: implementation of the CServerMsgWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "ServerMsgWin.h"
#include "UIControls.h"

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerMsgWin::CServerMsgWin()
{

}

CServerMsgWin::~CServerMsgWin()
{

}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 창 생성.
//*****************************************************************************
void CServerMsgWin::Create()
{
	SImgInfo aiiDescBg[WE_BG_MAX] = 
	{
		{ BITMAP_LOG_IN+11, 0, 0, 4, 4 },	// 중앙
		{ BITMAP_LOG_IN+12, 0, 0, 512, 6 },	// 상
		{ BITMAP_LOG_IN+12, 0, 6, 512, 6 },	// 하
		{ BITMAP_LOG_IN+13, 0, 0, 3, 4 },	// 좌
		{ BITMAP_LOG_IN+13, 3, 0, 3, 4 }	// 우
	};
	CWinEx::Create(aiiDescBg, 1, SMW_MSG_LINE_MAX * 5);

	::memset(m_aszMsg, 0, sizeof(char) * SMW_MSG_LINE_MAX * SMW_MSG_ROW_MAX);
	m_nMsgLine = 0;
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool CServerMsgWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)		// 보이지 않는다면 처리하지 않음.
		return false;

	switch (nArea)
	{
	case WA_ALL:
		return false;	// 모든 영역 반응 없음.
	}

	return CWinEx::CursorInWin(nArea);
}

//*****************************************************************************
// 함수 이름 : AddMsg()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
void CServerMsgWin::AddMsg(char* pszMsg)
{
	if (++m_nMsgLine > SMW_MSG_LINE_MAX)	// 메시지 최대 줄수가 넘어가면.
	{
		m_nMsgLine = SMW_MSG_LINE_MAX;
		for (int i = 0; i < SMW_MSG_LINE_MAX - 1; ++i)
			::strcpy(m_aszMsg[i], m_aszMsg[i + 1]);
	}
	else
		CWinEx::SetLine(m_nMsgLine * 5);

	::strcpy(m_aszMsg[m_nMsgLine - 1], pszMsg);

	CWinEx::Show(true);
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
void CServerMsgWin::RenderControls()
{
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);

	int i;
	for (i = 0; i < m_nMsgLine; ++i)
	{
		g_pRenderText->RenderText(int((CWin::GetXPos() + 11) / g_fScreenRate_x),
			int((CWin::GetYPos() + 12 + i * 20) / g_fScreenRate_y),
			m_aszMsg[i]);
	}
}
