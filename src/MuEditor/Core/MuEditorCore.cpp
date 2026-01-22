#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorUI.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"
#include "MuInputBlockerCore.h"
#include "../MuEditor/UI/Common/MuEditorCenterPaneUI.h"
#include "../MuEditor/UI/ItemEditor/MuItemEditorUI.h"
#include "Translation/i18n.h"

// Windows cursor display counter thresholds
// The cursor is visible when the counter is >= CURSOR_VISIBLE_THRESHOLD
// The cursor is hidden when the counter is < CURSOR_VISIBLE_THRESHOLD
constexpr int CURSOR_VISIBLE_THRESHOLD = 0;

// "Open Editor" button layout constants
constexpr float EDITOR_BTN_X_OFFSET = 110.0f;
constexpr float EDITOR_BTN_Y = 8.0f;
constexpr float EDITOR_BTN_WIDTH = 100.0f;
constexpr float EDITOR_BTN_HEIGHT = 24.0f;

CMuEditorCore::CMuEditorCore()
    : m_bEditorMode(false)
    , m_bInitialized(false)
    , m_bFrameStarted(false)
    , m_bShowItemEditor(false)
    , m_bHoveringUI(false)
    , m_bPreviousFrameHoveringUI(false)
{
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

void CMuEditorCore::Initialize(HWND hwnd, HDC hdc)
{
    if (m_bInitialized)
        return;

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

    // Dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL2_Init();

    fwprintf(stderr, L"[MuEditor] ImGui backends initialized\n");
    fflush(stderr);

    m_bInitialized = true;
    g_MuEditorConsoleUI.LogEditor("MU Editor initialized");

    // Load translation files (editor only - game translations loaded by main game code)
    i18n::Translator& translator = i18n::Translator::GetInstance();

    // Try multiple possible paths since working directory varies
    bool editorLoaded = translator.LoadTranslations(i18n::Domain::Editor,
        L"Translations\\en\\editor.json");
    if (!editorLoaded) editorLoaded = translator.LoadTranslations(i18n::Domain::Editor,
        L"bin\\Translations\\en\\editor.json");

    bool metadataLoaded = translator.LoadTranslations(i18n::Domain::Metadata,
        L"Translations\\en\\metadata.json");
    if (!metadataLoaded) metadataLoaded = translator.LoadTranslations(i18n::Domain::Metadata,
        L"bin\\Translations\\en\\metadata.json");

    translator.SetLocale("en");

    if (editorLoaded && metadataLoaded)
    {
        g_MuEditorConsoleUI.LogEditor("Editor translations loaded successfully");
    }
    else
    {
        g_MuEditorConsoleUI.LogEditor("WARNING: Some editor translation files not loaded");
        if (!editorLoaded) g_MuEditorConsoleUI.LogEditor("  - editor.json missing");
        if (!metadataLoaded) g_MuEditorConsoleUI.LogEditor("  - metadata.json missing");

        // Debug: Log current working directory
        wchar_t cwd[MAX_PATH];
        if (GetCurrentDirectoryW(MAX_PATH, cwd))
        {
            char cwdUtf8[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, cwd, -1, cwdUtf8, MAX_PATH, NULL, NULL);
            g_MuEditorConsoleUI.LogEditor(std::string("  Working directory: ") + cwdUtf8);
        }
    }

    fwprintf(stderr, L"[MuEditor] Initialize() completed\n");
    fflush(stderr);
}

void CMuEditorCore::Shutdown()
{
    if (!m_bInitialized)
        return;

    // Save item editor preferences before shutting down
    g_MuItemEditorUI.SaveColumnPreferences();

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_bInitialized = false;
}

void CMuEditorCore::Update()
{
    if (!m_bInitialized)
        return;

    // Only start a new frame if we haven't already
    if (!m_bFrameStarted)
    {
        ImGui_ImplOpenGL2_NewFrame();

        // Only let Win32 backend update mouse when editor is open
        if (m_bEditorMode)
        {
            ImGui_ImplWin32_NewFrame();
        }
        else
        {
            // When closed, manually create a minimal frame and update mouse position
            ImGuiIO& io = ImGui::GetIO();

            // Get window size
            extern HWND g_hWnd;
            RECT rect;
            GetClientRect(g_hWnd, &rect);
            io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
            io.DeltaTime = 1.0f / 60.0f;

            // Manually update mouse position for button detection
            POINT mousePos;
            if (GetCursorPos(&mousePos))
            {
                ScreenToClient(g_hWnd, &mousePos);
                io.MousePos = ImVec2((float)mousePos.x, (float)mousePos.y);
            }

            // Update mouse button states
            extern bool MouseLButton;
            io.MouseDown[0] = MouseLButton;
        }

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
            float buttonX = screenWidth - EDITOR_BTN_X_OFFSET;
            float buttonY = EDITOR_BTN_Y;
            float buttonWidth = EDITOR_BTN_WIDTH;
            float buttonHeight = EDITOR_BTN_HEIGHT;

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
        float buttonX = io.DisplaySize.x - EDITOR_BTN_X_OFFSET;
        float buttonY = EDITOR_BTN_Y;
        float buttonWidth = EDITOR_BTN_WIDTH;
        float buttonHeight = EDITOR_BTN_HEIGHT;

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
    g_MuEditorUI.RenderToolbar(m_bEditorMode, m_bShowItemEditor);

    if (m_bEditorMode)
    {
        // Render center pane (handles all editor windows and input blocking)
        g_MuEditorCenterPaneUI.Render(m_bShowItemEditor);

        // Render console
        g_MuEditorConsoleUI.Render();
    }

    // Store current hover state for next frame's input blocking
    m_bPreviousFrameHoveringUI = m_bHoveringUI;

    // Control game cursor rendering via global flag
    // When hovering UI, hide game cursor; otherwise show it
    extern bool g_bRenderGameCursor;
    g_bRenderGameCursor = !m_bHoveringUI;

    // Manage Windows cursor visibility
    // Windows maintains an internal display counter - cursor is visible when counter >= 0
    // We need to loop to force the counter to the correct state
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

    // Render ImGui and reset frame state
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Frame is complete, reset for next frame
    m_bFrameStarted = false;
}

#endif // _EDITOR
