//*****************************************************************************
// File: LoginMainWin.h
//*****************************************************************************

#if !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
#define AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_

#pragma once

#include "Win.h"
#include "Button.h"

#define	LMW_BTN_MENU		0
#define	LMW_BTN_CREDIT		1
#define	LMW_BTN_MAX			2

class CLoginMainWin : public CWin
{
protected:
    CButton	m_aBtn[LMW_BTN_MAX];
    CSprite	m_sprDeco;

public:
    CLoginMainWin();
    virtual ~CLoginMainWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    bool CursorInWin(int nArea);

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();
};

#endif // !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
