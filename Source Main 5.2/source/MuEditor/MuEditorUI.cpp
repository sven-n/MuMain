#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorUI.h"
#include "imgui.h"

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
    // When editor is disabled, show only "Open Editor" button (fully transparent background)
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 40), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    // NoMouseInputs allows game cursor to be visible and clickable, but ImGui widgets inside still work
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs;

    // Use same window background and border as Toolbar but fully transparent
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

    if (ImGui::Begin("ToolbarClosed", nullptr, flags))
    {
        ImGui::Spacing();
        ImGui::Indent(10.0f);

        // Open button on the far right (same position as Close button)
        ImGui::SameLine(ImGui::GetWindowWidth() - 110);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("Open Editor"))
        {
            editorEnabled = true;
        }
        ImGui::PopStyleColor(2);

        ImGui::Unindent(10.0f);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void CMuEditorUI::RenderToolbarFull(bool& editorEnabled, bool& showItemEditor)
{
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.95f));

    if (ImGui::Begin("Toolbar", nullptr, flags))
    {
        ImGui::Spacing();
        ImGui::Indent(10.0f);

        ImGui::Text("MU Editor");
        ImGui::SameLine();

        if (ImGui::Button("Item Editor"))
        {
            showItemEditor = !showItemEditor;
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
