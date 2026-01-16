#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorColumns.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "_struct.h"
#include "_define.h"
#include "imgui.h"
#include <algorithm>
#include <sstream>

extern ITEM_ATTRIBUTE* ItemAttribute;

void CItemEditorColumns::RenderByteColumn(
    const char* columnName, int& colIdx, int itemIndex, int uniqueId,
    BYTE& value, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int intValue = value;
    if (ImGui::InputInt("##input", &intValue, 0, 0))
    {
        if (intValue >= 0 && intValue <= 255)
        {
            value = (BYTE)intValue;
            std::stringstream ss;
            ss << "Changed item " << itemIndex << " " << columnName << " to " << intValue;
            g_MuEditorConsoleUI.LogEditor(ss.str());
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
    ImGui::PushID(itemIndex * 100 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int intValue = value;
    if (ImGui::InputInt("##input", &intValue, 0, 0))
    {
        if (intValue >= 0 && intValue <= 65535)
        {
            value = (WORD)intValue;
            std::stringstream ss;
            ss << "Changed item " << itemIndex << " " << columnName << " to " << intValue;
            g_MuEditorConsoleUI.LogEditor(ss.str());
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
    ImGui::PushID(itemIndex * 100 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::InputInt("##input", &value, 0, 0))
    {
        std::stringstream ss;
        ss << "Changed item " << itemIndex << " " << columnName << " to " << value;
        g_MuEditorConsoleUI.LogEditor(ss.str());
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
    ImGui::PushID(itemIndex * 100 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::Checkbox("##checkbox", &value))
    {
        std::stringstream ss;
        ss << "Changed item " << itemIndex << " " << columnName << " to " << (value ? "true" : "false");
        g_MuEditorConsoleUI.LogEditor(ss.str());
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderByteArrayColumn(
    const char* columnName, int& colIdx, int itemIndex, int uniqueId,
    BYTE* array, int arraySize, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100 + uniqueId);
    ImGui::SetNextItemWidth(-FLT_MIN);

    std::stringstream displayText;
    for (int j = 0; j < arraySize; j++)
    {
        if (j > 0) displayText << ",";
        displayText << (int)array[j];
    }

    std::string displayStr = displayText.str();
    char buffer[256];
    strncpy_s(buffer, displayStr.c_str(), sizeof(buffer) - 1);

    if (ImGui::InputText("##input", buffer, sizeof(buffer)))
    {
        // Parse comma-separated values
        std::stringstream ss(buffer);
        std::string token;
        int idx = 0;
        while (std::getline(ss, token, ',') && idx < arraySize)
        {
            int val = atoi(token.c_str());
            if (val >= 0 && val <= 255)
            {
                array[idx] = (BYTE)val;
            }
            idx++;
        }

        std::stringstream logSs;
        logSs << "Changed item " << itemIndex << " " << columnName;
        g_MuEditorConsoleUI.LogEditor(logSs.str());
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderIndexColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100 + 100);
    ImGui::SetNextItemWidth(-FLT_MIN);

    int newIndex = itemIndex;
    if (ImGui::InputInt("##index", &newIndex, 0, 0))
    {
        if (newIndex >= 0 && newIndex < MAX_ITEM && newIndex != itemIndex)
        {
            char targetName[128];
            WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[newIndex].Name, -1, targetName, sizeof(targetName), NULL, NULL);

            if (targetName[0] == '\0')
            {
                ITEM_ATTRIBUTE temp = ItemAttribute[itemIndex];
                ItemAttribute[itemIndex] = ItemAttribute[newIndex];
                ItemAttribute[newIndex] = temp;
                g_MuEditorConsoleUI.LogEditor("Moved item from index " + std::to_string(itemIndex) + " to " + std::to_string(newIndex));
            }
            else
            {
                g_MuEditorConsoleUI.LogEditor("Error: Index " + std::to_string(newIndex) + " already in use!");
            }
        }
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderNameColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100 + 101);
    ImGui::SetNextItemWidth(-FLT_MIN);

    char editableNameBuffer[128];
    WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[itemIndex].Name, -1, editableNameBuffer, sizeof(editableNameBuffer), NULL, NULL);

    if (ImGui::InputText("##name", editableNameBuffer, sizeof(editableNameBuffer)))
    {
        MultiByteToWideChar(CP_UTF8, 0, editableNameBuffer, -1, ItemAttribute[itemIndex].Name, MAX_ITEM_NAME);
        g_MuEditorConsoleUI.LogEditor("Changed item " + std::to_string(itemIndex) + " Name to " + editableNameBuffer);
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

void CItemEditorColumns::RenderTwoHandColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible)
{
    if (!isVisible) return;

    ImGui::TableSetColumnIndex(colIdx++);
    ImGui::PushID(itemIndex * 100 + 102);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::Checkbox("##twohand", &ItemAttribute[itemIndex].TwoHand))
    {
        g_MuEditorConsoleUI.LogEditor("Changed item " + std::to_string(itemIndex) + " TwoHand to " +
                                     std::string(ItemAttribute[itemIndex].TwoHand ? "true" : "false"));
    }

    if (ImGui::IsItemActivated()) rowInteracted = true;
    ImGui::PopID();
}

#endif // _EDITOR
