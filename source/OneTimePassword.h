// OneTimePassword.h: interface for the COneTimePassword class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ONETIMEPASSWORD_H__95395C89_543D_4E0E_BE58_E12713D8A301__INCLUDED_)
#define AFX_ONETIMEPASSWORD_H__95395C89_543D_4E0E_BE58_E12713D8A301__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"

#include "Button.h"

class CUITextInputBox;

class COneTimePassword : public CWin 
{
protected:
	CSprite		m_sprInputBox;	// 입력 박스 스프라이트.
	CButton		m_aBtn[2];		// 확인, 취소(localized) 버튼.
	CUITextInputBox*	m_pOTPInputBox;

public:
	COneTimePassword();
	virtual ~COneTimePassword();

	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);

	void ConnectConnectionServer();

	CUITextInputBox* GetOTPInputBox() const { return m_pOTPInputBox; }

protected:
	void PreRelease();
	void UpdateWhileActive(double dDeltaTick);
	void UpdateWhileShow(double dDeltaTick);
	void RenderControls();

	void CancelInputOTP();
	void SendingOTP();

	void ClearLoginInfoFromInputBox();
};

#endif // !defined(AFX_ONETIMEPASSWORD_H__95395C89_543D_4E0E_BE58_E12713D8A301__INCLUDED_)

