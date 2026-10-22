#include "stdafx.h"
#include "RmlUiSystemInterface.h"

#include "Core/Utilities/Log/MuLogger.h"
#include <SDL3/SDL.h>
#include <chrono>

namespace
{
    // RmlUi animation and double-click timing must advance independently of game pause/ticks.
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
    const auto logger = mu::log::Get("ui");
    switch (type)
    {
        case Rml::Log::LT_ERROR:
        case Rml::Log::LT_ASSERT:  MU_LOG_ERROR(logger, "[RmlUi] {}", message); break;
        case Rml::Log::LT_WARNING: MU_LOG_WARN(logger, "[RmlUi] {}", message);  break;
        case Rml::Log::LT_DEBUG:   MU_LOG_DEBUG(logger, "[RmlUi] {}", message); break;
        default:                   MU_LOG_INFO(logger, "[RmlUi] {}", message);  break;
    }
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
    // RmlUi supplies the caret rectangle in window pixels, as SDL_SetTextInputArea requires.
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
