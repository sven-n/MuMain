#include "stdafx.h"
#include "Render/Text/TextWrap.h"

#include "Render/Text/CUIRenderText.h"

#include <cassert>
#include <string>

int CutStr(const wchar_t* pszSrcText, wchar_t* pTextOut, const int iTargetPixelWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab /* = 0 */)
{
    if (iFirstLineTab < 0)
    {
      return 0;
    }

    if (pszSrcText == nullptr)
    {
        assert(!"CutStr Error");
        return 0;
    }

    auto tempString = std::wstring(pszSrcText);
    int iCharIndex = 0, iLineIndex = 0;
    constexpr int TextWrapPadding = 5;
    const int iTargetWidth = iTargetPixelWidth - TextWrapPadding;

    const int totalCharacters = tempString.length();
    int processedSourceCharacters = 0;
    while (!tempString.empty() && iLineIndex < iMaxOutLine)
    {
        SIZE iSize = g_pRenderText->MeasureText(tempString.c_str(), static_cast<int>(tempString.length()));

        if (iLineIndex == 0)
            iSize.cx += iFirstLineTab;

        const auto isTooWideInPixels = iSize.cx >= iTargetWidth;
        const auto isTooLongInCharacters = (int)tempString.length() >= iOutStrLength - 1;
        if (isTooWideInPixels || isTooLongInCharacters)
        {
          // then remove the last word/token from the string and try next loop iteration again ...
          const auto iPosLastSpace = tempString.find_last_of(L' ');
          iCharIndex = (iPosLastSpace == std::wstring::npos) ? tempString.length() - 1 : iPosLastSpace;
          tempString = tempString.substr(0, iCharIndex);
        }
        else
        {
            // we can copy that to the destination
            tempString.copy(pTextOut, tempString.length(), 0);
            iLineIndex++;
            processedSourceCharacters += tempString.length();

            pTextOut += iOutStrLength; // move destination pointer to the next line
            if (processedSourceCharacters < totalCharacters)
            {
              tempString = std::wstring(pszSrcText + processedSourceCharacters);
            }
            else
            {
              tempString = L"";
              break;
            }
        }
    }


    return iLineIndex;
}

int CutText3(const wchar_t* pszText, wchar_t* pTextOut, const int TargetWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab, const BOOL bReverseWrite)
{
    return CutStr(pszText, pTextOut, TargetWidth, iMaxOutLine, iOutStrLength, iFirstLineTab);
}

void CutText4(const wchar_t* pszSource, wchar_t* pszResult1, wchar_t* pszResult2, int iCutCount)
{
    if (pszSource == nullptr || pszSource[0] == '\0') return;
    auto sourceString = std::wstring(pszSource);
    int iLength = sourceString.length();
    int iMove = 2; // might be 4, too
    int iTextSize = 0;
    for (int i = 0; i < iLength; )
    {
        if (i + iMove > iCutCount) break;
        else i += iMove;

        iTextSize = i;
    }
    wcsncpy(pszResult1, pszSource, iTextSize);
    pszResult1[iTextSize] = '\0';
    if (pszResult2 != nullptr)
    {
        wcsncpy(pszResult2, pszSource + iTextSize, iLength - iTextSize);
        pszResult2[iLength - iTextSize] = '\0';
    }
}
