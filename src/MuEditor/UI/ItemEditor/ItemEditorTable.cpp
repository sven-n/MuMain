#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorTable.h"
#include "ItemEditorColumns.h"
#include "ItemEditorActions.h"
#include "Data/GameData/ItemData/ItemFieldMetadata.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "I18N/All.h"
#include "Core/Globals/_struct.h"
#include "Core/Globals/_define.h"
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

bool CItemEditorTable::IsListed(int itemIndex, const std::string& searchFilter)
{
    char nameBuffer[256];
    WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[itemIndex].Name, -1, nameBuffer, sizeof(nameBuffer), NULL, NULL);
    if (nameBuffer[0] == '\0')
    {
        return false;
    }
    if (searchFilter.empty())
    {
        return true;
    }
    std::string nameLower = nameBuffer;
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
    return nameLower.find(searchFilter) != std::string::npos;
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
        ImGui::Text(I18N::Editor::NoColumnsSelectedClickColumnsToShowColumns);
        return;
    }

    // Rebuild filtered list if search changed
    if (searchFilter != m_lastSearchFilter || !m_isInitialized)
    {
        m_filteredItems.clear();
        for (int i = 0; i < MAX_ITEM; i++)
        {
            if (IsListed(i, searchFilter))
            {
                m_filteredItems.push_back(i);
            }
        }
        m_lastSearchFilter = searchFilter;
        m_isInitialized = true;
    }

    // Push style to reduce input field padding
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));

    // Create a table with scrolling; it keeps some rows when the parts above it
    // (the Looks section) take the height of the window.
    const float minTableHeight = ImGui::GetFrameHeightWithSpacing() * 10.0f;
    const ImVec2 tableSize(0.0f, std::max(ImGui::GetContentRegionAvail().y, minTableHeight));
    if (!ImGui::BeginTable("ItemTable", visibleColumnCount,
                           ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                               ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable,
                           tableSize))
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
        ImGui::TableSetupColumn(I18N::Editor::Index, ImGuiTableColumnFlags_WidthFixed, 50.0f);
    }

    for (int i = 0; i < fieldCount; ++i)
    {
        if (columnVisibility.find(fields[i].name) != columnVisibility.end() &&
            columnVisibility[fields[i].name])
        {
            ImGui::TableSetupColumn(GetFieldDisplayName(fields[i]),
                                   ImGuiTableColumnFlags_WidthFixed,
                                   fields[i].width);
        }
    }

    ImGui::TableHeadersRow();

    // Use ImGuiListClipper for performance
    ImGuiListClipper clipper;
    clipper.Begin((int)m_filteredItems.size());

    // Handle scroll request: the item is selected, and the table scrolls to its
    // row when the search lists it.
    int scrollRow = -1;
    if (s_scrollToIndex >= 0)
    {
        const auto found = std::find(m_filteredItems.begin(), m_filteredItems.end(), s_scrollToIndex);
        if (found != m_filteredItems.end())
        {
            scrollRow = static_cast<int>(found - m_filteredItems.begin());
            clipper.IncludeItemByIndex(scrollRow);
        }
        selectedRow = s_scrollToIndex;
        s_scrollToIndex = -1; // Reset
    }

    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
        {
            int i = m_filteredItems[row];
            ImGui::TableNextRow();
            if (row == scrollRow)
            {
                ImGui::SetScrollHereY(0.5f);
            }

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
