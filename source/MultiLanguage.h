// MultiLanguage.h: interface for the CMultiLanguage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTILANGUAGE_H__01D5F840_67B5_4472_B89F_9C3EAF89107A__INCLUDED_)
#define AFX_MULTILANGUAGE_H__01D5F840_67B5_4472_B89F_9C3EAF89107A__INCLUDED_

#pragma once

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE

class CMultiLanguage
{

private:
	static CMultiLanguage *ms_Singleton;

	BYTE byLanguage;				// 서버에 사용자가 선택한 언어 보내줌(0:영어, 1:포르투갈어, 2:스페인어)
	int iCodePage;					// CodePage
	int iNumByteForOneCharUTF8;		// 그나라의 언어를 UTF-8로 표현하기 위해 필요한 byte 수

	CMultiLanguage()	{ ms_Singleton = this; };				

public:
	CMultiLanguage(string strSelectedML);
	~CMultiLanguage()	{ ms_Singleton = 0; };

	BYTE GetLanguage();				// Getters
	int GetCodePage();
	int GetNumByteForOneCharUTF8();

	BOOL IsCharUTF8(const char* pszText);
	int	 ConvertCharToWideStr(wstring& wstrDest, LPCSTR lpString);
	int  ConvertWideCharToStr(string& strDest, LPCWSTR lpwString, int iConversionType = CP_UTF8);
	void ConvertANSIToUTF8OrViceVersa(string& strDest, LPCSTR lpString);
	int	 GetClosestBlankFromCenter(const wstring wstrTarget); 
	
	WPARAM ConvertFulltoHalfWidthChar(DWORD wParam);
	// wrapper functions 
	// WIN32 APIs
	// MSDN에서는 GetTextExtentPoint보다 더 정확한 결과를 제공하는 GetTextExtentPoint32 사용을 강력히 권장한다.
	BOOL _GetTextExtentPoint32(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize);
	BOOL _GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize);

	BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString);
	BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString);
	
	static CMultiLanguage* GetSingletonPtr() { return ms_Singleton; };
};
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#endif // !defined(AFX_MULTILANGUAGE_H__01D5F840_67B5_4472_B89F_9C3EAF89107A__INCLUDED_)
