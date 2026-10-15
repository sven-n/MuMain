#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorPlatformWindows.h"

#include "Core/Platform/GameWindow.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace
{
// The platform backend's handlers that the editor's own wrap.
struct BackendHandlers
{
    void (*createWindow)(ImGuiViewport* viewport) = nullptr;
    void (*showWindow)(ImGuiViewport* viewport) = nullptr;
    void (*destroyWindow)(ImGuiViewport* viewport) = nullptr;
    void (*setWindowPos)(ImGuiViewport* viewport, ImVec2 position) = nullptr;
    void (*setWindowSize)(ImGuiViewport* viewport, ImVec2 size) = nullptr;
    bool (*getWindowMinimized)(ImGuiViewport* viewport) = nullptr;
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

#ifdef __linux__
SDL_Surface* g_gameIcon = nullptr;
bool g_gameIconLoaded = false;

SDL_Surface* GameIcon()
{
    if (!g_gameIconLoaded)
    {
        g_gameIcon = SDL_LoadPNG(Core::Platform::GameWindowIconFile);
        g_gameIconLoaded = true;
    }
    return g_gameIcon;
}

// With no window of the game under the mouse, SDL's global mouse position can
// be stale (XWayland stops updating it over other apps' windows), which would
// keep editor widgets hovered: no position then, as when the mouse leaves.
void ForgetMouseOutsideWindows()
{
    ImGuiIO& io = ImGui::GetIO();
    const bool platformWindowsOn = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
    if (platformWindowsOn && SDL_GetMouseFocus() == nullptr && SDL_GetMouseState(nullptr, nullptr) == 0)
    {
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }
}
#endif

#ifdef __APPLE__
// macOS' standard title bar; SDL cannot tell window borders there.
constexpr float MacTitleBarHeight = 28.0f;

// Sets an SDL hint for a scope and puts the earlier value back.
class ScopedHint
{
public:
    ScopedHint(const char* name, const char* value) : m_name(name)
    {
        if (const char* previous = SDL_GetHint(name))
        {
            m_previous = previous;
            m_hadValue = true;
        }
        SDL_SetHint(name, value);
    }
    ~ScopedHint()
    {
        if (m_hadValue)
        {
            SDL_SetHint(m_name, m_previous.c_str());
        }
        else
        {
            SDL_ResetHint(m_name);
        }
    }
    ScopedHint(const ScopedHint&) = delete;
    ScopedHint& operator=(const ScopedHint&) = delete;

private:
    const char* m_name;
    std::string m_previous;
    bool m_hadValue = false;
};

// macOS has no owned windows that keep the editor in front of the game
// without moving with it (child windows do), so the editor's windows float
// above normal windows while MU is the active app and drop to the normal
// level while another app is. Focus passes through no window for a frame
// between two of MU's windows, which does not count as MU going inactive.
constexpr int InactiveFramesBeforeNormalLevel = 2;
int g_framesInactive = 0;

bool IsAppActive()
{
    return g_framesInactive < InactiveFramesBeforeNormalLevel;
}

bool g_appWasActive = true;

void ApplyWindowLevel(ImGuiViewport* viewport, bool appActive)
{
    // Tooltips have ImGui's own always-on-top (raised again after a level
    // change); a full-screen window is in a Space of its own.
    SDL_Window* window = SdlWindowOf(viewport);
    if (window == nullptr || (viewport->Flags & ImGuiViewportFlags_TopMost) != 0 ||
        MuEditor::PlatformWindows::IsFullscreen(window))
    {
        return;
    }
    SDL_SetWindowAlwaysOnTop(window, appActive);
}

void RaiseWithoutActivation(SDL_Window* window)
{
    const ScopedHint noActivation(SDL_HINT_WINDOW_ACTIVATE_WHEN_RAISED, "0");
    SDL_RaiseWindow(window);
}

// AppKit puts a window that changes level in front of the others of its new
// level. When MU's active state changes, the windows change level in the
// order they were last focused, so the one focused last ends up in front, as
// EditorViewportAt assumes; tooltips go in front of them again.
void ChangeLevels(bool appActive)
{
    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    std::vector<ImGuiViewportP*> byFocus;
    for (int i = 1; i < platformIO.Viewports.Size; ++i)
    {
        byFocus.push_back(static_cast<ImGuiViewportP*>(platformIO.Viewports[i]));
    }
    std::sort(byFocus.begin(), byFocus.end(), [](const ImGuiViewportP* a, const ImGuiViewportP* b)
              { return a->LastFocusedStampCount < b->LastFocusedStampCount; });
    for (ImGuiViewportP* viewport : byFocus)
    {
        ApplyWindowLevel(viewport, appActive);
    }
    for (ImGuiViewportP* viewport : byFocus)
    {
        SDL_Window* window = SdlWindowOf(viewport);
        if ((viewport->Flags & ImGuiViewportFlags_TopMost) != 0 && window != nullptr &&
            (SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN) == 0)
        {
            RaiseWithoutActivation(window);
        }
    }
}

void KeepInFrontOfGame()
{
    g_framesInactive = SDL_GetKeyboardFocus() != nullptr ? 0 : g_framesInactive + 1;
    const bool appActive = IsAppActive();
    if (appActive != g_appWasActive)
    {
        g_appWasActive = appActive;
        ChangeLevels(appActive);
        return;
    }
    // A new window got its level when it was created; one back from a
    // full-screen Space gets it again here.
    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    for (int i = 1; i < platformIO.Viewports.Size; ++i)
    {
        ApplyWindowLevel(platformIO.Viewports[i], appActive);
    }
}

// An editor window that cannot be seen - in a full-screen Space the user left,
// or wholly covered - counts as minimized: ImGui then neither draws it nor
// finds the mouse over it.
bool IsEditorWindowMinimized(ImGuiViewport* viewport)
{
    if (g_backend.getWindowMinimized(viewport))
    {
        return true;
    }
    SDL_Window* window = viewport != ImGui::GetMainViewport() ? SdlWindowOf(viewport) : nullptr;
    return window != nullptr && (SDL_GetWindowFlags(window) & SDL_WINDOW_OCCLUDED) != 0;
}

// The editor window under the mouse: where several overlap, the one focused
// last. They float over the game, so one of them under the mouse is in front.
ImGuiViewportP* EditorViewportAt(ImVec2 mouse, bool withTitleBar)
{
    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    ImGuiViewportP* found = nullptr;
    for (int i = 1; i < platformIO.Viewports.Size; ++i)
    {
        auto* viewport = static_cast<ImGuiViewportP*>(platformIO.Viewports[i]);
        if ((viewport->Flags & (ImGuiViewportFlags_NoInputs | ImGuiViewportFlags_IsMinimized)) != 0)
        {
            continue;
        }
        ImRect rect = viewport->GetMainRect();
        if (withTitleBar && (viewport->Flags & ImGuiViewportFlags_NoDecoration) == 0)
        {
            rect.Min.y -= MacTitleBarHeight;
        }
        if (rect.Contains(mouse) &&
            (found == nullptr || viewport->LastFocusedStampCount > found->LastFocusedStampCount))
        {
            found = viewport;
        }
    }
    return found;
}

ImVec2 GlobalMousePosition()
{
    float x = 0.0f, y = 0.0f;
    SDL_GetGlobalMouseState(&x, &y);
    return ImVec2(x, y);
}

// The backend reports no hovered window on macOS, and ImGui's own guess from
// focus order can take a floating editor window for the game behind it.
void ReportHoveredWindow()
{
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0 || ImGui::GetDragDropPayload() != nullptr)
    {
        return;
    }
    // Over an editor window's title bar too, as IsMouseOverOsWindow: no editor
    // widget is there, and the game under it is not hovered.
    const ImVec2 mouse = GlobalMousePosition();
    ImGuiViewport* hovered = EditorViewportAt(mouse, true);
    ImGuiViewport* game = ImGui::GetMainViewport();
    if (hovered == nullptr && static_cast<ImGuiViewportP*>(game)->GetMainRect().Contains(mouse))
    {
        hovered = game;
    }
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
    io.AddMouseViewportEvent(hovered != nullptr ? hovered->ID : 0);
}

// The backend makes every window it shows on macOS the key window (AppKit
// puts one shown without that behind the key window). One that should not
// take the keyboard is shown without it and brought to the front instead.
void ShowEditorWindow(ImGuiViewport* viewport)
{
    SDL_Window* window = SdlWindowOf(viewport);
    const bool takesKeyboard = (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing) == 0;
    if (window == nullptr || takesKeyboard)
    {
        g_backend.showWindow(viewport);
        return;
    }
    {
        const ScopedHint noActivation(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "0");
        SDL_ShowWindow(window);
    }
    RaiseWithoutActivation(window);
}
#endif

// The game window's icon in place of the generic one: copied on Windows, from
// its file on Linux. macOS shows the app's icon in the Dock only.
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
#elif defined(__linux__)
    // Popups and tooltips have no title bar to show it in.
    SDL_Window* window = SdlWindowOf(viewport);
    if (window == nullptr || (viewport->Flags & ImGuiViewportFlags_NoDecoration) != 0)
    {
        return;
    }
    if (SDL_Surface* icon = GameIcon())
    {
        SDL_SetWindowIcon(window, icon);
    }
#else
    (void)viewport;
#endif
}

void CreateEditorWindow(ImGuiViewport* viewport)
{
    g_backend.createWindow(viewport);
    CopyGameIcon(viewport);
#ifndef _WIN32
    // Popups, menus and tooltips: clicks go to them, the keyboard stays where
    // it is (ImGui gets its keys from the window that has it).
    SDL_Window* window = SdlWindowOf(viewport);
    if (window != nullptr && (viewport->Flags & ImGuiViewportFlags_NoFocusOnClick) != 0)
    {
        SDL_SetWindowFocusable(window, false);
    }
#endif
#ifdef __APPLE__
    // Its level before it first shows.
    ApplyWindowLevel(viewport, IsAppActive());
#endif
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
    g_backend.showWindow = platformIO.Platform_ShowWindow;
    g_backend.destroyWindow = platformIO.Platform_DestroyWindow;
    g_backend.setWindowPos = platformIO.Platform_SetWindowPos;
    g_backend.setWindowSize = platformIO.Platform_SetWindowSize;
    platformIO.Platform_CreateWindow = CreateEditorWindow;
    platformIO.Platform_DestroyWindow = DestroyEditorWindow;
    platformIO.Platform_SetWindowPos = MoveEditorWindow;
    platformIO.Platform_SetWindowSize = ResizeEditorWindow;
#ifdef __APPLE__
    g_backend.getWindowMinimized = platformIO.Platform_GetWindowMinimized;
    platformIO.Platform_ShowWindow = ShowEditorWindow;
    if (g_backend.getWindowMinimized != nullptr)
    {
        platformIO.Platform_GetWindowMinimized = IsEditorWindowMinimized;
    }
#endif
}

void BeforeNewFrame()
{
#ifdef __APPLE__
    KeepInFrontOfGame();
    ReportHoveredWindow();
#elif defined(__linux__)
    ForgetMouseOutsideWindows();
#endif
}

void Release()
{
#ifdef __linux__
    if (g_gameIcon != nullptr)
    {
        SDL_DestroySurface(g_gameIcon);
        g_gameIcon = nullptr;
    }
    g_gameIconLoaded = false;
#endif
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
    if (game != nullptr && SDL_GetWindowBordersSize(game, &top, nullptr, nullptr, nullptr))
    {
        return static_cast<float>(top);
    }
#ifdef __APPLE__
    if (game != nullptr && (SDL_GetWindowFlags(game) & (SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN)) == 0)
    {
        return MacTitleBarHeight;
    }
#endif
    return 0.0f;
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
#elif defined(__APPLE__)
    return EditorViewportAt(GlobalMousePosition(), true) != nullptr;
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
    const bool enter = !IsFullscreen(window);
#ifdef __APPLE__
    // A full-screen Space takes a window at the normal level; back to
    // floating once it leaves (KeepInFrontOfGame).
    if (enter)
    {
        SDL_SetWindowAlwaysOnTop(window, false);
    }
#endif
    // Full screen on the window's monitor at the desktop's resolution.
    SDL_SetWindowFullscreen(window, enter);
}
} // namespace MuEditor::PlatformWindows

#endif // _EDITOR
