//*****************************************************************************
// File: UsefulDef.cpp
//
// Desc: 유용한 정의 모음.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "UsefulDef.h"
#include "UIControls.h"



bool ReduceStringByPixel(LPTSTR lpszDst, int nDstSize, LPCTSTR lpszSrc, int nPixel)
{
    SIZE size;
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), lpszSrc, lstrlen(lpszSrc), &size);
    int nSrcWidth = int(size.cx / g_fScreenRate_x);

    if (nSrcWidth <= nPixel)
    {
        ::wcsncpy(lpszDst, lpszSrc, nDstSize - 1);
        lpszDst[nDstSize - 1] = '\0';
        return false;
    }

    ::CutText3(lpszSrc, lpszDst, nPixel - 6, 1, nDstSize);
    ::wcscat(lpszDst, L"...");
    return true;
}

int DivideStringByPixel(wchar_t* alpszDst, int nDstRow, int nDstColumn, const wchar_t* lpszSrc, int nPixelPerLine, bool bSpaceInsert, const wchar_t szNewlineChar)
{
    if (nullptr == alpszDst || 0 >= nDstRow || 0 >= nDstColumn || nullptr == lpszSrc || 16 > nPixelPerLine)
        return 0;

    std::wstring szWorkSrc(lpszSrc);  // Convert lpszSrc to std::wstring

    wchar_t szWorkToken[1024];
    int nLine = 0;

    wchar_t* pszToken = wcstok(&szWorkSrc[0], &szNewlineChar);

    while (pszToken != nullptr)
    {
        if (bSpaceInsert)
        {
            swprintf(szWorkToken, L" %s", pszToken);
            nLine += CutText3(szWorkToken, alpszDst + nLine * nDstColumn, nPixelPerLine, nDstRow, nDstColumn);
        }
        else
        {
            nLine += CutText3(pszToken, alpszDst + nLine * nDstColumn, nPixelPerLine, nDstRow, nDstColumn);
        }

        pszToken = wcstok(nullptr, &szNewlineChar);
    }

    return nLine;
}

int DivideString(LPTSTR alpszDst, int nDstRow, int nDstColumn, LPCTSTR lpszSrc)
{
    if (NULL == lpszSrc)
        return 0;

    int nSrcLen = ::wcslen(lpszSrc);
    if (0 == nSrcLen)
        return 0;

    int nSrcPos = 0;
    int nDstStart = 0;
    int nDstLen = 1;
    int nLineCount = 0;

    while (TRUE)
    {
        if (0x80 & lpszSrc[nSrcPos])
        {
            ++nSrcPos;
            ++nDstLen;
        }

        if ('/' == lpszSrc[nSrcPos])
        {
            ::wcsncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen - 1);
            ++nLineCount;
            nDstStart = nSrcPos + 1;
            nDstLen = 0;
        }
        else if (nDstLen >= nDstColumn)
        {
            nSrcPos -= 2;
            nDstLen -= 2;
            ::wcsncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen);
            ++nLineCount;
            nDstStart = nSrcPos + 1;
            nDstLen = 0;
        }
        else if (nSrcPos == nSrcLen - 1)
        {
            ::wcsncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen);
            break;
        }
        else if (nDstLen == nDstColumn - 1)
        {
            ::wcsncpy(alpszDst + nLineCount * nDstColumn, lpszSrc + nDstStart, nDstLen);
            ++nLineCount;
            nDstStart = nSrcPos + 1;
            nDstLen = 0;
        }

        if (nDstRow == nLineCount)
            break;

        ++nSrcPos;
        ++nDstLen;
    }

    return nLineCount + 1;
}

BOOL CheckErrString(LPTSTR lpszTarget)
{
    int i = 0;
    int nLen = ::wcslen(lpszTarget);
    while (i < nLen)
    {
        if (0x80 & lpszTarget[i])
        {
            if (i == nLen - 1)
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