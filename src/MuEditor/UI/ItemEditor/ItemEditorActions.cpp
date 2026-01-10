#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorActions.h"
#include "DataHandler/ItemData/ItemDataHandler.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "imgui.h"
#include <string>
#include <sstream>

extern std::wstring g_strSelectedML;

void CItemEditorActions::ConvertItemName(char* outBuffer, size_t bufferSize, const wchar_t* name)
{
    WideCharToMultiByte(CP_UTF8, 0, name, -1, outBuffer, (int)bufferSize, NULL, NULL);
}

std::string CItemEditorActions::ExportItemToReadable(int itemIndex, ITEM_ATTRIBUTE& item)
{
    std::stringstream ss;
    char nameBuffer[256];
    ConvertItemName(nameBuffer, sizeof(nameBuffer), item.Name);

    ss << "Row " << itemIndex << "\n"
       << "Index = " << itemIndex << ", "
       << "Name = " << nameBuffer << ", "
       << "TwoHand = " << (item.TwoHand ? 1 : 0) << ", "
       << "Level = " << item.Level << ", "
       << "Slot = " << (int)item.m_byItemSlot << ", "
       << "Skill = " << item.m_wSkillIndex << ", "
       << "Width = " << (int)item.Width << ", "
       << "Height = " << (int)item.Height << ", "
       << "DamageMin = " << (int)item.DamageMin << ", "
       << "DamageMax = " << (int)item.DamageMax << ", "
       << "Block = " << (int)item.SuccessfulBlocking << ", "
       << "Defense = " << (int)item.Defense << ", "
       << "MagicDefense = " << (int)item.MagicDefense << ", "
       << "WeaponSpeed = " << (int)item.WeaponSpeed << ", "
       << "WalkSpeed = " << (int)item.WalkSpeed << ", "
       << "Durability = " << (int)item.Durability << ", "
       << "MagicDur = " << (int)item.MagicDur << ", "
       << "MagicPower = " << (int)item.MagicPower << ", "
       << "ReqStr = " << item.RequireStrength << ", "
       << "ReqDex = " << item.RequireDexterity << ", "
       << "ReqEne = " << item.RequireEnergy << ", "
       << "ReqVit = " << item.RequireVitality << ", "
       << "ReqCha = " << item.RequireCharisma << ", "
       << "ReqLevel = " << item.RequireLevel << ", "
       << "Value = " << (int)item.Value << ", "
       << "Zen = " << item.iZen << ", "
       << "AttType = " << (int)item.AttType << ", "
       << "DW/SM = " << (int)item.RequireClass[0] << ", "
       << "DK/BK = " << (int)item.RequireClass[1] << ", "
       << "ELF/ME = " << (int)item.RequireClass[2] << ", "
       << "MG = " << (int)item.RequireClass[3] << ", "
       << "DL = " << (int)item.RequireClass[4] << ", "
       << "SUM = " << (int)item.RequireClass[5] << ", "
       << "RF = " << (int)item.RequireClass[6] << ", "
       << "Res0 = " << (int)item.Resistance[0] << ", "
       << "Res1 = " << (int)item.Resistance[1] << ", "
       << "Res2 = " << (int)item.Resistance[2] << ", "
       << "Res3 = " << (int)item.Resistance[3] << ", "
       << "Res4 = " << (int)item.Resistance[4] << ", "
       << "Res5 = " << (int)item.Resistance[5] << ", "
       << "Res6 = " << (int)item.Resistance[6] << ", "
       << "Res7 = " << (int)item.Resistance[7];

    return ss.str();
}

std::string CItemEditorActions::ExportItemToCSV(int itemIndex, ITEM_ATTRIBUTE& item)
{
    std::stringstream ss;
    char nameBuffer[256];
    ConvertItemName(nameBuffer, sizeof(nameBuffer), item.Name);

    ss << itemIndex << ","
       << "\"" << nameBuffer << "\","
       << (item.TwoHand ? 1 : 0) << ","
       << item.Level << ","
       << (int)item.m_byItemSlot << ","
       << item.m_wSkillIndex << ","
       << (int)item.Width << ","
       << (int)item.Height << ","
       << (int)item.DamageMin << ","
       << (int)item.DamageMax << ","
       << (int)item.SuccessfulBlocking << ","
       << (int)item.Defense << ","
       << (int)item.MagicDefense << ","
       << (int)item.WeaponSpeed << ","
       << (int)item.WalkSpeed << ","
       << (int)item.Durability << ","
       << (int)item.MagicDur << ","
       << (int)item.MagicPower << ","
       << item.RequireStrength << ","
       << item.RequireDexterity << ","
       << item.RequireEnergy << ","
       << item.RequireVitality << ","
       << item.RequireCharisma << ","
       << item.RequireLevel << ","
       << (int)item.Value << ","
       << item.iZen << ","
       << (int)item.AttType << ","
       << (int)item.RequireClass[0] << ","
       << (int)item.RequireClass[1] << ","
       << (int)item.RequireClass[2] << ","
       << (int)item.RequireClass[3] << ","
       << (int)item.RequireClass[4] << ","
       << (int)item.RequireClass[5] << ","
       << (int)item.RequireClass[6] << ","
       << (int)item.Resistance[0] << ","
       << (int)item.Resistance[1] << ","
       << (int)item.Resistance[2] << ","
       << (int)item.Resistance[3] << ","
       << (int)item.Resistance[4] << ","
       << (int)item.Resistance[5] << ","
       << (int)item.Resistance[6] << ","
       << (int)item.Resistance[7];

    return ss.str();
}

std::string CItemEditorActions::ExportItemCombined(int itemIndex, ITEM_ATTRIBUTE& item)
{
    return ExportItemToReadable(itemIndex, item) + "\n" + ExportItemToCSV(itemIndex, item);
}

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
