#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorCore.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include "MuInputBlockerCore.h"
#include "MuEditorCursor.h"
#include "MuEditorPlatformWindows.h"
#include "Core/Input/KeyState.h"
#include "Core/Input/SyntheticInput.h"
#include "../Config/MuEditorConfig.h"
#include "../MuEditor/UI/Common/MuEditorCenterPaneUI.h"
#include "../MuEditor/UI/ItemEditor/MuItemEditorUI.h"
#include "../MuEditor/UI/SkillEditor/MuSkillEditorUI.h"
#include "../MuEditor/UI/DevEditor/DevEditorUI.h"
#include "../MuEditor/UI/MapEditor/MapEditorUI.h"
#include "../MuEditor/UI/EffectBrowser/MuEffectBrowserUI.h"
#include "../UI/Common/MuEditorUI.h"
#include "../UI/Console/MuEditorConsoleUI.h"
#include "I18N/All.h"
#include "Core/Utilities/AssetLoadWorld.h"
#include "Core/Utilities/WorldClearing.h"
#include "Core/Utilities/StringUtils.h"
#include "World/MapInfra/MapManager.h"
#include "Render/Renderer/MuRenderer.h"

namespace mu
{
void QueueEditorRenderCommand();
}

#ifndef _WIN32
#include <cstdio>    // popen / pclose
#include <unistd.h>  // access
#include <string>
#include <vector>
namespace
{
    // Resolve a font file via fontconfig (distro-agnostic), e.g.
    // EditorFcMatch("sans-serif"). Empty if fc-match is unavailable.
    std::string EditorFcMatch(const char* pattern)
    {
        const std::string cmd = std::string("fc-match -f '%{file}' '") + pattern + "' 2>/dev/null";
        FILE* p = popen(cmd.c_str(), "r");
        if (!p) return {};
        std::string out;
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), p)) > 0)
            out.append(buf, n);
        pclose(p);
        while (!out.empty() && (out.back() == '\n' || out.back() == '\r' || out.back() == ' '))
            out.pop_back();
        return out;
    }
    bool FontFileExists(const char* path) { return path && path[0] && ::access(path, R_OK) == 0; }
}
#endif

// Cursor display counter thresholds (Win32's counter, or the game's ShowCursor
// shim over SDL's cursor off Windows)
// The cursor is visible when the counter is >= CURSOR_VISIBLE_THRESHOLD
// The cursor is hidden when the counter is < CURSOR_VISIBLE_THRESHOLD
constexpr int CURSOR_VISIBLE_THRESHOLD = 0;

// The frames in a row the game skips while the GPU is behind; more, and its
// window is out of sight (RenderPlatformWindows).
constexpr int SkippedGameFramesWhileBehind = 1;

// Editor UI scale limits (the menu bar's -/+ buttons step by 0.1).
constexpr float UI_SCALE_MIN = 0.7f;
constexpr float UI_SCALE_MAX = 2.5f;

namespace
{
// The editor's own windows present without waiting for the display where the
// driver can, so that they never hold back the game's frame.
SDL_GPUPresentMode EditorWindowPresentMode(SDL_GPUDevice* device, SDL_Window* gameWindow)
{
    for (const SDL_GPUPresentMode mode : {SDL_GPU_PRESENTMODE_MAILBOX, SDL_GPU_PRESENTMODE_IMMEDIATE})
    {
        if (SDL_WindowSupportsGPUPresentMode(device, gameWindow, mode))
        {
            return mode;
        }
    }
    return SDL_GPU_PRESENTMODE_VSYNC;
}

bool InitializeImGuiBackends(SDL_Window* window)
{
    SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
    if (device == nullptr)
    {
        fwprintf(stderr, L"[MuEditor] Initialize failed: SDL GPU device is null\n");
        fflush(stderr);
        return false;
    }

    const SDL_GPUTextureFormat colorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
    if (colorTargetFormat == SDL_GPU_TEXTUREFORMAT_INVALID)
    {
        fwprintf(stderr, L"[MuEditor] Initialize failed: invalid SDL GPU swapchain format: %hs\n", SDL_GetError());
        fflush(stderr);
        return false;
    }

    if (!ImGui_ImplSDL3_InitForSDLGPU(window))
    {
        fwprintf(stderr, L"[MuEditor] ImGui_ImplSDL3_InitForSDLGPU failed\n");
        fflush(stderr);
        return false;
    }

    ImGui_ImplSDLGPU3_InitInfo initInfo = {};
    initInfo.Device = device;
    initInfo.ColorTargetFormat = colorTargetFormat;
    initInfo.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    initInfo.PresentMode = EditorWindowPresentMode(device, window);
    if (!ImGui_ImplSDLGPU3_Init(&initInfo))
    {
        fwprintf(stderr, L"[MuEditor] ImGui_ImplSDLGPU3_Init failed\n");
        fflush(stderr);
        ImGui_ImplSDL3_Shutdown();
        return false;
    }

    return true;
}
}

CMuEditorCore::CMuEditorCore()
    : m_bEditorMode(false)
    , m_bInitialized(false)
    , m_bFrameStarted(false)
    , m_bDrawDataReady(false)
    , m_bShowItemEditor(false)
    , m_bShowSkillEditor(false)
    , m_bShowDevEditor(false)
    , m_bShowMapEditor(false)
    , m_bShowEffectBrowser(false)
    , m_bShowConsole(true)
    , m_bHoveringUI(false)
    , m_bPreviousFrameHoveringUI(false)
    , m_UIScale(1.0f)
    , m_bScaleDirty(false)
{
}

void CMuEditorCore::SetUIScale(float scale)
{
    // Clamp to something sane; the menu bar steps this in 0.1 increments.
    if (scale < UI_SCALE_MIN) scale = UI_SCALE_MIN;
    if (scale > UI_SCALE_MAX) scale = UI_SCALE_MAX;
    if (scale == m_UIScale)
        return;
    m_UIScale = scale;
    m_bScaleDirty = true;   // applied in Update(), before the next NewFrame
}

void CMuEditorCore::ApplyUIScale()
{
    // ScaleAllSizes multiplies, so rebuild the style from scratch each time
    // (defaults -> our theme -> scale) rather than compounding.
    ImGuiStyle& style = ImGui::GetStyle();
    style = ImGuiStyle();
    ImGui::StyleColorsDark();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    style.ScaleAllSizes(m_UIScale);

    ImGui::GetIO().FontGlobalScale = m_UIScale;
}

CMuEditorCore::~CMuEditorCore()
{
    Shutdown();
}

CMuEditorCore& CMuEditorCore::GetInstance()
{
    static CMuEditorCore instance;
    return instance;
}

// Queried by game code that must behave differently while the editor is up (e.g.
// Input/Selection.cpp suppresses the interactable/pose-box pick, whose sit-down
// cursor would otherwise latch because the editor consumes the clicks that would
// normally clear it).
extern "C" bool MuEditor_IsOpen()
{
    return CMuEditorCore::GetInstance().IsEnabled();
}

void CMuEditorCore::Initialize(SDL_Window* window)
{
    if (m_bInitialized)
        return;

    ConnectGameHooks();

    if (window == nullptr)
    {
        fwprintf(stderr, L"[MuEditor] Initialize failed: window is null\n");
        fflush(stderr);
        return;
    }

    fwprintf(stderr, L"[MuEditor] Initialize() called\n");
    fflush(stderr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // The OS cursor over the editor; the game cursor is hidden there (see
    // UpdateCursorVisibility)
    io.MouseDrawCursor = false; // Don't let ImGui draw its own cursor, use the OS cursor
#ifndef _WIN32
    // Off Windows SDL has one cursor for all of the game's windows, which the
    // game keeps blank: the editor sets it over its windows itself
    // (UpdateCursorVisibility), so the backend must not.
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
#endif

    // The editor's windows dock into one another and can leave the game window
    // (an OS window each, see SelectWindowMode), staying in front of it: the
    // platform backend makes them children of the game window, except on
    // macOS, where they float above it while MU is the active app
    // (MuEditor::PlatformWindows::BeforeNewFrame). Those windows have the OS
    // title bar with its buttons.
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigViewportsNoDefaultParent = false;
    io.ConfigViewportsNoDecoration = false;
    m_pGameWindow = window;
    // ImGui sees the game window's keys too, and its Ctrl+Tab window switch
    // (force attack and the minimap in the game) would move the keyboard into
    // an editor window.
    ImGui::GetCurrentContext()->ConfigNavWindowingKeyNext = 0;
    ImGui::GetCurrentContext()->ConfigNavWindowingKeyPrev = 0;

    fwprintf(stderr, L"[MuEditor] ImGui context created\n");
    fflush(stderr);

    // Load font with extended Unicode support for multiple languages
    // This includes Latin, Cyrillic, Greek, and other common character sets
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    fontConfig.PixelSnapH = true;

    // Build font atlas with multiple Unicode ranges
    // CJK ranges are handled by a separate merged font below — primary font
    // (e.g. Segoe UI) does not contain Han glyphs, so listing CJK here would
    // just rasterize empty glyphs.
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());        // Basic Latin + Latin Supplement
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());       // Cyrillic (Russian, Ukrainian, etc.)
    builder.AddRanges(io.Fonts->GetGlyphRangesGreek());          // Greek

    // Add additional specific characters if needed
    static const ImWchar additionalRanges[] = {
        0x0100, 0x017F, // Latin Extended-A (Polish, etc.)
        0x0180, 0x024F, // Latin Extended-B
        0x1E00, 0x1EFF, // Latin Extended Additional (Vietnamese)
        0,
    };
    builder.AddRanges(additionalRanges);

    // ImGui stores the ranges pointer in ImFontConfig and reads it lazily when
    // the atlas is built (after Initialize returns), so the storage must outlive
    // this function — keep it static.
    static ImVector<ImWchar> ranges;
    ranges.clear();
    builder.BuildRanges(&ranges);

    // CJK ranges supplied by a merged system CJK font. Full covers Traditional
    // Chinese (zh-TW translations) and is a superset of Japanese kana/kanji.
    const ImWchar* cjkRanges = io.Fonts->GetGlyphRangesChineseFull();

    // Load default font with extended ranges
    // Try platform-specific fonts that support extended Unicode
    bool fontLoaded = false;

    // Config for the merged CJK font: same atlas, glyphs fill into the primary
    // ImFont so CJK codepoints render alongside Latin without manual font switching.
    // Skip oversampling for CJK — ChineseFull rasterizes ~21k ideographs and 2x2
    // oversampling can blow past the GPU's max texture size.
    ImFontConfig cjkConfig = fontConfig;
    cjkConfig.MergeMode = true;
    cjkConfig.OversampleH = 1;
    cjkConfig.OversampleV = 1;

#ifdef _WIN32
    // Windows: Get fonts directory dynamically
    wchar_t windowsDir[MAX_PATH];
    if (GetWindowsDirectoryW(windowsDir, MAX_PATH) > 0)
    {
        std::wstring fontDirW = std::wstring(windowsDir) + L"\\Fonts\\";
        std::wstring fontPathW = fontDirW + L"segoeui.ttf";

        // Convert to UTF-8 using StringUtils helper
        std::string fontPath = StringUtils::WideToNarrow(fontPathW.c_str());
        if (!fontPath.empty())
        {
            if (io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, &fontConfig, ranges.Data) != nullptr)
            {
                fontLoaded = true;
            }
        }

        if (fontLoaded)
        {
            // Try Traditional Chinese first (matches zh-TW translations), then
            // Simplified, then older fallbacks. First successful merge wins.
            const wchar_t* cjkFonts[] = {
                L"msjh.ttc",   // Microsoft JhengHei (Traditional)
                L"msyh.ttc",   // Microsoft YaHei (Simplified)
                L"mingliu.ttc",
                L"simsun.ttc",
            };
            for (const wchar_t* name : cjkFonts)
            {
                std::string cjkPath = StringUtils::WideToNarrow((fontDirW + name).c_str());
                if (!cjkPath.empty() &&
                    io.Fonts->AddFontFromFileTTF(cjkPath.c_str(), 16.0f, &cjkConfig, cjkRanges) != nullptr)
                {
                    break;
                }
            }
        }
    }
#elif __APPLE__
    // macOS: Try system fonts
    const char* macFonts[] = {
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial Unicode.ttf"
    };
    for (const char* fontPath : macFonts)
    {
        if (io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &fontConfig, ranges.Data) != nullptr)
        {
            fontLoaded = true;
            break;
        }
    }
    if (fontLoaded)
    {
        const char* cjkFonts[] = {
            "/System/Library/Fonts/PingFang.ttc",
            "/Library/Fonts/Arial Unicode.ttf",
        };
        for (const char* path : cjkFonts)
        {
            if (io.Fonts->AddFontFromFileTTF(path, 16.0f, &cjkConfig, cjkRanges) != nullptr)
            {
                break;
            }
        }
    }
#else
    // Linux/Unix: resolve a system font via fontconfig (distro-agnostic), with
    // well-known paths as a fallback. Skip paths that don't exist so ImGui does
    // not log "Could not load font file!" for each miss.
    std::vector<std::string> linuxFonts;
    if (std::string fc = EditorFcMatch("sans-serif"); !fc.empty())
        linuxFonts.push_back(std::move(fc));
    for (const char* p : {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf" })
        linuxFonts.emplace_back(p);
    for (const std::string& fontPath : linuxFonts)
    {
        if (FontFileExists(fontPath.c_str()) &&
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, &fontConfig, ranges.Data) != nullptr)
        {
            fontLoaded = true;
            break;
        }
    }
    if (fontLoaded)
    {
        // CJK is best-effort; keep the well-known paths (fc-match can return a
        // non-CJK font when none is installed), but only try ones that exist.
        const char* cjkFonts[] = {
            "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
            "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
            "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
            "/usr/share/fonts/wqy-microhei/wqy-microhei.ttc",
        };
        for (const char* path : cjkFonts)
        {
            if (FontFileExists(path) &&
                io.Fonts->AddFontFromFileTTF(path, 16.0f, &cjkConfig, cjkRanges) != nullptr)
            {
                break;
            }
        }
    }
#endif

    // Fallback to ImGui's default font if no system font loaded
    // Note: Default font only supports basic ASCII, not extended ranges
    if (!fontLoaded)
    {
        io.Fonts->AddFontDefault(&fontConfig);
    }

    // Note: Don't call io.Fonts->Build() - the backend will build it automatically

    fwprintf(stderr, L"[MuEditor] Font loaded with Unicode support\n");
    fflush(stderr);

    // Dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    if (!InitializeImGuiBackends(window))
    {
        ImGui::DestroyContext();
        return;
    }

    MuEditor::PlatformWindows::Install();
    LogVideoDriver();

    fwprintf(stderr, L"[MuEditor] ImGui backends initialized\n");
    fflush(stderr);

    m_bInitialized = true;
    g_MuEditorConsoleUI.LogEditor("MU Editor initialized");

    // Editor config holds editor-only preferences (column visibility etc.).
    // The active UI locale lives in GameConfig.UILocale and was applied by
    // Winmain before we got here; do not touch I18N::SetLocale from editor
    // init or it'll overwrite the game-side selection.
    g_MuEditorConfig.Load();
    g_MuEditorConsoleUI.LogEditor(std::string("Active locale: ") + I18N::GetCurrentLocale());

    fwprintf(stderr, L"[MuEditor] Initialize() completed\n");
    fflush(stderr);
}

void CMuEditorCore::ConnectGameHooks()
{
    // The effect browser shows what loaded each model and texture; the
    // records take the map of the moment from the map manager.
    Core::AssetLoadWorld::SetSource([] { return gMapManager.WorldActive; });
    // The effect browser's world preview removes its objects before the game
    // clears its pools.
    Core::WorldClearing::SetListener([] { g_MuEffectBrowserUI.OnWorldClearing(); });
    // The game's key reads leave out what is pressed in the editor's own windows.
    Core::Input::SetOtherWindowQuery([](int virtualKey) { return g_MuEditorCore.IsWithEditorWindow(virtualKey); });
}

void CMuEditorCore::Shutdown()
{
    if (!m_bInitialized)
        return;

    // Do not touch the editor singletons here: their destructors already save the column
    // preferences, and when this runs from ~CMuEditorCore during static destruction they
    // would be built from scratch or, if they were opened, already destroyed.

    Core::WorldClearing::SetListener(nullptr);
    Core::Input::SetOtherWindowQuery(nullptr);

    mu::WaitForSDLGpuIdle();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    MuEditor::PlatformWindows::Release();
    MuEditor::Cursor::Release();

    m_bInitialized = false;
    m_bDrawDataReady = false;
    m_bPlatformWindowsPending = false;
}

void CMuEditorCore::Update()
{
    if (!m_bInitialized)
        return;

    // Once per frame: the game's mouse-button reads ask it many times.
    m_bMouseInEditorWindow = FindMouseInEditorWindow();

    // Apply a pending UI scale change between frames (never mid-frame).
    if (m_bScaleDirty)
    {
        ApplyUIScale();
        m_bScaleDirty = false;
    }

    // ImGui needs the editor's own windows updated after each frame it ended,
    // also when that frame ended away from the main loop's.
    RenderPlatformWindows();

    // Only start a new frame if we haven't already
    if (!m_bFrameStarted)
    {
        m_bDrawDataReady = false;
        ImGui_ImplSDLGPU3_NewFrame();

        // Between frames: the effect browser's preview releases its texture
        // only before the renderer's frame starts.
        g_MuEffectBrowserUI.BeforeFrame();

        SelectWindowMode();
        // The SDL3 backend fills display size and mouse/keyboard from the SDL
        // events fed via ImGui_ImplSDL3_ProcessEvent, so it works the same
        // whether the editor is open or only the "Open Editor" button is shown
        // (issue #442) - no manual Win32 frame setup needed.
        ImGui_ImplSDL3_NewFrame();
        MuEditor::PlatformWindows::BeforeNewFrame();

        ImGui::NewFrame();
        m_bFrameStarted = true;

        // Once per ImGui frame: a frame that carries over keeps its key presses.
        ToggleFullscreenOnF11();
    }

    // Block the game's input under the editor: the Open Editor button while it
    // is closed, its windows while it is open
    if (!m_bEditorMode)
    {
        BlockGameInputOverOpenButton();
    }
    else
    {
        BlockGameInputOverEditor();
    }
}

void CMuEditorCore::ToggleFullscreenOnF11()
{
    // F11 in one of the editor's own windows; in the game window it stays the
    // game's (the game reads no keys pressed in the editor's windows). Not for
    // the borderless windows of popups and tooltips.
    if (!m_bEditorMode || !ImGui::IsKeyPressed(ImGuiKey_F11, false))
    {
        return;
    }
    SDL_Window* keyboardWindow = SDL_GetKeyboardFocus();
    if (IsEditorWindow(keyboardWindow) && (SDL_GetWindowFlags(keyboardWindow) & SDL_WINDOW_BORDERLESS) == 0)
    {
        MuEditor::PlatformWindows::ToggleFullscreen(keyboardWindow);
    }
}

void CMuEditorCore::BlockGameInputOverOpenButton()
{
    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureMouse = false;
    io.WantCaptureKeyboard = false;

    // Over the Open Editor button as it was last drawn (RenderToolbarOpen,
    // which also opens the editor on a click), with this frame's mouse. A
    // click the control socket injects is somewhere else than the real
    // pointer, which is what ImGui's mouse position follows.
    if (g_MuEditorUI.IsOnOpenButton(io.MousePos.x, io.MousePos.y) && !Core::Input::Synthetic::IsInjecting())
    {
        // Mouse is over button - block game input for this frame
        extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
        MouseLButtonDBClick = false;
    }
}

void CMuEditorCore::BlockGameInputOverEditor()
{
    // Block mouse input when hovering UI (from previous frame)
    if (m_bPreviousFrameHoveringUI)
    {
        extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
        extern bool MouseRButton, MouseRButtonPop, MouseRButtonPush;
        extern bool MouseMButton;

        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
        MouseLButtonDBClick = false;
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;
        MouseMButton = false;
    }

    // Process input blocking for editor UI (keyboard)
    g_MuInputBlockerCore.ProcessInputBlocking();

    // Capture the mouse for the Map Editor and stop painting clicks from
    // reaching the game (so the hero doesn't walk while painting). Runs here,
    // before the scene consumes input this frame.
    g_MapEditorUI.CaptureInputForPainting();

    // Note: Keyboard blocking is now handled in ScanAsyncKeyState() in WindowCommon.cpp
    // It prevents the game from scanning keyboard state when ImGui wants to capture it
}

void CMuEditorCore::LogVideoDriver() const
{
    const char* videoDriver = SDL_GetCurrentVideoDriver();
    const bool platformHasWindows = (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_PlatformHasViewports) != 0;
    g_MuEditorConsoleUI.LogEditor(std::string("Video driver: ") + (videoDriver != nullptr ? videoDriver : "none") +
                                  (platformHasWindows ? " (the editor's windows can leave the game window)"
                                                      : " (the editor stays inside the game window)"));
}

void CMuEditorCore::SelectWindowMode()
{
    // A full-screen game covers the editor's own windows and holds the cursor
    // in its window: the editor's windows then stay inside the game window,
    // as they do where the platform has no windows of their own (the Wayland
    // video driver; editor builds prefer X11 on Linux, see Winmain.cpp).
    ImGuiIO& io = ImGui::GetIO();
    const bool platformHasWindows = (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports) != 0;
    const bool gameFullscreen = (SDL_GetWindowFlags(m_pGameWindow) & SDL_WINDOW_FULLSCREEN) != 0;
    if (platformHasWindows && !gameFullscreen)
    {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
    else
    {
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    }
}

bool CMuEditorCore::IsEditorWindow(SDL_Window* window) const
{
    return m_bInitialized && window != nullptr && window != m_pGameWindow;
}

bool CMuEditorCore::IsKeyboardInEditorWindow() const
{
    return IsEditorWindow(SDL_GetKeyboardFocus());
}

bool CMuEditorCore::IsWithEditorWindow(int virtualKey) const
{
    const bool mouseButton = virtualKey == VK_LBUTTON || virtualKey == VK_RBUTTON || virtualKey == VK_MBUTTON;
    return mouseButton ? IsMouseInEditorWindow() : IsKeyboardInEditorWindow();
}

bool CMuEditorCore::FindMouseInEditorWindow() const
{
    if (!m_bInitialized)
    {
        return false;
    }
    // A held button keeps the window it was pressed in, whatever is under the
    // mouse now.
    SDL_Window* focus = SDL_GetMouseFocus();
    const bool mouseInEditorWindow = focus != nullptr && IsEditorWindow(focus);
    if (SDL_GetMouseState(nullptr, nullptr) != 0)
    {
        return mouseInEditorWindow;
    }
    // Over a window's frame and title bar SDL reports no window, and on macOS
    // the game window can have the mouse while an editor window floats over it.
    return mouseInEditorWindow || MuEditor::PlatformWindows::IsMouseOverOsWindow();
}

bool CMuEditorCore::ClaimsEvent(const SDL_Event& event)
{
    if (!m_bInitialized || ImGui::GetCurrentContext() == nullptr)
    {
        return false;
    }
    // What happens in the editor's own windows is the editor's, but for the
    // releases below. Over an editor window in the game window: the windows
    // that say so, and with the editor open also what ImGui captures (popups,
    // combos, the Dev Editor).
    const ImGuiIO& io = ImGui::GetIO();
    const bool ofEditorWindow = IsEditorWindow(SDL_GetWindowFromEvent(&event));
    const bool wantsMouse = ofEditorWindow || m_bHoveringUI || (m_bEditorMode && io.WantCaptureMouse);
    switch (event.type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (!wantsMouse)
        {
            return false;
        }
        m_claimedButtons |= SDL_BUTTON_MASK(event.button.button);
        return true;
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        const unsigned int button = SDL_BUTTON_MASK(event.button.button);
        const bool claimed = (m_claimedButtons & button) != 0;
        m_claimedButtons &= ~button;
        return claimed;
    }
    case SDL_EVENT_KEY_UP:
        return false;
    case SDL_EVENT_MOUSE_WHEEL:
        return wantsMouse;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING:
        return ofEditorWindow || (m_bEditorMode && io.WantCaptureKeyboard);
    default:
        return ofEditorWindow;
    }
}

void CMuEditorCore::Render()
{
    if (!m_bInitialized)
        return;

    // Only render if we have a frame started
    if (!m_bFrameStarted)
        return;

    // Reset hover state at start of frame: the mouse over one of the editor's
    // own windows is over the editor for every tool window below.
    m_bHoveringUI = IsMouseInEditorWindow();

    // The editor window when open, else the Open Editor button. Before the
    // tool windows: they dock into the editor window.
    g_MuEditorUI.Render(m_bEditorMode, m_bShowItemEditor, m_bShowSkillEditor, m_bShowDevEditor, m_bShowMapEditor,
                        m_bShowEffectBrowser, m_bShowConsole);

    if (m_bEditorMode)
    {
        // Render center pane (handles all editor windows and input blocking)
        g_MuEditorCenterPaneUI.Render(m_bShowItemEditor, m_bShowSkillEditor);

        // Render Dev Editor
        if (m_bShowDevEditor)
        {
            g_DevEditorUI.Render(&m_bShowDevEditor);
        }

        // Render Map Editor. Called every frame (not gated on the show flag) so
        // it can restore the game to normal mode the frame after it is closed;
        // it owns EditFlag while its window is open.
        g_MapEditorUI.Render(&m_bShowMapEditor);

        // After the Map Editor: its object browser starts a thumbnail only
        // while no capture is pending, and the effect preview leaves one
        // pending until the frame ends.
        if (m_bShowEffectBrowser)
        {
            g_MuEffectBrowserUI.Render(&m_bShowEffectBrowser);
        }

        // Render console (if enabled)
        if (m_bShowConsole)
        {
            g_MuEditorConsoleUI.Render(&m_bShowConsole);
        }
    }
    else
    {
        // Editor fully closed: make sure the Map Editor releases EditFlag so the
        // game doesn't stay stuck in an edit mode.
        g_MapEditorUI.Render(nullptr);
    }

    // After the game's move and draw, also while the editor is hidden: the
    // effect browser's world preview keeps what it created running.
    g_MuEffectBrowserUI.AfterRender(m_bShowEffectBrowser);

    // With the editor open, the mouse over any of its windows in the game
    // window, or holding one of its widgets, is over the editor.
    const bool overEditorWindow =
        ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    const bool draggingEditorWidget = ImGui::IsAnyItemActive() && ImGui::IsAnyMouseDown();
    if (m_bEditorMode && (overEditorWindow || draggingEditorWidget))
    {
        m_bHoveringUI = true;
    }

    // Store current hover state for next frame's input blocking
    m_bPreviousFrameHoveringUI = m_bHoveringUI;

    UpdateCursorVisibility();

    // The game draws this frame unless it skipped it (see RenderPlatformWindows).
    m_bGameFrameActive = mu::GetRenderer().IsFrameActive();
    m_skippedGameFrames = m_bGameFrameActive ? 0 : m_skippedGameFrames + 1;

    // Finalize draw data. The renderer draws the game window's part over the
    // frame, after RmlUi; the editor's own windows follow once that frame went
    // to the GPU (RenderPlatformWindows).
    ImGui::Render();
    m_bDrawDataReady = true;
    m_bPlatformWindowsPending = true;
    mu::QueueEditorRenderCommand();

    // Frame is complete, reset for next frame
    m_bFrameStarted = false;
}

void CMuEditorCore::UpdateCursorVisibility()
{
    // Control game cursor rendering via global flag
    // When hovering UI, hide game cursor; otherwise show it
    extern bool g_bRenderGameCursor;
    g_bRenderGameCursor = !m_bHoveringUI;

    // The OS cursor shows over the editor and hides over the game, which draws
    // its own. Windows maintains an internal display counter - cursor is
    // visible when counter >= 0 - and off Windows the game's ShowCursor keeps
    // the same counter over SDL's cursor. We need to loop to force the counter
    // to the correct state.
    static bool lastHoveringState = false;
    if (m_bHoveringUI != lastHoveringState)
    {
        if (m_bHoveringUI)
        {
            // Force cursor visible (counter >= CURSOR_VISIBLE_THRESHOLD)
            while (ShowCursor(TRUE) < CURSOR_VISIBLE_THRESHOLD);
        }
        else
        {
            // Force cursor hidden (counter < CURSOR_VISIBLE_THRESHOLD)
            while (ShowCursor(FALSE) >= CURSOR_VISIBLE_THRESHOLD);
        }
        lastHoveringState = m_bHoveringUI;
    }
    if (m_bHoveringUI)
    {
        MuEditor::Cursor::ShowImGuiShape();
    }
}

void CMuEditorCore::RenderPlatformWindows()
{
    if (!m_bPlatformWindowsPending)
    {
        return;
    }
    m_bPlatformWindowsPending = false;

    // ImGui wants the update after every frame it ended, also while the
    // editor's windows stay inside the game window (it does nothing then).
    ImGui::UpdatePlatformWindows();
    // A frame the game skipped while the GPU is behind (one at a time) keeps
    // the editor's windows' picture (their effect preview is drawn in the
    // game's frame). While the game window is minimized, hidden or covered (it
    // skips frame after frame) they are drawn, as they can stay on screen then
    // (macOS, some Linux window managers).
    const bool platformWindowsOn = (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
    const SDL_WindowFlags gameWindowFlags = SDL_GetWindowFlags(m_pGameWindow);
    const bool gameWindowHidden = (gameWindowFlags & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN)) != 0 ||
                                  m_skippedGameFrames > SkippedGameFramesWhileBehind;
    if (platformWindowsOn && (m_bGameFrameActive || gameWindowHidden))
    {
        ImGui::RenderPlatformWindowsDefault();
    }
}

void CMuEditorCore::PrepareDrawData(SDL_GPUCommandBuffer* commandBuffer)
{
    if (!m_bDrawDataReady)
    {
        return;
    }

    ImGui_ImplSDLGPU3_PrepareDrawData(ImGui::GetDrawData(), commandBuffer);
}

void CMuEditorCore::RenderDrawData(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass)
{
    if (!m_bDrawDataReady)
    {
        return;
    }

    ImGui_ImplSDLGPU3_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderPass);
    m_bDrawDataReady = false;
}

#endif // _EDITOR
