#include "stdafx.h"
#include "RmlUiSystemInterface.h"

#include "Core/Utilities/Log/ErrorReport.h"
#include "Core/Utilities/StringUtils.h"
#include <SDL3/SDL.h>
#include <chrono>

namespace
{
    // Real wall-clock time, independent of game pause/tick state -- matches this codebase's own
    // established pattern for render-frequency timing (std::chrono::steady_clock, same as the
    // FPS_ANIMATION_FACTOR handling elsewhere) rather than
    // reusing a game-simulation time global like WorldTime, which RmlUi's own animations/
    // transitions/double-click detection should not be coupled to.
    const std::chrono::steady_clock::time_point g_StartTime = std::chrono::steady_clock::now();
}

RmlUiSystemInterface::RmlUiSystemInterface(SDL_Window* window) : m_Window(window)
{
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
    g_ErrorReport.Write(L"%s %s\r\n", tag, StringUtils::NarrowToWide(message).c_str());
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

void RmlUiSystemInterface::ActivateKeyboard(Rml::Vector2f caret_position, float line_height)
{
    // Same call shape as the vendored sample's SystemInterface_SDL::ActivateKeyboard
    // (RmlUi_Platform_SDL.cpp) -- caret_position/line_height already arrive in real window-pixel
    // space (WidgetTextInput::SetKeyboardActive() derives them from GetAbsoluteOffset(), and this
    // engine's RmlUi context is created directly from real window pixel dimensions, see
    // RmlUiRuntime::Create()'s own ApplyUIScale comment), so no coordinate transform is needed
    // here -- unlike CUITextInputBox::GetCaretArea(), which is in the legacy reference-resolution
    // space and needs Winmain.cpp's own UI::Scaling::TransformForLayout() call.
    m_TextInputActive = true;
    if (!m_Window) return;
    const SDL_Rect rect = {static_cast<int>(caret_position.x), static_cast<int>(caret_position.y), 1, static_cast<int>(line_height)};
    SDL_SetTextInputArea(m_Window, &rect, 0);
    SDL_StartTextInput(m_Window);
}

void RmlUiSystemInterface::DeactivateKeyboard()
{
    m_TextInputActive = false;
    if (!m_Window) return;
    SDL_StopTextInput(m_Window);
}
