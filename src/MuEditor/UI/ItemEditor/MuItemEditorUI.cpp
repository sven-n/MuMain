#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemEditorUI.h"
#include "ItemEditorTable.h"
#include "ItemEditorActions.h"
#include "ItemEditorPopups.h"
#include "MuEditor/Core/MuEditorCore.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <direct.h>

#include "imgui_internal.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"

CMuItemEditorUI::CMuItemEditorUI()
    : m_selectedRow(-1)
    , m_bFreezeColumns(false)
    , m_pTable(nullptr)
{
    memset(m_szItemSearchBuffer, 0, sizeof(m_szItemSearchBuffer));
    m_pTable = new CItemEditorTable();

    // Initialize column visibility - default to commonly used columns
    m_columnVisibility["Index"] = true;
    m_columnVisibility["Name"] = true;
    m_columnVisibility["TwoHand"] = false;
    m_columnVisibility["Level"] = true;
    m_columnVisibility["Slot"] = false;
    m_columnVisibility["Skill"] = false;
    m_columnVisibility["Width"] = false;
    m_columnVisibility["Height"] = false;
    m_columnVisibility["DamageMin"] = true;
    m_columnVisibility["DamageMax"] = true;
    m_columnVisibility["SuccessfulBlocking"] = false;
    m_columnVisibility["Defense"] = true;
    m_columnVisibility["MagicDefense"] = false;
    m_columnVisibility["WeaponSpeed"] = true;
    m_columnVisibility["WalkSpeed"] = false;
    m_columnVisibility["Durability"] = true;
    m_columnVisibility["MagicDur"] = false;
    m_columnVisibility["MagicPower"] = false;
    m_columnVisibility["ReqStr"] = true;
    m_columnVisibility["ReqDex"] = true;
    m_columnVisibility["ReqEne"] = true;
    m_columnVisibility["ReqVit"] = true;
    m_columnVisibility["ReqCha"] = true;
    m_columnVisibility["ReqLevel"] = false;
    m_columnVisibility["Value"] = false;
    m_columnVisibility["Zen"] = false;
    m_columnVisibility["AttType"] = false;

    // RequireClass array (7 classes) - these are the class restrictions
    m_columnVisibility["DW/SM"] = false;
    m_columnVisibility["DK/BK"] = false;
    m_columnVisibility["ELF/ME"] = false;
    m_columnVisibility["MG"] = false;
    m_columnVisibility["DL"] = false;
    m_columnVisibility["SUM"] = false;
    m_columnVisibility["RF"] = false;

    // Load column preferences from file
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

        ImGui::Text("Edit Item Attributes - Total Items: %d", MAX_ITEM);
        ImGui::SameLine();

        // Render action buttons (Save, Export S6E3, Export CSV)
        CItemEditorActions::RenderAllButtons();
        ImGui::Separator();

        RenderSearchBar();
        ImGui::SameLine();
        RenderColumnVisibilityMenu();
        ImGui::SameLine();
        ImGui::Checkbox("Freeze Index/Name", &m_bFreezeColumns);
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
    ImGui::Text("Search:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("##ItemSearch", m_szItemSearchBuffer, sizeof(m_szItemSearchBuffer));
}

void CMuItemEditorUI::RenderColumnVisibilityMenu()
{
    if (ImGui::Button("Columns"))
    {
        ImGui::OpenPopup("ColumnVisibility");
    }

    // Track if popup was open in previous frame
    static bool wasPopupOpen = false;
    bool isPopupOpen = ImGui::IsPopupOpen("ColumnVisibility");

    if (ImGui::BeginPopup("ColumnVisibility"))
    {
        ImGui::Text("Toggle Column Visibility:");
        ImGui::Separator();

        // Select All / Unselect All buttons
        if (ImGui::Button("Select All"))
        {
            for (auto& col : m_columnVisibility)
            {
                col.second = true;
            }
            SaveColumnPreferences();
        }
        ImGui::SameLine();
        if (ImGui::Button("Unselect All"))
        {
            for (auto& col : m_columnVisibility)
            {
                col.second = false;
            }
            SaveColumnPreferences();
        }
        ImGui::Separator();

        // Group related columns - track if any checkbox changed
        bool changed = false;

        ImGui::Text("Basic Info:");
        changed |= ImGui::Checkbox("Index", &m_columnVisibility["Index"]);
        changed |= ImGui::Checkbox("Name", &m_columnVisibility["Name"]);
        changed |= ImGui::Checkbox("TwoHand", &m_columnVisibility["TwoHand"]);
        changed |= ImGui::Checkbox("Level", &m_columnVisibility["Level"]);
        changed |= ImGui::Checkbox("Slot", &m_columnVisibility["Slot"]);
        changed |= ImGui::Checkbox("Skill", &m_columnVisibility["Skill"]);

        ImGui::Separator();
        ImGui::Text("Dimensions:");
        changed |= ImGui::Checkbox("Width", &m_columnVisibility["Width"]);
        changed |= ImGui::Checkbox("Height", &m_columnVisibility["Height"]);

        ImGui::Separator();
        ImGui::Text("Combat Stats:");
        changed |= ImGui::Checkbox("DamageMin", &m_columnVisibility["DamageMin"]);
        changed |= ImGui::Checkbox("DamageMax", &m_columnVisibility["DamageMax"]);
        changed |= ImGui::Checkbox("SuccessfulBlocking", &m_columnVisibility["SuccessfulBlocking"]);
        changed |= ImGui::Checkbox("Defense", &m_columnVisibility["Defense"]);
        changed |= ImGui::Checkbox("MagicDefense", &m_columnVisibility["MagicDefense"]);
        changed |= ImGui::Checkbox("WeaponSpeed", &m_columnVisibility["WeaponSpeed"]);
        changed |= ImGui::Checkbox("WalkSpeed", &m_columnVisibility["WalkSpeed"]);

        ImGui::Separator();
        ImGui::Text("Durability & Magic:");
        changed |= ImGui::Checkbox("Durability", &m_columnVisibility["Durability"]);
        changed |= ImGui::Checkbox("MagicDur", &m_columnVisibility["MagicDur"]);
        changed |= ImGui::Checkbox("MagicPower", &m_columnVisibility["MagicPower"]);

        ImGui::Separator();
        ImGui::Text("Requirements:");
        changed |= ImGui::Checkbox("ReqStr", &m_columnVisibility["ReqStr"]);
        changed |= ImGui::Checkbox("ReqDex", &m_columnVisibility["ReqDex"]);
        changed |= ImGui::Checkbox("ReqEne", &m_columnVisibility["ReqEne"]);
        changed |= ImGui::Checkbox("ReqVit", &m_columnVisibility["ReqVit"]);
        changed |= ImGui::Checkbox("ReqCha", &m_columnVisibility["ReqCha"]);
        changed |= ImGui::Checkbox("ReqLevel", &m_columnVisibility["ReqLevel"]);

        ImGui::Separator();
        ImGui::Text("Other:");
        changed |= ImGui::Checkbox("Value", &m_columnVisibility["Value"]);
        changed |= ImGui::Checkbox("Zen", &m_columnVisibility["Zen"]);
        changed |= ImGui::Checkbox("AttType", &m_columnVisibility["AttType"]);

        ImGui::Separator();
        ImGui::Text("Class Requirements:");
        changed |= ImGui::Checkbox("DW/SM", &m_columnVisibility["DW/SM"]);
        changed |= ImGui::Checkbox("DK/BK", &m_columnVisibility["DK/BK"]);
        changed |= ImGui::Checkbox("ELF/ME", &m_columnVisibility["ELF/ME"]);
        changed |= ImGui::Checkbox("MG", &m_columnVisibility["MG"]);
        changed |= ImGui::Checkbox("DL", &m_columnVisibility["DL"]);
        changed |= ImGui::Checkbox("SUM", &m_columnVisibility["SUM"]);
        changed |= ImGui::Checkbox("RF", &m_columnVisibility["RF"]);

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
    // Create MuEditor directory if it doesn't exist
    _mkdir("MuEditor");

    std::ofstream file(muitemeditor_columns_cfg);
    if (!file.is_open())
        return;

    for (const auto& col : m_columnVisibility)
    {
        file << col.first << "=" << (col.second ? "1" : "0") << "\n";
    }
    file.close();
}

void CMuItemEditorUI::LoadColumnPreferences()
{
    std::ifstream file(muitemeditor_columns_cfg);
    if (!file.is_open())
        return; // File doesn't exist yet, use defaults

    std::string line;
    while (std::getline(file, line))
    {
        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Only update if the column exists in our map
            if (m_columnVisibility.find(key) != m_columnVisibility.end())
            {
                m_columnVisibility[key] = (value == "1");
            }
        }
    }
    file.close();
}

#endif // _EDITOR
