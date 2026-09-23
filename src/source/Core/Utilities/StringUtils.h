#pragma once

#include <string>
#include "Core/Platform/WinCompat.h"

namespace StringUtils
{
    // Convert wide string (UTF-16) to narrow string (UTF-8)
    inline std::string WideToNarrow(const wchar_t* wstr)
    {
        if (!wstr) return "";
        return mu_wchar_to_utf8(wstr);
    }

    // Convert narrow string (UTF-8) to wide string (UTF-16 on Windows, UTF-32 elsewhere) --
    // symmetric counterpart to WideToNarrow above. Added for the RmlUi port
    // (RmlUiRenderInterface::LoadTexture needs it for CGlobalBitmap::LoadImage's
    // std::wstring-based API).
    inline std::wstring NarrowToWide(const std::string& str)
    {
        return mu_utf8_to_wchar(str);
    }
}
