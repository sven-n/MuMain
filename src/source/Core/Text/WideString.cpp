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

bool Core::Text::AppendWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source)
{
    if (destination == nullptr || destinationCount == 0)
        return false;

    const wchar_t* end = std::find(destination, destination + destinationCount, L'\0');
    const auto used = static_cast<std::size_t>(end - destination);
    if (used == destinationCount)
    {
        // Not terminated: the last unit becomes the end.
        destination[destinationCount - 1] = L'\0';
        return source == nullptr || source[0] == L'\0';
    }
    if (source == nullptr)
        return true;

    return CopyWideString(destination + used, destinationCount - used, source);
}
