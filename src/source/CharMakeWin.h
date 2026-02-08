//*****************************************************************************
// File: CharMakeWin.h
//*****************************************************************************

#if !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
#define AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_

#pragma once

#include "Win.h"
#include "Button.h"

#define	CMW_SPR_INPUT		0
#define	CMW_SPR_STAT		1
#define	CMW_SPR_DESC		2
#define	CMW_SPR_MAX			3

#define	CMW_DESC_LINE_MAX	2
#define	CMW_DESC_ROW_MAX	75

class CCharMakeWin : public CWin
{
protected:
    CWin	m_winBack;
    CSprite	m_asprBack[CMW_SPR_MAX];
    CButton	m_abtnJob[MAX_CLASS];
    CButton	m_aBtn[2];

    CLASS_TYPE		m_nSelJob;
    wchar_t	m_aszJobDesc[CMW_DESC_LINE_MAX][CMW_DESC_ROW_MAX];
    int		m_nDescLine;

public:
    CCharMakeWin();
    virtual ~CCharMakeWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    bool CursorInWin(int nArea);
    void UpdateDisplay();
protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();

    void RequestCreateCharacter();

    void SelectCreateCharacter();
    void UpdateCreateCharacter();
    void RenderCreateCharacter();
};

#endif // !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
