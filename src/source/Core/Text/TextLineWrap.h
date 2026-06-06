#pragma once

#include <cstddef> // size_t

// Word-wrap utilities for wide-character UI text.
// Pure function unit — no project dependencies, can be linked from tests.

/**
 * @brief Splits a wide-character string into multiple lines with word-wrap support.
 * @param lpszText      Source string.
 * @param lpszSeparated Destination 2D buffer (wchar_t[iMaxLine][iLineSize]).
 * @param iMaxLine      Max number of rows.
 * @param iLineSize     Size of each row (including \0).
 * @return int          Total lines created (guaranteed not to exceed iMaxLine).
 */
int SeparateTextIntoLines(const wchar_t* lpszText, wchar_t* lpszSeparated, int iMaxLine, int iLineSize);

/**
 * @brief Splits Text into two parts near its midpoint at a space.
 * @param Text       Source string.
 * @param Text1      Receives the first part (up to maxLength, truncated).
 * @param Text2      Receives the second part, or empty if no space was found.
 * @param maxLength  Capacity of each destination buffer.
 */
void CutText(const wchar_t* Text, wchar_t* Text1, wchar_t* Text2, size_t maxLength);
