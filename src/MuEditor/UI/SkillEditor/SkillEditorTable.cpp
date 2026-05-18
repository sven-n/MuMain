#include "stdafx.h"

#ifdef _EDITOR

#include "SkillEditorTable.h"
#include "SkillEditorColumns.h"
#include "SkillTooltipEditor.h"
#include "Data/GameData/SkillData/SkillFieldMetadata.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/Translation/i18n.h"
#include "Core/Globals/_struct.h"
#include "Core/Globals/_define.h"
#include "imgui.h"
#include <algorithm>
#include <sstream>

extern SKILL_ATTRIBUTE* SkillAttribute;

// Static member initialization
int CSkillEditorTable::s_scrollToIndex = -1;

void CSkillEditorTable::RequestScrollToIndex(int index)
{
    s_scrollToIndex = index;
}

void CSkillEditorTable::InvalidateFilter()
{
    m_isInitialized = false;
}

CSkillEditorTable::CSkillEditorTable()
    : m_isInitialized(false)
    , m_pColumns(nullptr)
{
    m_pColumns = new CSkillEditorColumns();
    m_pColumns->SetTable(this);
}

CSkillEditorTable::~CSkillEditorTable()
{
    if (m_pColumns)
    {
        delete m_pColumns;
        m_pColumns = nullptr;
    }
}

void CSkillEditorTable::Render(
    const std::string& searchFilter,
    std::map<std::string, bool>& columnVisibility,
    int& selectedRow,
    bool freezeColumns,
    bool showHoverTooltip)
{
    // Get metadata fields once at function scope
    const SkillFieldDescriptor* fields = GetSkillFieldDescriptors();
    const int fieldCount = GetSkillFieldCount();

    // Count visible columns
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
        m_filteredSkills.clear();
        for (int i = 0; i < MAX_SKILLS; i++)
        {
            char nameBuffer[256];
            WideCharToMultiByte(CP_UTF8, 0, SkillAttribute[i].Name, -1, nameBuffer, sizeof(nameBuffer), NULL, NULL);

            if (nameBuffer[0] == '\0') continue;

            if (searchFilter.length() > 0)
            {
                std::string nameLower = nameBuffer;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                if (nameLower.find(searchFilter) == std::string::npos)
                    continue;
            }

            m_filteredSkills.push_back(i);
        }

        m_lastSearchFilter = searchFilter;
        m_isInitialized = true;
    }

    // Setup table flags
    ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_Sortable |
        ImGuiTableFlags_SortMulti |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV |
        ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_SizingFixedFit;

    if (freezeColumns)
    {
        flags |= ImGuiTableFlags_ScrollX;
    }

    // Begin table
    if (ImGui::BeginTable("##SkillTable", visibleColumnCount, flags))
    {
        // Freeze Index and Name columns if requested
        if (freezeColumns)
        {
            int frozenColumns = 0;
            if (columnVisibility.find("Index") != columnVisibility.end() && columnVisibility["Index"])
                frozenColumns++;
            if (columnVisibility.find("Name") != columnVisibility.end() && columnVisibility["Name"])
                frozenColumns++;

            ImGui::TableSetupScrollFreeze(frozenColumns, 1);
        }

        // Setup columns
        bool indexVisible = columnVisibility.find("Index") != columnVisibility.end() && columnVisibility["Index"];
        if (indexVisible)
        {
            ImGui::TableSetupColumn(EDITOR_TEXT("label_index"), ImGuiTableColumnFlags_None, 60.0f);
        }

        for (int i = 0; i < fieldCount; ++i)
        {
            if (columnVisibility.find(fields[i].name) != columnVisibility.end() &&
                columnVisibility[fields[i].name])
            {
                const char* displayName = GetSkillFieldDisplayName(fields[i].name);
                ImGui::TableSetupColumn(displayName, ImGuiTableColumnFlags_None, fields[i].width);
            }
        }

        ImGui::TableHeadersRow();

        // Render rows
        ImGuiListClipper clipper;
        clipper.Begin((int)m_filteredSkills.size());

        // Handle scroll request
        if (s_scrollToIndex >= 0)
        {
            for (size_t i = 0; i < m_filteredSkills.size(); i++)
            {
                if (m_filteredSkills[i] == s_scrollToIndex)
                {
                    ImGui::SetScrollY(i * ImGui::GetTextLineHeightWithSpacing());
                    selectedRow = (int)i;
                    break;
                }
            }
            s_scrollToIndex = -1;
        }

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                int skillIndex = m_filteredSkills[row];
                SKILL_ATTRIBUTE& skill = SkillAttribute[skillIndex];

                ImGui::TableNextRow();
                ImGui::PushID(skillIndex);

                // Invisible row-spanning selectable for hover detection.
                // Save/restore cursor Y so the Selectable's height doesn't
                // push column content onto a second line, and use AllowOverlap
                // so column items still receive their own clicks.
                ImGui::TableSetColumnIndex(0);
                const float rowCursorY = ImGui::GetCursorPosY();
                ImGui::Selectable("##rowhover", false,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
                    ImVec2(0, ImGui::GetTextLineHeight()));
                const bool rowHovered = ImGui::IsItemHovered();
                ImGui::SetCursorPosY(rowCursorY);

                bool rowInteracted = false;
                int colIdx = 0;

                // Render Index column
                if (indexVisible)
                {
                    m_pColumns->RenderIndexColumn(colIdx, skillIndex, rowInteracted, true);
                }

                // Render all metadata fields
                for (int i = 0; i < fieldCount; ++i)
                {
                    bool isVisible = columnVisibility.find(fields[i].name) != columnVisibility.end() &&
                                     columnVisibility[fields[i].name];
                    m_pColumns->RenderFieldByDescriptor(fields[i], colIdx, skillIndex, skill, rowInteracted, isVisible);
                }

                if (rowHovered && showHoverTooltip)
                {
                    MuEditor::Skills::Tooltip::Render(skillIndex);
                }

                // Update selected row
                if (rowInteracted)
                {
                    selectedRow = row;
                }

                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }
}

#endif // _EDITOR
