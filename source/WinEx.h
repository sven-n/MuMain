//*****************************************************************************
// File: WinEx.h
//
// Desc: interface for the CWinEx class.
//		 확장(상하로 늘어나는) 윈도우 class.(CWin에서 상속)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_WINEX_H__9E5A6837_4907_4DD0_95E8_14FFD2BF57DC__INCLUDED_)
#define AFX_WINEX_H__9E5A6837_4907_4DD0_95E8_14FFD2BF57DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"

#define	WINEX_BGSIDE_HEIGHT		8

#define	WE_BG_CENTER	0	// 중앙 바탕 스트라이트.
#define WE_BG_TOP		1	// 상단 바탕 스프라이트.
#define	WE_BG_BOTTOM	2	// 하단 바탕 스프라이트.
#define	WE_BG_LEFT		3	// 좌측 바탕 스프라이트.
#define	WE_BG_RIGHT		4	// 우측 바탕 스프라이트.
#define	WE_BG_MAX		5	// WinEx 바탕 스프라이트 최대 개수.

class CWinEx : public CWin  
{
protected:
	int			m_nBgSideMax;	// 좌우측 스프라이트의 최대 개수.
	int			m_nBgSideMin;	// 좌우측 스프라이트의 최소 개수.
	int			m_nBgSideNow;	// 좌우측 스프라이트의 현재 개수.

	int			m_nBasisY;		// 기준 Y위치.

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
