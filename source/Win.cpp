//*****************************************************************************
// File: Win.cpp
//
// Desc: implementation of the CWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "Win.h"

#include "Input.h"
#include "Button.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWin::CWin() : m_psprBg(NULL)
{

}

// 자식 소멸자에서는 자식 클래스 내에 동적 할당된 것만 처리하면 됨.
CWin::~CWin()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 윈도우 생성.
// 매개 변수 : nWidth	: 너비.(텍스처가 있다면 텍스처의 이미지 너비.)
//			   nHeight	: 높이.(텍스처가 있다면 텍스처의 이미지 높이.)
//			   nTexID	: 윈도우 텍스처 ID.(기본값 -1)
//						  -2라면 아무것도 랜더를 안하는 윈도우.
//						  -1이라면 텍스처 없는 단색 윈도우.
//			   bTile	: 타일링 여부.(기본값 false)
//*****************************************************************************
void CWin::Create(int nWidth, int nHeight, int nTexID, bool bTile)
{
	Release();

	if (-2 < nTexID)
	{
		m_psprBg = new CSprite;
		m_psprBg->Create(nWidth, nHeight, nTexID, 0, NULL, 0, 0, bTile);
		// 텍스처 ID가 -1이면 반투명 검정색.
		if (-1 == nTexID)
		{
			m_psprBg->SetAlpha(128);
			m_psprBg->SetColor(0, 0, 0);
		}
	}

	m_ptPos.x = m_ptPos.y = 0;
	m_ptHeld = m_ptTemp = m_ptPos;
	m_Size.cx = nWidth;
	m_Size.cy = nHeight;
	m_bDocking = m_bActive = m_bShow = false;
	m_nState = WS_NORMAL;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 윈도우 메모리에서 삭제.
//*****************************************************************************
void CWin::Release()
{
	PreRelease();	// 파생 클래스 데이터 멤버의 Release().

	// 버튼 리스트에 등록 되어 있는 버튼들 삭제와 리스트 삭제.
	CButton* pBtn;
	while (m_BtnList.GetCount())
	{
		pBtn = (CButton*)m_BtnList.RemoveHead();
		pBtn->Release();
	}

	SAFE_DELETE(m_psprBg);
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 윈도우 위치 이동.
// 매개 변수 : nXCoord	: X좌표.
//			   nYCoord	: Y좌표.
//*****************************************************************************
void CWin::SetPosition(int nXCoord, int nYCoord)
{
	m_ptPos.x = nXCoord;
	m_ptPos.y = nYCoord;
	if (m_psprBg)
		m_psprBg->SetPosition(nXCoord, nYCoord);
}

//*****************************************************************************
// 함수 이름 : SetSize()
// 함수 설명 : 크기조절 기준으로 윈도우 크기 변경.
// 매개 변수 : nWidth		: 너비.(픽셀 단위.)
//			   nHeight		: 높이.(픽셀 단위.)
//			   eChangedPram	: 변경할 파라미터. X는 너비만, Y는 높이만, XY는 모
//							 두 변경.(기본값 XY)
//*****************************************************************************
void CWin::SetSize(int nWidth, int nHeight, CHANGE_PRAM eChangedPram)
{
	if (eChangedPram & X)	// 가로 크기 변경인가?
		m_Size.cx = nWidth;
	if (eChangedPram & Y)	// 세로 크기 변경인가?
		m_Size.cy = nHeight;

	if (m_psprBg)
		m_psprBg->SetSize(nWidth, nHeight, eChangedPram);
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(WA_ALL, WA_MOVE 중 하나)
//*****************************************************************************
bool CWin::CursorInWin(int nArea)
{
	if (!m_bShow)		// 보이지 않는다면 처리하지 않음.
		return false;

	CInput& rInput = CInput::Instance();
	RECT rc = { 0, 0, 0, 0 };
	NODE* position;
	CButton* pBtn;

	switch (nArea)
	{
	case WA_ALL:
		::SetRect(&rc, m_ptPos.x, m_ptPos.y, m_ptPos.x + m_Size.cx,
			m_ptPos.y + m_Size.cy);
		if (::PtInRect(&rc, rInput.GetCursorPos()))
			return true;
		break;

	case WA_MOVE:
		::SetRect(&rc, m_ptPos.x, m_ptPos.y, m_ptPos.x + m_Size.cx,
			m_ptPos.y + 26);
		if (::PtInRect(&rc, rInput.GetCursorPos()))
			return true;
		break;
	// 버튼 영역 체크 목적도 있지만 버튼 위에 마우스 커서 이미지 변경에도 사용.
	case WA_BUTTON:
		position = m_BtnList.GetHeadPosition();	// list의 Head부터 검사.
		while (position)	// position이 NULL일 때까지 반복.
		{
			// 이 position의 값을 얻고 이 position를 다음 position으로.
			pBtn = (CButton*)m_BtnList.GetNext(position);
			if (pBtn->CursorInObject())
				return true;
		}
		if (CursorInButtonlike())
			return true;
		break;
	}

	return false;
}

void CWin::ActiveBtns(bool bActive)
{
	CButton* pBtn;
	NODE* position = m_BtnList.GetHeadPosition();	// list의 Head부터 검사.
	while (position)	// position이 NULL일 때까지 반복.
	{
		// 이 position의 값을 얻고 이 position를 다음 position으로.
		pBtn = (CButton*)m_BtnList.GetNext(position);
		pBtn->SetActive(bActive);
	}
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 윈도우를 보이고 안보이고의 선택.
// 매개 변수 : bShow	: true이면 보여줌.(기본값 true)
//*****************************************************************************
void CWin::Show(bool bShow)
{
	if (m_psprBg)
		m_psprBg->Show(bShow);
	m_bShow = bShow;
	if (!m_bShow)		// 보이지 않는다면.
		m_bActive = false;	// 항상 비활성화.
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 윈도우 업데이트.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//							  (기본값 0.0)
//*****************************************************************************
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
void CWin::Update()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CWin::Update(double dDeltaTick)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	if (!m_bShow)	// 보이지 않으면 리턴.
		return;

	CInput& rInput = CInput::Instance();

	if (rInput.IsLBtnUp())	// 마우스 버튼을 놓았는가?
		m_nState = WS_NORMAL;		// 보통 상태로 초기화.

	if (m_nState == WS_NORMAL)	// 보통 상태라면 각 버튼 Update().
	{
		CButton* pBtn;
		NODE* position = m_BtnList.GetHeadPosition();	// list의 Head부터 검사.
		while (position)	// position이 NULL일 때까지 반복.
		{
			// 이 position의 값을 얻고 이 position를 다음 position으로.
			pBtn = (CButton*)m_BtnList.GetNext(position);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
			pBtn->Update();
#else //KWAK_FIX_COMPILE_LEVEL4_WARNING
			pBtn->Update(dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		}
	}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	UpdateWhileShow();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	UpdateWhileShow(dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	if (!m_bActive)	// 비활성화이면 리턴.
		return;

	// 이동.
	if (rInput.IsLBtnDn())// 마우스 버튼을 눌렀는가?
	{
		if (CursorInWin(WA_MOVE))	// 이동시킬 수 있는 영역을 눌렀는가?
		{
			// 현재 커서 위치 기억.
			m_ptHeld = rInput.GetCursorPos();
			m_ptTemp = m_ptPos;
			m_nState = WS_MOVE;		// 이동 상태임.
		}
	}

	if (WS_MOVE == m_nState)		// 이동상태인가?
	{
		// 변화량 만큼 증가.
		m_ptTemp.x += rInput.GetCursorX() - m_ptHeld.x;
		m_ptTemp.y += rInput.GetCursorY() - m_ptHeld.y;
		if (!m_bDocking)		// 도킹이 아니라면.
			// 윈도우를 변화된 위치로 위치시킴.
			SetPosition(m_ptTemp.x, m_ptTemp.y);
		m_ptHeld = rInput.GetCursorPos();	// 현재 커서 위치를 이전 커서 위치에 기억.
	}

	CheckAdditionalState();

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	UpdateWhileActive();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	UpdateWhileActive(dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 윈도우 렌더.
//*****************************************************************************
void CWin::Render()
{
	if (m_bShow)
	{
		if (m_psprBg)
			m_psprBg->Render();

		RenderControls();
	}
}

//*****************************************************************************
// 함수 이름 : RegisterButton()
// 함수 설명 : 버튼 리스트에 버튼 등록.
//			 CWin을 상속 받은 윈도우는 Create()에서 이 함수로 생성된 모든 버튼
//			들을 등록하면 윈도우에서의 기본적인 버튼 동작들을 자동으로 처리한다.
// 매개 변수 : pBtn	: 등록할 버튼 포인터.
//*****************************************************************************
void CWin::RegisterButton(CButton* pBtn)
{
	m_BtnList.AddTail(pBtn);
}

void CWin::RenderButtons()
{
	CButton* pBtn;
	NODE* position = m_BtnList.GetHeadPosition();	// list의 Head부터 검사.
	while (position)	// position이 NULL일 때까지 반복.
	{
		// 이 position의 값을 얻고 이 position를 다음 position으로.
		pBtn = (CButton*)m_BtnList.GetNext(position);
		pBtn->Render();
	}
}