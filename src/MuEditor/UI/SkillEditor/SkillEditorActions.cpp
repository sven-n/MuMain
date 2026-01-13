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
    if (ImGui::Button(EDITOR_TEXT("btn_save_skills"), ImVec2(150, 0)))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Skill_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        std::string changeLog;
        bool success = g_SkillDataHandler.Save(fileName, &changeLog);

        if (success)
        {
            CSkillEditorPopups::ShowSaveSuccessPopup(changeLog);
        }
        else
        {
            CSkillEditorPopups::ShowSaveFailedPopup();
        }
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Save all skill changes to Skill_%ls.bmd", g_strSelectedML.c_str());
    }
}

void CSkillEditorActions::RenderExportCSVButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.7f, 1.0f));

    if (ImGui::Button(EDITOR_TEXT("btn_export_csv_skills"), ImVec2(150, 0)))
    {
        wchar_t fileName[256];
        swprintf(fileName, L"Skills_%ls.csv", g_strSelectedML.c_str());

        if ( g_SkillDataHandler.ExportToCsv(fileName))
        {
            CSkillEditorPopups::ShowExportCSVSuccessPopup();
        }
        else
        {
            CSkillEditorPopups::ShowExportCSVFailedPopup();
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
    RenderExportCSVButton();
}

#endif // _EDITOR
