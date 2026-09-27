#include "stdafx.h"

#include "ErrorDialog.h"
#include "IPlatformWindow.h"
#include "MuPlatform.h"
#include "ScopedSystemCursor.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL.h>

#include <vector>

namespace Core::Platform::ErrorDialog
{
namespace
{
enum ButtonId
{
    CopyButton,
    ContinueButton,
    QuitButton,
};

// Return and Escape choose "Continue" (or "Quit" when the game cannot go
// on), never the copy button.
constexpr SDL_MessageBoxButtonFlags DefaultButtonFlags =
    SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;

std::vector<SDL_MessageBoxButtonData> MakeButtons(bool canContinue)
{
    std::vector<SDL_MessageBoxButtonData> buttons{{0, CopyButton, "Copy text"}};
    if (canContinue)
    {
        buttons.push_back({DefaultButtonFlags, ContinueButton, "Continue"});
        buttons.push_back({0, QuitButton, "Quit"});
    }
    else
    {
        buttons.push_back({DefaultButtonFlags, QuitButton, "Quit"});
    }
    return buttons;
}

// The game window, so the dialog stays in front of it even when the game
// does not have the focus (e.g. during the long loading).
SDL_Window* GetGameWindow()
{
    if (mu::IPlatformWindow* window = mu::MuPlatform::GetWindow())
    {
        return static_cast<SDL_Window*>(window->GetNativeHandle());
    }
    return SDL_GetKeyboardFocus();
}

// On Linux the game hands copied text to other programs itself, from its
// event loop; the text may be gone after the game ends. The log has it too.
void CopyToClipboard(const std::string& text)
{
    if (!SDL_SetClipboardText(text.c_str()))
    {
        MU_LOG_WARN(mu::log::Get("core"), "The error text could not be copied: {}", SDL_GetError());
    }
}
} // namespace

Choice Show(const std::string& title, const std::string& message, bool canContinue)
{
    const std::vector<SDL_MessageBoxButtonData> buttons = MakeButtons(canContinue);
    const SDL_MessageBoxData data = {SDL_MESSAGEBOX_ERROR | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT,
                                     GetGameWindow(),
                                     title.c_str(),
                                     message.c_str(),
                                     static_cast<int>(buttons.size()),
                                     buttons.data(),
                                     nullptr};
    const Choice defaultChoice = canContinue ? Choice::Continue : Choice::Quit;
    const ScopedSystemCursor cursor;

    while (true)
    {
        int buttonId = -1;
        if (!SDL_ShowMessageBox(&data, &buttonId))
        {
            // Like before the dialog had a copy button: an error nobody could
            // see stops the game.
            MU_LOG_ERROR(mu::log::Get("core"), "The error could not be shown ({}): {}", SDL_GetError(), message);
            return Choice::Quit;
        }

        switch (buttonId)
        {
        case CopyButton:
            CopyToClipboard(title + "\n\n" + message);
            continue;
        case ContinueButton:
            return Choice::Continue;
        case QuitButton:
            return Choice::Quit;
        default:
            // Closed without a button.
            return defaultChoice;
        }
    }
}
} // namespace Core::Platform::ErrorDialog
