#pragma once

#include <string>
#ifdef _WIN32
#include <windows.h>
#include "Platform/PlatformCompat.h"
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

namespace StringUtils
{
// Convert wide string (UTF-16) to narrow string (UTF-8)
inline std::string WideToNarrow(const wchar_t* wstr)
{
    if (!wstr)
        return "";
    return mu_wchar_to_utf8(wstr);
}
} // namespace StringUtils
