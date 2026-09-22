// UTF-8 conversion for the wide strings the game holds (object and map names,
// chat lines, versions).
//
// Kept apart from WideString.h because it needs the platform's code-page
// conversion, and WideString is compiled on its own into tests that must not
// pull a platform shim in with it.
#pragma once

#include <string>

namespace Core::Text
{
// A null, empty or unconvertible source yields an empty string.
[[nodiscard]] std::string ToUtf8(const wchar_t* text);

// The other direction, for text that arrives from outside the game (a
// file path a script asked a screenshot to be written to, for example).
[[nodiscard]] std::wstring FromUtf8(const std::string& text);
} // namespace Core::Text
