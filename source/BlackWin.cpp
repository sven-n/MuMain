// BlackWin.cpp: implementation of the CBlackWin class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BlackWin.h"
#include "Input.h"
#include "UIMng.h"

#define BLW_FADE_TIME	1000.0

CBlackWin::CBlackWin()
{

}

CBlackWin::~CBlackWin()
{

}

void CBlackWin::Create()
{
	CInput& rInput = CInput::Instance();
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());
}

bool CBlackWin::CursorInWin(int nArea)
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
