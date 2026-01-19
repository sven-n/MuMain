//*****************************************************************************
// File: WinEx.h
//*****************************************************************************

#if !defined(AFX_WINEX_H__9E5A6837_4907_4DD0_95E8_14FFD2BF57DC__INCLUDED_)
#define AFX_WINEX_H__9E5A6837_4907_4DD0_95E8_14FFD2BF57DC__INCLUDED_

#pragma once

#include "Win.h"

#define	WINEX_BGSIDE_HEIGHT		8

#define	WE_BG_CENTER	0
#define WE_BG_TOP		1
#define	WE_BG_BOTTOM	2
#define	WE_BG_LEFT		3
#define	WE_BG_RIGHT		4
#define	WE_BG_MAX		5

class CWinEx : public CWin
{
protected:
    int			m_nBgSideMax;
    int			m_nBgSideMin;
    int			m_nBgSideNow;

    int			m_nBasisY;

public:
    CWinEx();
    virtual ~CWinEx();

    void Create(SImgInfo* aImgInfo, int nBgSideMin, int nBgSideMax);
    void Release();
    void SetPosition(int nXCoord, int nYCoord);
    int SetLine(int nLine);
    void SetSize(int nHeight);
    bool CursorInWin(int nArea);
    void Show(bool bShow);
    void Render();

protected:
    void CheckAdditionalState();
};

#endif // !defined(AFX_WINEX_H__9E5A6837_4907_4DD0_95E8_14FFD2BF57DC__INCLUDED_)
