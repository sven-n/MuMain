//*****************************************************************************
// File: SysMenuWin.h
//*****************************************************************************

#if !defined(AFX_SYSMENUWIN_H__66B83E6D_1A2F_43FB_812F_78F24BB0EF6C__INCLUDED_)
#define AFX_SYSMENUWIN_H__66B83E6D_1A2F_43FB_812F_78F24BB0EF6C__INCLUDED_

#pragma once

#include "WinEx.h"
#include "Button.h"

#define	SMW_BTN_GAME_END	0
#define	SMW_BTN_SERVER_SEL	1
#define	SMW_BTN_OPTION		2
#define	SMW_BTN_CLOSE		3
#define	SMW_BTN_MAX			4

class CSysMenuWin : public CWin  
{
protected:
	CWinEx		m_winBack;
	CButton		m_aBtn[SMW_BTN_MAX];

public:
	CSysMenuWin();
	virtual ~CSysMenuWin();

	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);

protected:
	void PreRelease();
	void UpdateWhileActive(double dDeltaTick);
	void RenderControls();
};

#endif // !defined(AFX_SYSMENUWIN_H__66B83E6D_1A2F_43FB_812F_78F24BB0EF6C__INCLUDED_)
