//*****************************************************************************
// File: ServerSelWin.h
//*****************************************************************************
#pragma once

#include "Win.h"
#include "Button.h"
#include "GaugeBar.h"
#include "WinEx.h"

#define SSW_SERVER_G_MAX	21
#define SSW_SERVER_MAX		16
#define SSW_DESC_LINE_MAX	2
#define SSW_DESC_ROW_MAX	83
#define SSW_LEFT_SERVER_G_MAX	10
#define SSW_RIGHT_SERVER_G_MAX	10


class CServerGroup;

class CServerSelWin : public CWin
{
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
	void UpdateWhileActive(double dDeltaTick);
	void RenderControls();
};

