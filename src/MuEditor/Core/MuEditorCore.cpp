#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorCore.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl2.h"
#include "MuInputBlockerCore.h"
#include "../Config/MuEditorConfig.h"
#include "../MuEditor/UI/Common/MuEditorCenterPaneUI.h"
#include "../MuEditor/UI/ItemEditor/MuItemEditorUI.h"
#include "../MuEditor/UI/SkillEditor/MuSkillEditorUI.h"
#include "../MuEditor/UI/DevEditor/DevEditorUI.h"
#include "../MuEditor/UI/MapEditor/MapEditorUI.h"
#include "../UI/Common/MuEditorUI.h"
#include "../UI/Console/MuEditorConsoleUI.h"
#include "I18N/All.h"
#include "Core/Utilities/StringUtils.h"

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

// Windows cursor display counter thresholds
// The cursor is visible when the counter is >= CURSOR_VISIBLE_THRESHOLD
// The cursor is hidden when the counter is < CURSOR_VISIBLE_THRESHOLD
constexpr int CURSOR_VISIBLE_THRESHOLD = 0;

// "Open Editor" button layout constants (scaled by the editor UI scale)
constexpr float EDITOR_BTN_X_OFFSET = 110.0f;
constexpr float EDITOR_BTN_Y = 8.0f;
constexpr float EDITOR_BTN_WIDTH = 100.0f;
constexpr float EDITOR_BTN_HEIGHT = 24.0f;

// Editor UI scale limits (the toolbar's -/+ buttons step by 0.1).
constexpr float UI_SCALE_MIN = 0.7f;
constexpr float UI_SCALE_MAX = 2.5f;

CMuEditorCore::CMuEditorCore()
    : m_bEditorMode(false)
    , m_bInitialized(false)
    , m_bFrameStarted(false)
    , m_bShowItemEditor(false)
    , m_bShowSkillEditor(false)
    , m_bShowDevEditor(false)
    , m_bShowMapEditor(false)
    , m_bShowConsole(true)
    , m_bHoveringUI(false)
    , m_bPreviousFrameHoveringUI(false)
    , m_UIScale(1.0f)
    , m_bScaleDirty(false)
{
}

void CMuEditorCore::SetUIScale(float scale)
{
    // Clamp to something sane; the toolbar steps this in 0.1 increments.
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

void CMuEditorCore::Initialize(SDL_Window* window, void* glContext)
{
    if (m_bInitialized)
        return;

    if (window == nullptr || glContext == nullptr)
    {
        fwprintf(stderr, L"[MuEditor] Initialize failed: window or glContext is null\n");
        fflush(stderr);
        return;
    }

    fwprintf(stderr, L"[MuEditor] Initialize() called\n");
    fflush(stderr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Let ImGui manage the mouse cursor (Windows cursor) when editor is open
    // Game cursor is disabled when editor is open (see ZzzScene.cpp)
    io.MouseDrawCursor = false; // Don't let ImGui draw its own cursor, use Windows cursor

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

    if (!ImGui_ImplSDL3_InitForOpenGL(window, glContext))
    {
        fwprintf(stderr, L"[MuEditor] ImGui_ImplSDL3_InitForOpenGL failed\n");
        fflush(stderr);
        ImGui::DestroyContext();
        return;
    }
    if (!ImGui_ImplOpenGL2_Init())
    {
        fwprintf(stderr, L"[MuEditor] ImGui_ImplOpenGL2_Init failed\n");
        fflush(stderr);
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return;
    }

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

void CMuEditorCore::Shutdown()
{
    if (!m_bInitialized)
        return;

    // Save item editor preferences before shutting down
    g_MuItemEditorUI.SaveColumnPreferences();

    // Save skill editor preferences before shutting down
    g_MuSkillEditorUI.SaveColumnPreferences();

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    m_bInitialized = false;
}

void CMuEditorCore::Update()
{
    if (!m_bInitialized)
        return;

    // Apply a pending UI scale change between frames (never mid-frame).
    if (m_bScaleDirty)
    {
        ApplyUIScale();
        m_bScaleDirty = false;
    }

    // Only start a new frame if we haven't already
    if (!m_bFrameStarted)
    {
        ImGui_ImplOpenGL2_NewFrame();

        // The SDL3 backend fills display size and mouse/keyboard from the SDL
        // events fed via ImGui_ImplSDL3_ProcessEvent, so it works the same
        // whether the editor is open or only the "Open Editor" button is shown
        // (issue #442) - no manual Win32 frame setup needed.
        ImGui_ImplSDL3_NewFrame();

        ImGui::NewFrame();
        m_bFrameStarted = true;
    }

    // When editor is closed, check for "Open Editor" button click (after ImGui frame started)
    if (!m_bEditorMode)
    {
        ImGuiIO& io = ImGui::GetIO();

        // Get mouse position
        POINT mousePos;
        if (GetCursorPos(&mousePos))
        {
            extern HWND g_hWnd;
            ScreenToClient(g_hWnd, &mousePos);

            // Check if mouse is over button area (top-right corner)
            RECT rect;
            GetClientRect(g_hWnd, &rect);
            float screenWidth = (float)(rect.right - rect.left);
            float buttonX = screenWidth - EDITOR_BTN_X_OFFSET * m_UIScale;
            float buttonY = EDITOR_BTN_Y * m_UIScale;
            float buttonWidth = EDITOR_BTN_WIDTH * m_UIScale;
            float buttonHeight = EDITOR_BTN_HEIGHT * m_UIScale;

            if (mousePos.x >= buttonX && mousePos.x <= (buttonX + buttonWidth) &&
                mousePos.y >= buttonY && mousePos.y <= (buttonY + buttonHeight))
            {
                // Check for click
                extern bool MouseLButtonPush;
                if (MouseLButtonPush)
                {
                    m_bEditorMode = true;

                    // Clear the click so it doesn't go to game
                    extern bool MouseLButton, MouseLButtonPop, MouseLButtonDBClick;
                    MouseLButton = false;
                    MouseLButtonPop = false;
                    MouseLButtonPush = false;
                    MouseLButtonDBClick = false;
                }
            }
        }
    }

    // When editor is closed, handle "Open Editor" button hover to block game input
    if (!m_bEditorMode)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.WantCaptureMouse = false;
        io.WantCaptureKeyboard = false;

        // Check if mouse is over button area (top-right corner)
        float buttonX = io.DisplaySize.x - EDITOR_BTN_X_OFFSET * m_UIScale;
        float buttonY = EDITOR_BTN_Y * m_UIScale;
        float buttonWidth = EDITOR_BTN_WIDTH * m_UIScale;
        float buttonHeight = EDITOR_BTN_HEIGHT * m_UIScale;

        if (io.MousePos.x >= buttonX && io.MousePos.x <= (buttonX + buttonWidth) &&
            io.MousePos.y >= buttonY && io.MousePos.y <= (buttonY + buttonHeight))
        {
            // Mouse is over button - block game input for this frame
            extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
            MouseLButton = false;
            MouseLButtonPop = false;
            MouseLButtonPush = false;
            MouseLButtonDBClick = false;
        }
    }
    // When editor is open, block input based on UI state
    else if (m_bEditorMode)
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

        // Note: Keyboard blocking is now handled in ScanAsyncKeyState() in NewUICommon.cpp
        // It prevents the game from scanning keyboard state when ImGui wants to capture it
    }
}

void CMuEditorCore::Render()
{
    if (!m_bInitialized)
        return;

    // Only render if we have a frame started
    if (!m_bFrameStarted)
        return;

    // Reset hover state at start of frame
    m_bHoveringUI = false;

    // Render toolbar (handles both open and closed states)
    g_MuEditorUI.RenderToolbar(m_bEditorMode, m_bShowItemEditor, m_bShowSkillEditor, m_bShowDevEditor, m_bShowMapEditor, m_bShowConsole);

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

        // Render console (if enabled)
        if (m_bShowConsole)
        {
            g_MuEditorConsoleUI.Render();
        }
    }
    else
    {
        // Editor fully closed: make sure the Map Editor releases EditFlag so the
        // game doesn't stay stuck in an edit mode.
        g_MapEditorUI.Render(nullptr);
    }

    // Store current hover state for next frame's input blocking
    m_bPreviousFrameHoveringUI = m_bHoveringUI;

    // Control game cursor rendering via global flag
    // When hovering UI, hide game cursor; otherwise show it
    extern bool g_bRenderGameCursor;
    g_bRenderGameCursor = !m_bHoveringUI;

#ifdef _WIN32
    // Manage Windows cursor visibility
    // Windows maintains an internal display counter - cursor is visible when counter >= 0
    // We need to loop to force the counter to the correct state.
    // Off Windows the SDL/ImGui backend drives the OS cursor itself, and the
    // ShowCursor stub returns a constant so these loops would never terminate.
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
#endif

    // Render ImGui and reset frame state
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Frame is complete, reset for next frame
    m_bFrameStarted = false;
}

#endif // _EDITOR
