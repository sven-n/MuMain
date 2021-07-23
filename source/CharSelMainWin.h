//*****************************************************************************
// File: CharSelMainWin.h
//
// Desc: interface for the CCharSelMainWin class.
//		 캐릭터 선택씬 메인 창 클래스.(화면 하단의 메뉴)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_CHARSELMAINWIN_H__50FDE1CA_28E7_4709_A90A_726EDD2517C2__INCLUDED_)
#define AFX_CHARSELMAINWIN_H__50FDE1CA_28E7_4709_A90A_726EDD2517C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"
#include "Button.h"

#define	CSMW_SPR_DECO			0	// 우측 장식.
#define	CSMW_SPR_INFO			1	// 하단 서비스 정보.
#define	CSMW_SPR_MAX			2	// 스프라이트 개수.

#define	CSMW_BTN_CREATE			0	// 캐릭터 생성 버튼.
#define	CSMW_BTN_MENU			1	// 메뉴 버튼.
#define	CSMW_BTN_CONNECT		2	// 게임 시작 버튼.
#define	CSMW_BTN_DELETE			3	// 캐릭터 삭제 버튼.
#define	CSMW_BTN_MAX			4	// 버튼 개수.

class CCharSelMainWin : public CWin  
{
protected:
	CSprite	m_asprBack[CSMW_SPR_MAX];	// 스프라이트.
	CButton	m_aBtn[CSMW_BTN_MAX];		// 버튼.
	bool	m_bAccountBlockItem;		// 계정 블럭 아이템?
#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef TEENAGER_REGULATION
	short	m_nWarningText;				// 경고 문장 번호.
#endif // TEENAGER_REGULATION
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if (SELECTED_LANGUAGE == LANGUAGE_KOREAN) && defined TEENAGER_REGULATION
	short	m_nWarningText;				// 경고 문장 번호.
#endif	// (SELECTED_LANGUAGE == LANGUAGE_KOREAN) && defined TEENAGER_REGULATION
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

public:
	CCharSelMainWin();
	virtual ~CCharSelMainWin();
	
	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);
	void UpdateDisplay();

protected:
	void PreRelease();
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive(double dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void RenderControls();

	void DeleteCharacter();
};

#endif // !defined(AFX_CHARSELMAINWIN_H__50FDE1CA_28E7_4709_A90A_726EDD2517C2__INCLUDED_)
