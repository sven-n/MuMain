#pragma once

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
