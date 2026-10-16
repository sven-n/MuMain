#pragma once

#ifdef _EDITOR

struct SDL_Window;
struct ImGuiViewport;

// The editor's own OS windows (ImGui viewports outside the game window).
namespace MuEditor::PlatformWindows
{
// Hooks the platform backend's window handlers, after the backends are set
// up: the editor's windows show the game's icon (Windows, Linux); popups and
// tooltips do not take the keyboard (Linux, macOS); on macOS the editor's
// windows appear without taking the keyboard where ImGui says so, and one that
// cannot be seen counts as minimized; a maximized
// or full-screen window ImGui moves or sizes turns normal first; one closed
// while full screen keeps its windowed place.
void Install();

// Once per frame, after the platform backend's NewFrame and before ImGui's: on
// macOS keeps the editor's windows in front of the game while MU is the
// active app, and tells ImGui which of them the mouse is over (the backend
// cannot there); on Linux drops a mouse position SDL may hold stale while no
// window of the game is under the mouse.
void BeforeNewFrame();

// Frees what the editor's windows share, at shutdown.
void Release();

// The OS window of an editor viewport; null for the game window and while the
// viewport has no OS window yet.
SDL_Window* WindowOf(const ImGuiViewport* viewport);

// The height of the game window's title bar and top border; the editor's own
// windows have the same. 0 without one.
float GameTitleBarHeight();

// Whether the mouse is over one of the editor's own OS windows, for where SDL
// reports no window or the game window under the mouse: on Windows frame and
// title bar included, on macOS by where they float over the game. Always false
// on Linux, where the window manager draws the frames.
bool IsMouseOverOsWindow();

// Whether the viewport's OS window is minimized; not one that is only covered
// (which ImGui also skips on macOS).
bool IsMinimized(const ImGuiViewport* viewport);

bool IsFullscreen(SDL_Window* window);
void ToggleFullscreen(SDL_Window* window);
} // namespace MuEditor::PlatformWindows

#endif // _EDITOR
