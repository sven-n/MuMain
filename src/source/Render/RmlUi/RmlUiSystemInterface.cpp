#include "stdafx.h"
#include "RmlUiSystemInterface.h"

#include "Core/Utilities/Log/ErrorReport.h"
#include <SDL3/SDL.h>
#include <chrono>

namespace
{
    std::wstring Utf8ToWide(const std::string& s)
    {
        if (s.empty()) return std::wstring();
        const int len = static_cast<int>(s.size());
        const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), len, nullptr, 0);
        if (n <= 0) return std::wstring();
        std::wstring out(static_cast<size_t>(n), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), len, out.data(), n);
        return out;
    }

    // Real wall-clock time, independent of game pause/tick state -- matches this codebase's own
    // established pattern for render-frequency timing (std::chrono::steady_clock, see the
    // FPS_ANIMATION_FACTOR gotchas in docs/GPU Skinning/gotchas-and-patterns.md) rather than
    // reusing a game-simulation time global like WorldTime, which RmlUi's own animations/
    // transitions/double-click detection should not be coupled to.
    const std::chrono::steady_clock::time_point g_StartTime = std::chrono::steady_clock::now();
}

double RmlUiSystemInterface::GetElapsedTime()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - g_StartTime).count();
}

bool RmlUiSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    const wchar_t* tag = L"[RmlUi]";
    switch (type)
    {
        case Rml::Log::LT_ERROR:   tag = L"[RmlUi][Error]";   break;
        case Rml::Log::LT_ASSERT:  tag = L"[RmlUi][Assert]";  break;
        case Rml::Log::LT_WARNING: tag = L"[RmlUi][Warning]"; break;
        case Rml::Log::LT_INFO:    tag = L"[RmlUi][Info]";    break;
        case Rml::Log::LT_DEBUG:   tag = L"[RmlUi][Debug]";   break;
        default: break;
    }
    g_ErrorReport.Write(L"%s %s\r\n", tag, Utf8ToWide(message).c_str());
    return true; // continue execution -- never break into the debugger from here
}

void RmlUiSystemInterface::SetClipboardText(const Rml::String& text)
{
    // SDL3's clipboard API (UTF-8, cross-platform) rather than raw Win32 OpenClipboard/
    // SetClipboardData -- consistent with this engine's SDL-based platform abstraction and its
    // existing portability discipline (GdiText.cpp etc.).
    SDL_SetClipboardText(text.c_str());
}

void RmlUiSystemInterface::GetClipboardText(Rml::String& text)
{
    char* clipboard = SDL_GetClipboardText();
    text = clipboard ? clipboard : "";
    if (clipboard) SDL_free(clipboard);
}
