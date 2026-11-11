#pragma once

#include <cstddef>

namespace Core::Text
{
    bool CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source);
    // Adds `source` to the end of the text in `destination`, as much of it as fits: the text stays
    // terminated within `destinationCount` units. False when not all of `source` fit.
    bool AppendWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source);
}
