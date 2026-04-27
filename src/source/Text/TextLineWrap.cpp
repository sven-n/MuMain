#include "TextLineWrap.h"

namespace
{
// True for codepoints drawn at twice the width of a half-width glyph (CJK
// ideographs, kana, Hangul, full-width Latin/punctuation, ...). A naive
// `c > 255` test misclassifies Cyrillic/Greek/Hebrew/Arabic, which sit above
// U+00FF but render at single width.
bool IsFullWidthCharacter(wchar_t c)
{
    return (c >= 0x1100 && c <= 0x115F)   // Hangul Jamo
        || (c >= 0x2E80 && c <= 0x9FFF)   // CJK radicals / ideographs / kana
        || (c >= 0xA000 && c <= 0xA4CF)   // Yi
        || (c >= 0xAC00 && c <= 0xD7A3)   // Hangul syllables
        || (c >= 0xF900 && c <= 0xFAFF)   // CJK compatibility ideographs
        || (c >= 0xFE30 && c <= 0xFE4F)   // CJK compatibility forms
        || (c >= 0xFF00 && c <= 0xFF60)   // full-width Latin / punctuation
        || (c >= 0xFFE0 && c <= 0xFFE6);  // full-width signs
}
}

int SeparateTextIntoLines(const wchar_t* lpszText, wchar_t* lpszSeparated, int iMaxLine, int iLineSize)
{
    if (!lpszText || !lpszSeparated || iMaxLine <= 0 || iLineSize <= 0) return 0;

    int iLine = 0;
    const int iVisualLimit = iLineSize - 1;
    const wchar_t* lpSeek = lpszText;
    const wchar_t* lpLastSpaceInSource = nullptr;
    wchar_t* lpDstInCurrentRow = nullptr;

    int iCurrentWidth = 0;
    wchar_t* lpWrite = lpszSeparated;

    while (*lpSeek && iLine < iMaxLine) {
        int iCharWidth = IsFullWidthCharacter(*lpSeek) ? 2 : 1;

        if (*lpSeek == L' ') {
            lpLastSpaceInSource = lpSeek;
            lpDstInCurrentRow = lpWrite;
        }

        if (iCurrentWidth + iCharWidth > iVisualLimit) {
            // Wrap at the preceding space whenever one was seen on this line.
            // Works for the overflow being half-width or full-width — important
            // for mixed strings like "Hello 你好" where the CJK char triggers
            // the overflow but breaking at the space gives cleaner output.
            if (lpLastSpaceInSource && *lpSeek != L' ') {
                *lpDstInCurrentRow = L'\0';
                lpSeek = lpLastSpaceInSource + 1;
            }
            else {
                // Forward-progress guard: at the start of a line with no space
                // to wrap at, a character whose width exceeds the visual limit
                // would otherwise loop forever (iLine increments, lpSeek does
                // not). Place the character on its own row anyway — when there
                // is room for any payload — and advance past it so the loop
                // makes progress.
                if (iCurrentWidth == 0) {
                    if (iLineSize > 1) {
                        *lpWrite++ = *lpSeek;
                    }
                    ++lpSeek;
                }
                *lpWrite = L'\0';
            }

            iLine++;
            if (iLine >= iMaxLine) break;

            lpWrite = lpszSeparated + (iLine * iLineSize);
            iCurrentWidth = 0;
            lpLastSpaceInSource = nullptr;
            lpDstInCurrentRow = nullptr;

            while (*lpSeek == L' ') lpSeek++;
            continue;
        }

        *lpWrite++ = *lpSeek++;
        iCurrentWidth += iCharWidth;
    }

    if (iLine < iMaxLine) {
        *lpWrite = L'\0';
        return iLine + 1;
    }

    return iMaxLine;
}
