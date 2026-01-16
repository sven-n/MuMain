#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorTable.h"
#include "ItemEditorColumns.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "_struct.h"
#include "_define.h"
#include "imgui.h"
#include <algorithm>
#include <sstream>

extern ITEM_ATTRIBUTE* ItemAttribute;

CItemEditorTable::CItemEditorTable()
    : m_isInitialized(false)
    , m_pColumns(nullptr)
{
    m_pColumns = new CItemEditorColumns();
}

CItemEditorTable::~CItemEditorTable()
{
    if (m_pColumns)
    {
        delete m_pColumns;
        m_pColumns = nullptr;
    }
}

void CItemEditorTable::Render(
    const std::string& searchFilter,
    std::map<std::string, bool>& columnVisibility,
    int& selectedRow,
    bool freezeColumns)
{
    // Count visible columns
    int visibleColumnCount = 0;
    for (const auto& col : columnVisibility)
    {
        if (col.second) visibleColumnCount++;
    }

    if (visibleColumnCount == 0)
    {
        ImGui::Text("No columns selected. Click 'Columns' to show columns.");
        return;
    }

    // Rebuild filtered list if search changed
    if (searchFilter != m_lastSearchFilter || !m_isInitialized)
    {
        m_filteredItems.clear();
        for (int i = 0; i < MAX_ITEM; i++)
        {
            char nameBuffer[256];
            WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[i].Name, -1, nameBuffer, sizeof(nameBuffer), NULL, NULL);

            if (nameBuffer[0] == '\0') continue;

            if (searchFilter.length() > 0)
            {
                std::string nameLower = nameBuffer;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                if (nameLower.find(searchFilter) == std::string::npos)
                    continue;
            }

            m_filteredItems.push_back(i);
        }
        m_lastSearchFilter = searchFilter;
        m_isInitialized = true;
    }

    // Push style to reduce input field padding
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));

    // Create a table with scrolling
    if (!ImGui::BeginTable("ItemTable", visibleColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable))
    {
        ImGui::PopStyleVar(2);
        return;
    }

    // Setup frozen columns based on toggle state
    if (freezeColumns && columnVisibility["Index"] && columnVisibility["Name"])
    {
        ImGui::TableSetupScrollFreeze(2, 1); // Freeze first 2 columns + header row
    }
    else if (freezeColumns && columnVisibility["Index"])
    {
        ImGui::TableSetupScrollFreeze(1, 1); // Freeze first column + header row
    }
    else
    {
        ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row only
    }

    // Setup columns based on visibility
    if (columnVisibility["Index"]) ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    if (columnVisibility["Name"]) ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
    if (columnVisibility["TwoHand"]) ImGui::TableSetupColumn("TwoHand", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["Level"]) ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["Slot"]) ImGui::TableSetupColumn("Slot", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["Skill"]) ImGui::TableSetupColumn("Skill", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["Width"]) ImGui::TableSetupColumn("Width", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["Height"]) ImGui::TableSetupColumn("Height", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["DamageMin"]) ImGui::TableSetupColumn("DmgMin", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["DamageMax"]) ImGui::TableSetupColumn("DmgMax", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["SuccessfulBlocking"]) ImGui::TableSetupColumn("Block", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["Defense"]) ImGui::TableSetupColumn("Defense", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["MagicDefense"]) ImGui::TableSetupColumn("MagDef", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["WeaponSpeed"]) ImGui::TableSetupColumn("WpnSpd", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["WalkSpeed"]) ImGui::TableSetupColumn("WalkSpd", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["Durability"]) ImGui::TableSetupColumn("Dur", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["MagicDur"]) ImGui::TableSetupColumn("MagDur", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["MagicPower"]) ImGui::TableSetupColumn("MagPow", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["ReqStr"]) ImGui::TableSetupColumn("ReqStr", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["ReqDex"]) ImGui::TableSetupColumn("ReqDex", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["ReqEne"]) ImGui::TableSetupColumn("ReqEne", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["ReqVit"]) ImGui::TableSetupColumn("ReqVit", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["ReqCha"]) ImGui::TableSetupColumn("ReqCha", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["ReqLevel"]) ImGui::TableSetupColumn("ReqLvl", ImGuiTableColumnFlags_WidthFixed, 70.0f);
    if (columnVisibility["Value"]) ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["Zen"]) ImGui::TableSetupColumn("Zen", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    if (columnVisibility["AttType"]) ImGui::TableSetupColumn("AttType", ImGuiTableColumnFlags_WidthFixed, 70.0f);

    // Individual class requirement columns
    if (columnVisibility["DW/SM"]) ImGui::TableSetupColumn("DW/SM", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["DK/BK"]) ImGui::TableSetupColumn("DK/BK", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    if (columnVisibility["ELF/ME"]) ImGui::TableSetupColumn("ELF/ME", ImGuiTableColumnFlags_WidthFixed, 65.0f);
    if (columnVisibility["MG"]) ImGui::TableSetupColumn("MG", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    if (columnVisibility["DL"]) ImGui::TableSetupColumn("DL", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    if (columnVisibility["SUM"]) ImGui::TableSetupColumn("SUM", ImGuiTableColumnFlags_WidthFixed, 55.0f);
    if (columnVisibility["RF"]) ImGui::TableSetupColumn("RF", ImGuiTableColumnFlags_WidthFixed, 50.0f);

    ImGui::TableHeadersRow();

    // Use ImGuiListClipper for performance
    ImGuiListClipper clipper;
    clipper.Begin((int)m_filteredItems.size());

    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
        {
            int i = m_filteredItems[row];
            ImGui::TableNextRow();

            if (selectedRow == i)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.3f, 0.5f, 0.8f, 0.3f)));
            }

            int colIdx = 0;
            bool rowInteracted = false;

            // Render all columns using CItemEditorColumns - MUST match setup order!
            m_pColumns->RenderIndexColumn(colIdx, i, rowInteracted, columnVisibility["Index"]);
            m_pColumns->RenderNameColumn(colIdx, i, rowInteracted, columnVisibility["Name"]);
            m_pColumns->RenderTwoHandColumn(colIdx, i, rowInteracted, columnVisibility["TwoHand"]);
            m_pColumns->RenderWordColumn("Level", colIdx, i, 1, ItemAttribute[i].Level, rowInteracted, columnVisibility["Level"]);
            m_pColumns->RenderByteColumn("Slot", colIdx, i, 2, ItemAttribute[i].m_byItemSlot, rowInteracted, columnVisibility["Slot"]);
            m_pColumns->RenderWordColumn("Skill", colIdx, i, 3, ItemAttribute[i].m_wSkillIndex, rowInteracted, columnVisibility["Skill"]);
            m_pColumns->RenderByteColumn("Width", colIdx, i, 4, ItemAttribute[i].Width, rowInteracted, columnVisibility["Width"]);
            m_pColumns->RenderByteColumn("Height", colIdx, i, 5, ItemAttribute[i].Height, rowInteracted, columnVisibility["Height"]);
            m_pColumns->RenderByteColumn("DamageMin", colIdx, i, 6, ItemAttribute[i].DamageMin, rowInteracted, columnVisibility["DamageMin"]);
            m_pColumns->RenderByteColumn("DamageMax", colIdx, i, 7, ItemAttribute[i].DamageMax, rowInteracted, columnVisibility["DamageMax"]);
            m_pColumns->RenderByteColumn("SuccessfulBlocking", colIdx, i, 8, ItemAttribute[i].SuccessfulBlocking, rowInteracted, columnVisibility["SuccessfulBlocking"]);
            m_pColumns->RenderByteColumn("Defense", colIdx, i, 9, ItemAttribute[i].Defense, rowInteracted, columnVisibility["Defense"]);
            m_pColumns->RenderByteColumn("MagicDefense", colIdx, i, 10, ItemAttribute[i].MagicDefense, rowInteracted, columnVisibility["MagicDefense"]);
            m_pColumns->RenderByteColumn("WeaponSpeed", colIdx, i, 11, ItemAttribute[i].WeaponSpeed, rowInteracted, columnVisibility["WeaponSpeed"]);
            m_pColumns->RenderByteColumn("WalkSpeed", colIdx, i, 12, ItemAttribute[i].WalkSpeed, rowInteracted, columnVisibility["WalkSpeed"]);
            m_pColumns->RenderByteColumn("Durability", colIdx, i, 13, ItemAttribute[i].Durability, rowInteracted, columnVisibility["Durability"]);
            m_pColumns->RenderByteColumn("MagicDur", colIdx, i, 14, ItemAttribute[i].MagicDur, rowInteracted, columnVisibility["MagicDur"]);
            m_pColumns->RenderByteColumn("MagicPower", colIdx, i, 15, ItemAttribute[i].MagicPower, rowInteracted, columnVisibility["MagicPower"]);
            m_pColumns->RenderWordColumn("ReqStr", colIdx, i, 16, ItemAttribute[i].RequireStrength, rowInteracted, columnVisibility["ReqStr"]);
            m_pColumns->RenderWordColumn("ReqDex", colIdx, i, 17, ItemAttribute[i].RequireDexterity, rowInteracted, columnVisibility["ReqDex"]);
            m_pColumns->RenderWordColumn("ReqEne", colIdx, i, 18, ItemAttribute[i].RequireEnergy, rowInteracted, columnVisibility["ReqEne"]);
            m_pColumns->RenderWordColumn("ReqVit", colIdx, i, 19, ItemAttribute[i].RequireVitality, rowInteracted, columnVisibility["ReqVit"]);
            m_pColumns->RenderWordColumn("ReqCha", colIdx, i, 20, ItemAttribute[i].RequireCharisma, rowInteracted, columnVisibility["ReqCha"]);
            m_pColumns->RenderWordColumn("ReqLevel", colIdx, i, 21, ItemAttribute[i].RequireLevel, rowInteracted, columnVisibility["ReqLevel"]);
            m_pColumns->RenderByteColumn("Value", colIdx, i, 22, ItemAttribute[i].Value, rowInteracted, columnVisibility["Value"]);
            m_pColumns->RenderIntColumn("Zen", colIdx, i, 23, ItemAttribute[i].iZen, rowInteracted, columnVisibility["Zen"]);
            m_pColumns->RenderByteColumn("AttType", colIdx, i, 24, ItemAttribute[i].AttType, rowInteracted, columnVisibility["AttType"]);

            // Individual class requirement columns
            m_pColumns->RenderByteColumn("DW/SM", colIdx, i, 25, ItemAttribute[i].RequireClass[0], rowInteracted, columnVisibility["DW/SM"]);
            m_pColumns->RenderByteColumn("DK/BK", colIdx, i, 26, ItemAttribute[i].RequireClass[1], rowInteracted, columnVisibility["DK/BK"]);
            m_pColumns->RenderByteColumn("ELF/ME", colIdx, i, 27, ItemAttribute[i].RequireClass[2], rowInteracted, columnVisibility["ELF/ME"]);
            m_pColumns->RenderByteColumn("MG", colIdx, i, 28, ItemAttribute[i].RequireClass[3], rowInteracted, columnVisibility["MG"]);
            m_pColumns->RenderByteColumn("DL", colIdx, i, 29, ItemAttribute[i].RequireClass[4], rowInteracted, columnVisibility["DL"]);
            m_pColumns->RenderByteColumn("SUM", colIdx, i, 30, ItemAttribute[i].RequireClass[5], rowInteracted, columnVisibility["SUM"]);
            m_pColumns->RenderByteColumn("RF", colIdx, i, 31, ItemAttribute[i].RequireClass[6], rowInteracted, columnVisibility["RF"]);

            // Handle row selection
            if (rowInteracted)
            {
                selectedRow = i;
            }
        }
    }

    ImGui::EndTable();
    ImGui::PopStyleVar(2);
}

#endif // _EDITOR
