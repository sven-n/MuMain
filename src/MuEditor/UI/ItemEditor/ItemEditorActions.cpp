#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorActions.h"
#include "DataHandler/ItemData/ItemDataHandler.h"
#include "GameData/ItemData/ItemFieldMetadata.h"
#include "Translation/i18n.h"
#include "imgui.h"
#include <string>
#include <sstream>
#include "../Console/MuEditorConsoleUI.h"

extern std::wstring g_strSelectedML;

// ===== HELPER FUNCTIONS =====

void CItemEditorActions::ConvertItemName(char* outBuffer, size_t bufferSize, const wchar_t* name)
{
    WideCharToMultiByte(CP_UTF8, 0, name, -1, outBuffer, (int)bufferSize, NULL, NULL);
}

std::string CItemEditorActions::GetFieldValueAsString(const ITEM_ATTRIBUTE& item, const ItemFieldDescriptor& desc)
{
    std::stringstream ss;

    // Get pointer to the field using offset
    const BYTE* itemPtr = reinterpret_cast<const BYTE*>(&item);
    const void* fieldPtr = itemPtr + desc.offset;

    switch (desc.type)
    {
    case EItemFieldType::Bool:
        ss << (*reinterpret_cast<const bool*>(fieldPtr) ? 1 : 0);
        break;

    case EItemFieldType::Byte:
        ss << (int)*reinterpret_cast<const BYTE*>(fieldPtr);
        break;

    case EItemFieldType::Word:
        ss << *reinterpret_cast<const WORD*>(fieldPtr);
        break;

    case EItemFieldType::Int:
        ss << *reinterpret_cast<const int*>(fieldPtr);
        break;

    case EItemFieldType::WCharArray:
    {
        char buffer[256];
        ConvertItemName(buffer, sizeof(buffer), reinterpret_cast<const wchar_t*>(fieldPtr));
        ss << buffer;
        break;
    }
    }

    return ss.str();
}

// ===== EXPORT FUNCTIONS =====

std::string CItemEditorActions::GetCSVHeader()
{
    std::stringstream ss;
    const ItemFieldDescriptor* fields = GetFieldDescriptors(); const int fieldCount = GetFieldCount();

    ss << "Index";
    for (int i = 0; i < fieldCount; ++i)
    {
        ss << "," << GetFieldDisplayName(fields[i].name);
    }

    return ss.str();
}

std::string CItemEditorActions::ExportItemToReadable(int itemIndex, ITEM_ATTRIBUTE& item)
{
    std::stringstream ss;
    const ItemFieldDescriptor* fields = GetFieldDescriptors(); const int fieldCount = GetFieldCount();

    ss << "Row " << itemIndex << "\n";
    ss << "Index = " << itemIndex;

    for (int i = 0; i < fieldCount; ++i)
    {
        ss << ", " << GetFieldDisplayName(fields[i].name) << " = ";
        ss << GetFieldValueAsString(item, fields[i]);
    }

    return ss.str();
}

std::string CItemEditorActions::ExportItemToCSV(int itemIndex, ITEM_ATTRIBUTE& item)
{
    std::stringstream ss;
    const ItemFieldDescriptor* fields = GetFieldDescriptors(); const int fieldCount = GetFieldCount();

    ss << itemIndex;

    for (int i = 0; i < fieldCount; ++i)
    {
        ss << ",";

        // Quote strings for CSV
        if (fields[i].type == EItemFieldType::WCharArray)
        {
            ss << "\"" << GetFieldValueAsString(item, fields[i]) << "\"";
        }
        else
        {
            ss << GetFieldValueAsString(item, fields[i]);
        }
    }

    return ss.str();
}

std::string CItemEditorActions::ExportItemCombined(int itemIndex, ITEM_ATTRIBUTE& item)
{
    return ExportItemToReadable(itemIndex, item) + "\n" + ExportItemToCSV(itemIndex, item);
}

// ===== BUTTON RENDERING =====

void CItemEditorActions::RenderSaveButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.9f, 1.0f));

    if (ImGui::Button(EDITOR_TEXT("btn_save")))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Item_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        std::string changeLog;
        if (g_ItemDataHandler.Save(fileName, &changeLog))
        {
            // Log change details first, then save completion message
            g_MuEditorConsoleUI.LogEditor(changeLog);
            g_MuEditorConsoleUI.LogEditor("=== SAVE COMPLETED ===");
            ImGui::OpenPopup("Save Success");
        }
        else
        {
            // Check if it failed due to no changes
            if (!changeLog.empty() && changeLog.find("No changes") != std::string::npos)
            {
                g_MuEditorConsoleUI.LogEditor(changeLog);
            }
            else
            {
                g_MuEditorConsoleUI.LogEditor(EDITOR_TEXT("msg_save_failed"));
                ImGui::OpenPopup("Save Failed");
            }
        }
    }

    ImGui::PopStyleColor(2);
}

void CItemEditorActions::RenderExportS6E3Button()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.4f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));

    if (ImGui::Button(EDITOR_TEXT("btn_export_s6e3")))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Item_S6E3.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        if (g_ItemDataHandler.ExportAsS6E3(fileName))
        {
            std::string filename_str = "Item_" + std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_S6E3.bmd";
            g_MuEditorConsoleUI.LogEditor("Exported items as S6E3 legacy format: " + filename_str);
            ImGui::OpenPopup("Export S6E3 Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor(EDITOR_TEXT("msg_export_s6e3_failed"));
            ImGui::OpenPopup("Export S6E3 Failed");
        }
    }

    ImGui::PopStyleColor(2);
}

void CItemEditorActions::RenderExportCSVButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.7f, 1.0f));

    if (ImGui::Button(EDITOR_TEXT("btn_export_csv")))
    {
        wchar_t csvFileName[256];
        swprintf(csvFileName, L"Data\\Local\\%ls\\Item.csv", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        if (g_ItemDataHandler.ExportToCsv(csvFileName))
        {
            std::string filename_str = "Item_" + std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_export.csv";
            g_MuEditorConsoleUI.LogEditor("Exported items as CSV: " + filename_str);
            ImGui::OpenPopup("Export CSV Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor(EDITOR_TEXT("msg_export_csv_failed"));
            ImGui::OpenPopup("Export CSV Failed");
        }
    }

    ImGui::PopStyleColor(2);
}

void CItemEditorActions::RenderAllButtons()
{
    RenderSaveButton();
    ImGui::SameLine();
    RenderExportS6E3Button();
    ImGui::SameLine();
    RenderExportCSVButton();
}

#endif // _EDITOR
