#include "stdafx.h"

#include "Quit.h"
#include "ErrorDialog.h"

#include "Core/Platform/WinCompat.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL.h>

#include <atomic>

namespace Core::Platform
{
namespace
{
std::atomic<bool> s_quitRequested = false;
}

void RequestQuit()
{
    s_quitRequested = true;
    // Wakes the loop sooner; before SDL's events run the push fails and the flag alone carries the request.
    SDL_Event quit;
    SDL_zero(quit);
    quit.type = SDL_EVENT_QUIT;
    if (!SDL_PushEvent(&quit))
    {
        MU_LOG_INFO(mu::log::Get("core"), "The quit request waits for the loop: {}", SDL_GetError());
    }
}

bool QuitRequested()
{
    return s_quitRequested;
}

void ShowFatalError(const std::string& title, const std::string& message)
{
    MU_LOG_ERROR(mu::log::Get("core"), "{}: {}", title, message);
    // A failed load often goes on and fails again before the loop quits: the player sees the first error only.
    if (!s_quitRequested.exchange(true))
    {
        ErrorDialog::Show(title, message, false);
    }
    RequestQuit();
}

void ShowFatalError(const wchar_t* title, const wchar_t* message)
{
    ShowFatalError(title != nullptr ? mu_wchar_to_utf8(title) : std::string("Error"), mu_wchar_to_utf8(message));
}

void ShowError(const std::string& title, const std::string& message)
{
    MU_LOG_ERROR(mu::log::Get("core"), "{}: {}", title, message);
    if (!s_quitRequested && ErrorDialog::Show(title, message, true) == ErrorDialog::Choice::Quit)
    {
        RequestQuit();
    }
}
} // namespace Core::Platform
