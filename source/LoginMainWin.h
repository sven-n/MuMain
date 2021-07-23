//*****************************************************************************
// File: LoginMainWin.h
//
// Desc: interface for the CLoginMainWin class.
//		 로그인 씬(서버 선택 씬) 메인 창 클래스.(화면 하단의 메뉴)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
#define AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"
#include "Button.h"

#define	LMW_BTN_MENU		0	// 메뉴 버튼.
#define	LMW_BTN_CREDIT		1	// 크레딧 버튼.
#ifdef MOVIE_DIRECTSHOW
#define	LMW_BTN_MOVIE		2	// 동영상 버튼.
#define	LMW_BTN_MAX			3	// 버튼 개수.
#else	// MOVIE_DIRECTSHOW
#define	LMW_BTN_MAX			2	// 버튼 개수.
#endif	// MOVIE_DIRECTSHOW

class CLoginMainWin : public CWin  
{
protected:
	CButton	m_aBtn[LMW_BTN_MAX];	// 버튼.
	CSprite	m_sprDeco;				// 우측 하단 장식 버튼.

public:
	CLoginMainWin();
	virtual ~CLoginMainWin();

	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);

protected:
	void PreRelease();
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive(double dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void RenderControls();
};

#endif // !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
