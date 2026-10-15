#pragma once

struct SDL_Window;

namespace Core::Platform
{
// The game's window: the client opens it first (the editor's own windows come
// later), and SDL numbers its windows in the order they open. Null before it
// exists.
SDL_Window* GameWindow();
} // namespace Core::Platform
