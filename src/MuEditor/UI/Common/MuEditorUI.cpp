#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorUI.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "MuEditorLayout.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/Core/MuEditorPlatformWindows.h"
#include "../MuEditor/Config/MuEditorConfig.h"
#include "Data/GameConfig/GameConfig.h"
#include "I18N/All.h"

#include <algorithm>
#include <cstring>
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "MuEditorToolWindows.h"
#include "ToolGroups.h"
#include "UndoTarget.h"

// Layout of the Open Editor strip over the game while the editor is closed. The
// pixel sizes are multiplied by the editor UI scale so the strip grows with it
// instead of clipping its own contents.
constexpr float TOOLBAR_HEIGHT = 40.0f;
constexpr float TOOLBAR_PADDING = 8.0f;
constexpr float TOOLBAR_INDENT = 10.0f;
constexpr float BUTTON_WIDTH_OFFSET = 110.0f;
constexpr int MOUSE_BUTTON_LEFT = 0;

// Step used by the menu bar's -/+ editor-UI-scale buttons.
constexpr float UI_SCALE_STEP = 0.1f;
constexpr float LANGUAGE_COMBO_WIDTH = 100.0f;

// The editor window; its id stays when its title changes.
constexpr char EDITOR_WINDOW_NAME[] = "MU Editor###MuEditor";

CMuEditorUI& CMuEditorUI::GetInstance()
{
    static CMuEditorUI instance;
    return instance;
}

void CMuEditorUI::Render(bool& editorEnabled, MuEditor::ToolWindows& windows)
{
    if (editorEnabled)
    {
        RenderEditorWindow(editorEnabled, windows);
    }
    else
    {
        RenderToolbarOpen(editorEnabled);
    }
}

bool CMuEditorUI::IsOnOpenButton(float x, float y) const
{
    return x >= m_openButtonMinX && x <= m_openButtonMaxX && y >= m_openButtonMinY && y <= m_openButtonMaxY &&
           m_openButtonMaxX > m_openButtonMinX;
}

void CMuEditorUI::RenderToolbarOpen(bool& editorEnabled)
{
    // When editor is disabled, show only "Open Editor" button
    // Use NoInputs flag to allow game mouse to pass through, but handle button clicks manually
    ImGuiIO& io = ImGui::GetIO();

    // Prevent ImGui from wanting mouse input when editor is closed
    io.WantCaptureMouse = false;
    io.WantCaptureKeyboard = false;

    // Along the top of the game window, also when the editor's windows can
    // leave it.
    const float uiScale = g_MuEditorCore.GetUIScale();
    const ImGuiViewport* game = ImGui::GetMainViewport();
    ImGui::SetNextWindowViewport(game->ID);
    ImGui::SetNextWindowSize(ImVec2(game->Size.x, TOOLBAR_HEIGHT * uiScale), ImGuiCond_Always);
    ImGui::SetNextWindowPos(game->Pos, ImGuiCond_Always);

    // NoInputs prevents ImGui from capturing mouse/keyboard, allowing game cursor to work
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs;

    // Fully transparent background and border: only the button shows
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(TOOLBAR_PADDING, TOOLBAR_PADDING));

    if (ImGui::Begin("ToolbarClosed", nullptr, flags))
    {
        ImGui::Spacing();
        ImGui::Indent(TOOLBAR_INDENT);

        // Open button on the far right
        ImGui::SameLine(ImGui::GetWindowWidth() - BUTTON_WIDTH_OFFSET * uiScale);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));

        // Draw the button (won't be interactive due to NoInputs flag)
        ImGui::Button("Open Editor");

        // Get button rect for manual click detection
        ImVec2 buttonMin = ImGui::GetItemRectMin();
        ImVec2 buttonMax = ImGui::GetItemRectMax();
        m_openButtonMinX = buttonMin.x;
        m_openButtonMinY = buttonMin.y;
        m_openButtonMaxX = buttonMax.x;
        m_openButtonMaxY = buttonMax.y;

        ImGui::PopStyleColor(2);

        // Check if mouse is hovering over the button specifically
        bool isHoveringButton = (io.MousePos.x >= buttonMin.x && io.MousePos.x <= buttonMax.x &&
                                 io.MousePos.y >= buttonMin.y && io.MousePos.y <= buttonMax.y);

        if (isHoveringButton)
        {
            g_MuEditorCore.SetHoveringUI(true);

            if (ImGui::IsMouseClicked(MOUSE_BUTTON_LEFT))
            {
                editorEnabled = true;

                // Clear game mouse button states to prevent click from going through to game
                extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
                MouseLButton = false;
                MouseLButtonPop = false;
                MouseLButtonPush = false;
                MouseLButtonDBClick = false;
            }
        }
        else
        {
            g_MuEditorCore.SetHoveringUI(false);
        }

        ImGui::Unindent(10.0f);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void CMuEditorUI::RenderEditorWindow(bool& editorEnabled, MuEditor::ToolWindows& windows)
{
    // Only for its first appearance: no window and no imgui.ini entry yet.
    if (ImGui::FindWindowByName(EDITOR_WINDOW_NAME) == nullptr)
    {
        const MuEditor::Layout::WindowRect first = MuEditor::Layout::FirstEditorWindowRect(g_MuEditorCore.GetUIScale());
        ImGui::SetNextWindowPos(first.position, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(first.size, ImGuiCond_FirstUseEver);
    }
    // An OS window of its own also where it overlaps the game window, so it
    // never merges into the game's frame. It appears without taking the
    // keyboard from the game.
    ImGuiWindowClass ownWindow;
    ownWindow.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge | ImGuiViewportFlags_NoFocusOnAppearing;
    ImGui::SetNextWindowClass(&ownWindow);

    // In an OS window of its own the OS title bar and frame replace ImGui's
    // title bar, moving and sizing.
    const bool ownOsWindow = (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
    const ImGuiWindowFlags osFrame =
        ownOsWindow ? ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                    : ImGuiWindowFlags_None;
    bool open = true;
    ImGui::Begin(EDITOR_WINDOW_NAME, &open,
                 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | osFrame);
    // Its menu bar, dock space and docked windows count as the editor, also
    // while one of their widgets is held.
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_DockHierarchy |
                               ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
        g_MuEditorCore.SetHoveringUI(true);
    }
    if (ImGui::BeginMenuBar())
    {
        RenderBar(windows);
        RenderCloseButton(editorEnabled);
        ImGui::EndMenuBar();
    }
    // Also while the window is hidden: that keeps the tool windows docked.
    MuEditor::Layout::SubmitDockSpace(m_bResetLayout);
    m_bResetLayout = false;
    ImGui::End();

    if (!open)
    {
        editorEnabled = false;
    }
}

void CMuEditorUI::RenderToolWindowBar()
{
    if (ImGui::BeginMenuBar())
    {
        RenderBar(g_MuEditorCore.GetToolWindows());
        ImGui::EndMenuBar();
    }
}

void CMuEditorUI::RenderBar(MuEditor::ToolWindows& windows)
{
    RenderWindowsMenu(windows, MuEditor::PlatformWindows::WindowOf(ImGui::GetWindowViewport()));
    MuEditor::UndoTargets::RenderBarButtons(MuEditor::Items::ToolWindow::GetUndoTarget());
    RenderUIScale();
    RenderLanguageSelector();
}

void CMuEditorUI::RenderWindowsMenu(MuEditor::ToolWindows& windows, SDL_Window* osWindow)
{
    if (!ImGui::BeginMenu("Windows"))
    {
        return;
    }
    // The item tools, which share one selected item, and the monster tools.
    RenderToolGroupMenu(MuEditor::GetItemTools(), windows);
    RenderToolGroupMenu(MuEditor::GetMonsterTools(), windows);
    ImGui::MenuItem(I18N::Editor::ServerList, nullptr, &windows.serverList);
    ImGui::MenuItem("Skill Editor", nullptr, &windows.skillEditor);
    ImGui::MenuItem("Dev Editor", nullptr, &windows.devEditor);
    ImGui::MenuItem("Map Editor", nullptr, &windows.mapEditor);
    ImGui::MenuItem("Effect Browser", nullptr, &windows.effectBrowser);
    ImGui::MenuItem("Console", nullptr, &windows.console);
    ImGui::Separator();
    if (ImGui::MenuItem("Reset Layout"))
    {
        m_bResetLayout = true;
    }
    // The window the menu is in full screen (F11 in any of the editor's own
    // windows); not while it is inside the game window.
    if (ImGui::MenuItem("Fullscreen", "F11", MuEditor::PlatformWindows::IsFullscreen(osWindow), osWindow != nullptr))
    {
        MuEditor::PlatformWindows::ToggleFullscreen(osWindow);
    }
    ImGui::EndMenu();
}

void CMuEditorUI::RenderToolGroupMenu(const MuEditor::ToolGroup& group, MuEditor::ToolWindows& windows)
{
    if (!ImGui::BeginMenu(*group.menuTitle))
    {
        return;
    }
    ImGui::MenuItem(*group.browser.label, nullptr, &(windows.*(group.browser.open)));
    for (const MuEditor::ToolWindowEntry& tool : group.tools)
    {
        ImGui::MenuItem(*tool.label, nullptr, &(windows.*(tool.open)));
    }
    ImGui::EndMenu();
}

void CMuEditorUI::RenderUIScale()
{
    // Global editor UI scale (affects every MuEditor window, not just this one).
    if (ImGui::Button("-##uiscale"))
        g_MuEditorCore.SetUIScale(g_MuEditorCore.GetUIScale() - UI_SCALE_STEP);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Smaller editor UI");
    ImGui::Text("%d%%", (int)(g_MuEditorCore.GetUIScale() * 100.0f + 0.5f));
    if (ImGui::Button("+##uiscale"))
        g_MuEditorCore.SetUIScale(g_MuEditorCore.GetUIScale() + UI_SCALE_STEP);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Bigger editor UI");
}

void CMuEditorUI::RenderLanguageSelector()
{
    ImGui::SetNextItemWidth(LANGUAGE_COMBO_WIDTH * g_MuEditorCore.GetUIScale());

    const char* currentLocale = I18N::GetCurrentLocale();
    std::span<const char* const> availableLocales = I18N::GetAvailableLocales();

    const char* currentLanguageName = I18N::GetLanguageDisplayName(currentLocale);

    if (!ImGui::BeginCombo("##Language", currentLanguageName))
    {
        return;
    }
    for (size_t i = 0; i < availableLocales.size(); i++)
    {
        const char* locale = availableLocales[i];
        const bool isSelected = (std::strcmp(locale, currentLocale) == 0);
        const char* displayName = I18N::GetLanguageDisplayName(locale);

        if (ImGui::Selectable(displayName, isSelected))
        {
            I18N::SetLocale(locale);
            // Persist to GameConfig.UILocale so the game options
            // window and editor stay in sync across restarts.
            std::wstring wide(locale, locale + std::strlen(locale));
            GameConfig::GetInstance().SetUILocale(wide);
            GameConfig::GetInstance().Save();
            g_MuEditorConsoleUI.LogEditor(std::string("Language switched to: ") + displayName);
        }

        if (isSelected)
        {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();
}

void CMuEditorUI::RenderCloseButton(bool& editorEnabled)
{
    // On the far right of the menu bar, but after the other items in a narrow
    // window
    const float rightAligned = ImGui::GetWindowWidth() - BUTTON_WIDTH_OFFSET * g_MuEditorCore.GetUIScale();
    ImGui::SameLine(std::max(rightAligned, ImGui::GetCursorPosX()));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button("Close Editor"))
    {
        editorEnabled = false;
    }
    ImGui::PopStyleColor(2);
}

#endif // _EDITOR
