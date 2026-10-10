#pragma once

// Pixel-width text wrapping, measured through g_pRenderText. Native drawing uses these to
// pre-split a string into fixed-width line buffers; RmlUi documents wrap text themselves and
// do not need them.

// Wraps pszSrcText to iTargetPixelWidth, writing at most iMaxOutLine lines of iOutStrLength
// characters each into pTextOut, and returns the number of lines written. Breaks on spaces where
// it can, mid-word otherwise. iFirstLineTab indents the first line's measurement only.
// Note: no terminator is written for a line that fills its buffer, so the destination must be
// zeroed before the call.
int CutStr(const wchar_t* pszText, wchar_t* pTextOut, const int iTargetPixelWidth, const int iMaxOutLine,
           const int iOutStrLength, const int iFirstLineTab = 0);

// CutStr with a bReverseWrite parameter that has never been honoured; kept for its call sites.
int CutText3(const wchar_t* pszText, wchar_t* pTextOut, const int TargetWidth, const int iMaxOutLine,
             const int iOutStrLength, const int iFirstLineTab = 0, const BOOL bReverseWrite = FALSE);

// Splits pszSource at the last whole character pair that fits within iCutCount units.
void CutText4(const wchar_t* pszSource, wchar_t* pszResult1, wchar_t* pszResult2, int iCutCount);
