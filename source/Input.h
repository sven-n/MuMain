//*****************************************************************************
// File: Input.h
//
// Desc: interface for the CInput class.
//		 Singleton 패턴.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_INPUT_H__56D42544_2BF6_4B84_B368_F36CACDFEF1E__INCLUDED_)
#define AFX_INPUT_H__56D42544_2BF6_4B84_B368_F36CACDFEF1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include "NewUICommon.h"
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

class CInput
{
protected:
	HWND	m_hWnd;

	POINT	m_ptCursor;			// 마우스 커서 좌표.
	long	m_lDX;				// X축 변화량.
	long	m_lDY;				// Y축 변화량.
//	long	m_lDZ;				// Z축 변화량(mouse의 휠).
	bool	m_bLBtnDn;			// 왼쪽 버튼 Down.
	bool	m_bLBtnHeldDn;		// 왼쪽 버튼 Down 유지.
	bool	m_bLBtnUp;			// 왼쪽 버튼 Up.
	bool	m_bLBtnDbl;			// 왼쪽 버튼 더블 클릭.
	bool	m_bRBtnDn;			// 오른쪽 버튼 Down.
	bool	m_bRBtnHeldDn;		// 오른쪽 버튼 Down 유지.
	bool	m_bRBtnUp;			// 오른쪽 버튼 Up.
	bool	m_bRBtnDbl;			// 오른쪽 버튼 더블 클릭.
	bool	m_bMBtnDn;			// 가운데(휠) 버튼 Down.
	bool	m_bMBtnHeldDn;		// 가운데(휠) 버튼 Down 유지.
	bool	m_bMBtnUp;			// 가운데(휠) 버튼 Up.
	bool	m_bMBtnDbl;			// 가운데(휠) 버튼 더블 클릭.

	long	m_lScreenWidth;		// 스크린(BackBuffer) 가로 크기.
	long	m_lScreenHeight;	// 스크린(BackBuffer) 세로 크기.
	bool	m_bLeftHand;		// 왼손잡이인가?
	bool	m_bTextEditMode;	// 채팅 모드인가?(TextEdit중.)

	POINT	m_ptFormerCursor;		// 이전 마우스 커서 좌표.
	double	m_dDoubleClickTime;		// 더블 클릭 시간.
	double	m_dBtn0LastClickTime;	// 버튼0이 마지막으로 클릭한 시간.
	double	m_dBtn1LastClickTime;	// 버튼1이 마지막으로 클릭한 시간.
	double	m_dBtn2LastClickTime;	// 버튼2가 마지막으로 클릭한 시간.
	bool	m_bFormerBtn0Dn;		// 이전에 버튼0가 Down이었나?
	bool	m_bFormerBtn1Dn;		// 이전에 버튼1가 Down이었나?
	bool	m_bFormerBtn2Dn;		// 이전에 버튼2가 Down이었나?
	
#ifdef NEW_USER_INTERFACE_UTIL
	typedef std::list< BoostWeak_Ptr(InputMessageHandler) >   HandlerList;
	
    HandlerList										m_HandlerList;
	int												m_PosValue;
#endif //NEW_USER_INTERFACE_UTIL

protected:						// 생성자를 protected로 선언해서
	CInput();					//외부에서의 생성을 강제로 막음.

public:
	virtual ~CInput();

	static CInput& Instance();
	bool Create(HWND hWnd, long lScreenWidth, long lScreenHeight);
	void Update();

	// 키를 눌렀는가?(누르는 순간 true)
	// 이 함수는 아직 불안 함. 키를 누른 후 첫 프레임에 true가 되고 다음 프레임
	//이후엔 당분간 false가 되지만 곧 true로 고정. 아마도 메모장 등에서 키를 누
	//르고 있으면 당분간 문자가 안찍히다가 연속으로 찍히는 것과 관련이 있을 듯.
	bool IsKeyDown(int nVirtualKeyCode)
	{
		if (m_bTextEditMode)
			return false;
		return SEASON3B::IsPress(nVirtualKeyCode);
	}
	// 키를 누루고 있는가?
	bool IsKeyHeldDown(int nVirtualKeyCode)
	{
		if (m_bTextEditMode)
			return false;
		return SEASON3B::IsRepeat(nVirtualKeyCode);
	}

	POINT GetCursorPos(){ return m_ptCursor; }
	long GetCursorX(){ return m_ptCursor.x; }
	long GetCursorY(){ return m_ptCursor.y; }
	long GetDX(){ return m_lDX; }
	long GetDY(){ return m_lDY; }
	bool IsLBtnDn(){ return m_bLBtnDn; }
	bool IsLBtnHeldDn(){ return m_bLBtnHeldDn; }
	bool IsLBtnUp(){ return m_bLBtnUp; }
	bool IsLBtnDbl(){ return m_bLBtnDbl; }
	bool IsRBtnDn(){ return m_bRBtnDn; }
	bool IsRBtnHeldDn(){ return m_bRBtnHeldDn; }
	bool IsRBtnUp(){ return m_bRBtnUp; }
	bool IsRBtnDbl(){ return m_bRBtnDbl; }
	bool IsMBtnDn(){ return m_bMBtnDn; }
	bool IsMBtnHeldDn(){ return m_bMBtnHeldDn; }
	bool IsMBtnUp(){ return m_bMBtnUp; }
	bool IsMBtnDbl(){ return m_bMBtnDbl; }

	long GetScreenWidth(){ return m_lScreenWidth; }
	long GetScreenHeight(){ return m_lScreenHeight; }
	void SetLeftHandMode(bool bLeftHand){ m_bLeftHand = bLeftHand; }
	bool IsLeftHandMode(){ return m_bLeftHand; }
	void SetTextEditMode(bool bTextEditMode){ m_bTextEditMode = bTextEditMode; }
	bool IsTextEditMode(){ return m_bTextEditMode; }

#ifdef NEW_USER_INTERFACE_UTIL
public:
	void RegisterInputMessageHandler( BoostWeak_Ptr(InputMessageHandler) handler, bool first = false );
	
private:
	//Mouse
	void HandleMouseMessage( eKeyType key, eKeyStateType type, int value );
#endif //NEW_USER_INTERFACE_UTIL
};

#endif // !defined(AFX_INPUT_H__56D42544_2BF6_4B84_B368_F36CACDFEF1E__INCLUDED_)
