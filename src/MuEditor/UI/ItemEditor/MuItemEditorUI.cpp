#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemEditorUI.h"
#include "ItemEditorTable.h"
#include "ItemEditorActions.h"
#include "ItemEditorPopups.h"
#include "GameData/ItemData/ItemFieldMetadata.h"
#include "../MuEditor/Config/MuEditorConfig.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "Translation/i18n.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <direct.h>
#include <filesystem>

#include "imgui_internal.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"

CMuItemEditorUI::CMuItemEditorUI()
    : m_selectedRow(-1)
    , m_bFreezeColumns(false)
    , m_pTable(nullptr)
{
    memset(m_szItemSearchBuffer, 0, sizeof(m_szItemSearchBuffer));
    m_pTable = new CItemEditorTable();

    // Initialize column visibility - AUTO-GENERATED from metadata
    m_columnVisibility["Index"] = true;  // Special column (not in metadata)

    // Get all fields from metadata and set default visibility
    const FieldDescriptor* fields = GetFieldDescriptors(); const int fieldCount = GetFieldCount();
    for (int i = 0; i < fieldCount; ++i)
    {
        // Default commonly used columns to visible, rest to hidden
        bool defaultVisible = false;

        // Check for commonly used fields
        if (strcmp(fields[i].name, "Name") == 0 ||
            strcmp(fields[i].name, "Level") == 0 ||
            strcmp(fields[i].name, "DamageMin") == 0 ||
            strcmp(fields[i].name, "DamageMax") == 0 ||
            strcmp(fields[i].name, "Defense") == 0 ||
            strcmp(fields[i].name, "WeaponSpeed") == 0 ||
            strcmp(fields[i].name, "Durability") == 0 ||
            strcmp(fields[i].name, "RequireStrength") == 0 ||
            strcmp(fields[i].name, "RequireDexterity") == 0 ||
            strcmp(fields[i].name, "RequireEnergy") == 0 ||
            strcmp(fields[i].name, "RequireVitality") == 0 ||
            strcmp(fields[i].name, "RequireCharisma") == 0)
        {
            defaultVisible = true;
        }

        m_columnVisibility[fields[i].name] = defaultVisible;
    }

    // Load column preferences from file (will override defaults if file exists)
    LoadColumnPreferences();
}

CMuItemEditorUI::~CMuItemEditorUI()
{
    // Save column preferences when destroying
    SaveColumnPreferences();

    if (m_pTable)
    {
        delete m_pTable;
        m_pTable = nullptr;
    }
}

CMuItemEditorUI& CMuItemEditorUI::GetInstance()
{
    static CMuItemEditorUI instance;
    return instance;
}

void CMuItemEditorUI::Render(bool& showEditor)
{
    // Access external item data
    extern ITEM_ATTRIBUTE* ItemAttribute;
    if (!ItemAttribute)
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
    if (ImGui::Begin("Item Editor", &showEditor, flags))
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

        // Render action buttons (Save, Export S6E3, Export CSV)
        CItemEditorActions::RenderAllButtons();;
        ImGui::Separator();
        
        RenderSearchBar();
        ImGui::SameLine();
        RenderColumnVisibilityMenu();
        ImGui::SameLine();
        ImGui::Checkbox(EDITOR_TEXT("label_freeze_columns"), &m_bFreezeColumns);
        ImGui::Separator();

        // Convert search to lowercase for case-insensitive search
        std::string searchLower = m_szItemSearchBuffer;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        // Render the item table
        if (m_pTable)
        {
            m_pTable->Render(searchLower, m_columnVisibility, m_selectedRow, m_bFreezeColumns);
        }

        // Render all popups
        CItemEditorPopups::RenderAll();
    }
    ImGui::End();
}

void CMuItemEditorUI::RenderSearchBar()
{
    // Search bar
    ImGui::Text(EDITOR_TEXT("label_search_text"));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("##ItemSearch", m_szItemSearchBuffer, sizeof(m_szItemSearchBuffer));
}

void CMuItemEditorUI::RenderColumnVisibilityMenu()
{
    if (ImGui::Button(EDITOR_TEXT("btn_columns")))
    {
        ImGui::OpenPopup("ColumnVisibility");
    }

    // Track if popup was open in previous frame
    static bool wasPopupOpen = false;
    bool isPopupOpen = ImGui::IsPopupOpen("ColumnVisibility");

    if (ImGui::BeginPopup("ColumnVisibility"))
    {
        ImGui::Text(EDITOR_TEXT("popup_toggle_columns"));
        ImGui::Separator();

        // Select All / Unselect All buttons
        if (ImGui::Button(EDITOR_TEXT("btn_select_all")))
        {
            for (auto& col : m_columnVisibility)
            {
                col.second = true;
            }
            SaveColumnPreferences();
        }
        ImGui::SameLine();
        if (ImGui::Button(EDITOR_TEXT("btn_unselect_all")))
        {
            for (auto& col : m_columnVisibility)
            {
                col.second = false;
            }
            SaveColumnPreferences();
        }
        ImGui::Separator();

        // METADATA-DRIVEN: Render all columns using metadata with translations
        bool changed = false;

        // Index column (special case, not in metadata)
        changed |= ImGui::Checkbox(EDITOR_TEXT("label_index"), &m_columnVisibility["Index"]);

        // Get all fields from metadata and render checkboxes
        const FieldDescriptor* fields = GetFieldDescriptors(); const int fieldCount = GetFieldCount();
        for (int i = 0; i < fieldCount; ++i)
        {
            const char* displayName = GetFieldDisplayName(fields[i].name);
            changed |= ImGui::Checkbox(displayName, &m_columnVisibility[fields[i].name]);
        }

        // Save immediately when any checkbox changes
        if (changed)
        {
            SaveColumnPreferences();
        }

        ImGui::EndPopup();
        wasPopupOpen = true;
    }
    else if (wasPopupOpen)
    {
        // Popup just closed - save one final time
        SaveColumnPreferences();
        wasPopupOpen = false;
    }
}

void CMuItemEditorUI::SaveColumnPreferences()
{
    // Save to unified config system
    g_MuEditorConfig.SetAllColumnVisibility(m_columnVisibility);
    g_MuEditorConfig.Save();
}

void CMuItemEditorUI::LoadColumnPreferences()
{
    // Load from unified config system
    const auto& savedVisibility = g_MuEditorConfig.GetAllColumnVisibility();

    // Only update columns that exist in saved config
    for (const auto& col : savedVisibility)
    {
        // Only update if the column exists in our default map
        if (m_columnVisibility.find(col.first) != m_columnVisibility.end())
        {
            m_columnVisibility[col.first] = col.second;
        }
    }
}

#endif // _EDITOR
