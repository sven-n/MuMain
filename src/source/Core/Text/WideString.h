#pragma once

#include <cstddef>

namespace Core::Text
{
    bool CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source);
    // Adds `source` to the end of the text in `destination`, as much of it as fits: the text stays
    // terminated within `destinationCount` units. False when not all of `source` fit.
    bool AppendWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source);
    // Ends the text in `text` where its UTF-8 form would grow past `maxBytes`, never inside a
    // character (a UTF-16 surrogate pair stays whole). False when it already fit.
    bool CutWideStringToUtf8Bytes(wchar_t* text, std::size_t maxBytes);
}
