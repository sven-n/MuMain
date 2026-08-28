#pragma once

#include <cstddef> // size_t
#include <functional>
#include <string>
#include <vector>

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
 * @brief Measures how wide a piece of text is drawn, in the unit maxWidth is given in.
 */
using MeasureTextWidth = std::function<int(const wchar_t* text, size_t length)>;

/**
 * @brief Splits a string into lines which each stay within maxWidth.
 *
 * Unlike SeparateTextIntoLines, which counts characters, this asks the caller
 * how wide a piece of text actually is - which is what a proportional font
 * needs. Lines are broken at spaces; a single word which is wider than
 * maxWidth on its own is broken where it doesn't fit anymore.
 *
 * @param text         Source string. Line breaks in it start a new line.
 * @param maxWidth     The width a line may take.
 * @param measureWidth Measures a piece of the source string.
 * @return             The lines, in order. Empty when there is nothing to draw.
 */
std::vector<std::wstring> WrapTextToWidth(const std::wstring& text, int maxWidth, const MeasureTextWidth& measureWidth);

/**
 * @brief Splits Text into two parts near its midpoint at a space.
 * @param Text       Source string.
 * @param Text1      Receives the first part (up to maxLength, truncated).
 * @param Text2      Receives the second part, or empty if no space was found.
 * @param maxLength  Capacity of each destination buffer.
 */
void CutText(const wchar_t* Text, wchar_t* Text1, wchar_t* Text2, size_t maxLength);
