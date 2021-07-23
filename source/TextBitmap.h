
#ifdef NEW_USER_INTERFACE_FONT

//*****************************************************************************
// File: TextBitmap.h
//
// Desc: interface for the CTextBitmap class.
//		 글자를 쓸 비트맵, 폰트 관리 클래스. Singleton 패턴.
//
// producer: Ahn Sang-Kyu (2007. 2. 6)
//*****************************************************************************

#if !defined(AFX_TEXTBITMAP_H__B29A08B3_BF8F_4C9D_BA9C_23A24AC4C0A5__INCLUDED_)
#define AFX_TEXTBITMAP_H__B29A08B3_BF8F_4C9D_BA9C_23A24AC4C0A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <string>

using namespace std;

#define	TB_WIDTH			1024	// 비트맵의 너비.
#define TB_HEIGHT			32		// 비트맵의 높이.

enum TEXT_STYLE { TS_NORMAL, TS_SHADOW, TS_OUTLINE };

class CTextBitmap  
{
friend class CText;

	// 폰트 정보 구조체.
	struct SFontInfo
	{
		HFONT	hFont;				// 폰트 핸들.
		int		nCharMaxWidth;		// 문자 최대 너비.
	};

	typedef map<string, SFontInfo> FontMap;
	typedef FontMap::iterator FontMapItor;

protected:
	HDC		m_hDC;						// DC 핸들.
	HBITMAP	m_hBitmap;					// 비트맵 핸들.
	WORD*	m_pwBitmap;					// 비트맵의 포인터.
	FontMap	m_mapFont;					// 폰트 정보 맵.
	char	m_szSelFont[LF_FACESIZE];	// 선택된 폰트 이름.
	int		m_nSelFontHeight;			// 선택된 폰트 높이.(픽셀 단위)
	
protected:
	CTextBitmap();

public:
	virtual ~CTextBitmap();

	static CTextBitmap& Instance();
	void Create();
	void Release();

protected:
// 이하 함수들은 CText 클래스에서만 사용함.
	int AddFont(LPCTSTR lpszFace, int nHeight);
	void SelFont(LPCTSTR lpszFace, int nHeight);
	int TextOut(LPCTSTR lpszText, COLORREF crColor, TEXT_STYLE eStyle,
		COLORREF crStyleColor);
	WORD* GetBitmap() const { return m_pwBitmap; }
};

#endif // !defined(AFX_TEXTBITMAP_H__B29A08B3_BF8F_4C9D_BA9C_23A24AC4C0A5__INCLUDED_)

#endif //NEW_USER_INTERFACE_FONT