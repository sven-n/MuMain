//*****************************************************************************
// File: TextBitmap.cpp
//
// Desc: implementation of the CTextBitmap class.
//
// producer: Ahn Sang-Kyu (2007. 2. 6)
//*****************************************************************************

#include "stdafx.h"
#include "TextBitmap.h"
#include <crtdbg.h>  // _ASSERT() 사용.

#ifdef NEW_USER_INTERFACE_FONT

extern DWORD g_dwCharSet[];
// 그 나라에 맞는 캐릭터셋으로 변경해야 함.
// 한국 : HANGUL_CHARSET
// 중국 : GB2312_CHARSET
// 일본 : SHIFTJIS_CHARSET
// 대만 : CHINESEBIG5_CHARSET
#define	TB_CHARSET	(g_dwCharSet[SELECTED_LANGUAGE])

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTextBitmap::CTextBitmap() : m_hDC(NULL), m_hBitmap(NULL)
{
}

CTextBitmap::~CTextBitmap()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Instance()
// 함수 설명 : 객체를 단 하나만 생성.
//			  이 클래스에 접근하기 위한 인터페이스임
//*****************************************************************************
CTextBitmap& CTextBitmap::Instance()
{
	static CTextBitmap s_TextBitmap;                  
    return s_TextBitmap;
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 비트맵 생성 및 초기화.
//*****************************************************************************
void CTextBitmap::Create()
{
	Release();	// DC핸들이 있으면 삭제.

	m_hDC = ::CreateCompatibleDC(NULL);	// DC 생성.

	// 비트맵 정보 구조체의 헤더 채우기.
	BITMAPINFO bmi;
	::memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = TB_WIDTH;
	// Height가 -인 이유는 비트맵은 상하 반전되어 나타나기 때문.
	bmi.bmiHeader.biHeight      = -TB_HEIGHT;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount    = 16;	// 16bit 칼라임.

	// 비트맵을 생성하고 비트맵 핸들과 비트맵 포인터(m_pwBitmap)를 얻음.
	m_hBitmap = ::CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS,
		(void**)&m_pwBitmap, NULL, 0);

	::SelectObject(m_hDC, m_hBitmap);		// 비트맵 선택.
	// 비트맵 선택 후 삭제해도 비트맵 포인터만 있으면 됨.

	::SetBkMode(m_hDC, TRANSPARENT);	// 배경 투명.
	::SetTextAlign(m_hDC, TA_TOP);		// Text 윗쪽 정렬.

	::strcpy(m_szSelFont, "");			// 선택된 폰트 이름.
	m_nSelFontHeight = 0;				// 선택된 폰트 높이.(픽셀 단위)
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : DC, 비트맵, 폰트 정보 맵 삭제.
//*****************************************************************************
void CTextBitmap::Release()
{
	if (m_hDC)
	{
		::DeleteDC(m_hDC);			// DC 삭제.
		m_hDC = NULL;
	}

	if (m_hBitmap)
	{
		::DeleteObject(m_hBitmap);	// 비트맵 삭제.
		m_hBitmap = NULL;
	}

	if (!m_mapFont.empty())
	{
		// 폰트 정보 맵에 있는 폰트들 삭제.
		for (FontMapItor i = m_mapFont.begin(); i != m_mapFont.end(); ++i)
			::DeleteObject(i->second.hFont);

		m_mapFont.clear();			// 폰트 정보 맵 삭제.
	}
}

//*****************************************************************************
// 함수 이름 : AddFont()
// 함수 설명 : 폰트 맵에 폰트 추가. 리턴값은 문자 최대 너비.(픽셀 단위)
// 매개 변수 : lpszFace			: 폰트 이름 문자열. (예 "굴림")
//			   nHeight			: 폰트 높이.(픽셀 단위)
//*****************************************************************************
int CTextBitmap::AddFont(LPCTSTR lpszFace, int nHeight)
{
	// 폰트 퐆이는 TB_HEIGHT 이하여야 됨.
	_ASSERT(0 < nHeight && TB_HEIGHT >= nHeight);
	_ASSERT(m_hDC);

	// 키(폰트 이름 + 폰트 높이) 생성.
	char szMapKey[LF_FACESIZE + 2];
	::sprintf(szMapKey, "%s%d", lpszFace, nHeight);
	// 폰트가 맵에 있는지 검색.
	FontMapItor itor = m_mapFont.find(szMapKey);
	if (itor != m_mapFont.end())	// 있으면.
		return itor->second.nCharMaxWidth;
	else							// 없다면.
	{
		SFontInfo fi;

	// 폰트 생성.
		LOGFONT lf;
		::memset(&lf, 0, sizeof(LOGFONT));
		lf.lfHeight = nHeight;
		lf.lfWeight = FW_MEDIUM;
		lf.lfCharSet = TB_CHARSET;
		::_tcscpy(lf.lfFaceName, _T(lpszFace));

		fi.hFont = ::CreateFontIndirect(&lf);

	// 가장 큰 문자의 너비를 얻음.(한자, 한글 등 2byte 문자는 제외)
		// 가장 큰 문자의 너비를 얻기 위해 폰트 선택.
		::SelectObject(m_hDC, fi.hFont);

		// 해외 폰트 경우엔 TEXTMETRIC 구조체의 tmMaxCharWidth는 부정확(원하는
		//크기보다 큼)하므로 아래와 같이 함.
		const int nStrLen = 94;
		char szTestChars[nStrLen + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijk\
lmnopqrstuvwxyz`1234567890-=\\[];',./~!@#$%^&*()_+|{}:\"<>?";
		int anDx[nStrLen];	// 각 문자의 위치가 저장 될 곳.
		SIZE size;			// 필요없지만 아래 함수에서 반드시 넣어줘야 함.
		// szTestChars[]의 각 문자 위치를 구하여 anDx[]에 넣어줌.
		::GetTextExtentExPoint(m_hDC, szTestChars, nStrLen, 0, NULL, anDx,
			&size);
		// 너비가 가장 큰 문자의 길이를 얻음.
		fi.nCharMaxWidth = anDx[0];
		int nCharTempWidth;
		for (int i = 1; i < nStrLen; ++i)
		{
			nCharTempWidth = anDx[i] - anDx[i - 1];
			if (fi.nCharMaxWidth < nCharTempWidth)
				fi.nCharMaxWidth = nCharTempWidth;
		}

	// 맵에 폰트 정보 추가.
		m_mapFont[szMapKey] = fi;

	// 선택된 폰트의 정보 저장.
		::strcpy(m_szSelFont, lpszFace);		// 선택된 폰트 이름.
		m_nSelFontHeight = nHeight;				// 선택된 폰트 높이.

	// 리턴 값.
		return fi.nCharMaxWidth;
	}
}

//*****************************************************************************
// 함수 이름 : SelFont()
// 함수 설명 : 폰트 선택.
// 매개 변수 : lpszFace	: 폰트 이름 문자열. (예 "굴림")
//			   nHeight	: 폰트 높이.(픽셀 단위)
//*****************************************************************************
void CTextBitmap::SelFont(LPCTSTR lpszFace, int nHeight)
{
	// 폰트 퐆이는 TB_HEIGHT 이하여야 됨.
	_ASSERT(0 < nHeight && TB_HEIGHT >= nHeight);
	_ASSERT(m_hDC);

	// 선택할 폰트 이름과 폰트 높이가 전에 선택되었던 것들과 같다면 return.
	if (0 == ::strcmp(m_szSelFont, lpszFace) && m_nSelFontHeight == nHeight)
		return;

	// 키(폰트 이름 + 폰트 높이)를 생성하여 맵에서 검색.
	char szMapKey[LF_FACESIZE + 2];
	::sprintf(szMapKey, "%s%d", lpszFace, nHeight);
	FontMapItor itor = m_mapFont.find(szMapKey);
	_ASSERT(itor != m_mapFont.end());	// 맵에 없으면 안됨.

	// DC에 폰트 선택.
	::SelectObject(m_hDC, itor->second.hFont);

	::strcpy(m_szSelFont, lpszFace);	// 선택된 폰트 이름.
	m_nSelFontHeight = nHeight;			// 선택된 폰트 높이
}

//*****************************************************************************
// 함수 이름 : TextOut()
// 함수 설명 : 비트맵에 글자를 씀. 리턴값은 문자열의 픽셀 너비.
// 매개 변수 : lpszText		: 쓸 문자열.
//			   crTextColor	: Text 색. RGB() 매크로 사용.
//			   eStyle		: TS_NORMAL, TS_SHADOW, TS_OUTLINE 중 하나.
//			   crStyleColor	: 그림자(TS_SHADOW) 또는 외곽선(TS_OUTLINE) 색.
//							RGB() 매크로 사용.
//*****************************************************************************
int CTextBitmap::TextOut(LPCTSTR lpszText, COLORREF crColor, TEXT_STYLE eStyle,
						  COLORREF crStyleColor)
{
	_ASSERT(m_hDC);

	int nStrLen = ::strlen(lpszText);	// 문자열 길이.
	SIZE size;

	switch (eStyle)
	{
	case TS_NORMAL :	// 그냥 쓰기.
		// m_pwBitmap의 높이는 TB_HEIGHT 이하여야 함.
		_ASSERT(0 < m_nSelFontHeight && TB_HEIGHT >= m_nSelFontHeight);
		// 비트맵를 지움.
		::memset(m_pwBitmap, 0, TB_WIDTH * m_nSelFontHeight * sizeof(WORD));

		// 비트맵에 글자를 씀.
		::SetTextColor(m_hDC, crColor);
		::TextOut(m_hDC, 0, 0, lpszText, nStrLen);

		// 문자열의 픽셀 너비를 얻어 리턴.
		::GetTextExtentPoint(m_hDC, lpszText, nStrLen, &size);
		return size.cx;

	case TS_SHADOW :	// 그림자.
		// m_pwBitmap의 높이는 TB_HEIGHT 이하여야 함.
		_ASSERT(0 < m_nSelFontHeight && TB_HEIGHT >= m_nSelFontHeight + 1);
		// 비트맵를 지움. 그림자로 인해 높이가 1픽셀 늘어남.
		::memset(m_pwBitmap, 0,
			TB_WIDTH * (m_nSelFontHeight + 1) * sizeof(WORD));

		// 비트맵에 그림자 글자 쓰기.
		::SetTextColor(m_hDC, crStyleColor);
		::TextOut(m_hDC, 1, 1, lpszText, nStrLen);

		// 비트맵에 글자를 씀.
		::SetTextColor(m_hDC, crColor);
		::TextOut(m_hDC, 0, 0, lpszText, nStrLen);

		// 문자열의 픽셀 너비를 얻어 리턴.
		::GetTextExtentPoint(m_hDC, lpszText, nStrLen, &size);
		return size.cx + 1;

	case TS_OUTLINE :	// 외곽선.
		// m_pwBitmap의 높이는 TB_HEIGHT 이하여야 함.
		_ASSERT(0 < m_nSelFontHeight && TB_HEIGHT >= m_nSelFontHeight + 2);
		// 비트맵를 지움. 외곽선으로 인해 높이가 2픽셀 늘어남.
		::memset(m_pwBitmap, 0,
			TB_WIDTH * (m_nSelFontHeight + 2) * sizeof(WORD));

		// 비트맵에 글자를 4번 써서 외곽선을 만듬.
		::SetTextColor(m_hDC, crStyleColor);
		::TextOut(m_hDC, 0, 1, lpszText, nStrLen);	// 좌측.
		::TextOut(m_hDC, 1, 0, lpszText, nStrLen);	// 상단.
		::TextOut(m_hDC, 1, 2, lpszText, nStrLen);	// 하단.
		::TextOut(m_hDC, 2, 1, lpszText, nStrLen);	// 우측.

		// 비트맵에 글자를 씀.
		::SetTextColor(m_hDC, crColor);
		::TextOut(m_hDC, 1, 1, lpszText, nStrLen);

		// 문자열의 픽셀 너비를 얻어 리턴.
		::GetTextExtentPoint(m_hDC, lpszText, nStrLen, &size);
		return size.cx + 2;
	}

	return 0;
}

#endif //NEW_USER_INTERFACE_FONT