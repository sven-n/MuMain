//*****************************************************************************
// File: Button.h
//
// Desc: interface for the CButton class.
//		 버튼 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_BUTTON_H__7A986F2A_91A7_4525_8790_AC6C2E95AE63__INCLUDED_)
#define AFX_BUTTON_H__7A986F2A_91A7_4525_8790_AC6C2E95AE63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Sprite.h"

// 버튼 이미지 정의.
#define	BTN_UP				0	// 보통 상태.
#define	BTN_DOWN			1	// 눌린 상태.
#define	BTN_ACTIVE			2	// 마우스 커서가 버튼 위에 있는 상태.
#define	BTN_DISABLE			3	// 사용 불가능 상태.
#define	BTN_UP_CHECK		4	// 이하 위 상태 + 체크 된 상태.
#define	BTN_DOWN_CHECK		5
#define	BTN_ACTIVE_CHECK	6
#define	BTN_DISABLE_CHECK	7
#define	BTN_IMG_MAX			8

class CButton : public CSprite  
{
protected:
	static	CButton*	m_pBtnHeld;		// 붙잡은 버튼의 포인터.

	bool	m_bEnable;					// 사용가능한지 여부.(여부에 따른 텍스처 변화)
	bool	m_bActive;					// 활성화 여부.(타 윈도우에 가린 부분이 마우스 좌표와 겹친다면 반응하면 안되므로 false여야 함.)
	bool	m_bClick;					// 클릭인가?
	bool	m_bCheck;					// 체크인가?(체크 버튼용)

	int		m_anImgMap[BTN_IMG_MAX];	// 버튼 이미지 맵.
	// 체크버튼 여부는 m_anImgMap[BTN_UP_CHECK]가 0 이상이면 체크 버튼임.

	char*	m_szText;					// 버튼위에 쓰여질 텍스트 문자열.
	DWORD*	m_adwTextColorMap;			// 텍스트 색 맵.
	DWORD	m_dwTextColor;				// 현재 텍스트 색.
	float	m_fTextAddYPos;				// 텍스트 Y 위치 보정 값.

public:
	CButton();
	virtual ~CButton();

	void Release();
	void Create(int nWidth, int nHeight, int nTexID, int nMaxFrame = 1,
		int nDownFrame = -1, int nActiveFrame = -1, int nDisableFrame = -1,
		int nCheckUpFrame = -1, int nCheckDownFrame = -1,
		int nCheckActiveFrame = -1, int nCheckDisableFrame = -1);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void Update();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void Update(double dDeltaTick = 0.0);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void Render();
	void Show(bool bShow = true);
	BOOL CursorInObject();
	void SetEnable(bool bEnable = true) { m_bEnable = bEnable; }
	bool IsEnable(){ return m_bEnable; }
	void SetActive(bool bActive = true) { m_bActive = bActive; }
//	bool IsActive(){ return m_bActive; }
	bool IsClick(){ return m_bClick; }
	void SetCheck(bool bCheck = true) { m_bCheck = bCheck; }
	bool IsCheck(){ return m_bCheck; }
	void SetText(const char* pszText, DWORD* adwColor);
	char* GetText() const { return m_szText; }

protected:
	void ReleaseText();
};

#endif // !defined(AFX_BUTTON_H__7A986F2A_91A7_4525_8790_AC6C2E95AE63__INCLUDED_)
