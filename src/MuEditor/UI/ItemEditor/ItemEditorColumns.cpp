#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorColumns.h"
#include "ItemEditorTable.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "GameData/ItemData/ItemFieldDefs.h"
#include "Translation/i18n.h"
#include "_struct.h"
#include "_define.h"
#include "imgui.h"
#include <algorithm>
#include <sstream>

extern ITEM_ATTRIBUTE* ItemAttribute;

// ===== X-MACRO-DRIVEN RENDERING =====

void CItemEditorColumns::RenderFieldByDescriptor(const FieldDescriptor& desc, int& colIdx, int itemIndex,
                                                  ITEM_ATTRIBUTE& item, bool& rowInteracted, bool isVisible)
{
    // Use the template helper function from ItemFieldDefs.h
    ::RenderFieldByDescriptor(desc, this, item, colIdx, itemIndex, rowInteracted, isVisible);
}

// ===== LOW-LEVEL TYPE-SPECIFIC RENDERING =====

void CItemEditorColumns::RenderByteColumn(
    const char* columnName, int& colIdx, int itemIndex, int uniqueId,
    BYTE& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int intValue = value;
    if (ImGui::InputInt("##input", &intValue, 0, 0))
    {
        if (intValue >= 0 && intValue <= 255)
        {
            value = (BYTE)intValue;
            g_MuEditorConsoleUI.LogEditor("Changed item " + std::to_string(itemIndex) + " " + columnName + " to " + std::to_string(intValue));
        }
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderWordColumn(
    const char* columnName, int& colIdx, int itemIndex, int uniqueId,
    WORD& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int intValue = value;
    if (ImGui::InputInt("##input", &intValue, 0, 0))
    {
        if (intValue >= 0 && intValue <= 65535)
        {
            value = (WORD)intValue;
            g_MuEditorConsoleUI.LogEditor("Changed item " + std::to_string(itemIndex) + " " + columnName + " to " + std::to_string(intValue));
        }
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderIntColumn(
    const char* columnName, int& colIdx, int itemIndex, int uniqueId,
    int& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::InputInt("##input", &value, 0, 0))
    {
        std::string logMsg = i18n::FormatEditor("log_changed_item", {
            std::to_string(itemIndex),
            columnName,
            std::to_string(value)
        });
        g_MuEditorConsoleUI.LogEditor(logMsg);
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderBoolColumn(
    const char* columnName, int& colIdx, int itemIndex, int uniqueId,
    bool& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::Checkbox("##checkbox", &value))
    {
        std::string logMsg = i18n::FormatEditor("log_changed_item", {
            std::to_string(itemIndex),
            columnName,
            value ? "true" : "false"
        });
        g_MuEditorConsoleUI.LogEditor(logMsg);
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderWCharArrayColumn(
    const char* columnName, int& colIdx, int itemIndex, int uniqueId,
    wchar_t* value, int arraySize, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100000 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    char editableBuffer[256];
    WideCharToMultiByte(CP_UTF8, 0, value, -1, editableBuffer, sizeof(editableBuffer), NULL, NULL);

    if (ImGui::InputText("##input", editableBuffer, sizeof(editableBuffer)))
    {
        MultiByteToWideChar(CP_UTF8, 0, editableBuffer, -1, value, arraySize);
        g_MuEditorConsoleUI.LogEditor("Changed item " + std::to_string(itemIndex) + " " + columnName + " to " + std::string(editableBuffer));
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

// ===== SPECIAL COLUMNS =====

void CItemEditorColumns::RenderIndexColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100000 + 999999);
    ImGui::SetNextItemWidth(-FLT_MIN);

    static int s_lastEditedIndex = -1;
    static bool s_errorLogged = false;

    int newIndex = itemIndex;
    ImGui::InputInt("##index", &newIndex, 0, 0);
    
    // Only process the change when the input is deactivated (Enter pressed or focus lost)
    bool wasActive = ImGui::IsItemActive();
    bool wasDeactivated = ImGui::IsItemDeactivatedAfterEdit();
    
    if (wasDeactivated && newIndex >= 0 && newIndex < MAX_ITEM && newIndex != itemIndex)
    {
        char targetName[128];
        WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[newIndex].Name, -1, targetName, sizeof(targetName), NULL, NULL);

        if (targetName[0] == '\0')
        {
            ITEM_ATTRIBUTE temp = ItemAttribute[itemIndex];
            ItemAttribute[itemIndex] = ItemAttribute[newIndex];
            ItemAttribute[newIndex] = temp;

            g_MuEditorConsoleUI.LogEditor("Moved item from index " + std::to_string(itemIndex) + " to " + std::to_string(newIndex));
            
            s_errorLogged = false;
            
            // Invalidate the filter to rebuild the item list
            if (m_pTable)
            {
                m_pTable->InvalidateFilter();
            }
            
            // Scroll to new index position
            CItemEditorTable::RequestScrollToIndex(newIndex);
        }
        else if (!s_errorLogged)
        {
            std::string errorMsg = i18n::FormatEditor("error_index_in_use", {
                std::to_string(newIndex)
            });
            g_MuEditorConsoleUI.LogEditor(errorMsg);
            s_errorLogged = true;
        }
    }
    
    // Reset error flag when user starts editing a different field
    if (wasActive)
    {
        if (s_lastEditedIndex != itemIndex)
        {
            s_errorLogged = false;
            s_lastEditedIndex = itemIndex;
        }
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

#endif // _EDITOR
