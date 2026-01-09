#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemEditor.h"
#include "MuEditor/MuEditor.h"
#include "MuEditor/MuEditorConsole.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <direct.h>

#include "imgui_internal.h"

CMuItemEditor::CMuItemEditor()
    : m_selectedRow(-1)
    , m_bFreezeColumns(false)
{
    memset(m_szItemSearchBuffer, 0, sizeof(m_szItemSearchBuffer));

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

CMuItemEditor::~CMuItemEditor()
{
    // Save column preferences when destroying
    SaveColumnPreferences();
}

CMuItemEditor& CMuItemEditor::GetInstance()
{
    static CMuItemEditor instance;
    return instance;
}

void CMuItemEditor::Render(bool& showEditor)
{
    // Access external item data
    extern ITEM_ATTRIBUTE* ItemAttribute;
    if (!ItemAttribute)
        return;

    // Item editor is now rendered inside the center pane
    // No need to set position/size here - the center pane handles layout

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("Item Editor", &showEditor, flags))
    {
        // Check if hovering this window OR any popup
        bool isHovering = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup) ||
                         ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
                         ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);

        if (isHovering)
        {
            g_MuEditor.SetHoveringUI(true);
        }

        ImGui::Text("Edit Item Attributes - Total Items: %d", MAX_ITEM);
        ImGui::SameLine();

        RenderSaveButton();
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

        RenderItemTable(searchLower);
    }
    ImGui::End();
}

void CMuItemEditor::RenderSaveButton()
{
    // Save button on the right (accounting for three buttons)
    ImGui::SameLine(ImGui::GetWindowWidth() - 390);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.9f, 1.0f));
    if (ImGui::Button("Save Items"))
    {
        extern bool SaveItemScript(wchar_t* FileName, std::string* outChangeLog);
        extern std::wstring g_strSelectedML;

        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Item_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        std::string changeLog;
        if (SaveItemScript(fileName, &changeLog))
        {
            // Log to editor console
            g_MuEditorConsole.LogEditor("=== SAVE COMPLETED ===\n");
            g_MuEditorConsole.LogEditor(changeLog);
            g_MuEditorConsole.LogEditor("======================\n");
            ImGui::OpenPopup("Save Success");
        }
        else
        {
            g_MuEditorConsole.LogEditor("ERROR: Failed to save item attributes!\n");
            ImGui::OpenPopup("Save Failed");
        }
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine();

    // Save as S6E3 Legacy button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.4f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));
    if (ImGui::Button("Save as S6E3"))
    {
        extern bool SaveItemScriptLegacy(wchar_t* FileName);
        extern std::wstring g_strSelectedML;

        wchar_t fileName[256];
        swprintf(fileName, L"Data\\Local\\%ls\\Item_%ls_S6E3.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        if (SaveItemScriptLegacy(fileName))
        {
            g_MuEditorConsole.LogEditor("Saved items in S6E3 legacy format: Item_" + std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_S6E3.bmd\n");
            ImGui::OpenPopup("Save S6E3 Success");
        }
        else
        {
            g_MuEditorConsole.LogEditor("ERROR: Failed to save item attributes in S6E3 format!\n");
            ImGui::OpenPopup("Save S6E3 Failed");
        }
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine();

    // Export to CSV button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.7f, 1.0f));
    if (ImGui::Button("Export to CSV"))
    {
        extern bool ExportItemAttributeToCsv(wchar_t* FileName);
        extern std::wstring g_strSelectedML;

        wchar_t csvFileName[256];
        swprintf(csvFileName, L"Data\\Local\\%ls\\Item_%ls_export.csv", g_strSelectedML.c_str(), g_strSelectedML.c_str());

        if (ExportItemAttributeToCsv(csvFileName))
        {
            g_MuEditorConsole.LogEditor("Exported items to CSV: Item_" + std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_export.csv");
            ImGui::OpenPopup("Export Success");
        }
        else
        {
            g_MuEditorConsole.LogEditor("ERROR: Failed to export items to CSV!");
            ImGui::OpenPopup("Export Failed");
        }
    }
    ImGui::PopStyleColor(2);

    // Success popup
    if (ImGui::BeginPopupModal("Save Success", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Item attributes saved successfully!");
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Failed popup
    if (ImGui::BeginPopupModal("Save Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Failed to save item attributes!");
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Export Success popup
    if (ImGui::BeginPopupModal("Export Success", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Items exported to CSV successfully!");
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // S6E3 Success popup
    if (ImGui::BeginPopupModal("Save S6E3 Success", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Items saved in S6E3 legacy format successfully!");
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // S6E3 Failed popup
    if (ImGui::BeginPopupModal("Save S6E3 Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Failed to save items in S6E3 format!");
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Export Failed popup (if missing)
    if (ImGui::BeginPopupModal("Export Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Failed to export items to CSV!");
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void CMuItemEditor::RenderSearchBar()
{
    // Search bar
    ImGui::Text("Search:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("##ItemSearch", m_szItemSearchBuffer, sizeof(m_szItemSearchBuffer));
}

void CMuItemEditor::RenderItemTable(const std::string& searchLower)
{
    extern ITEM_ATTRIBUTE* ItemAttribute;

    // Count visible columns
    int visibleColumnCount = 0;
    for (const auto& col : m_columnVisibility)
    {
        if (col.second) visibleColumnCount++;
    }

    if (visibleColumnCount == 0)
    {
        ImGui::Text("No columns selected. Click 'Columns' to show columns.");
        return;
    }

    // Push style to reduce input field padding
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));

    // Create a table with scrolling
    if (ImGui::BeginTable("ItemTable", visibleColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable))
    {
        // Setup columns based on visibility
        // Freeze columns based on toggle state (Index and Name columns if enabled)
        if (m_bFreezeColumns && m_columnVisibility["Index"] && m_columnVisibility["Name"])
        {
            ImGui::TableSetupScrollFreeze(2, 1); // Freeze first 2 columns + header row
        }
        else if (m_bFreezeColumns && m_columnVisibility["Index"])
        {
            ImGui::TableSetupScrollFreeze(1, 1); // Freeze first column + header row
        }
        else
        {
            ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row only
        }

        if (m_columnVisibility["Index"]) ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        if (m_columnVisibility["Name"]) ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        if (m_columnVisibility["TwoHand"]) ImGui::TableSetupColumn("TwoHand", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["Level"]) ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["Slot"]) ImGui::TableSetupColumn("Slot", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["Skill"]) ImGui::TableSetupColumn("Skill", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["Width"]) ImGui::TableSetupColumn("Width", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["Height"]) ImGui::TableSetupColumn("Height", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["DamageMin"]) ImGui::TableSetupColumn("DmgMin", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["DamageMax"]) ImGui::TableSetupColumn("DmgMax", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["SuccessfulBlocking"]) ImGui::TableSetupColumn("Block", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["Defense"]) ImGui::TableSetupColumn("Defense", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["MagicDefense"]) ImGui::TableSetupColumn("MagDef", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["WeaponSpeed"]) ImGui::TableSetupColumn("WpnSpd", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["WalkSpeed"]) ImGui::TableSetupColumn("WalkSpd", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["Durability"]) ImGui::TableSetupColumn("Dur", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["MagicDur"]) ImGui::TableSetupColumn("MagDur", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["MagicPower"]) ImGui::TableSetupColumn("MagPow", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["ReqStr"]) ImGui::TableSetupColumn("ReqStr", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["ReqDex"]) ImGui::TableSetupColumn("ReqDex", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["ReqEne"]) ImGui::TableSetupColumn("ReqEne", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["ReqVit"]) ImGui::TableSetupColumn("ReqVit", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["ReqCha"]) ImGui::TableSetupColumn("ReqCha", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["ReqLevel"]) ImGui::TableSetupColumn("ReqLvl", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        if (m_columnVisibility["Value"]) ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["Zen"]) ImGui::TableSetupColumn("Zen", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        if (m_columnVisibility["AttType"]) ImGui::TableSetupColumn("AttType", ImGuiTableColumnFlags_WidthFixed, 70.0f);

        // RequireClass array columns (7 classes)
        if (m_columnVisibility["DW/SM"]) ImGui::TableSetupColumn("DW/SM", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["DK/BK"]) ImGui::TableSetupColumn("DK/BK", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        if (m_columnVisibility["ELF/ME"]) ImGui::TableSetupColumn("ELF/ME", ImGuiTableColumnFlags_WidthFixed, 65.0f);
        if (m_columnVisibility["MG"]) ImGui::TableSetupColumn("MG", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        if (m_columnVisibility["DL"]) ImGui::TableSetupColumn("DL", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        if (m_columnVisibility["SUM"]) ImGui::TableSetupColumn("SUM", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        if (m_columnVisibility["RF"]) ImGui::TableSetupColumn("RF", ImGuiTableColumnFlags_WidthFixed, 50.0f);

        ImGui::TableHeadersRow();

        // Render all items with filtering
        for (int i = 0; i < MAX_ITEM; i++)
        {
            // Get item name
            char nameBuffer[256];
            WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[i].Name, -1, nameBuffer, sizeof(nameBuffer), NULL, NULL);

            // Skip uninitialized items (no name)
            if (nameBuffer[0] == '\0')
                continue;

            // Apply search filter
            if (searchLower.length() > 0)
            {
                std::string nameLower = nameBuffer;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                if (nameLower.find(searchLower) == std::string::npos)
                    continue;
            }

            ImGui::TableNextRow();

            // Apply row highlighting if this row is selected
            if (m_selectedRow == i)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.3f, 0.5f, 0.8f, 0.3f)));
            }

            // Get the row bounds for click detection
            ImGui::TableSetColumnIndex(0);
            ImVec2 rowStartPos = ImGui::GetCursorScreenPos();
            float rowHeight = ImGui::GetTextLineHeightWithSpacing();

            int colIdx = 0;
            bool rowInteracted = false;

            // Index
            if (m_columnVisibility["Index"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 100);
                int newIndex = i;
                if (ImGui::InputInt("##index", &newIndex, 0, 0))
                {
                    // Check if new index is valid and not a duplicate
                    if (newIndex >= 0 && newIndex < MAX_ITEM && newIndex != i)
                    {
                        // Check if target index is already used (has a name)
                        char targetName[128];
                        WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[newIndex].Name, -1, targetName, sizeof(targetName), NULL, NULL);

                        if (targetName[0] == '\0')
                        {
                            // Target index is empty, safe to swap
                            ITEM_ATTRIBUTE temp = ItemAttribute[i];
                            ItemAttribute[i] = ItemAttribute[newIndex];
                            ItemAttribute[newIndex] = temp;
                            g_MuEditorConsole.LogEditor("Moved item from index " + std::to_string(i) + " to " + std::to_string(newIndex));
                        }
                        else
                        {
                            g_MuEditorConsole.LogEditor("Error: Index " + std::to_string(newIndex) + " already in use!");
                        }
                    }
                }
                if (ImGui::IsItemActivated()) rowInteracted = true;
                ImGui::PopID();
            }

            // Name
            if (m_columnVisibility["Name"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 101);
                char editableNameBuffer[128];
                strcpy_s(editableNameBuffer, sizeof(editableNameBuffer), nameBuffer);
                if (ImGui::InputText("##name", editableNameBuffer, sizeof(editableNameBuffer)))
                {
                    // Check if name already exists in another item
                    bool nameExists = false;
                    for (int j = 0; j < MAX_ITEM; j++)
                    {
                        if (j == i) continue;

                        char existingName[128];
                        WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[j].Name, -1, existingName, sizeof(existingName), NULL, NULL);

                        if (strcmp(editableNameBuffer, existingName) == 0 && existingName[0] != '\0')
                        {
                            nameExists = true;
                            break;
                        }
                    }

                    if (!nameExists)
                    {
                        // Convert to wide char and save
                        MultiByteToWideChar(CP_UTF8, 0, editableNameBuffer, -1, ItemAttribute[i].Name, sizeof(ItemAttribute[i].Name) / sizeof(WCHAR));
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Name to " + std::string(editableNameBuffer));
                    }
                    else
                    {
                        g_MuEditorConsole.LogEditor("Error: Name '" + std::string(editableNameBuffer) + "' already exists!");
                    }
                }
                if (ImGui::IsItemActivated()) rowInteracted = true;
                ImGui::PopID();
            }

            // TwoHand
            if (m_columnVisibility["TwoHand"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 0);
                bool twoHand = ItemAttribute[i].TwoHand;
                if (ImGui::Checkbox("##twohand", &twoHand))
                {
                    ItemAttribute[i].TwoHand = twoHand;
                    g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " TwoHand to " + std::to_string(twoHand));
                }
                // if (ImGui::IsItemClicked(0)) rowInteracted = true;
                ImGui::PopID();
            }

            // Level
            if (m_columnVisibility["Level"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 1);
                int level = ItemAttribute[i].Level;
                if (ImGui::InputInt("##lvl", &level, 0, 0))
                {
                    if (level >= 0 && level <= 65535)
                    {
                        ItemAttribute[i].Level = (WORD)level;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Level to " + std::to_string(level));
                    }
                }
                // Select row when activating this input field
                if (ImGui::IsItemActivated()) rowInteracted = true;
                ImGui::PopID();
            }

            // Slot
            if (m_columnVisibility["Slot"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 2);
                int slot = ItemAttribute[i].m_byItemSlot;
                if (ImGui::InputInt("##slot", &slot, 0, 0))
                {
                    if (slot >= 0 && slot <= 255)
                    {
                        ItemAttribute[i].m_byItemSlot = (BYTE)slot;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Slot to " + std::to_string(slot));
                    }
                }
                if (ImGui::IsItemActivated()) rowInteracted = true;
                ImGui::PopID();
            }

            // Skill
            if (m_columnVisibility["Skill"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 3);
                int skill = ItemAttribute[i].m_wSkillIndex;
                if (ImGui::InputInt("##skill", &skill, 0, 0))
                {
                    if (skill >= 0 && skill <= 65535)
                    {
                        ItemAttribute[i].m_wSkillIndex = (WORD)skill;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Skill to " + std::to_string(skill));
                    }
                }
                // if (ImGui::IsItemClicked(0)) rowInteracted = true;
                ImGui::PopID();
            }

            // Width
            if (m_columnVisibility["Width"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 4);
                int width = ItemAttribute[i].Width;
                if (ImGui::InputInt("##width", &width, 0, 0))
                {
                    if (width >= 0 && width <= 255)
                    {
                        ItemAttribute[i].Width = (BYTE)width;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Width to " + std::to_string(width));
                    }
                }
                // if (ImGui::IsItemClicked(0)) rowInteracted = true;
                ImGui::PopID();
            }

            // Height
            if (m_columnVisibility["Height"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 5);
                int height = ItemAttribute[i].Height;
                if (ImGui::InputInt("##height", &height, 0, 0))
                {
                    if (height >= 0 && height <= 255)
                    {
                        ItemAttribute[i].Height = (BYTE)height;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Height to " + std::to_string(height));
                    }
                }
                // if (ImGui::IsItemClicked(0)) rowInteracted = true;
                ImGui::PopID();
            }

            // DamageMin
            if (m_columnVisibility["DamageMin"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 6);
                int dmgMin = ItemAttribute[i].DamageMin;
                if (ImGui::InputInt("##dmgmin", &dmgMin, 0, 0))
                {
                    if (dmgMin >= 0 && dmgMin <= 255)
                    {
                        ItemAttribute[i].DamageMin = (BYTE)dmgMin;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " DamageMin to " + std::to_string(dmgMin));
                    }
                }
                // if (ImGui::IsItemClicked(0)) rowInteracted = true;
                ImGui::PopID();
            }

            // DamageMax
            if (m_columnVisibility["DamageMax"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 7);
                int dmgMax = ItemAttribute[i].DamageMax;
                if (ImGui::InputInt("##dmgmax", &dmgMax, 0, 0))
                {
                    if (dmgMax >= 0 && dmgMax <= 255)
                    {
                        ItemAttribute[i].DamageMax = (BYTE)dmgMax;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " DamageMax to " + std::to_string(dmgMax));
                    }
                }
                // if (ImGui::IsItemClicked(0)) rowInteracted = true;
                ImGui::PopID();
            }

            // SuccessfulBlocking
            if (m_columnVisibility["SuccessfulBlocking"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 8);
                int block = ItemAttribute[i].SuccessfulBlocking;
                if (ImGui::InputInt("##block", &block, 0, 0))
                {
                    if (block >= 0 && block <= 255)
                    {
                        ItemAttribute[i].SuccessfulBlocking = (BYTE)block;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " SuccessfulBlocking to " + std::to_string(block));
                    }
                }
                ImGui::PopID();
            }

            // Defense
            if (m_columnVisibility["Defense"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 9);
                int def = ItemAttribute[i].Defense;
                if (ImGui::InputInt("##def", &def, 0, 0))
                {
                    if (def >= 0 && def <= 255)
                    {
                        ItemAttribute[i].Defense = (BYTE)def;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Defense to " + std::to_string(def));
                    }
                }
                ImGui::PopID();
            }

            // MagicDefense
            if (m_columnVisibility["MagicDefense"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 10);
                int magDef = ItemAttribute[i].MagicDefense;
                if (ImGui::InputInt("##magdef", &magDef, 0, 0))
                {
                    if (magDef >= 0 && magDef <= 255)
                    {
                        ItemAttribute[i].MagicDefense = (BYTE)magDef;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " MagicDefense to " + std::to_string(magDef));
                    }
                }
                ImGui::PopID();
            }

            // WeaponSpeed
            if (m_columnVisibility["WeaponSpeed"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 11);
                int wpnSpd = ItemAttribute[i].WeaponSpeed;
                if (ImGui::InputInt("##wpnspd", &wpnSpd, 0, 0))
                {
                    if (wpnSpd >= 0 && wpnSpd <= 255)
                    {
                        ItemAttribute[i].WeaponSpeed = (BYTE)wpnSpd;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " WeaponSpeed to " + std::to_string(wpnSpd));
                    }
                }
                ImGui::PopID();
            }

            // WalkSpeed
            if (m_columnVisibility["WalkSpeed"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 12);
                int walkSpd = ItemAttribute[i].WalkSpeed;
                if (ImGui::InputInt("##walkspd", &walkSpd, 0, 0))
                {
                    if (walkSpd >= 0 && walkSpd <= 255)
                    {
                        ItemAttribute[i].WalkSpeed = (BYTE)walkSpd;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " WalkSpeed to " + std::to_string(walkSpd));
                    }
                }
                ImGui::PopID();
            }

            // Durability
            if (m_columnVisibility["Durability"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 13);
                int dur = ItemAttribute[i].Durability;
                if (ImGui::InputInt("##dur", &dur, 0, 0))
                {
                    if (dur >= 0 && dur <= 255)
                    {
                        ItemAttribute[i].Durability = (BYTE)dur;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Durability to " + std::to_string(dur));
                    }
                }
                ImGui::PopID();
            }

            // MagicDur
            if (m_columnVisibility["MagicDur"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 14);
                int magDur = ItemAttribute[i].MagicDur;
                if (ImGui::InputInt("##magdur", &magDur, 0, 0))
                {
                    if (magDur >= 0 && magDur <= 255)
                    {
                        ItemAttribute[i].MagicDur = (BYTE)magDur;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " MagicDur to " + std::to_string(magDur));
                    }
                }
                ImGui::PopID();
            }

            // MagicPower
            if (m_columnVisibility["MagicPower"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 15);
                int magPow = ItemAttribute[i].MagicPower;
                if (ImGui::InputInt("##magpow", &magPow, 0, 0))
                {
                    if (magPow >= 0 && magPow <= 255)
                    {
                        ItemAttribute[i].MagicPower = (BYTE)magPow;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " MagicPower to " + std::to_string(magPow));
                    }
                }
                ImGui::PopID();
            }

            // ReqStr
            if (m_columnVisibility["ReqStr"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 16);
                int reqStr = ItemAttribute[i].RequireStrength;
                if (ImGui::InputInt("##str", &reqStr, 0, 0))
                {
                    if (reqStr >= 0 && reqStr <= 65535)
                    {
                        ItemAttribute[i].RequireStrength = (WORD)reqStr;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireStrength to " + std::to_string(reqStr));
                    }
                }
                ImGui::PopID();
            }

            // ReqDex
            if (m_columnVisibility["ReqDex"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 17);
                int reqDex = ItemAttribute[i].RequireDexterity;
                if (ImGui::InputInt("##dex", &reqDex, 0, 0))
                {
                    if (reqDex >= 0 && reqDex <= 65535)
                    {
                        ItemAttribute[i].RequireDexterity = (WORD)reqDex;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireDexterity to " + std::to_string(reqDex));
                    }
                }
                ImGui::PopID();
            }

            // ReqEne
            if (m_columnVisibility["ReqEne"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 18);
                int reqEne = ItemAttribute[i].RequireEnergy;
                if (ImGui::InputInt("##ene", &reqEne, 0, 0))
                {
                    if (reqEne >= 0 && reqEne <= 65535)
                    {
                        ItemAttribute[i].RequireEnergy = (WORD)reqEne;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireEnergy to " + std::to_string(reqEne));
                    }
                }
                ImGui::PopID();
            }

            // ReqVit
            if (m_columnVisibility["ReqVit"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 19);
                int reqVit = ItemAttribute[i].RequireVitality;
                if (ImGui::InputInt("##vit", &reqVit, 0, 0))
                {
                    if (reqVit >= 0 && reqVit <= 65535)
                    {
                        ItemAttribute[i].RequireVitality = (WORD)reqVit;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireVitality to " + std::to_string(reqVit));
                    }
                }
                ImGui::PopID();
            }

            // ReqCha
            if (m_columnVisibility["ReqCha"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 20);
                int reqCha = ItemAttribute[i].RequireCharisma;
                if (ImGui::InputInt("##cha", &reqCha, 0, 0))
                {
                    if (reqCha >= 0 && reqCha <= 65535)
                    {
                        ItemAttribute[i].RequireCharisma = (WORD)reqCha;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireCharisma to " + std::to_string(reqCha));
                    }
                }
                ImGui::PopID();
            }

            // ReqLevel
            if (m_columnVisibility["ReqLevel"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 21);
                int reqLvl = ItemAttribute[i].RequireLevel;
                if (ImGui::InputInt("##reqlvl", &reqLvl, 0, 0))
                {
                    if (reqLvl >= 0 && reqLvl <= 65535)
                    {
                        ItemAttribute[i].RequireLevel = (WORD)reqLvl;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireLevel to " + std::to_string(reqLvl));
                    }
                }
                ImGui::PopID();
            }

            // Value
            if (m_columnVisibility["Value"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 22);
                int value = ItemAttribute[i].Value;
                if (ImGui::InputInt("##value", &value, 0, 0))
                {
                    if (value >= 0 && value <= 255)
                    {
                        ItemAttribute[i].Value = (BYTE)value;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Value to " + std::to_string(value));
                    }
                }
                ImGui::PopID();
            }

            // Zen
            if (m_columnVisibility["Zen"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 23);
                int zen = ItemAttribute[i].iZen;
                if (ImGui::InputInt("##zen", &zen, 0, 0))
                {
                    ItemAttribute[i].iZen = zen;
                    g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " Zen to " + std::to_string(zen));
                }
                ImGui::PopID();
            }

            // AttType
            if (m_columnVisibility["AttType"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 100 + 24);
                int attType = ItemAttribute[i].AttType;
                if (ImGui::InputInt("##atttype", &attType, 0, 0))
                {
                    if (attType >= 0 && attType <= 255)
                    {
                        ItemAttribute[i].AttType = (BYTE)attType;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " AttType to " + std::to_string(attType));
                    }
                }
                if (ImGui::IsItemActivated()) rowInteracted = true;
                ImGui::PopID();
            }

            // RequireClass array - DW/SM (index 0)
            if (m_columnVisibility["DW/SM"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 1000 + 25);
                int reqClass = ItemAttribute[i].RequireClass[0];
                if (ImGui::InputInt("##dwsm", &reqClass, 0, 0))
                {
                    if (reqClass >= 0 && reqClass <= 255)
                    {
                        ItemAttribute[i].RequireClass[0] = (BYTE)reqClass;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireClass[DW/SM] to " + std::to_string(reqClass));
                    }
                }
                ImGui::PopID();
            }

            // RequireClass array - DK/BK (index 1)
            if (m_columnVisibility["DK/BK"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 1000 + 26);
                int reqClass = ItemAttribute[i].RequireClass[1];
                if (ImGui::InputInt("##dkbk", &reqClass, 0, 0))
                {
                    if (reqClass >= 0 && reqClass <= 255)
                    {
                        ItemAttribute[i].RequireClass[1] = (BYTE)reqClass;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireClass[DK/BK] to " + std::to_string(reqClass));
                    }
                }
                ImGui::PopID();
            }

            // RequireClass array - ELF/ME (index 2)
            if (m_columnVisibility["ELF/ME"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 1000 + 27);
                int reqClass = ItemAttribute[i].RequireClass[2];
                if (ImGui::InputInt("##elfme", &reqClass, 0, 0))
                {
                    if (reqClass >= 0 && reqClass <= 255)
                    {
                        ItemAttribute[i].RequireClass[2] = (BYTE)reqClass;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireClass[ELF/ME] to " + std::to_string(reqClass));
                    }
                }
                ImGui::PopID();
            }

            // RequireClass array - MG (index 3)
            if (m_columnVisibility["MG"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 1000 + 28);
                int reqClass = ItemAttribute[i].RequireClass[3];
                if (ImGui::InputInt("##mg", &reqClass, 0, 0))
                {
                    if (reqClass >= 0 && reqClass <= 255)
                    {
                        ItemAttribute[i].RequireClass[3] = (BYTE)reqClass;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireClass[MG] to " + std::to_string(reqClass));
                    }
                }
                ImGui::PopID();
            }

            // RequireClass array - DL (index 4)
            if (m_columnVisibility["DL"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 1000 + 29);
                int reqClass = ItemAttribute[i].RequireClass[4];
                if (ImGui::InputInt("##dl", &reqClass, 0, 0))
                {
                    if (reqClass >= 0 && reqClass <= 255)
                    {
                        ItemAttribute[i].RequireClass[4] = (BYTE)reqClass;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireClass[DL] to " + std::to_string(reqClass));
                    }
                }
                ImGui::PopID();
            }

            // RequireClass array - SUM (index 5)
            if (m_columnVisibility["SUM"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 1000 + 30);
                int reqClass = ItemAttribute[i].RequireClass[5];
                if (ImGui::InputInt("##sum", &reqClass, 0, 0))
                {
                    if (reqClass >= 0 && reqClass <= 255)
                    {
                        ItemAttribute[i].RequireClass[5] = (BYTE)reqClass;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireClass[SUM] to " + std::to_string(reqClass));
                    }
                }
                ImGui::PopID();
            }

            // RequireClass array - RF (index 6)
            if (m_columnVisibility["RF"])
            {
                ImGui::TableSetColumnIndex(colIdx++);
                ImGui::PushID(i * 1000 + 31);
                int reqClass = ItemAttribute[i].RequireClass[6];
                if (ImGui::InputInt("##rf", &reqClass, 0, 0))
                {
                    if (reqClass >= 0 && reqClass <= 255)
                    {
                        ItemAttribute[i].RequireClass[6] = (BYTE)reqClass;
                        g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " RequireClass[RF] to " + std::to_string(reqClass));
                    }
                }
                ImGui::PopID();
            }

            // Detect clicks on empty row space
            if (!rowInteracted && ImGui::IsMouseClicked(0))
            {
                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 rowEndPos = ImGui::GetCursorScreenPos();

                // Check if mouse is within this row's vertical bounds
                if (mousePos.y >= rowStartPos.y && mousePos.y <= rowStartPos.y + rowHeight)
                {
                    rowInteracted = true;
                }
            }

            // Update selected row if any interaction occurred
            if (rowInteracted)
            {
                m_selectedRow = i;
            }
        }

        ImGui::EndTable();
    }

    // Pop the style variables
    ImGui::PopStyleVar(2);
}

void CMuItemEditor::RenderColumnVisibilityMenu()
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

void CMuItemEditor::SaveColumnPreferences()
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

void CMuItemEditor::LoadColumnPreferences()
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
