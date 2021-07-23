
#ifdef NEW_USER_INTERFACE_FONT

//*****************************************************************************
// File: Text.h
//
// Desc: interface for the CText class.
//		 텍스트 클래스.
//
// producer: Ahn Sang-Kyu (2007. 2. 6)
//*****************************************************************************

#if !defined(AFX_TEXT_H__187E89A7_433E_496C_994F_0D3B2BDE8144__INCLUDED_)
#define AFX_TEXT_H__187E89A7_433E_496C_994F_0D3B2BDE8144__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIBaseDef.h"
#include "TextBitmap.h"

#define	TEXT_MAX_LEN	170		// Text 최대 글자 수.
// "굴림체" 높이 12 기준으로 최대 표현할 수 있는 글자 수.
// "굴림체" 높이 12 미만은 글자를 알아보기 힘들다.

enum TEXT_TYPE { TT_NORMAL, TT_MONEY, TT_PASSWORD, TT_NUMBER };

extern unsigned int WindowHeight;

class CText  
{
protected:
// 텍스처.
	GLuint		m_uiTexID;				// 텍스처 ID.
	WORD*		m_pwTexImg;				// 텍스처 이미지 데이터.
	int			m_nTexWidth;			// Texture 너비.
	int			m_nTexHeight;			// Texture 높이.

// 버텍스.
	SScrCoord	m_aScrCoord[POS_MAX];	// 버텍스의 스크린 좌표.
	STexCoord	m_aTexCoord[POS_MAX];	// 버텍스의 텍스처 좌표.
	BYTE		m_byRed;				// 버텍스 컬러(R).
	BYTE		m_byGreen;				// 버텍스 컬러(G).
	BYTE		m_byBlue;				// 버텍스 컬러(B).
	BYTE		m_byAlpha;				// 버텍스 알파값.

	int			m_nDatumX;				// 정렬시 X좌표 기준점.

	char		m_szFont[LF_FACESIZE];	// 폰트 이름.
	int			m_nFontHeight;			// 폰트 높이.(픽셀 단위)
	int			m_nCharMaxWidth;		// 한 문자 최대 너비.(픽셀 단위)
	COLORREF	m_crTextColor;			// Text 색상.
	COLORREF	m_crStyleColor;			// 그림자(TS_SHADOW) 또는 외곽선(TS_OUTLINE) 색.
	TEXT_STYLE	m_eStyle;				// TS_NORMAL, TS_SHADOW, TS_OUTLINE 중 하나.
	TEXT_TYPE	m_eType;				// TT_NORMAL, TT_MONEY, TT_PASSWORD 중 하나.
	BYTE		m_byAlign;				// TA_LEFT, TA_RIGHT, TA_CENTER 중 하나.

	char		m_szText[TEXT_MAX_LEN + 1];// Text 버퍼.
	int			m_nTextMaxLen;			// Text 제한 글자 수.
	int			m_nTextWidth;			// Text 픽셀 너비.

	bool		m_bShow;				// 렌더 여부.

public:
	CText();
	virtual ~CText();

	void Create(LPCTSTR lpszFont, int nFontHeight, int nTextMaxLen,
		TEXT_STYLE eStyle = TS_SHADOW, TEXT_TYPE eType = TT_NORMAL);
	void Release();
//	void SetSize(int nWidth, int nHeight);
	void SetPosition(int nXCoord, int nYCoord, CHANGE_PRAM eChangedPram = XY);
	int GetXPos(){ return int(m_aScrCoord[LT].fX); }	// 정렬 방식이 바뀌면 글자수에 따라 달라짐.
	int GetYPos(){ return int(float(WindowHeight) - m_aScrCoord[LT].fY); }
	int GetWidth(){ return int(m_aScrCoord[RT].fX - m_aScrCoord[LT].fX); }
	int GetHeight(){ return int(m_aScrCoord[LT].fY - m_aScrCoord[LB].fY); }
	int GetTextWidth(){ return m_nTextWidth; }
	int GetTextHeight(){ return m_nFontHeight + m_eStyle; }

	void SetText(LPCTSTR lpszText);
	LPCTSTR GetText() const { return m_szText; }
	// Text 색 설정. RGB() 매크로 사용.
	void SetTextColor(COLORREF crTextColor,
		COLORREF crStyleColor = RGB(30, 30, 30))
	{
		m_crTextColor = crTextColor;
		m_crStyleColor = crStyleColor;
	}
	// pcrStyleColor가 NULL이면 pcrTextColor값만 얻음.
	void GetTextColor(COLORREF* pcrTextColor, COLORREF* pcrStyleColor = NULL)
	{
		*pcrTextColor = m_crTextColor;
		if (pcrStyleColor)
			*pcrStyleColor = m_crStyleColor;
	}
	void SetAlign(BYTE byAlign){ m_byAlign = byAlign; }

	void SetAlpha(BYTE byAlpha){ m_byAlpha = byAlpha; }
	BYTE GetAlpha(){ return m_byAlpha; }
	void SetColor(BYTE byRed, BYTE byGreen, BYTE byBlue)	// 버텍스 색 설정.
	{ m_byRed = byRed;	m_byGreen = byGreen;	m_byBlue = byBlue; }

	void Show(bool bShow = true){ m_bShow = bShow; }
	bool IsShow(){ return m_bShow; }

	void Render();
};

#endif // !defined(AFX_TEXT_H__187E89A7_433E_496C_994F_0D3B2BDE8144__INCLUDED_)

#endif //NEW_USER_INTERFACE_FONT