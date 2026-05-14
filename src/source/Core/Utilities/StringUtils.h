#pragma once

#include <string>
#include <windows.h>

namespace StringUtils
{
    // Convert wide string (UTF-16) to narrow string (UTF-8)
    inline std::string WideToNarrow(const wchar_t* wstr)
    {
        if (!wstr) return "";
        size_t len = wcslen(wstr);
        if (len == 0) return "";

        int size = WideCharToMultiByte(CP_UTF8, 0, wstr, (int)len, NULL, 0, NULL, NULL);
        std::string result(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr, (int)len, &result[0], size, NULL, NULL);
        return result;
    }
}
