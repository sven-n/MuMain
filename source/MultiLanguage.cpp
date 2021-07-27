// MultiLanguage.cpp: implementation of the CMultiLanguage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MultiLanguage.h"

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////
CMultiLanguage* CMultiLanguage::ms_Singleton = NULL;

CMultiLanguage::CMultiLanguage(std::string strSelectedML)
{
	ms_Singleton = this;

	// 서유럽어 국가권(영어 포함)
	if (_stricmp(strSelectedML.c_str(), "ENG") == 0)
	{
		byLanguage = 0;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
	else if(_stricmp(strSelectedML.c_str(), "POR") == 0)
	{	
		byLanguage = 1;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
	else if(_stricmp(strSelectedML.c_str(), "SPN") == 0)
	{	
		byLanguage = 2;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
	// 다국어지원 언어 추가 시 아래와 같이 추가
/*
	else if(_stricmp(strSelectedML.c_str(), "KOR") == 0)
	{
		byLanguage = 3;
		iCodePage = 949;
		iNumByteForOneCharUTF8 = 3;
	}
*/
	// Default: 영어
	else
	{
		byLanguage = 0;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
}

BYTE CMultiLanguage::GetLanguage()
{
	return byLanguage;
}

int CMultiLanguage::GetCodePage()
{
	return iCodePage;
}

int CMultiLanguage::GetNumByteForOneCharUTF8()
{
	return iNumByteForOneCharUTF8;
}

// UTF-8의 표기 방법을 이용 지금 현재 char가 UTF-8인지 아닌지 구분
// 반환값: TRUE(UTF-8), FALSE(ANSI)
BOOL CMultiLanguage::IsCharUTF8(const char* pszText)
{
    if (pszText == NULL || strlen(pszText) <= 0)
        return TRUE;
    
    const char* pbyCurr = pszText;
	BOOL        bUTF8   = TRUE;
    
	// UTF-8의 표기 방법을 이용 지금 현재 char가 UTF-8인지 아닌지 구분
    // (참고)http://en.wikipedia.org/wiki/UTF-8
    while ( (*pbyCurr != 0x00) && bUTF8)
    {
        // U+0000 ~ U+007F (0 ~ 127)
        if ((*pbyCurr & 0x80) == 0x00)
        {
            pbyCurr++;
        }
        // U+0080 ~ U+07FF (128 ~ 2,047)
        else if (((*pbyCurr & 0xE0) == 0xC0) && ((*(pbyCurr+1) & 0xC0) == 0x80))
        {
            pbyCurr += 2;
        }
        // U+0800 ~ U+FFFF (2,048 ~ 65,535)
        else if (((*pbyCurr & 0xF0) == 0xE0) && ((*(pbyCurr+1) & 0xC0) == 0x80) && ((*(pbyCurr+2) & 0xC0) == 0x80))
        {
            pbyCurr += 3;
        }
        // U+10000 ~ U+10FFFF (65,536 ~ 1,114,111)
        else if (((*pbyCurr & 0xF8) == 0xF0) && 
            ((*(pbyCurr+1) & 0xC0) == 0x80) && ((*(pbyCurr+2) & 0xC0) == 0x80) && ((*(pbyCurr+3) & 0xC0) == 0x80))
        {
            pbyCurr += 4;
        }
        // not UTF-8
        else
        {
            bUTF8 = FALSE;
        }
    }
    return bUTF8;
}

// 내용: 일반 const char*을 wstring으로 변환(lpString은 마지막 문자로 null을 가지고 있어야 함)
// 인자: wstrDest - 변환 된 wstring이 저장될 인자
//       lpString - 원본 
// 반환 값: wstring의 length
int CMultiLanguage::ConvertCharToWideStr(std::wstring& wstrDest, LPCSTR lpString)
{
    wstrDest = L"";

    if (lpString == NULL || strlen(lpString) <= 0)
        return 0;

    int nLenOfWideCharStr;
	int iConversionType;
	if (strcmp(lpString, "멀티 로긴 버젼") == 0)
		iConversionType = (IsCharUTF8(lpString)) ? CP_UTF8 : 949;
	else 
		iConversionType = (IsCharUTF8(lpString)) ? CP_UTF8 : iCodePage;

    // calculate the number of characters needed to hold the wide-character version of the string.
    nLenOfWideCharStr = MultiByteToWideChar(iConversionType, 0, lpString, -1, NULL, 0);
    // memory allocation
    wchar_t* pwszStr = new wchar_t[nLenOfWideCharStr];
    
    // convert the multi-byte string to a wide-character string.
    MultiByteToWideChar(iConversionType, 0, lpString, -1, pwszStr, nLenOfWideCharStr);
    
    //assign
    wstrDest += pwszStr;
    
    // release the allocated memory.
    delete[] pwszStr;
    
    return nLenOfWideCharStr-1;     
}

// 내용: 일반 const wchar_t*을 string으로 변환(lpwString은 마지막 문자로 null을 가지고 있어야 함)
// 인자: pszDest - 변환 된 string이 저장될 인자
//       lpwString - 원본 
// 반환 값: string의 length
int CMultiLanguage::ConvertWideCharToStr(std::string& strDest, LPCWSTR lpwString, int iConversionType)
{
    strDest = "";

    if (lpwString == NULL || wcslen(lpwString) <= 0)
        return 0;

    int nLenOfWideCharStr;
    
    // calculate the number of characters needed to hold the wide-character version of the string.
    nLenOfWideCharStr = WideCharToMultiByte(iConversionType, 0, lpwString, -1, NULL, 0, 0, 0);
    // memory allocation
    char* pszStr = new char[nLenOfWideCharStr];
    
    // convert the multi-byte string to a wide-character string.
    WideCharToMultiByte(iConversionType, 0, lpwString, -1, pszStr, nLenOfWideCharStr, 0, 0);

    //assign
    strDest += pszStr;
    
    // release the allocated memory.
    delete[] pszStr;

    return nLenOfWideCharStr-1;
}
// 내용: lpString이 ANSI인 경우 UTF-8로 변환해 strDest에 저장
//       lpString이 UTF-8인 경우 ANSI로 변환해 strDest에 저장
void CMultiLanguage::ConvertANSIToUTF8OrViceVersa(std::string& strDest, LPCSTR lpString)
{
	std::wstring wstrUTF16 = L"";
	int iDestType = (IsCharUTF8(lpString)) ? CP_ACP : CP_UTF8;

	ConvertCharToWideStr(wstrUTF16, lpString);
	ConvertWideCharToStr(strDest, wstrUTF16.c_str(), iDestType);
}	

// 내용: wstrTarget이 가지고 있는 빈칸중 중앙에서 가장 가까운 빈칸의 인덱스를 리턴
int	CMultiLanguage::GetClosestBlankFromCenter(const std::wstring wstrTarget)
{
	int iLength = wstrTarget.length();
	
	// 가장 가까운 space 찾기
	// 텍스트의 반을 자른다.
	std::wstring wstrText1 = wstrTarget.substr(iLength/2, std::wstring::npos);
	std::wstring wstrText2 = wstrTarget.substr(0, iLength/2);
	
	// 앞쪽 텍스트에서 가장 뒤쪽 빈칸과 뒤쪽 텍스트에서 가장 앞쪽 빈칸을 찾는다.
	int iPosLastBlankFromFirstHalf = wstrText2.find_last_of(L" ");
	int iPosFirstBlankFromSecondHalf = wstrText1.find_first_of(L" ", 1);
	int iClosestBlankFromCenter = 0;
	
	// 띄어쓰기가 없이 이루어진 텍스트는 그냥 절반으로 자른다.
	if (iPosLastBlankFromFirstHalf == std::wstring::npos && iPosFirstBlankFromSecondHalf == std::wstring::npos)
	{
		iClosestBlankFromCenter = iLength/2;
	}
	// 둘 중 하나의 텍스트만 띄어쓰기를 가지고 있을 경우 그 텍스트의 빈칸을 기준으로 나눈다. 
	else if (iPosLastBlankFromFirstHalf == std::wstring::npos)
	{
		iClosestBlankFromCenter = iPosFirstBlankFromSecondHalf+iLength/2;
	}
	else if (iPosFirstBlankFromSecondHalf == std::wstring::npos)
	{
		iClosestBlankFromCenter = iPosLastBlankFromFirstHalf;
	}
	// 둘다 빈칸을 가지고 있을 경우 중간에서 더 가까운 쪽의 빈칸을 기준으로 나눈다.
	// 거리가 같을 경우 뒤쪽의 텍스트의 빈칸이 더 우선권을 가진다.
	else if (iPosFirstBlankFromSecondHalf <= iLength/2-iPosLastBlankFromFirstHalf)
	{
		iClosestBlankFromCenter = iPosFirstBlankFromSecondHalf+iLength/2;
	}
	else
	{
		iClosestBlankFromCenter = iPosLastBlankFromFirstHalf;
	}
	
	return iClosestBlankFromCenter;
}

// wchar_t 타입(UTF-16)
BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize)
{
	return GetTextExtentPoint32W(hdc, lpString, cbString, lpSize); 
}

// lpString의 Encoding에 따라 ANSI나 UNICODE용 GetTextExtentPoint32 함수를 호출한다. 
BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
{
	std::wstring wstrText = L"";
	ConvertCharToWideStr(wstrText, lpString);
	
	return GetTextExtentPoint32W(hdc, wstrText.c_str(), wstrText.length(), lpSize);
}

// GetTextExtentPoint32와 마찬가지의 역할을 함.
BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString)
{
	return TextOutW(hdc, nXStart, nYStart, lpString, cbString); 
}

BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
{
	std::wstring wstrText = L"";
	ConvertCharToWideStr(wstrText, lpString);
	
	return TextOutW(hdc, nXStart, nYStart, wstrText.c_str(), wstrText.length()); 
}
	
// 전각으로 된 기본 글자들을 반각으로 변환
WPARAM CMultiLanguage::ConvertFulltoHalfWidthChar(DWORD wParam)
{
	wchar_t Char = (wchar_t)(wParam);

	// 기본 기호 및 숫자, 대소문자 기본 알파벳
	if (Char >= 0xFF01 && Char <= 0xFF5A) 
		wParam -= 0xFEE0;
	// 전각 띄어쓰기
	else if (Char == 0x3000)
		wParam = 0x0020;

	return wParam;
}

#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE