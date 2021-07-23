//*****************************************************************************
// File: UsefulDef.cpp
//
// Desc: 유용한 정의 모음.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include <windows.h>
#include "UsefulDef.h"
#include "UIControls.h"

#ifdef ASG_ADD_NEW_DIVIDE_STRING
extern float g_fScreenRate_x;

//*****************************************************************************
// 함수 이름 : ReduceStringByPixel()
// 함수 설명 : 문자열(lpszSrc)의 너비가 nPixel보다 크면 nPixel 만큼 잘라내고 말
//			  줄임표'...'를 붙인다.
// 주     의 : GetTextExtentPoint()함수를 쓰기 때문에 본 함수 호출 전에 원하는
//			  폰트를 지정하는 것이 좋다.
// 매개 변수 : lpszDst		: 원본 문자열.
//			   nDstSize		: alpszDst 문자열의 크기.
//			   lpszSrc		: 결과 문자열.
//			   nPixel		: 원하는 픽셀 수.
// 반환 값	 : 말줄임표가 붙는다면 true.
//*****************************************************************************
bool ReduceStringByPixel(LPTSTR lpszDst, int nDstSize, LPCTSTR lpszSrc, int nPixel)
{
	// 원본 텍스트의 가로 픽셀 크기를 구함.
	SIZE size;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), lpszSrc, lstrlen(lpszSrc), &size);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), lpszSrc, lstrlen(lpszSrc), &size);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	int nSrcWidth = int(size.cx / g_fScreenRate_x);

	// 원하는 픽셀 크기(nPixelPerLine)보다 같거나 적으면 그냥 복사하고 리턴.
	if (nSrcWidth <= nPixel)
	{
		::strncpy(lpszDst, lpszSrc, nDstSize - 1);
		lpszDst[nDstSize - 1] = '\0';
		return false;
	}

	// 크면 원하는 픽셀 크기 - 6 만큼 잘라내고 "..."를 붙이고 리턴.
	::CutText3(lpszSrc, lpszDst, nPixel - 6, 1, nDstSize);
	::strcat(lpszDst, "...");

	return true;
}

//*****************************************************************************
// 함수 이름 : DivideStringByPixel()
// 함수 설명 : 문자열(lpszSrc)을 한 줄에 nPixelPerLine 픽셀 만큼 나오도록
//			  나누어 alpszDst에 저장. 나눈 결과 줄 수 리턴.
//			   문자열(lpszSrc)에서 ';'가 있으면 다음 라인으로 넘김.
// 주     의 : CutText3()함수 안에서 GetTextExtentPoint()함수를 쓰기 때문에 원
//			  하는 픽셀로 나누고 싶다면 본 함수 호출 전에 원하는 폰트를 지정하
//			  는 것이 좋다.
// 매개 변수 : alpszDst		: 나누어 저장할 문자열 배열의 주소.
//			   nDstRow		: alpszDst 배열의 행수 또는 제한 줄수.
//			   nDstColumn	: alpszDst 배열의 열수.
//			   lpszSrc		: 나눌 문자열 포인터.
//			   nPixelPerLine: 한줄당 픽셀 수.
//			   bSpaceInsert	: 첫 문단 첫 칸에 공백을 넣는다면 true.(기본값 true)
// 반환 값	 : 나눈 문자열 줄 수. nDstRow보다 같거나 작은 값을 반환.
//*****************************************************************************
#if defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
int DivideStringByPixel(LPTSTR alpszDst, int nDstRow, int nDstColumn, LPCTSTR lpszSrc,
						int nPixelPerLine, bool bSpaceInsert, const char szNewlineChar)
#else // defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
int DivideStringByPixel(LPTSTR alpszDst, int nDstRow, int nDstColumn, LPCTSTR lpszSrc,
						int nPixelPerLine, bool bSpaceInsert)
#endif // defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
{
	if (NULL == alpszDst || 0 >= nDstRow || 0 >= nDstColumn || NULL == lpszSrc || 16 > nPixelPerLine)
		return 0;

	char szWorkSrc[1024];
	::strcpy(szWorkSrc, lpszSrc);

	char szWorkToken[1024];
	int nLine = 0;

#if defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
	char* pszToken = ::strtok(szWorkSrc, &szNewlineChar);
#else // defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
	char* pszToken = ::strtok(szWorkSrc, ";");
#endif // defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
	while (pszToken != NULL)
	{
		if (bSpaceInsert)
		{
			::sprintf(szWorkToken, " %s", pszToken);
			nLine += ::CutText3(szWorkToken, alpszDst + nLine * nDstColumn, nPixelPerLine,
				nDstRow, nDstColumn);
		}
		else
		{
			nLine += ::CutText3(pszToken, alpszDst + nLine * nDstColumn, nPixelPerLine,
				nDstRow, nDstColumn);
		}
#if defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
		pszToken = ::strtok(NULL, &szNewlineChar);
#else // defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
		pszToken = ::strtok(NULL, ";");
#endif // defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
	}

	return nLine;
}

//*****************************************************************************
// 함수 이름 : DivideString()
// 함수 설명 : 문자열(lpszSrc)을 한 줄에 글자수가 nDstColumn -1 만큼 나오도록
//			  나누어 alpszDst에 저장. 나눈 결과 줄 수 리턴.
//			   문자열(lpszSrc)에서 '/'가 있으면 다음 라인으로 넘김.
// 매개 변수 : alpszDst		: 나누어 저장할 문자열 배열의 주소.
//			   nDstRow		: alpszDst 배열의 행수 또는 제한 줄수.
//			   nDstColumn	: alpszDst 배열의 열수.
//			   lpszSrc		: 나눌 문자열 포인터.
// 반환 값	 : 나눈 문자열 줄 수. nDstRow보다 같거나 작은 값을 반환.
//*****************************************************************************
int DivideString(LPTSTR alpszDst, int nDstRow, int nDstColumn, LPCTSTR lpszSrc)
{
	if (NULL == lpszSrc)
		return 0;

	int nSrcLen = ::strlen(lpszSrc);	// 원본 문자열 전체 길이.
	if (0 == nSrcLen)
		return 0;

	int nSrcPos = 0;			// 원본 문자열(lpszSrc)에서 검사중인 Byte 위치.
	int nDstStart = 0;			// 복사할 문자열 시작 위치.
	int nDstLen = 1;			// 복사할 문자열 길이.
	int nLineCount = 0;			// 줄수.
	
	while (TRUE)
	{
		if (0x80 & lpszSrc[nSrcPos])	// 확장 코드인가?
		{
			// 확장코드이면 2Byte이므로 ++.
			++nSrcPos;
			++nDstLen;
		}

		// '/'문자이면 '/'문자 이전까지 복사후 다음 줄로.
		if ('/' == lpszSrc[nSrcPos])
		{
			::strncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen - 1);
			++nLineCount;
			nDstStart = nSrcPos + 1;
			nDstLen = 0;
		}
		// 한 줄을 만들 되 확장 코드가 잘리는가?
		else if (nDstLen >= nDstColumn)
		{
			// 잘리지 않게 위치 후퇴.
			nSrcPos -= 2;
			nDstLen -= 2;
			::strncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen);
			++nLineCount;
			nDstStart = nSrcPos + 1;
			nDstLen = 0;
		}
		// 원본 문자열 끝인가?
		else if (nSrcPos == nSrcLen - 1)
		{
			::strncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen);
			break;
		}
		// 한 줄을 만들 수 있는가?
		else if (nDstLen == nDstColumn - 1)
		{
			::strncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen);
			++nLineCount;
			nDstStart = nSrcPos + 1;
			nDstLen = 0;
		}

		// 제한 줄수인가?
		if (nDstRow == nLineCount)
			break;
		
		++nSrcPos;
		++nDstLen;
	}
	
	return nLineCount + 1;
}
#else	// ASG_ADD_NEW_DIVIDE_STRING
//*****************************************************************************
// 함수 이름 : DivideString()
// 함수 설명 : 문자열(lpszSrc)을 한 줄에 글자수가 nBytePerLine 만큼 나오도록 나
//			  누어 alpszDst에 저장. 나눈 결과 줄 수 리턴.
//			   alpszDst 배열 개수가 리턴 값보다 작거나 같아야 오류가 나지 않음.
// 매개 변수 : lpszSrc		: 나눌 문자열 포인터.
//			   nBytePerLine	: 한 줄에 몇 글자로 나눌 것인가.
//			   alpszDst		: 나누어 저장할 문자열 포인터의 배열.
//*****************************************************************************
int DivideString(LPCTSTR lpszSrc, int nBytePerLine, LPTSTR* alpszDst)
{
	int nSrcPos = 0;			// 원본 문자열(lpszSrc)에서 검사중인 Byte 위치.
	int nSrcLen = ::strlen(lpszSrc);	// 원본 문자열 전체 길이.
	int nDstStart = 0;			// 복사할 문자열 시작 위치.
	int nDstLen = 1;			// 복사할 문자열 길이.
	int nLineCount = 0;			// 줄 수.

	while (TRUE)
	{
		if (0x80 & lpszSrc[nSrcPos])	// 확장 코드인가?
		{
			// 확장코드이면 2Byte이므로 ++.
			nSrcPos++;
			nDstLen++;
		}

		// 한 줄을 만들 되 확장 코드가 잘리는가?
		if (nDstLen > nBytePerLine)
		{
			// 잘리지 않게 위치 후퇴.
			nSrcPos -= 2;
			nDstLen -= 2;
			::strncpy(alpszDst[nLineCount], lpszSrc + nDstStart, nDstLen);
			nLineCount++;
			nDstStart = nSrcPos + 1;
			nDstLen = 0;
		}
		// 원본 문자열 끝인가?
		else if (nSrcPos == nSrcLen - 1)
		{
			::strncpy(alpszDst[nLineCount], lpszSrc + nDstStart, nDstLen);
			break;
		}
		// 한 줄을 만들 수 있는가?
		else if (nDstLen == nBytePerLine)
		{
			::strncpy(alpszDst[nLineCount], lpszSrc + nDstStart, nDstLen);
			nLineCount++;
			nDstStart = nSrcPos + 1;
			nDstLen = 0;
		}

		nSrcPos++;
		nDstLen++;
	}

	return nLineCount + 1;
}
#endif	// ASG_ADD_NEW_DIVIDE_STRING

// 마지막 열(NULL 전)이 확장 코드라면 NULL을 넣어줘서 에러 방지.
// 에러 검출이면 FALSE 리턴.
BOOL CheckErrString(LPTSTR lpszTarget)
{
	int i = 0;
	int nLen = ::strlen(lpszTarget);
	while (i < nLen)	// 문자열 끝이 아니면 반복.
	{
		if (0x80 & lpszTarget[i])	// 확장 코드인가?
		{
			if (i == nLen - 1)		// 마지막 열인가?(NULL 전)
			{
				lpszTarget[i] = NULL;
				return FALSE;
			}
			else
				++i;
		}
		++i;
	}

	return TRUE;
}
