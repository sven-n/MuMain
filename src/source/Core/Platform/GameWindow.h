#pragma once

struct SDL_Window;

namespace Core::Platform
{
// The file the game window's icon comes from.
inline constexpr char GameWindowIconFile[] = "MuMainIcon.png";

// The game's window, as set when it was created; before that (and in tests)
// the window SDL opened first, as the editor's own windows come later and SDL
// numbers its windows in the order they open. Null without any.
SDL_Window* GameWindow();

// Called when the game window is created, and with null before it goes.
void SetGameWindow(SDL_Window* window);

// The window a message box is shown over: preferred, but none in macOS editor
// builds, where the editor's windows float above the game window and would
// cover a dialog sheet on it (a free-standing alert shows above them).
SDL_Window* MessageBoxParent(SDL_Window* preferred);
} // namespace Core::Platform
