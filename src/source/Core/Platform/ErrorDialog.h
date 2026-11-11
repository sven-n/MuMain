#pragma once

#include <string>

// An error dialog with a "Copy text" button, so players can paste the
// message into a bug report. Built on SDL's message box, so it works on
// every platform.
namespace Core::Platform::ErrorDialog
{
enum class Choice
{
    Continue,
    Quit,
};

// Shows the error with the buttons "Copy text", "Continue" and "Quit"
// ("Copy text" and "Quit" when canContinue is false). "Copy text" puts the
// title and the message on the clipboard and shows the dialog again. The
// texts are UTF-8.
Choice Show(const std::string& title, const std::string& message, bool canContinue);
} // namespace Core::Platform::ErrorDialog
