//*****************************************************************************
// File: UsefulDef.cpp
//
// Desc: ������ ���� ����.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "Core/Utilities/UsefulDef.h"
#include "UI/Legacy/UIControls.h"
#include "Core/Text/TextLineWrap.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <algorithm>
#include <string>
#include <vector>



bool ReduceStringByPixel(LPTSTR lpszDst, int nDstSize, LPCTSTR lpszSrc, int nPixel)
{
    const SIZE size = g_pRenderText->MeasureText(lpszSrc, lstrlen(lpszSrc));
    const int nSrcWidth = size.cx;

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

namespace
{
// Same margin CutStr keeps from the line width: a line must stay narrower than width - 5.
constexpr int kTextWrapPadding = 5;

int MeasureUiTextWidth(const wchar_t* text, size_t length)
{
    return g_pRenderText->MeasureText(text, static_cast<int>(length)).cx;
}
} // namespace

int DivideStringByPixel(wchar_t* alpszDst, int nDstRow, int nDstColumn, const wchar_t* lpszSrc, int nPixelPerLine, bool bSpaceInsert, const wchar_t szNewlineChar)
{
    if (nullptr == alpszDst || 0 >= nDstRow || 1 >= nDstColumn || nullptr == lpszSrc || 16 > nPixelPerLine)
        return 0;

    const int maxWidth = nPixelPerLine - kTextWrapPadding - 1;
    const size_t maxCharactersPerLine = static_cast<size_t>(nDstColumn - 1);
    const std::vector<std::wstring> lines = WrapParagraphsToWidth(lpszSrc, szNewlineChar, maxWidth, maxCharactersPerLine,
                                                                  bSpaceInsert, MeasureUiTextWidth);

    const int lineCount = std::min(static_cast<int>(lines.size()), nDstRow);
    if (lineCount < static_cast<int>(lines.size()))
    {
        mu::log::Get("ui")->warn("DivideStringByPixel: the text needs {} lines, only {} fit; the rest is not shown.",
                                 lines.size(), nDstRow);
    }

    for (int i = 0; i < lineCount; ++i)
    {
        wchar_t* row = alpszDst + i * nDstColumn;
        const size_t length = lines[i].copy(row, maxCharactersPerLine);
        row[length] = L'\0';
    }

    return lineCount;
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
                lpszTarget[i] = 0;
                return FALSE;
            }
            else
                ++i;
        }
        ++i;
    }

    return TRUE;
}
