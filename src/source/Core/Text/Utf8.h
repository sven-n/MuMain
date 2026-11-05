// UTF-8 conversion for the wide strings the game holds (object and map names,
// chat lines, versions).
//
// Kept apart from WideString.h because it needs the platform's code-page
// conversion, and WideString is compiled on its own into tests that must not
// pull a platform shim in with it.
#pragma once

#include <string>
#include <string_view>

namespace Core::Text
{
// A null, empty or unconvertible source yields an empty string.
[[nodiscard]] std::string ToUtf8(const wchar_t* text);

// The other direction, for text that arrives from outside the game (a
// file path a script asked a screenshot to be written to, for example).
[[nodiscard]] std::wstring FromUtf8(const std::string& text);

// Cuts UTF-8 text to at most maxUnits UTF-16 units, never inside a
// character; false when it is not longer. Counted as the client's wide strings
// hold the text on Windows, the same on every platform: an emoji counts as two.
bool CutToUtf16Length(std::string& text, std::size_t maxUnits);

// Whether the text is UTF-8 as RFC 3629 allows it: each character a lead byte
// followed by as many continuation bytes as it announces, in its shortest
// form, and neither a UTF-16 surrogate nor above U+10FFFF. Text in a code page
// such as Windows-1252 fails this as soon as it holds a letter outside ASCII.
bool IsValidUtf8(std::string_view text);
} // namespace Core::Text
