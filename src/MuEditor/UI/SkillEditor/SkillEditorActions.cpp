#include "stdafx.h"

#ifdef _EDITOR

#include "SkillEditorActions.h"
#include "SkillEditorPopups.h"
#include "DataHandler/SkillData/SkillDataHandler.h"
#include "GameData/SkillData/SkillFieldMetadata.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Translation/i18n.h"
#include "imgui.h"
#include <string>
#include <sstream>

extern std::wstring g_strSelectedML;

// ===== HELPER FUNCTIONS =====

void CSkillEditorActions::ConvertSkillName(char* outBuffer, size_t bufferSize, const wchar_t* name)
{
    WideCharToMultiByte(CP_UTF8, 0, name, -1, outBuffer, (int)bufferSize, NULL, NULL);
}

std::string CSkillEditorActions::GetFieldValueAsString(const SKILL_ATTRIBUTE& skill, const SkillFieldDescriptor& desc)
{
    std::stringstream ss;

    // Get pointer to the field using offset
    const BYTE* skillPtr = reinterpret_cast<const BYTE*>(&skill);
    const void* fieldPtr = skillPtr + desc.offset;

    switch (desc.type)
    {
    case ESkillFieldType::Bool:
        ss << (*reinterpret_cast<const bool*>(fieldPtr) ? 1 : 0);
        break;

    case ESkillFieldType::Byte:
        ss << (int)*reinterpret_cast<const BYTE*>(fieldPtr);
        break;

    case ESkillFieldType::Word:
        ss << *reinterpret_cast<const WORD*>(fieldPtr);
        break;

    case ESkillFieldType::Int:
        ss << *reinterpret_cast<const int*>(fieldPtr);
        break;

    case ESkillFieldType::DWord:
        ss << *reinterpret_cast<const DWORD*>(fieldPtr);
        break;

    case ESkillFieldType::WCharArray:
    {
        char buffer[256];
        ConvertSkillName(buffer, sizeof(buffer), reinterpret_cast<const wchar_t*>(fieldPtr));
        ss << buffer;
        break;
    }
    }

    return ss.str();
}

// ===== EXPORT FUNCTIONS =====

std::string CSkillEditorActions::GetCSVHeader()
{
    std::stringstream ss;
    const SkillFieldDescriptor* fields = GetSkillFieldDescriptors();
    const int fieldCount = GetSkillFieldCount();

    ss << "Index";
    for (int i = 0; i < fieldCount; ++i)
    {
        ss << "," << GetSkillFieldDisplayName(fields[i].name);
    }

    return ss.str();
}

std::string CSkillEditorActions::ExportSkillToReadable(int skillIndex, SKILL_ATTRIBUTE& skill)
{
    std::stringstream ss;
    const SkillFieldDescriptor* fields = GetSkillFieldDescriptors();
    const int fieldCount = GetSkillFieldCount();

    ss << "Skill " << skillIndex << ":\n";

    for (int i = 0; i < fieldCount; ++i)
    {
        const char* displayName = GetSkillFieldDisplayName(fields[i].name);
        std::string value = GetFieldValueAsString(skill, fields[i]);
        ss << "  " << displayName << ": " << value << "\n";
    }

    return ss.str();
}

std::string CSkillEditorActions::ExportSkillToCSV(int skillIndex, SKILL_ATTRIBUTE& skill)
{
    std::stringstream ss;
    const SkillFieldDescriptor* fields = GetSkillFieldDescriptors();
    const int fieldCount = GetSkillFieldCount();

    ss << skillIndex;
    for (int i = 0; i < fieldCount; ++i)
    {
        std::string value = GetFieldValueAsString(skill, fields[i]);
        ss << "," << value;
    }

    return ss.str();
}

std::string CSkillEditorActions::ExportSkillCombined(int skillIndex, SKILL_ATTRIBUTE& skill)
{
    std::stringstream ss;
    ss << ExportSkillToReadable(skillIndex, skill);
    ss << "\n";
    ss << "CSV: " << ExportSkillToCSV(skillIndex, skill);
    ss << "\n";
    return ss.str();
}

// ===== ACTION BUTTONS =====

void CSkillEditorActions::RenderSaveButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.9f, 1.0f));

    if (ImGui::Button(EDITOR_TEXT("btn_save_skills")))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Skill_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        std::string changeLog;
        if (g_SkillDataHandler.Save(fileName, &changeLog))
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

void CSkillEditorActions::RenderExportLegacyButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.4f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));

    if (ImGui::Button(EDITOR_TEXT("btn_export_s6e3")))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Skill_S6E3.bmd", g_strSelectedML.c_str());

        if (g_SkillDataHandler.ExportAsS6E3(fileName))
        {
            std::string filename_str = "Skill_" +
                std::string(g_strSelectedML.begin(), g_strSelectedML.end()) +
                "_Legacy.bmd";
            g_MuEditorConsoleUI.LogEditor("Exported skills as legacy format (32-byte names): " + filename_str);
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

void CSkillEditorActions::RenderExportCSVButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.7f, 1.0f));

    if (ImGui::Button(EDITOR_TEXT("btn_export_csv_skills"), ImVec2(150, 0)))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Skills.csv", g_strSelectedML.c_str());

        if (g_SkillDataHandler.ExportToCsv(fileName))
        {
            std::string filename_str = "Skills_" +
                std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + ".csv";
            g_MuEditorConsoleUI.LogEditor("Exported skills as CSV: " + filename_str);
            ImGui::OpenPopup("Export CSV Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor(EDITOR_TEXT("msg_export_csv_skills_failed"));
            ImGui::OpenPopup("Export CSV Failed");
        }
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Export all skills to CSV format");
    }

    ImGui::PopStyleColor(2);
}

void CSkillEditorActions::RenderAllButtons()
{
    RenderSaveButton();
    ImGui::SameLine();
    RenderExportLegacyButton();
    ImGui::SameLine();
    RenderExportCSVButton();
}

#endif // _EDITOR
