//*****************************************************************************
// File: Input.cpp
//
// Desc: implementation of the CInput class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "Input.h"
#include "UsefulDef.h"
#include "./Time/Timer.h"
#if	defined WINDOWMODE
#include "UIManager.h"
extern bool g_bWndActive;
#endif
extern CTimer*	g_pTimer;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInput::CInput()
{

}

CInput::~CInput()
{

}

//*****************************************************************************
// 함수 이름 : Instance()
// 함수 설명 : 객체를 단 하나만 생성.
//			  이 클래스에 접근하기 위한 인터페이스임
//*****************************************************************************
CInput& CInput::Instance()
{
	static CInput s_Input;                  
    return s_Input;
}

//****************************************************************************
// 함수 이름 : Create()
// 함수 설명 :
// 매개 변수 : hWnd			: 윈도우 핸들.
//			   lScreenWidth	: 스크린(Back Buffer) 너비.
//			   lScreenHeight: 스크린(Back Buffer) 높이.
//*****************************************************************************
bool CInput::Create(HWND hWnd, long lScreenWidth, long lScreenHeight)
{
	if (NULL == hWnd)
		return false;

	m_hWnd = hWnd;

	// 스크린(Back Buffer) 크기 세팅.
	m_lScreenWidth = lScreenWidth;
	m_lScreenHeight = lScreenHeight;

	// X, Y좌표 초기화.
	::GetCursorPos(&m_ptCursor);
	::ScreenToClient(m_hWnd, &m_ptCursor);
	m_ptFormerCursor = m_ptCursor;

	m_bLeftHand = false;			// 왼손잡이 여부 세팅.

	// 더블 클릭 시간을 얻음.
	m_dDoubleClickTime = (double)::GetDoubleClickTime();
	// 각 버튼의 마지막으로 클릭한 시간 초기화.
	m_dBtn0LastClickTime = m_dBtn1LastClickTime = m_dBtn2LastClickTime = 0.0;
	// 이전에 Down여부 상태값 초기화.
	m_bFormerBtn0Dn = m_bFormerBtn1Dn = m_bFormerBtn2Dn = false;

	m_bLBtnHeldDn = m_bRBtnHeldDn = m_bMBtnHeldDn = false;

	m_bTextEditMode	= false;

	return true;
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : Input 상태를 처리. Main 무한루프에서 한 번만 호출.
//*****************************************************************************
void CInput::Update()
{
	// 각 축의 변화량 초기화.
	m_lDX = m_lDY = 0L;
	// Down, Up, Double Click은 순간에만 true가 되야되므로 false로 초기화 함.
	m_bLBtnUp = m_bRBtnUp = m_bMBtnUp
		= m_bLBtnDn = m_bRBtnDn = m_bMBtnDn
		= m_bLBtnDbl = m_bRBtnDbl = m_bMBtnDbl = false;

// 마우스 커서 위치 처리.
	// 마우스 커서 위치 얻기.
	::GetCursorPos(&m_ptCursor);
	::ScreenToClient(m_hWnd, &m_ptCursor);

	// X, Y좌표값을 제한함.
	m_ptCursor.x = LIMIT(m_ptCursor.x, 0, m_lScreenWidth - 1);
	m_ptCursor.y = LIMIT(m_ptCursor.y, 0, m_lScreenHeight - 1);

	// 각 축의 변화량 Data를 계산.
	m_lDX = m_ptCursor.x - m_ptFormerCursor.x;
	m_lDY = m_ptCursor.y - m_ptFormerCursor.y;

	if (m_lDX || m_lDY)	// 움직임이 있었다면 더블클릭이 아니므로.
		m_bFormerBtn0Dn = m_bFormerBtn1Dn = m_bFormerBtn2Dn = false;

	m_ptFormerCursor = m_ptCursor;

	double dAbsTime;

	if (SEASON3B::IsPress(VK_LBUTTON))
	{
		dAbsTime = g_pTimer->GetAbsTime();

		if (dAbsTime - m_dBtn0LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn0Dn)
		{
			if (m_bLeftHand)
			{
				m_bRBtnDn =	m_bRBtnHeldDn = m_bRBtnDbl = true;
				m_bRBtnUp = false;
			}
			else
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = true;
				m_bLBtnUp = false;
			}
			m_bFormerBtn0Dn = false;
		}
		else
		{
			if (m_bLeftHand)
			{
				m_bRBtnDn =	m_bRBtnHeldDn =	true;
				m_bRBtnUp = m_bRBtnDbl = false;
			}
			else
			{
				m_bLBtnDn = m_bLBtnHeldDn = true;
				m_bLBtnUp = m_bLBtnDbl = false;
			}
			m_bFormerBtn0Dn = true;
		}
		m_dBtn0LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_LBUTTON))
	{
		if (m_bLeftHand)
		{
			if (m_bRBtnHeldDn)
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = false;
				m_bRBtnUp = true;
			}
		}
		else
		{
			if (m_bLBtnHeldDn)
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = false;
				m_bLBtnUp = true;
			}
		}
	}

	if (SEASON3B::IsPress(VK_RBUTTON))
	{
		dAbsTime = g_pTimer->GetAbsTime();

		if (dAbsTime - m_dBtn1LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn1Dn)
		{
			if (m_bLeftHand)
			{
				m_bLBtnDn =	m_bLBtnHeldDn = m_bLBtnDbl = true;
				m_bLBtnUp = false;
			}
			else
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = true;
				m_bRBtnUp = false;
			}
			m_bFormerBtn1Dn = false;
		}
		else
		{
			if (m_bLeftHand)
			{
				m_bLBtnDn =	m_bLBtnHeldDn = true;
				m_bLBtnUp = m_bLBtnDbl = false;
			}
			else
			{
				m_bRBtnDn = m_bRBtnHeldDn = true;
				m_bRBtnUp = m_bRBtnDbl = false;
			}
			m_bFormerBtn1Dn = true;
		}
		m_dBtn1LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_RBUTTON))
	{
		if (m_bLeftHand)
		{
			if (m_bLBtnHeldDn)
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = false;
				m_bLBtnUp = true;
			}
		}
		else
		{
			if (m_bRBtnHeldDn)
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = false;
				m_bRBtnUp = true;
			}
		}
	}

	if (SEASON3B::IsPress(VK_MBUTTON))
	{
		dAbsTime = g_pTimer->GetAbsTime();

		if (dAbsTime - m_dBtn2LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn2Dn)
		{
			m_bMBtnDbl = true;
			m_bFormerBtn2Dn = false;
		}
		else
		{
			m_bFormerBtn2Dn = true;
			m_bMBtnDbl = false;
		}
		m_bMBtnDn = m_bMBtnHeldDn = true;
		m_bMBtnUp = false;
		m_dBtn2LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_MBUTTON))
	{
		if (m_bMBtnHeldDn)
		{
			m_bMBtnDn = m_bMBtnHeldDn = m_bMBtnDbl = false;
			m_bMBtnUp = true;
		}
	}
#if	defined WINDOWMODE
	if(GetActiveWindow() == NULL)
	{
		m_lDX = m_lDY = 0L;

		m_bLBtnUp = m_bRBtnUp = m_bMBtnUp
			= m_bLBtnDn = m_bRBtnDn = m_bMBtnDn
			= m_bLBtnDbl = m_bRBtnDbl = m_bMBtnDbl = false;

		m_ptCursor.x = m_ptCursor.y = 0;
		return;
	}
#endif
}