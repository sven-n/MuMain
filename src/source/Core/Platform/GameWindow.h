#pragma once

struct SDL_Window;

namespace Core::Platform
{
// The file the game window's icon comes from.
inline constexpr char GameWindowIconFile[] = "MuMainIcon.png";

// The game's window: the client opens it first (the editor's own windows come
// later), and SDL numbers its windows in the order they open. Null before it
// exists.
SDL_Window* GameWindow();

// The window a message box is shown over: preferred, but none in macOS editor
// builds, where the editor's windows float above the game window and would
// cover a dialog sheet on it (a free-standing alert shows above them).
SDL_Window* MessageBoxParent(SDL_Window* preferred);
} // namespace Core::Platform
