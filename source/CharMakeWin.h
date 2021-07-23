//*****************************************************************************
// File: CharMakeWin.h
//
// Desc: interface for the CCharMakeWin class.
//		 캐릭터 생성창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
#define AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"
#include "Button.h"

#define	CMW_SPR_INPUT		0	// 이름 입력 배경 스프라이트.
#define	CMW_SPR_STAT		1	// 능력치 배경 스프라이트.
#define	CMW_SPR_DESC		2	// 직업 설명 배경 스프라이트.
#define	CMW_SPR_MAX			3	// 스프라이트 개수.

#define	CMW_DESC_LINE_MAX	2	// 직업 설명 줄 수.
#define	CMW_DESC_ROW_MAX	75	// 직업 설명 한 줄 글자 수.

class CCharMakeWin : public CWin  
{
protected:
	CWin	m_winBack;				// 아무것도 안보이는 배경 윈도우.
	CSprite	m_asprBack[CMW_SPR_MAX];// 스프라이트.
	CButton	m_abtnJob[MAX_CLASS];	// 직업 버튼.
	CButton	m_aBtn[2];				// OK, Cancel 버튼.

	int		m_nSelJob;				// 선택된 직업.
	char	m_aszJobDesc[CMW_DESC_LINE_MAX][CMW_DESC_ROW_MAX];// 직업 설명.
	int		m_nDescLine;			// 현재 직업 설명 줄 수.

public:
	CCharMakeWin();
	virtual ~CCharMakeWin();

	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);
	void UpdateDisplay();
#ifdef PJH_CHARACTER_RENAME
	bool ReName_Inter;
	void Set_State(bool Set = false);
#endif //PJH_CHARACTER_RENAME

protected:
	void PreRelease();
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive(double dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void RenderControls();

	void RequestCreateCharacter();

	void SelectCreateCharacter();
	void UpdateCreateCharacter();
	void RenderCreateCharacter();
};

#endif // !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
