//*****************************************************************************
// File: SysMenuWin.h
//
// Desc: interface for the CSysMenuWin class.
//		 시스템 메뉴 창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_SYSMENUWIN_H__66B83E6D_1A2F_43FB_812F_78F24BB0EF6C__INCLUDED_)
#define AFX_SYSMENUWIN_H__66B83E6D_1A2F_43FB_812F_78F24BB0EF6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinEx.h"
#include "Button.h"

#define	SMW_BTN_GAME_END	0	// 게임 종료.
#define	SMW_BTN_SERVER_SEL	1	// 서버 선택 씬으로.
#define	SMW_BTN_OPTION		2	// 옵션 창.
#define	SMW_BTN_CLOSE		3	// 닫기.
#define	SMW_BTN_MAX			4	// 버튼 개수.

class CSysMenuWin : public CWin  
{
protected:
	CWinEx		m_winBack;			// 배경.(배경 이미지로만 사용)
	CButton		m_aBtn[SMW_BTN_MAX];// 버튼들.

public:
	CSysMenuWin();
	virtual ~CSysMenuWin();

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

#endif // !defined(AFX_SYSMENUWIN_H__66B83E6D_1A2F_43FB_812F_78F24BB0EF6C__INCLUDED_)
