#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorPlatformWindows.h"

#include "imgui.h"
#include "imgui_internal.h"
#include <SDL3/SDL_video.h>

#include <cstdint>

namespace
{
// The platform backend's handlers that the editor's own wrap.
struct BackendHandlers
{
    void (*createWindow)(ImGuiViewport* viewport) = nullptr;
    void (*destroyWindow)(ImGuiViewport* viewport) = nullptr;
    void (*setWindowPos)(ImGuiViewport* viewport, ImVec2 position) = nullptr;
    void (*setWindowSize)(ImGuiViewport* viewport, ImVec2 size) = nullptr;
};
BackendHandlers g_backend;

SDL_Window* SdlWindowOf(const ImGuiViewport* viewport)
{
    if (viewport == nullptr || viewport->PlatformHandle == nullptr)
    {
        return nullptr;
    }
    // The SDL3 platform backend keeps the SDL window id as the handle.
    return SDL_GetWindowFromID(static_cast<SDL_WindowID>(reinterpret_cast<std::intptr_t>(viewport->PlatformHandle)));
}

// The game window's icon (set from MuMainIcon.png) in place of the generic one.
void CopyGameIcon(ImGuiViewport* viewport)
{
#ifdef _WIN32
    const HWND window = static_cast<HWND>(viewport->PlatformHandleRaw);
    const HWND game = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);
    if (window == nullptr || game == nullptr)
    {
        return;
    }
    for (const WPARAM size : {static_cast<WPARAM>(ICON_BIG), static_cast<WPARAM>(ICON_SMALL)})
    {
        if (const LRESULT icon = SendMessageW(game, WM_GETICON, size, 0))
        {
            SendMessageW(window, WM_SETICON, size, icon);
        }
    }
#else
    (void)viewport;
#endif
}

void CreateEditorWindow(ImGuiViewport* viewport)
{
    g_backend.createWindow(viewport);
    CopyGameIcon(viewport);
}

// SDL ignores moving or sizing a maximized or full-screen window, so ImGui
// would no longer match it. Back to a normal window first, as dragging a
// maximized window's title bar does.
void LeaveMaximizedOrFullscreen(ImGuiViewport* viewport)
{
    SDL_Window* window = SdlWindowOf(viewport);
    if (window == nullptr)
    {
        return;
    }
    const SDL_WindowFlags flags = SDL_GetWindowFlags(window);
    if ((flags & SDL_WINDOW_FULLSCREEN) != 0)
    {
        SDL_SetWindowFullscreen(window, false);
    }
    if ((flags & SDL_WINDOW_MAXIMIZED) != 0)
    {
        SDL_RestoreWindow(window);
    }
}

void MoveEditorWindow(ImGuiViewport* viewport, ImVec2 position)
{
    LeaveMaximizedOrFullscreen(viewport);
    g_backend.setWindowPos(viewport, position);
}

void ResizeEditorWindow(ImGuiViewport* viewport, ImVec2 size)
{
    LeaveMaximizedOrFullscreen(viewport);
    g_backend.setWindowSize(viewport, size);
}

// Hands ImGui the window's windowed rect, which it keeps (and saves to
// imgui.ini) for when the window opens again.
void KeepWindowedRect(ImGuiViewport* viewport, SDL_Window* window)
{
    ImGuiWindow* imguiWindow = static_cast<ImGuiViewportP*>(viewport)->Window;
    if (imguiWindow == nullptr)
    {
        return;
    }
    int x = 0, y = 0, width = 0, height = 0;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_GetWindowSize(window, &width, &height);
    const ImVec2 position(static_cast<float>(x), static_cast<float>(y));
    const ImVec2 size(static_cast<float>(width), static_cast<float>(height));
    imguiWindow->Pos = position;
    imguiWindow->Size = imguiWindow->SizeFull = size;
    // A floating tool window is shown by its dock node, whose rect imgui.ini keeps.
    if (ImGuiDockNode* node = imguiWindow->DockNodeAsHost)
    {
        node->Pos = position;
        node->Size = node->SizeRef = size;
    }
    ImGui::MarkIniSettingsDirty();
}

// A window closed while full screen would come back as a plain window covering
// the monitor, its title bar above the screen: it goes back to windowed first.
void DestroyEditorWindow(ImGuiViewport* viewport)
{
    SDL_Window* window = SdlWindowOf(viewport);
    if (MuEditor::PlatformWindows::IsFullscreen(window))
    {
        SDL_SetWindowFullscreen(window, false);
        SDL_SyncWindow(window);
        KeepWindowedRect(viewport, window);
    }
    g_backend.destroyWindow(viewport);
}
} // namespace

namespace MuEditor::PlatformWindows
{
void Install()
{
    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    // No windows of their own on this platform, or installed already.
    if (platformIO.Platform_CreateWindow == nullptr || platformIO.Platform_CreateWindow == CreateEditorWindow)
    {
        return;
    }
    g_backend.createWindow = platformIO.Platform_CreateWindow;
    g_backend.destroyWindow = platformIO.Platform_DestroyWindow;
    g_backend.setWindowPos = platformIO.Platform_SetWindowPos;
    g_backend.setWindowSize = platformIO.Platform_SetWindowSize;
    platformIO.Platform_CreateWindow = CreateEditorWindow;
    platformIO.Platform_DestroyWindow = DestroyEditorWindow;
    platformIO.Platform_SetWindowPos = MoveEditorWindow;
    platformIO.Platform_SetWindowSize = ResizeEditorWindow;
}

SDL_Window* WindowOf(const ImGuiViewport* viewport)
{
    if (viewport == ImGui::GetMainViewport())
    {
        return nullptr;
    }
    return SdlWindowOf(viewport);
}

float GameTitleBarHeight()
{
    SDL_Window* game = SdlWindowOf(ImGui::GetMainViewport());
    int top = 0;
    if (game == nullptr || !SDL_GetWindowBordersSize(game, &top, nullptr, nullptr, nullptr))
    {
        return 0.0f;
    }
    return static_cast<float>(top);
}

bool IsMouseOverOsWindow()
{
#ifdef _WIN32
    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    POINT cursor{};
    if (platformIO.Viewports.Size <= 1 || !GetCursorPos(&cursor))
    {
        return false;
    }
    const HWND under = GetAncestor(WindowFromPoint(cursor), GA_ROOT);
    for (int i = 1; i < platformIO.Viewports.Size; ++i)
    {
        if (platformIO.Viewports[i]->PlatformHandleRaw == under)
        {
            return true;
        }
    }
#endif
    return false;
}

bool IsFullscreen(SDL_Window* window)
{
    return window != nullptr && (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void ToggleFullscreen(SDL_Window* window)
{
    if (window == nullptr)
    {
        return;
    }
    // Full screen on the window's monitor at the desktop's resolution.
    SDL_SetWindowFullscreen(window, !IsFullscreen(window));
}
} // namespace MuEditor::PlatformWindows

#endif // _EDITOR
