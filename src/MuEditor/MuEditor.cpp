#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditor.h"
#include "MuEditorUI.h"
#include "MuEditorConsole.h"
#include "MuEditor/MuItemEditor/MuItemEditor.h"
#include "MuInputBlocker.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"

// Windows cursor display counter thresholds
// The cursor is visible when the counter is >= CURSOR_VISIBLE_THRESHOLD
// The cursor is hidden when the counter is < CURSOR_VISIBLE_THRESHOLD
constexpr int CURSOR_VISIBLE_THRESHOLD = 0;

// "Open Editor" button layout constants
constexpr float EDITOR_BTN_X_OFFSET = 110.0f;
constexpr float EDITOR_BTN_Y = 8.0f;
constexpr float EDITOR_BTN_WIDTH = 100.0f;
constexpr float EDITOR_BTN_HEIGHT = 24.0f;

CMuEditor::CMuEditor()
    : m_bEditorMode(false)
    , m_bInitialized(false)
    , m_bFrameStarted(false)
    , m_bShowItemEditor(false)
    , m_bHoveringUI(false)
{
}

CMuEditor::~CMuEditor()
{
    Shutdown();
}

CMuEditor& CMuEditor::GetInstance()
{
    static CMuEditor instance;
    return instance;
}

void CMuEditor::Initialize(HWND hwnd, HDC hdc)
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
    g_MuEditorConsole.LogEditor("MU Editor initialized");

    fwprintf(stderr, L"[MuEditor] Initialize() completed\n");
    fflush(stderr);
}

void CMuEditor::Shutdown()
{
    if (!m_bInitialized)
        return;

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_bInitialized = false;
}

void CMuEditor::Update()
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

    // When editor is closed, check if mouse is over "Open Editor" button area and block input
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
            // Mouse is over button - block game input
            extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
            MouseLButton = false;
            MouseLButtonPop = false;
            MouseLButtonPush = false;
            MouseLButtonDBClick = false;
        }
    }
    // Only block game input when editor is fully open and ImGui is capturing mouse/keyboard
    else
    {
        g_MuInputBlocker.ProcessInputBlocking();
    }
}

void CMuEditor::Render()
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
        // Render center viewport
        g_MuEditorUI.RenderCenterViewport();

        // Render console
        g_MuEditorConsole.Render();

        // Render editor windows
        if (m_bShowItemEditor)
        {
            g_MuItemEditor.Render(m_bShowItemEditor);
        }
    }

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
