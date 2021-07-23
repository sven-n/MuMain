//*****************************************************************************
// File: ServerSelWin.h
//
// Desc: interface for the CServerSelWin class.
//		 서버 선택 창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_SERVERSELWIN_H__E3C0BEAA_B9CF_42C4_964F_A5339999BA7B__INCLUDED_)
#define AFX_SERVERSELWIN_H__E3C0BEAA_B9CF_42C4_964F_A5339999BA7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"

#include "Button.h"
#include "GaugeBar.h"
#include "WinEx.h"

#define SSW_SERVER_G_MAX	21	// 서버 그룹 최대 개수.
#define SSW_SERVER_MAX		16	// 그룹내 서버 최대 개수.
#define SSW_DESC_LINE_MAX	2	// 서버 설명 줄 수.
#define SSW_DESC_ROW_MAX	83	// 한줄에 최대 글자(byte)수 + 1

#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#define SSW_LEFT_SERVER_G_MAX	10	// 좌측 서버 최대 갯수
#define SSW_RIGHT_SERVER_G_MAX	10	// 우측 서버 최대 갯수
#endif // KJH_ADD_SERVER_LIST_SYSTEM

class CServerGroup;

class CServerSelWin : public CWin
{
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
private:
	enum SERVER_SELECT_WIN
	{
		SERVER_GROUP_BTN_WIDTH	= 108,
		SERVER_GROUP_BTN_HEIGHT = 26,
		SERVER_BTN_WIDTH		= 193,
		SERVER_BTN_HEIGHT		= 26,
	};

protected:
	CButton		m_aServerGroupBtn[SSW_SERVER_G_MAX];	// 서버 그룹 버튼.
	CButton		m_aServerBtn[SSW_SERVER_MAX];			// 서버 버튼.
	CGaugeBar	m_aServerGauge[SSW_SERVER_MAX];			// 서버 혼잡 게이지.
	CSprite		m_aBtnDeco[2];					// 장식 스프라이트 좌우.
	CSprite		m_aArrowDeco[2];				// 화살표 스프라이트 좌우.
	CWinEx		m_winDescription;				// 서버 설명 배경.

	int			m_icntServerGroup;				// 서버그룹 갯수
	int			m_icntLeftServerGroup;			// 왼쪽 서버 그룹 갯수
	int			m_icntRightServerGroup;			// 오른쪽 서버그룹 갯수
	int			m_icntServer;					// 현재 선택된 서버그룹의 서버 갯수
	bool		m_bTestServerBtn;				// 테스트 서버 버튼이 있는가?
	
	int				m_iSelectServerBtnIndex;	// 선택된 서버버튼 인덱스
	CServerGroup*	m_pSelectServerGroup;		// 선택된 서버그룹
	
	unicode::t_char		m_szDescription[SSW_DESC_LINE_MAX][SSW_DESC_ROW_MAX];	// 서버 설명 문자열.
#else // KJH_ADD_SERVER_LIST_SYSTEM
protected:
	CButton		m_abtnServerG[SSW_SERVER_G_MAX];// 서버 그룹 버튼.
	CButton		m_abtnServer[SSW_SERVER_MAX];	// 서버 버튼.
	CGaugeBar	m_agbServer[SSW_SERVER_MAX];	// 서버 혼잡 게이지.
	CSprite		m_asprDeco[2];					// 장식 스프라이트 좌우.
	CSprite		m_asprArrow[2];					// 화살표 스프라이트 좌우.
	CWinEx		m_winDescBg;					// 서버 설명 배경.
	char		m_aszDesc[SSW_DESC_LINE_MAX][SSW_DESC_ROW_MAX];// 서버 설명 문자열.
	int			m_nServerGCount;				// 서버 그룹 개수.
	int			m_nSelServerG;	// 선택된 서버 그룹 인덱스.(-1이면 선택 없음)
	int			m_anServerListIndex[SSW_SERVER_G_MAX];// 서버 그룹 버튼의 ServerList[] 인덱스.
#endif // KJH_ADD_SERVER_LIST_SYSTEM

public:
	CServerSelWin();
	virtual ~CServerSelWin();

	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void UpdateDisplay();
	void Show(bool bShow);
	bool CursorInWin(int nArea);

protected:
	void PreRelease();
	void SetServerBtnPosition();
	void SetArrowSpritePosition();
	void ShowServerGBtns();
	void ShowDecoSprite();
	void ShowArrowSprite();
	void ShowServerBtns();

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void UpdateWhileActive(double dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void RenderControls();
};

#endif // !defined(AFX_SERVERSELWIN_H__E3C0BEAA_B9CF_42C4_964F_A5339999BA7B__INCLUDED_)
