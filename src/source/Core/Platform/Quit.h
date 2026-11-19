#pragma once

#include <string>

namespace Core::Platform
{
// Asks the main loop to stop. Safe from any thread and at any point, also before SDL's events run (during startup):
// the loop handles the request once, at its next frame, and the teardown then undoes the startup in reverse order.
void RequestQuit();
bool QuitRequested();

// Shows the error in the error dialog, without "Continue", and then asks the loop to stop. Once a quit is requested
// no further dialog is shown; the error is only logged. The texts are UTF-8.
void ShowFatalError(const std::string& title, const std::string& message);
void ShowFatalError(const wchar_t* title, const wchar_t* message);

// Shows an error the game can go on after: "Continue" returns, "Quit" asks the loop to stop. Shows nothing once a quit
// is requested. The texts are UTF-8.
void ShowError(const std::string& title, const std::string& message);
} // namespace Core::Platform
