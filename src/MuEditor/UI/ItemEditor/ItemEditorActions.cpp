#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorActions.h"
#include "DataHandler/ItemData/ItemDataHandler.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "imgui.h"
#include <string>

extern std::wstring g_strSelectedML;

void CItemEditorActions::RenderSaveButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.9f, 1.0f));

    if (ImGui::Button("Save Items"))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Item_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        std::string changeLog;
        if (g_ItemDataHandler.Save(fileName, &changeLog))
        {
            g_MuEditorConsoleUI.LogEditor("=== SAVE COMPLETED ===\n");
            g_MuEditorConsoleUI.LogEditor(changeLog);
            ImGui::OpenPopup("Save Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor("ERROR: Failed to save item attributes!\n");
            ImGui::OpenPopup("Save Failed");
        }
    }

    ImGui::PopStyleColor(2);
}

void CItemEditorActions::RenderExportS6E3Button()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.4f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));

    if (ImGui::Button("Export as S6E3"))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Item_%ls_S6E3.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        if (g_ItemDataHandler.SaveLegacy(fileName))
        {
            g_MuEditorConsoleUI.LogEditor("Exported items as S6E3 legacy format: Item_" +
                                         std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_S6E3.bmd\n");
            ImGui::OpenPopup("Export S6E3 Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor("ERROR: Failed to export item attributes as S6E3 format!\n");
            ImGui::OpenPopup("Export S6E3 Failed");
        }
    }

    ImGui::PopStyleColor(2);
}

void CItemEditorActions::RenderExportCSVButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.7f, 1.0f));

    if (ImGui::Button("Export as CSV"))
    {
        wchar_t csvFileName[256];
        swprintf(csvFileName, L"Data\\Local\\%ls\\Item_%ls_export.csv", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        if (g_ItemDataHandler.ExportToCsv(csvFileName))
        {
            g_MuEditorConsoleUI.LogEditor("Exported items as CSV: Item_" +
                                         std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_export.csv\n");
            ImGui::OpenPopup("Export CSV Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor("ERROR: Failed to export item attributes as CSV!\n");
            ImGui::OpenPopup("Export CSV Failed");
        }
    }

    ImGui::PopStyleColor(2);
}

void CItemEditorActions::RenderAllButtons()
{
    // Position buttons on the right side
    ImGui::SameLine(ImGui::GetWindowWidth() - 390);

    RenderSaveButton();
    ImGui::SameLine();
    RenderExportS6E3Button();
    ImGui::SameLine();
    RenderExportCSVButton();
}

#endif // _EDITOR
