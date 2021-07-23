// BlackWin.cpp: implementation of the CBlackWin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BlackWin.h"

#include "Input.h"
#include "UIMng.h"

#define BLW_FADE_TIME	1000.0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlackWin::CBlackWin()
{

}

CBlackWin::~CBlackWin()
{

}

void CBlackWin::Create()
{
	CInput& rInput = CInput::Instance();
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());	// 검정 화면.
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool CBlackWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)		// 보이지 않는다면 처리하지 않음.
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;	// 이동 영역은 없음.(이동을 막음)
	}

	return CWin::CursorInWin(nArea);
}

void CBlackWin::UpdateWhileActive(double dDeltaTick)
{
	short nAlpha;
	nAlpha = short(CWin::GetBgAlpha());
	nAlpha -= short(255.0 * dDeltaTick / BLW_FADE_TIME);
	if (0 >= nAlpha)
	{
		nAlpha = 0;
		CUIMng::Instance().HideWin(this);
	}
	CWin::SetBgAlpha((BYTE)nAlpha);
}
