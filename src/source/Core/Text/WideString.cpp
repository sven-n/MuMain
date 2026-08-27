#include "Core/Text/WideString.h"

#include <algorithm>
#include <cwchar>

bool Core::Text::CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source)
{
    if (destination == nullptr || destinationCount == 0)
        return false;

    if (source == nullptr)
    {
        destination[0] = L'\0';
        return false;
    }

    const std::size_t sourceLength = std::wcslen(source);
    const std::size_t copyLength = std::min(sourceLength, destinationCount - 1);
    std::wmemcpy(destination, source, copyLength);
    destination[copyLength] = L'\0';
    return copyLength == sourceLength;
}
