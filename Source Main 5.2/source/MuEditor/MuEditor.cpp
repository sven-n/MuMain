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

CMuEditor::CMuEditor()
    : m_bEditorMode(false)
    , m_bInitialized(false)
    , m_bFrameStarted(false)
    , m_bShowItemEditor(false)
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

    std::fwprintf(stderr, L"[MuEditor] Initialize() called\n");
    std::fflush(stderr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Don't let ImGui change the mouse cursor - we'll use the game's custom cursor
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.MouseDrawCursor = false; // Don't let ImGui draw its own cursor

    std::fwprintf(stderr, L"[MuEditor] ImGui context created\n");
    std::fflush(stderr);

    // Dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL2_Init();

    std::fwprintf(stderr, L"[MuEditor] ImGui backends initialized\n");
    std::fflush(stderr);

    m_bInitialized = true;
    g_MuEditorConsole.LogEditor("MU Editor initialized");

    std::fwprintf(stderr, L"[MuEditor] Initialize() completed\n");
    std::fflush(stderr);
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
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        m_bFrameStarted = true;
    }

    // Block game input when ImGui is capturing mouse/keyboard
    g_MuInputBlocker.ProcessInputBlocking();
}

void CMuEditor::Render()
{
    if (!m_bInitialized)
        return;

    // Only render if we have a frame started
    if (!m_bFrameStarted)
        return;

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

    // Render ImGui and reset frame state
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Frame is complete, reset for next frame
    m_bFrameStarted = false;
}

#endif // _EDITOR
