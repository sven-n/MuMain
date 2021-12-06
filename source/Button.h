//*****************************************************************************
// File: Button.h
//*****************************************************************************

#if !defined(AFX_BUTTON_H__7A986F2A_91A7_4525_8790_AC6C2E95AE63__INCLUDED_)
#define AFX_BUTTON_H__7A986F2A_91A7_4525_8790_AC6C2E95AE63__INCLUDED_

#pragma once

#include "Sprite.h"

#define	BTN_UP				0
#define	BTN_DOWN			1
#define	BTN_ACTIVE			2
#define	BTN_DISABLE			3
#define	BTN_UP_CHECK		4
#define	BTN_DOWN_CHECK		5
#define	BTN_ACTIVE_CHECK	6
#define	BTN_DISABLE_CHECK	7
#define	BTN_IMG_MAX			8

class CButton : public CSprite  
{
protected:
	static	CButton*	m_pBtnHeld;

	bool	m_bEnable;
	bool	m_bActive;
	bool	m_bClick;
	bool	m_bCheck;

	int		m_anImgMap[BTN_IMG_MAX];

	char*	m_szText;					
	DWORD*	m_adwTextColorMap;
	DWORD	m_dwTextColor;
	float	m_fTextAddYPos;

public:
	CButton();
	virtual ~CButton();

	void Release();
	void Create(int nWidth, int nHeight, int nTexID, int nMaxFrame = 1,int nDownFrame = -1, int nActiveFrame = -1, int nDisableFrame = -1,int nCheckUpFrame = -1, int nCheckDownFrame = -1,int nCheckActiveFrame = -1, int nCheckDisableFrame = -1);
	void Update();
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
