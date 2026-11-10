#pragma once

#include <optional>
#include <string_view>

namespace Core::Text
{
// `text` without leading and trailing whitespace. std::iswspace decides what whitespace is:
// command arguments reach the parser from the chat box (space, tab) and from the control
// socket, where the text can end in CRLF.
std::wstring_view TrimWhitespace(std::wstring_view text);

// One number and nothing else, or nullopt: surrounding whitespace is ignored, while a missing
// number, trailing characters and values too large for the type are not. Range checks belong
// to the caller, which knows what its command accepts.
std::optional<double> TryParseDouble(std::wstring_view text);
std::optional<int> TryParseInt(std::wstring_view text);
} // namespace Core::Text
