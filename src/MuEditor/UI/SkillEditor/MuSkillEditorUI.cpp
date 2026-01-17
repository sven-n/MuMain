#include "stdafx.h"

#ifdef _EDITOR

#include "MuSkillEditorUI.h"
#include "SkillEditorTable.h"
#include "SkillEditorActions.h"
#include "SkillEditorPopups.h"
#include "GameData/SkillData/SkillFieldMetadata.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/Config/MuEditorConfig.h"
#include "Translation/i18n.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>

#include "imgui_internal.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"

CMuSkillEditorUI::CMuSkillEditorUI()
    : m_selectedRow(-1)
    , m_bFreezeColumns(false)
    , m_pTable(nullptr)
{
    memset(m_szSkillSearchBuffer, 0, sizeof(m_szSkillSearchBuffer));
    m_pTable = new CSkillEditorTable();

    // Initialize column visibility - AUTO-GENERATED from metadata
    m_columnVisibility["Index"] = true;  // Special column (not in metadata)

    // Get all fields from metadata and set default visibility
    const SkillFieldDescriptor* fields = GetSkillFieldDescriptors();
    const int fieldCount = GetSkillFieldCount();
    for (int i = 0; i < fieldCount; ++i)
    {
        // Default commonly used columns to visible, rest to hidden
        bool defaultVisible = false;

        // Check for commonly used fields
        if (strcmp(fields[i].name, "Name") == 0 ||
            strcmp(fields[i].name, "Level") == 0 ||
            strcmp(fields[i].name, "Damage") == 0 ||
            strcmp(fields[i].name, "Mana") == 0 ||
            strcmp(fields[i].name, "Distance") == 0 ||
            strcmp(fields[i].name, "Delay") == 0 ||
            strcmp(fields[i].name, "Energy") == 0 ||
            strcmp(fields[i].name, "Strength") == 0 ||
            strcmp(fields[i].name, "Dexterity") == 0)
        {
            defaultVisible = true;
        }

        m_columnVisibility[fields[i].name] = defaultVisible;
    }

    // Load column preferences from config (will override defaults if exists)
    LoadColumnPreferences();
}

CMuSkillEditorUI::~CMuSkillEditorUI()
{
    // Save column preferences when destroying
    SaveColumnPreferences();

    if (m_pTable)
    {
        delete m_pTable;
        m_pTable = nullptr;
    }
}

CMuSkillEditorUI& CMuSkillEditorUI::GetInstance()
{
    static CMuSkillEditorUI instance;
    return instance;
}

void CMuSkillEditorUI::Render(bool& showEditor)
{
    // Access external skill data
    extern SKILL_ATTRIBUTE* SkillAttribute;
    if (!SkillAttribute)
        return;

    // Constants for layout
    constexpr float TOOLBAR_HEIGHT = 40.0f;
    constexpr float CONSOLE_HEIGHT = 200.0f;

    ImGuiIO& io = ImGui::GetIO();

    // Calculate available space between toolbar and console
    float availableTop = TOOLBAR_HEIGHT;
    float availableBottom = io.DisplaySize.y - CONSOLE_HEIGHT;
    float availableHeight = availableBottom - availableTop;

    // Set initial window size and constraints
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(400, 300),  // Min size
        ImVec2(io.DisplaySize.x, availableHeight)  // Max size
    );

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin(EDITOR_TEXT("label_skill_editor_title"), &showEditor, flags))
    {
        // Clamp window position to stay within bounds
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Clamp to keep window between toolbar and console
        if (windowPos.y < availableTop)
        {
            ImGui::SetWindowPos(ImVec2(windowPos.x, availableTop));
        }
        if (windowPos.y + windowSize.y > availableBottom)
        {
            ImGui::SetWindowPos(ImVec2(windowPos.x, availableBottom - windowSize.y));
        }

        // Clamp horizontally to stay within screen bounds
        if (windowPos.x < 0)
        {
            ImGui::SetWindowPos(ImVec2(0, windowPos.y));
        }
        if (windowPos.x + windowSize.x > io.DisplaySize.x)
        {
            ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - windowSize.x, windowPos.y));
        }

        // Check if hovering this window OR any popup
        bool isHovering = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup) ||
                         ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
                         ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);

        if (isHovering)
        {
            g_MuEditorCore.SetHoveringUI(true);
        }

        // Render action buttons (Save, Export CSV)
        CSkillEditorActions::RenderAllButtons();
        ImGui::Separator();

        RenderSearchBar();
        ImGui::SameLine();
        RenderColumnVisibilityMenu();

        // Freeze columns checkbox
        ImGui::SameLine();
        ImGui::Checkbox(EDITOR_TEXT("label_freeze_columns"), &m_bFreezeColumns);

        ImGui::Separator();

        // Render table with current search filter
        std::string searchFilter = m_szSkillSearchBuffer;
        std::transform(searchFilter.begin(), searchFilter.end(), searchFilter.begin(), ::tolower);

        m_pTable->Render(searchFilter, m_columnVisibility, m_selectedRow, m_bFreezeColumns);

        // Render popups
        CSkillEditorPopups::RenderPopups();
    }
    ImGui::End();
}

void CMuSkillEditorUI::RenderSearchBar()
{
    ImGui::SetNextItemWidth(200);
    ImGui::InputTextWithHint("##SkillSearch", EDITOR_TEXT("label_search_skills"), m_szSkillSearchBuffer, sizeof(m_szSkillSearchBuffer));
}

void CMuSkillEditorUI::RenderColumnVisibilityMenu()
{
    if (ImGui::Button(EDITOR_TEXT("btn_columns")))
    {
        ImGui::OpenPopup("ColumnVisibility");
    }

    if (ImGui::BeginPopup("ColumnVisibility"))
    {
        ImGui::Text("%s", EDITOR_TEXT("popup_toggle_columns"));
        ImGui::Separator();

        // Select All / Unselect All buttons
        if (ImGui::Button(EDITOR_TEXT("btn_select_all"), ImVec2(120, 0)))
        {
            for (auto& pair : m_columnVisibility)
            {
                pair.second = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(EDITOR_TEXT("btn_unselect_all"), ImVec2(120, 0)))
        {
            for (auto& pair : m_columnVisibility)
            {
                pair.second = false;
            }
        }

        ImGui::Separator();

        // Index column (special)
        ImGui::Checkbox(EDITOR_TEXT("label_index"), &m_columnVisibility["Index"]);

        // All metadata fields
        const SkillFieldDescriptor* fields = GetSkillFieldDescriptors();
        const int fieldCount = GetSkillFieldCount();

        for (int i = 0; i < fieldCount; ++i)
        {
            const char* displayName = GetSkillFieldDisplayName(fields[i].name);
            ImGui::Checkbox(displayName, &m_columnVisibility[fields[i].name]);
        }

        ImGui::EndPopup();
    }
}

void CMuSkillEditorUI::SaveColumnPreferences()
{
    g_MuEditorConfig.SetSkillEditorColumnVisibility(m_columnVisibility);
    g_MuEditorConfig.Save();
}

void CMuSkillEditorUI::LoadColumnPreferences()
{
    const auto& savedPrefs = g_MuEditorConfig.GetSkillEditorColumnVisibility();
    if (!savedPrefs.empty())
    {
        // Merge saved preferences with current defaults
        for (const auto& pair : savedPrefs)
        {
            m_columnVisibility[pair.first] = pair.second;
        }
    }
}

#endif // _EDITOR
