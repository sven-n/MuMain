#include "stdafx.h"

#ifdef _EDITOR

#include "../MuEditor\UI\Common\MuEditorUI.h"
#include "imgui.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/Config/MuEditorConfig.h"
#include "Translation/i18n.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"

// UI Layout constants
constexpr float TOOLBAR_HEIGHT = 40.0f;
constexpr float TOOLBAR_PADDING = 8.0f;
constexpr float TOOLBAR_INDENT = 10.0f;
constexpr float BUTTON_WIDTH_OFFSET = 110.0f;
constexpr int MOUSE_BUTTON_LEFT = 0;

CMuEditorUI& CMuEditorUI::GetInstance()
{
    static CMuEditorUI instance;
    return instance;
}

void CMuEditorUI::RenderToolbar(bool& editorEnabled, bool& showItemEditor)
{
    if (editorEnabled)
    {
        RenderToolbarFull(editorEnabled, showItemEditor);
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

    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, TOOLBAR_HEIGHT), ImGuiCond_Always);
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
        ImGui::SameLine(ImGui::GetWindowWidth() - BUTTON_WIDTH_OFFSET);
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

        ImGui::Unindent(10.0f);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void CMuEditorUI::RenderToolbarFull(bool& editorEnabled, bool& showItemEditor)
{
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, TOOLBAR_HEIGHT), ImGuiCond_Always);
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

        // Language selector
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        
        i18n::Translator& translator = i18n::Translator::GetInstance();
        const std::string& currentLocale = translator.GetLocale();
        
        // Language options
        const char* languages[] = { "English", "Español", "Português" };
        const char* locales[] = { "en", "es", "pt" };
        int currentIndex = 0;
        
        // Find current language index
        for (int i = 0; i < 3; i++)
        {
            if (currentLocale == locales[i])
            {
                currentIndex = i;
                break;
            }
        }
        
        if (ImGui::Combo("##Language", &currentIndex, languages, 3))
        {
            // Language changed
            if (translator.SwitchLanguage(locales[currentIndex]))
            {
                // Save language preference to config
                g_MuEditorConfig.SetLanguage(locales[currentIndex]);
                g_MuEditorConfig.Save();

                g_MuEditorConsoleUI.LogEditor(std::string("Language switched to: ") + languages[currentIndex]);
            }
            else
            {
                g_MuEditorConsoleUI.LogEditor(std::string("Failed to load translations for: ") + languages[currentIndex]);
            }
        }

        // Close button on the far right
        ImGui::SameLine(ImGui::GetWindowWidth() - 110);
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
