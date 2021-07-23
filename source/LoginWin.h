//*****************************************************************************
// File: LoginWin.h
//
// Desc: interface for the CLoginWin class.
//		 로그인 창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_LOGINWIN_H__17032735_0A4C_49F2_A436_00A9311FFAB4__INCLUDED_)
#define AFX_LOGINWIN_H__17032735_0A4C_49F2_A436_00A9311FFAB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"

#include "Button.h"

class CUITextInputBox;

class CLoginWin : public CWin  
{
protected:
	CSprite		m_asprInputBox[2];	// 입력 박스 스프라이트.
	CButton		m_aBtn[2];			// OK, Cancel 버튼.
	CUITextInputBox*	m_pIDInputBox, * m_pPassInputBox;

public:
	CLoginWin();
	virtual ~CLoginWin();

	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);

	void ConnectConnectionServer();

	CUITextInputBox* GetIDInputBox() const { return m_pIDInputBox; }
	CUITextInputBox* GetPassInputBox() const { return m_pPassInputBox; }

protected:
	void PreRelease();
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive();
	void UpdateWhileShow();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive(double dDeltaTick);
	void UpdateWhileShow(double dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void RenderControls();

	void RequestLogin();
	void CancelLogin();
};

#endif // !defined(AFX_LOGINWIN_H__17032735_0A4C_49F2_A436_00A9311FFAB4__INCLUDED_)
