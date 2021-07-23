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
#ifdef NEW_USER_INTERFACE_UTIL
: m_PosValue( 0 )
#endif //NEW_USER_INTERFACE_UTIL
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

#ifdef NEW_USER_INTERFACE_UTIL
	m_PosValue = m_ptCursor.y | ( m_ptCursor.x << 16 );
#endif //NEW_USER_INTERFACE_UTIL	

	// 마우스 각 버튼 처리.
	double dAbsTime;

	// 마우스 왼쪽 버튼.
	if (SEASON3B::IsPress(VK_LBUTTON))	// 왼쪽 버튼이 눌렸다면.
	{
		dAbsTime = g_pTimer->GetAbsTime();	// 현재 시간.
		// 더블 클릭이고 이전에 원 클릭이었나.
		// 'm_bFormerBtn1Dn'는 버튼을 연타했을 때 Dbl이 교대로
		//나오게하기 위해서.
		if (dAbsTime - m_dBtn0LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn0Dn)
		{
			if (m_bLeftHand)	// 왼손잡이면.
			{
				m_bRBtnDn =	m_bRBtnHeldDn = m_bRBtnDbl = true;
				m_bRBtnUp = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_RButton, eKeyState_DblClk, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			else				// 오른손잡이면.
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = true;
				m_bLBtnUp = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_LButton, eKeyState_DblClk, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			m_bFormerBtn0Dn = false;
		}
		else	// 원 클릭이면.
		{
			if (m_bLeftHand)	// 왼손잡이면.
			{
				m_bRBtnDn =	m_bRBtnHeldDn =	true;
				m_bRBtnUp = m_bRBtnDbl = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_RButton, eKeyState_Down, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			else				// 오른손잡이면.
			{
				m_bLBtnDn = m_bLBtnHeldDn = true;
				m_bLBtnUp = m_bLBtnDbl = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_LButton, eKeyState_Down, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			m_bFormerBtn0Dn = true;
		}
		m_dBtn0LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_LBUTTON))	// 왼쪽 버튼을 안누른 상태라면.
	{
		if (m_bLeftHand)		// 왼손잡이면.
		{
			if (m_bRBtnHeldDn)	// 이전 프레임에서 눌린 상태였다면.
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = false;
				m_bRBtnUp = true;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_RButton, eKeyState_Up, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
		}
		else					// 오른손잡이면.
		{
			if (m_bLBtnHeldDn)	// 이전 프레임에서 눌린 상태였다면.
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = false;
				m_bLBtnUp = true;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_LButton, eKeyState_Up, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
		}
	}

	// 마우스 오른쪽 버튼.
	if (SEASON3B::IsPress(VK_RBUTTON))	// 오른쪽 버튼이 눌렸다면.
	{
		dAbsTime = g_pTimer->GetAbsTime();	// 현재 시간.
		// 더블 클릭이고 이전에 원 클릭이었나.
		// 'm_bFormerBtn1Dn'는 버튼을 연타했을 때 Dbl이 교대로
		//나오게하기 위해서.
		if (dAbsTime - m_dBtn1LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn1Dn)
		{
			if (m_bLeftHand)	// 왼손잡이면.
			{
				m_bLBtnDn =	m_bLBtnHeldDn = m_bLBtnDbl = true;
				m_bLBtnUp = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_LButton, eKeyState_DblClk, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			else				// 오른손잡이면.
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = true;
				m_bRBtnUp = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_RButton, eKeyState_DblClk, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			m_bFormerBtn1Dn = false;
		}
		else	// 원 클릭이면.
		{
			if (m_bLeftHand)	// 왼손잡이면.
			{
				m_bLBtnDn =	m_bLBtnHeldDn = true;
				m_bLBtnUp = m_bLBtnDbl = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_LButton, eKeyState_Down, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			else				// 오른손잡이면.
			{
				m_bRBtnDn = m_bRBtnHeldDn = true;
				m_bRBtnUp = m_bRBtnDbl = false;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_RButton, eKeyState_Down, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
			m_bFormerBtn1Dn = true;
		}
		m_dBtn1LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_RBUTTON))	// 오른쪽 버튼을 안누른 상태라면.
	{
		if (m_bLeftHand)		// 왼손잡이면.
		{
			if (m_bLBtnHeldDn)	// 이전 프레임에서 눌린 상태였다면.
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = false;
				m_bLBtnUp = true;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_LButton, eKeyState_Up, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
		}
		else					// 오른손잡이면.
		{
			if (m_bRBtnHeldDn)	// 이전 프레임에서 눌린 상태였다면.
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = false;
				m_bRBtnUp = true;
#ifdef NEW_USER_INTERFACE_UTIL
				HandleMouseMessage( eKey_RButton, eKeyState_Up, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
			}
		}
	}

	// 마우스 휠(가운데) 버튼.
	if (SEASON3B::IsPress(VK_MBUTTON))	// 휠 버튼이 눌렸다면.
	{
		dAbsTime = g_pTimer->GetAbsTime();	// 현재 시간.
		// 더블 클릭이고 이전에 원 클릭이었나.
		// 'm_bFormerBtn2Dn'는 버튼을 연타했을 때 Dbl이 교대로
		//나오게하기 위해서.
		if (dAbsTime - m_dBtn2LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn2Dn)
		{
			m_bMBtnDbl = true;
			m_bFormerBtn2Dn = false;
#ifdef NEW_USER_INTERFACE_UTIL
			HandleMouseMessage( eKey_Wheel, eKeyState_DblClk, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
		}
		else	// 원 클릭이면.
		{
			m_bFormerBtn2Dn = true;
			m_bMBtnDbl = false;
#ifdef NEW_USER_INTERFACE_UTIL
			HandleMouseMessage( eKey_Wheel, eKeyState_Down, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
		}
		m_bMBtnDn = m_bMBtnHeldDn = true;
		m_bMBtnUp = false;
		m_dBtn2LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_MBUTTON))	// 오른쪽 버튼을 안누른 상태라면.
	{
		if (m_bMBtnHeldDn)	// 이전 프레임에서 눌린 상태였다면.
		{
			m_bMBtnDn = m_bMBtnHeldDn = m_bMBtnDbl = false;
			m_bMBtnUp = true;
#ifdef NEW_USER_INTERFACE_UTIL
			HandleMouseMessage( eKey_Wheel, eKeyState_Up, m_PosValue );
#endif //NEW_USER_INTERFACE_UTIL
		}
	}
#if	defined WINDOWMODE
	if(GetActiveWindow() == NULL)
	{
		m_lDX = m_lDY = 0L;
		// Down, Up, Double Click은 순간에만 true가 되야되므로 false로 초기화 함.
		m_bLBtnUp = m_bRBtnUp = m_bMBtnUp
			= m_bLBtnDn = m_bRBtnDn = m_bMBtnDn
			= m_bLBtnDbl = m_bRBtnDbl = m_bMBtnDbl = false;

		m_ptCursor.x = m_ptCursor.y = 0;
		return;
	}
#endif
}

#ifdef NEW_USER_INTERFACE_UTIL

void CInput::RegisterInputMessageHandler( BoostWeak_Ptr(InputMessageHandler) handler, bool first )
{
	if( first )
		m_HandlerList.push_front( handler );
	else
		m_HandlerList.push_back( handler );
}


void CInput::HandleMouseMessage( eKeyType key, eKeyStateType type, int value )
{
    for( HandlerList::iterator iter = m_HandlerList.begin()
        ; iter != m_HandlerList.end(); )
    {
        HandlerList::iterator curIter = iter;
        ++iter;
        BoostWeak_Ptr(InputMessageHandler) handler = *curIter;
        if( handler.expired() == false )
        {
            if( handler.lock()->HandleInputMessage( key, type, value ) == true )
            {
                return;
            }
        }
        else
        {
            m_HandlerList.erase( curIter );
        }
    }
}

#endif //NEW_USER_INTERFACE_UTIL