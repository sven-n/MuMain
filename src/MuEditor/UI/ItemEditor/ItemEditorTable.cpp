#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorTable.h"
#include "ItemEditorColumns.h"
#include "ItemEditorActions.h"
#include "GameData/ItemData/ItemFieldMetadata.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Translation/i18n.h"
#include "_struct.h"
#include "_define.h"
#include "imgui.h"
#include <algorithm>
#include <sstream>

extern ITEM_ATTRIBUTE* ItemAttribute;

// Static member initialization
int CItemEditorTable::s_scrollToIndex = -1;

void CItemEditorTable::RequestScrollToIndex(int index)
{
    s_scrollToIndex = index;
}

void CItemEditorTable::InvalidateFilter()
{
    m_isInitialized = false;
}

CItemEditorTable::CItemEditorTable()
    : m_isInitialized(false)
    , m_pColumns(nullptr)
{
    m_pColumns = new CItemEditorColumns();
    m_pColumns->SetTable(this);
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
    // Get metadata fields once at function scope
    const ItemFieldDescriptor* fields = GetFieldDescriptors(); const int fieldCount = GetFieldCount();

    // Count visible columns - only count columns that actually exist
    int visibleColumnCount = 0;

    // Count Index column if visible
    if (columnVisibility.find("Index") != columnVisibility.end() && columnVisibility["Index"])
    {
        visibleColumnCount++;
    }

    // Count metadata fields that are visible
    for (int i = 0; i < fieldCount; ++i)
    {
        if (columnVisibility.find(fields[i].name) != columnVisibility.end() &&
            columnVisibility[fields[i].name])
        {
            visibleColumnCount++;
        }
    }

    if (visibleColumnCount == 0)
    {
        ImGui::Text(EDITOR_TEXT("label_no_columns"));
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
    bool hasIndex = columnVisibility.find("Index") != columnVisibility.end() && columnVisibility["Index"];
    bool hasName = columnVisibility.find("Name") != columnVisibility.end() && columnVisibility["Name"];

    if (freezeColumns && hasIndex && hasName)
    {
        ImGui::TableSetupScrollFreeze(2, 1); // Freeze first 2 columns + header row
    }
    else if (freezeColumns && hasIndex)
    {
        ImGui::TableSetupScrollFreeze(1, 1); // Freeze first column + header row
    }
    else
    {
        ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row only
    }

    // Setup columns based on visibility - METADATA-DRIVEN
    if (hasIndex)
    {
        ImGui::TableSetupColumn(EDITOR_TEXT("label_index"), ImGuiTableColumnFlags_WidthFixed, 50.0f);
    }

    for (int i = 0; i < fieldCount; ++i)
    {
        if (columnVisibility.find(fields[i].name) != columnVisibility.end() &&
            columnVisibility[fields[i].name])
        {
            ImGui::TableSetupColumn(GetFieldDisplayName(fields[i].name),
                                   ImGuiTableColumnFlags_WidthFixed,
                                   fields[i].width);
        }
    }

    ImGui::TableHeadersRow();

    // Use ImGuiListClipper for performance
    ImGuiListClipper clipper;
    clipper.Begin((int)m_filteredItems.size());

    // Handle scroll request
    if (s_scrollToIndex >= 0)
    {
        // Find the row index in filtered items
        for (int row = 0; row < (int)m_filteredItems.size(); row++)
        {
            if (m_filteredItems[row] == s_scrollToIndex)
            {
                // Force the clipper to include this row and scroll to it
                ImGui::SetScrollY(ImGui::GetTextLineHeightWithSpacing() * (row + 1)); // +1 for header
                selectedRow = s_scrollToIndex;
                break;
            }
        }
        s_scrollToIndex = -1; // Reset
    }

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

            // Render Index column (special - not in metadata)
            if (hasIndex)
            {
                m_pColumns->RenderIndexColumn(colIdx, i, rowInteracted, true);
            }

            // Render all other columns via metadata - AUTO-ADAPTING
            for (int fieldIdx = 0; fieldIdx < fieldCount; ++fieldIdx)
            {
                bool isVisible = columnVisibility.find(fields[fieldIdx].name) != columnVisibility.end() &&
                                 columnVisibility[fields[fieldIdx].name];

                if (isVisible)
                {
                    m_pColumns->RenderFieldByDescriptor(fields[fieldIdx], colIdx, i, ItemAttribute[i], rowInteracted, true);
                }
            }

            // Handle row selection
            if (rowInteracted)
            {
                selectedRow = i;
            }
        }
    }

    ImGui::EndTable();
    ImGui::PopStyleVar(2);

    // Handle Ctrl+C to copy selected row as CSV
    if (selectedRow >= 0 && selectedRow < MAX_ITEM && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_C))
    {
        std::string output = CItemEditorActions::ExportItemCombined(selectedRow, ItemAttribute[selectedRow]);
        ImGui::SetClipboardText(output.c_str());
    }
}

#endif // _EDITOR
