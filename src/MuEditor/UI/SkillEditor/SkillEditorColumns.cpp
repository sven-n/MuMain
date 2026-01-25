#include "stdafx.h"

#ifdef _EDITOR

#include "SkillEditorColumns.h"
#include "SkillEditorTable.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "GameData/SkillData/SkillFieldDefs.h"
#include "Translation/i18n.h"
#include "_struct.h"
#include "_define.h"
#include "imgui.h"
#include <algorithm>
#include <sstream>

extern SKILL_ATTRIBUTE* SkillAttribute;

// ===== HELPER FUNCTIONS =====

static std::string GetSkillNameUtf8(int skillIndex)
{
    char nameBuf[256]{};
    if (SkillAttribute && skillIndex >= 0 && skillIndex < MAX_SKILLS)
    {
        WideCharToMultiByte(CP_UTF8, 0, SkillAttribute[skillIndex].Name, -1, nameBuf, sizeof(nameBuf), NULL, NULL);
    }
    if (nameBuf[0] == '\0')
        return "<unnamed>";
    return nameBuf;
}

static void LogSkillFieldChange(int skillIndex, const char* columnName, const std::string& newValue)
{
    g_MuEditorConsoleUI.LogEditor(
        "Changed skill " + std::to_string(skillIndex) +
        " (" + GetSkillNameUtf8(skillIndex) + ") " +
        columnName + " to " + newValue
    );
}

// ===== X-MACRO-DRIVEN RENDERING =====

void CSkillEditorColumns::RenderFieldByDescriptor(const SkillFieldDescriptor& desc, int& colIdx, int skillIndex,
                                                  SKILL_ATTRIBUTE& skill, bool& rowInteracted, bool isVisible)
{
    // Use the template helper function from SkillFieldMetadata.h
    ::RenderFieldByDescriptor(desc, this, skill, colIdx, skillIndex, rowInteracted, isVisible, MAX_SKILL_NAME);
}

// ===== LOW-LEVEL TYPE-SPECIFIC RENDERING =====

void CSkillEditorColumns::RenderByteColumn(
    const char* columnName, int& colIdx, int skillIndex, int uniqueId,
    BYTE& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(skillIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int intValue = value;
    if (ImGui::InputInt("##input", &intValue, 0, 0))
    {
        if (intValue >= 0 && intValue <= 255)
        {
            value = (BYTE)intValue;
            LogSkillFieldChange(skillIndex, columnName, std::to_string(intValue));
        }
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CSkillEditorColumns::RenderWordColumn(
    const char* columnName, int& colIdx, int skillIndex, int uniqueId,
    WORD& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(skillIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int intValue = value;
    if (ImGui::InputInt("##input", &intValue, 0, 0))
    {
        if (intValue >= 0 && intValue <= 65535)
        {
            value = (WORD)intValue;
            LogSkillFieldChange(skillIndex, columnName, std::to_string(intValue));
        }
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CSkillEditorColumns::RenderIntColumn(
    const char* columnName, int& colIdx, int skillIndex, int uniqueId,
    int& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(skillIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::InputInt("##input", &value, 0, 0))
    {
        LogSkillFieldChange(skillIndex, columnName, std::to_string(value));
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CSkillEditorColumns::RenderDWordColumn(
    const char* columnName, int& colIdx, int skillIndex, int uniqueId,
    DWORD& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(skillIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    // DWORD is unsigned, range 0 to 4294967295
    if (ImGui::InputScalar("##input", ImGuiDataType_U32, &value, nullptr, nullptr, "%u"))
    {
        LogSkillFieldChange(skillIndex, columnName, std::to_string(value));
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CSkillEditorColumns::RenderBoolColumn(
    const char* columnName, int& colIdx, int skillIndex, int uniqueId,
    bool& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(skillIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::Checkbox("##checkbox", &value))
    {
        LogSkillFieldChange(skillIndex, columnName, value ? "true" : "false");
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CSkillEditorColumns::RenderWCharArrayColumn(
    const char* columnName, int& colIdx, int skillIndex, int uniqueId,
    wchar_t* value, int arraySize, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(skillIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    char editableBuffer[256];
    WideCharToMultiByte(CP_UTF8, 0, value, -1, editableBuffer, sizeof(editableBuffer), NULL, NULL);

    if (ImGui::InputText("##input", editableBuffer, sizeof(editableBuffer)))
    {
        MultiByteToWideChar(CP_UTF8, 0, editableBuffer, -1, value, arraySize);
        LogSkillFieldChange(skillIndex, columnName, std::string(editableBuffer));
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

// ===== SPECIAL COLUMNS =====

void CSkillEditorColumns::RenderIndexColumn(int& colIdx, int skillIndex, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(skillIndex * 100000 + 999999);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int newIndex = skillIndex;
    ImGui::InputInt("##index", &newIndex, 0, 0);

    // Only process the change when the input is deactivated (Enter pressed or focus lost)
    bool wasActive = ImGui::IsItemActive();
    bool wasDeactivated = ImGui::IsItemDeactivatedAfterEdit();

    if (wasDeactivated && newIndex >= 0 && newIndex < MAX_SKILLS && newIndex != skillIndex)
    {
        char targetName[128];
        WideCharToMultiByte(CP_UTF8, 0, SkillAttribute[newIndex].Name, -1, targetName, sizeof(targetName), NULL, NULL);

        if (targetName[0] == '\0')
        {
            SKILL_ATTRIBUTE temp = SkillAttribute[skillIndex];
            SkillAttribute[skillIndex] = SkillAttribute[newIndex];
            SkillAttribute[newIndex] = temp;

            g_MuEditorConsoleUI.LogEditor("Moved skill from index " + std::to_string(skillIndex) + " to " + std::to_string(newIndex));

            m_errorLogged = false;

            // Invalidate the filter to rebuild the skill list
            if (m_pTable)
            {
                m_pTable->InvalidateFilter();
            }

            // Scroll to new index position
            CSkillEditorTable::RequestScrollToIndex(newIndex);
        }
        else if (!m_errorLogged)
        {
            std::string errorMsg = i18n::FormatEditor("error_index_in_use", {
                std::to_string(newIndex)
            });
            g_MuEditorConsoleUI.LogEditor(errorMsg);
            m_errorLogged = true;
        }
    }

    // Reset error flag when user starts editing a different field
    if (wasActive)
    {
        if (m_lastEditedIndex != skillIndex)
        {
            m_errorLogged = false;
            m_lastEditedIndex = skillIndex;
        }
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

#endif // _EDITOR
