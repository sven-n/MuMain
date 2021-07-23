//*****************************************************************************
// File: Win.h
//
// Desc: interface for the CWin class.
//		 윈도우 기초 class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_WIN_H__A06C4911_F775_4A87_97B9_42FE76BC1DEB__INCLUDED_)
#define AFX_WIN_H__A06C4911_F775_4A87_97B9_42FE76BC1DEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIBaseDef.h"
#include "PList.h"
#include "Sprite.h"

// 윈도우 상태.
#define	WS_NORMAL				0
#define	WS_ETC					1
#define	WS_MOVE					2
#define	WS_EXTEND_DN			3	// CWinEx용.
#define	WS_EXTEND_UP			4	// CWinEx용.

// 윈도우 반응 영역
#define	WA_ALL					0
#define	WA_TOOLTIP				1
#define	WA_MOVE					2
#define	WA_BUTTON				3
#define	WA_EXTEND_DN			4	// CWinEx용.
#define	WA_EXTEND_UP			5	// CWinEx용.

class CSprite;
class CButton;

class CWin  
{
protected:
	POINT		m_ptHeld;			// 커서로 잡을 당시 커서 좌표.

	bool		m_bShow;			// 보이는지 여부.
	bool		m_bActive;			// 활성화 여부.
	bool		m_bDocking;			// 도킹 여부.
	int			m_nState;			// 윈도우 상태.

	CSprite*	m_psprBg;			// 바탕 스프라이트.
	POINT		m_ptPos;			// 윈도우 위치.
	SIZE		m_Size;				// 윈도우 크기.
	POINT		m_ptTemp;			// 임시 위치.(도킹 시 사용)

	CPList		m_BtnList;			// 자동화를 위한 버튼 포인터 리스트.

public:
	CWin();
	virtual ~CWin();

	void Create(int nWidth, int nHeight, int nTexID = -1, bool bTile = false);
	virtual void Release();
	virtual void SetPosition(int nXCoord, int nYCoord);
	int GetXPos(){ return m_ptPos.x; }
	int GetYPos(){ return m_ptPos.y; }
	void SetSize(int nWidth, int nHeight, CHANGE_PRAM eChangedPram = XY);
	int GetWidth(){ return m_Size.cx; }
	int GetHeight(){ return m_Size.cy; }
	int GetTempXPos(){ return m_ptTemp.x; }
	int GetTempYPos(){ return m_ptTemp.y; }
	virtual bool CursorInWin(int nArea);
	virtual void Show(bool bShow);
	bool IsShow(){ return m_bShow; }
	virtual void Active(bool bActive) { m_bActive = bActive; }
	bool IsActive(){ return m_bActive; }
	void SetDocking(bool bDocking) { m_bDocking = bDocking; }
//	bool IsDocking(){ return m_bDocking; }
	int GetState(){ return m_nState; }
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void Update();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void Update(double dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void Render();

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual int SetLine()			{ return 0; }
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual int SetLine(int nLine)	{ return 0; }
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void ActiveBtns(bool bActive);	// 파생 클래스에서 재정의 할 필요 없다. CUIMng에서만 쓰임.


	BYTE GetBgAlpha() {return m_psprBg->GetAlpha(); }
	void SetBgAlpha(BYTE byAlpha) { m_psprBg->SetAlpha(byAlpha); }
	void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue)
	{ m_psprBg->SetColor(byRed, byGreen, byBlue); }

protected:
// 필요시 파생 클래스에서 재정의 함수.(CCharSelMainWin의 각 함수 주석 참조)
	// 파생 클래스 데이터 멤버의 Release().
	virtual void PreRelease() {};
	// 버튼 영역 체크와 버튼 위 마우스 커서 변화와 동일한 효과를 주기위해 이 함수를 재정의 함.
	virtual bool CursorInButtonlike() { return false; }
	// 보일 때의 업데이트.(액티브 상태 여부와 관계 없는 부분 처리.)
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void UpdateWhileShow()					{};
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void UpdateWhileShow(double dDeltaTick)	{};
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	// 추가 윈도우 상태 체크.
	virtual void CheckAdditionalState() {};
	// 액티브일 때의 업데이트.
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void UpdateWhileActive()					{};
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void UpdateWhileActive(double dDeltaTick)	{};
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void RenderControls() {};

	void RegisterButton(CButton* pBtn);
	void RenderButtons();
};

#endif // !defined(AFX_WIN_H__A06C4911_F775_4A87_97B9_42FE76BC1DEB__INCLUDED_)
