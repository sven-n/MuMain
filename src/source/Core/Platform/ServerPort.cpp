#include "stdafx.h"

#include "ServerPort.h"

#include <cerrno>
#include <cwchar>
#include <string>

namespace Core::Platform
{
bool ParseServerPort(std::wstring_view value, WORD& port)
{
    if (value.empty())
    {
        return false;
    }

    const std::wstring text(value);
    wchar_t* end = nullptr;
    errno = 0;
    const long parsed = std::wcstol(text.c_str(), &end, 10);
    if (errno != 0 || end != text.c_str() + text.size() || parsed <= 0 || parsed > 65535)
    {
        return false;
    }

    port = static_cast<WORD>(parsed);
    return true;
}
}
