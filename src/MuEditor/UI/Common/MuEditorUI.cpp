#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorUI.h"

#include "imgui.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/Config/MuEditorConfig.h"
#include "Data/GameConfig/GameConfig.h"
#include "I18N/All.h"

#include <cstring>
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"

// UI Layout constants. The pixel sizes are multiplied by the editor UI scale so
// the toolbar grows with the -/+ buttons instead of clipping its own contents.
constexpr float TOOLBAR_HEIGHT = 40.0f;
constexpr float TOOLBAR_PADDING = 8.0f;
constexpr float TOOLBAR_INDENT = 10.0f;
constexpr float BUTTON_WIDTH_OFFSET = 110.0f;
constexpr int MOUSE_BUTTON_LEFT = 0;

// Step used by the toolbar's -/+ editor-UI-scale buttons.
constexpr float UI_SCALE_STEP = 0.1f;

CMuEditorUI& CMuEditorUI::GetInstance()
{
    static CMuEditorUI instance;
    return instance;
}

void CMuEditorUI::RenderToolbar(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor, bool& showMapEditor, bool& showConsole)
{
    if (editorEnabled)
    {
        RenderToolbarFull(editorEnabled, showItemEditor, showSkillEditor, showDevEditor, showMapEditor, showConsole);
    }
    else
    {
        RenderToolbarOpen(editorEnabled);
    }
}

void CMuEditorUI::RenderToolbarOpen(bool& editorEnabled)
{
    // When editor is disabled, show only "Open Editor" button
    // Use NoInputs flag to allow game mouse to pass through, but handle button clicks manually
    ImGuiIO& io = ImGui::GetIO();

    // Prevent ImGui from wanting mouse input when editor is closed
    io.WantCaptureMouse = false;
    io.WantCaptureKeyboard = false;

    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, TOOLBAR_HEIGHT * uiScale), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    // NoInputs prevents ImGui from capturing mouse/keyboard, allowing game cursor to work
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs;

    // Use same window background and border as Toolbar but fully transparent
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(TOOLBAR_PADDING, TOOLBAR_PADDING));

    if (ImGui::Begin("ToolbarClosed", nullptr, flags))
    {
        ImGui::Spacing();
        ImGui::Indent(TOOLBAR_INDENT);

        // Open button on the far right (same position as Close button)
        ImGui::SameLine(ImGui::GetWindowWidth() - BUTTON_WIDTH_OFFSET * uiScale);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));

        // Draw the button (won't be interactive due to NoInputs flag)
        ImGui::Button("Open Editor");

        // Get button rect for manual click detection
        ImVec2 buttonMin = ImGui::GetItemRectMin();
        ImVec2 buttonMax = ImGui::GetItemRectMax();

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

void CMuEditorUI::RenderToolbarFull(bool& editorEnabled, bool& showItemEditor, bool& showSkillEditor, bool& showDevEditor, bool& showMapEditor, bool& showConsole)
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, TOOLBAR_HEIGHT * uiScale), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.95f));

    if (ImGui::Begin("Toolbar", nullptr, flags))
    {
        // Check if hovering this window
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            g_MuEditorCore.SetHoveringUI(true);
        }

        ImGui::Spacing();
        ImGui::Indent(10.0f);

        ImGui::Text("MU Editor");
        ImGui::SameLine();

        if (ImGui::Button("Item Editor"))
        {
            showItemEditor = !showItemEditor;
        }

        ImGui::SameLine();
        if (ImGui::Button("Skill Editor"))
        {
            showSkillEditor = !showSkillEditor;
        }

        ImGui::SameLine();
        if (ImGui::Button("Dev Editor"))
        {
            showDevEditor = !showDevEditor;
        }

        ImGui::SameLine();
        if (ImGui::Button("Map Editor"))
        {
            showMapEditor = !showMapEditor;
        }

        // Global editor UI scale (affects every MuEditor window, not just this one).
        ImGui::SameLine();
        if (ImGui::Button("-##uiscale"))
            g_MuEditorCore.SetUIScale(g_MuEditorCore.GetUIScale() - UI_SCALE_STEP);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Smaller editor UI");
        ImGui::SameLine();
        ImGui::Text("%d%%", (int)(g_MuEditorCore.GetUIScale() * 100.0f + 0.5f));
        ImGui::SameLine();
        if (ImGui::Button("+##uiscale"))
            g_MuEditorCore.SetUIScale(g_MuEditorCore.GetUIScale() + UI_SCALE_STEP);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Bigger editor UI");

        // Console toggle
        ImGui::SameLine();
        if (ImGui::Checkbox("Console", &showConsole))
        {
            // Toggle is handled by reference
        }

        // Language selector
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);

        const char* currentLocale = I18N::GetCurrentLocale();
        std::span<const char* const> availableLocales = I18N::GetAvailableLocales();

        const char* currentLanguageName = I18N::GetLanguageDisplayName(currentLocale);

        if (ImGui::BeginCombo("##Language", currentLanguageName))
        {
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

        // Close button on the far right
        ImGui::SameLine(ImGui::GetWindowWidth() - BUTTON_WIDTH_OFFSET * uiScale);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Close Editor"))
        {
            editorEnabled = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::Unindent(10.0f);
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void CMuEditorUI::RenderCenterViewport()
{
    // The game renders automatically in the full window
    // The ImGui toolbar and console are overlays on top
    // No need to render anything here
}

#endif // _EDITOR
