#pragma once

#ifdef _EDITOR

struct SDL_Window;
struct ImGuiViewport;

// The editor's own OS windows (ImGui viewports outside the game window).
namespace MuEditor::PlatformWindows
{
// Hooks the platform backend's window handlers, after the backends are set
// up: on Windows the editor's windows show the game's icon in their title bar
// and taskbar button; a maximized or full-screen window ImGui moves or sizes
// turns normal first; one closed while full screen keeps its windowed place.
void Install();

// The OS window of an editor viewport; null for the game window and while the
// viewport has no OS window yet.
SDL_Window* WindowOf(const ImGuiViewport* viewport);

// The height of the game window's title bar and top border; the editor's own
// windows have the same. 0 without one.
float GameTitleBarHeight();

// Whether the mouse is over one of the editor's own OS windows, frame and
// title bar included (Windows only): for where SDL reports no window under the
// mouse.
bool IsMouseOverOsWindow();

bool IsFullscreen(SDL_Window* window);
void ToggleFullscreen(SDL_Window* window);
} // namespace MuEditor::PlatformWindows

#endif // _EDITOR
