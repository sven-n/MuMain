//*****************************************************************************
// File: WinEx.cpp
//
// Desc: implementation of the CWinEx class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "WinEx.h"

#include "Input.h"
#include "Button.h"
#include "UsefulDef.h"

//#include "ZzzMathLib.h"
//#include "ZzzOpenglUtil.h"

#define	WE_CENTER_SPR_POS		3	// 배경 중앙 스프라이트 상대 위치.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinEx::CWinEx()
{

}

CWinEx::~CWinEx()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 확장 윈도우 생성.
// 매개 변수 : aImgInfo		: WE_BG_MAX 개의 SImgInfo의 배열.
//							  이하 <배열 순서> 참조.
//			   nBgSideMin	: 좌우측 스프라이트 최소 개수.
//			   nBgSideMax	: 좌우측 스프라이트 최대 개수.
//
// <배열 순서>
//#define	WE_BG_CENTER	0	// 중앙 바탕 스트라이트.
//#define	WE_BG_TOP		1	// 상단 바탕 스프라이트.
//#define	WE_BG_BOTTOM	2	// 하단 바탕 스프라이트.
//#define	WE_BG_LEFT		3	// 좌측 바탕 스프라이트.
//#define	WE_BG_RIGHT		4	// 우측 바탕 스프라이트.
//*****************************************************************************
void CWinEx::Create(SImgInfo* aImgInfo, int nBgSideMin, int nBgSideMax)
{
	Release();

	// 각 배경 스프라이트 생성.
	CWin::m_psprBg = new CSprite[WE_BG_MAX];

	CWin::m_psprBg[WE_BG_CENTER].Create(aImgInfo, 0, 0, true);
	CWin::m_psprBg[WE_BG_TOP].Create(aImgInfo + 1);
	CWin::m_psprBg[WE_BG_BOTTOM].Create(aImgInfo + 2);
	CWin::m_psprBg[WE_BG_LEFT].Create(aImgInfo + 3, 0, 0, true);
	CWin::m_psprBg[WE_BG_RIGHT].Create(aImgInfo + 4, 0, 0, true);

	// 배경 중앙 스프라이트를 너비에 맞게 늘림.
	CWin::m_psprBg[WE_BG_CENTER].SetSize(CWin::m_psprBg[WE_BG_TOP].GetWidth()
		- WE_CENTER_SPR_POS * 2, 0, X);	

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

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 확장 윈도우 메모리에서 삭제.
//*****************************************************************************
void CWinEx::Release()
{
	PreRelease();	// 파생 클래스 데이터 멤버의 Release().

	// 버튼 리스트에 등록 되어 있는 버튼들 삭제와 리스트 삭제.
	CButton* pBtn;
	while (CWin::m_BtnList.GetCount())
	{
		pBtn = (CButton*)CWin::m_BtnList.RemoveHead();
		pBtn->Release();
	}

	SAFE_DELETE_ARRAY(CWin::m_psprBg);
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 확장 윈도우 위치 이동.
// 매개 변수 : nXCoord	: X좌표.
//			   nYCoord	: Y좌표.
//*****************************************************************************
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

//*****************************************************************************
// 함수 이름 : SetLine()
// 함수 설명 : 윈도우 크기 세팅.
// 매개 변수 : nLine	: 좌우 테두리 바탕 스프라이트 타일 개수.
//*****************************************************************************
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

//*****************************************************************************
// 함수 이름 : SetSize()
// 함수 설명 : 윈도우 크기 세팅.
// 매개 변수 : nHeight	: 원하는 윈도우 높이.(픽셀 단위)
//*****************************************************************************
void CWinEx::SetSize(int nHeight)
{
	int nLine = (nHeight - CWin::m_psprBg[WE_BG_TOP].GetHeight()
		- CWin::m_psprBg[WE_BG_BOTTOM].GetHeight())
		/ CWin::m_psprBg[WE_BG_LEFT].GetTexHeight();

	SetLine(nLine);
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(WA_ALL, WA_MOVE, WA_EXTEND_UP,
//						 WA_EXTEND_DN 중 하나)
//*****************************************************************************
bool CWinEx::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)		// 보이지 않는다면 처리하지 않음.
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

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 윈도우 보이기.
// 매개 변수 : bShow	: true이면 보임. (기본값 true)
//******************************************************************************/
void CWinEx::Show(bool bShow)
{
	for (int i = 0; i < WE_BG_MAX; ++i)
		CWin::m_psprBg[i].Show(bShow);

	CWin::m_bShow = bShow;
	if (!CWin::m_bShow)		// 보이지 않는다면.
		CWin::m_bActive = false;	// 항상 비활성화.
}

//*****************************************************************************
// 함수 이름 : CheckAdditionalState()
// 함수 설명 : 추가 윈도우 상태 체크.
//*****************************************************************************
void CWinEx::CheckAdditionalState()
{
	CInput& rInput = CInput::Instance();

	if (rInput.IsLBtnDn())	// 마우스 버튼을 눌렀는가?
	{
		// 위로 늘림.
		if (CursorInWin(WA_EXTEND_UP))
		{
			// Top과 Bottom 스프라이트만 있을 경우의 예상 Y위치.
			m_nBasisY = CWin::m_ptPos.y
				+ CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideNow;
			CWin::m_nState = WS_EXTEND_UP;	// 위로 늘리는 상태임.
		}

		// 하로 늘림.
		if (CursorInWin(WA_EXTEND_DN))
		{
			// Top과 Bottom 스프라이트만 있을 경우의 예상 바닥 Y위치.
			m_nBasisY = CWin::m_ptPos.y + CWin::m_psprBg[WE_BG_TOP].GetHeight()
				+ CWin::m_psprBg[WE_BG_BOTTOM].GetHeight();
			CWin::m_nState = WS_EXTEND_DN;	// 하로 늘리는 상태임.
		}
	}

	int nBgSideHeight;
	switch (CWin::m_nState)	// 윈도우 상태는?
	{
	case WS_EXTEND_UP:		// 위로 크기 조절 중.
		nBgSideHeight = m_nBasisY - rInput.GetCursorY();
		// nBgSideHeight가 0이 되면 안되므로 최소크기 조건처리를 함.
		if (nBgSideHeight
			< CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideMin)
			SetLine(m_nBgSideMin);
		else
			SetLine(nBgSideHeight / CWin::m_psprBg[WE_BG_LEFT].GetTexHeight()
				+ 1);

		SetPosition(CWin::m_ptPos.x, m_nBasisY
			- CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideNow);

		break;

	case WS_EXTEND_DN:		// 하로 크기 조절 중.
		nBgSideHeight = rInput.GetCursorY() - m_nBasisY;
		// nBgSideHeight가 0이 되면 안되므로 최소크기 조건처리를 함.
		if (nBgSideHeight
			< CWin::m_psprBg[WE_BG_LEFT].GetTexHeight() * m_nBgSideMin)
			SetLine(m_nBgSideMin);
		else
			SetLine(nBgSideHeight / CWin::m_psprBg[WE_BG_LEFT].GetTexHeight()
				+ 1);
	
		break;
	}
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 확장 윈도우 그리기.
//*****************************************************************************
void CWinEx::Render()
{
	if (CWin::m_bShow)
	{
		for (int i = 0; i < WE_BG_MAX; ++i)
			CWin::m_psprBg[i].Render();

		RenderControls();
	}
}