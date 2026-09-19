#pragma once

#include <cstddef>

namespace Core::Text
{
    bool CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source);
}
