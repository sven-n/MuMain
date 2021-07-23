//*****************************************************************************
// File: MsgWin.h
//
// Desc: interface for the CMsgWin class.
//		 메시지 창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_MSGWIN_H__4935BB60_ADC3_47E5_BA5F_832A03874FBA__INCLUDED_)
#define AFX_MSGWIN_H__4935BB60_ADC3_47E5_BA5F_832A03874FBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"

#include "Button.h"

#define	MW_MSG_LINE_MAX		2
#define	MW_MSG_ROW_MAX		52	// winmain.cpp의 nFixFontHeight가 14일 때 45, 13일 때 52

#ifdef LJH_ADD_LOCALIZED_BTNS
enum MSG_WIN_TYPE		// 메시지 윈도우 타잎.
{
	MWT_NON,			// 아무것도 없음.
		MWT_BTN_CANCEL,		// 취소 버튼만 있음.
		MWT_BTN_OK,			// 확인 버튼만 있음.
		MWT_BTN_BOTH,		// 확인, 취소 버튼 있음.
		MWT_STR_INPUT,		// 문자 입력.
		MWT_BTN_LOCALIZED_YESNO,			// (Localized)예, 아니오 버튼 있음
		MWT_BTN_LOCALIZED_CONFIRM_CANCEL,	// (Localized)확인,취소 버튼만 있음
		MWT_BTN_LOCALIZED_CONFIRM_ONLY,		// (Localized)확인 버튼만 있음
};
#endif //LJH_ADD_LOCALIZED_BTNS


class CMsgWin : public CWin  
{
protected:
#ifndef LJH_ADD_LOCALIZED_BTNS
	enum MSG_WIN_TYPE		// 메시지 윈도우 타잎.
	{
		MWT_NON,			// 아무것도 없음.
		MWT_BTN_CANCEL,		// 취소 버튼만 있음.
		MWT_BTN_OK,			// 확인 버튼만 있음.
		MWT_BTN_BOTH,		// 확인, 취소 버튼 있음.
		MWT_STR_INPUT,		// 문자 입력.
	};
#endif  //LJH_ADD_LOCALIZED_BTNS

	CSprite			m_sprBack;		// 메시지 윈도우 배경 스프라이트.
	CSprite			m_sprInput;		// 입력란 스프라이트.
	CButton			m_aBtn[2];		// OK, Cancel 버튼.
	char			m_aszMsg[MW_MSG_LINE_MAX][MW_MSG_ROW_MAX];	// 메시지 문자열.
	int				m_nMsgLine;		// 메시지 라인 수.
	int				m_nMsgCode;		// 메시지 코드.
	MSG_WIN_TYPE	m_eType;		// 메시지 윈도우 타잎.
	short			m_nGameExit;	// 게임종료 카운트.(-1 ~ 5)
	double			m_dDeltaTickSum;// 게임종료 카운트에 사용되는 시간차의 합.

public:
	CMsgWin();
	virtual ~CMsgWin();

#ifdef LJH_ADD_LOCALIZED_BTNS
	void Create(int p_iBtnType = 0);
#else  //LJH_ADD_LOCALIZED_BTNS
	void Create();
#endif //LJH_ADD_LOCALIZED_BTNS
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);
	void PopUp(int nMsgCode, char* pszMsg = NULL);

protected:
	void PreRelease();
	void UpdateWhileActive(double dDeltaTick);
	void RenderControls();

	void SetCtrlPosition();
	void SetMsg(MSG_WIN_TYPE eType, LPCTSTR lpszMsg, LPCTSTR lpszMsg2 = NULL);
	void ManageOKClick();
	void ManageCancelClick();
	void InitResidentNumInput();
	void RequestDeleteCharacter();

#ifdef LJH_ADD_ONETIME_PASSWORD
	void ClearLoginInfoFromInputBox();
	void SendingLoginInfoWOOPT();
	void ReturnToConnServerWin();
#endif //LJH_ADD_ONETIME_PASSWORD
};

#endif // !defined(AFX_MSGWIN_H__4935BB60_ADC3_47E5_BA5F_832A03874FBA__INCLUDED_)
