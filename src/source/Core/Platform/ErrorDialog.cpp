#include "stdafx.h"

#include "ErrorDialog.h"

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
} // namespace

Choice Show(const std::string& title, const std::string& message, bool canContinue)
{
    const std::vector<SDL_MessageBoxButtonData> buttons = MakeButtons(canContinue);
    // Parented to the focused game window, so it stays in front of it.
    const SDL_MessageBoxData data = {SDL_MESSAGEBOX_ERROR | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT,
                                     SDL_GetKeyboardFocus(),
                                     title.c_str(),
                                     message.c_str(),
                                     static_cast<int>(buttons.size()),
                                     buttons.data(),
                                     nullptr};
    const Choice defaultChoice = canContinue ? Choice::Continue : Choice::Quit;

    while (true)
    {
        int buttonId = -1;
        if (!SDL_ShowMessageBox(&data, &buttonId))
        {
            return defaultChoice;
        }

        switch (buttonId)
        {
        case CopyButton:
            SDL_SetClipboardText((title + "\n\n" + message).c_str());
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
