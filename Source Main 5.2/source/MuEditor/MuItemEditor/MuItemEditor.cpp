#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemEditor.h"
#include "MuEditor/MuEditor.h"
#include "MuEditor/MuEditorConsole.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>

CMuItemEditor::CMuItemEditor()
{
    memset(m_szItemSearchBuffer, 0, sizeof(m_szItemSearchBuffer));
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
        // ALWAYS check if hovering, with multiple fallbacks
        bool isHovering = false;

        // Method 1: Standard hover check
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
        {
            isHovering = true;
        }

        // Method 2: Check with AllowWhenBlockedByActiveItem flag
        if (!isHovering && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            isHovering = true;
        }

        // Method 3: Check if any ImGui window is hovered and it's THIS one
        if (!isHovering && ImGui::IsAnyItemHovered())
        {
            isHovering = true;
        }

        if (isHovering)
        {
            g_MuEditor.SetHoveringUI(true);
        }

        ImGui::Text("Edit Item Attributes - Total Items: %d", MAX_ITEM);
        ImGui::SameLine();

        RenderSaveButton();
        ImGui::Separator();

        RenderSearchBar();
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
    // Save button on the right
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
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

    // Create a table with scrolling
    if (ImGui::BeginTable("ItemTable", 12, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable))
    {
        // Setup columns
        ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row
        ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Req Str", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Req Dex", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Req Ene", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Req Vit", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Req Cha", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Damage", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Atk Spd", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Defense", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Durability", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        // Render all items with filtering
        for (int i = 0; i < MAX_ITEM; i++)
        {
            // Get item name
            char nameBuffer[128];
            WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[i].Name, -1, nameBuffer, sizeof(nameBuffer), NULL, NULL);

            // Skip uninitialized items (no name)
            if (nameBuffer[0] == '\0')
            {
                continue;
            }

            // Apply search filter (only if search is not empty)
            if (searchLower.length() > 0)
            {
                std::string nameLower = nameBuffer;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

                // Skip if search string not found in name
                if (nameLower.find(searchLower) == std::string::npos)
                {
                    continue;
                }
            }

            ImGui::TableNextRow();

            // Index
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", i);

            // Name
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", nameBuffer);

            // Level
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", ItemAttribute[i].Level);

            // Required Strength (EDITABLE)
            ImGui::TableSetColumnIndex(3);
            ImGui::PushID(i * 10 + 0);
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

            // Required Dexterity (EDITABLE)
            ImGui::TableSetColumnIndex(4);
            ImGui::PushID(i * 10 + 1);
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

            // Required Energy (EDITABLE)
            ImGui::TableSetColumnIndex(5);
            ImGui::PushID(i * 10 + 2);
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

            // Required Vitality (EDITABLE)
            ImGui::TableSetColumnIndex(6);
            ImGui::PushID(i * 10 + 3);
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

            // Required Charisma (EDITABLE)
            ImGui::TableSetColumnIndex(7);
            ImGui::PushID(i * 10 + 9);
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

            // Damage Min/Max (EDITABLE)
            ImGui::TableSetColumnIndex(8);
            ImGui::PushID(i * 10 + 4);
            int dmgMin = ItemAttribute[i].DamageMin;
            ImGui::SetNextItemWidth(40);
            if (ImGui::InputInt("##dmgmin", &dmgMin, 0, 0))
            {
                if (dmgMin >= 0 && dmgMin <= 255)
                {
                    ItemAttribute[i].DamageMin = (BYTE)dmgMin;
                    g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " DamageMin to " + std::to_string(dmgMin));
                }
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("-");
            ImGui::SameLine();
            ImGui::PushID(i * 10 + 5);
            int dmgMax = ItemAttribute[i].DamageMax;
            ImGui::SetNextItemWidth(40);
            if (ImGui::InputInt("##dmgmax", &dmgMax, 0, 0))
            {
                if (dmgMax >= 0 && dmgMax <= 255)
                {
                    ItemAttribute[i].DamageMax = (BYTE)dmgMax;
                    g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " DamageMax to " + std::to_string(dmgMax));
                }
            }
            ImGui::PopID();

            // Attack Speed (EDITABLE)
            ImGui::TableSetColumnIndex(9);
            ImGui::PushID(i * 10 + 6);
            int atkSpd = ItemAttribute[i].WeaponSpeed;
            if (ImGui::InputInt("##atkspd", &atkSpd, 0, 0))
            {
                if (atkSpd >= 0 && atkSpd <= 255)
                {
                    ItemAttribute[i].WeaponSpeed = (BYTE)atkSpd;
                    g_MuEditorConsole.LogEditor("Changed item " + std::to_string(i) + " WeaponSpeed to " + std::to_string(atkSpd));
                }
            }
            ImGui::PopID();

            // Defense (EDITABLE)
            ImGui::TableSetColumnIndex(10);
            ImGui::PushID(i * 10 + 7);
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

            // Durability (EDITABLE)
            ImGui::TableSetColumnIndex(11);
            ImGui::PushID(i * 10 + 8);
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

        ImGui::EndTable();
    }
}

#endif // _EDITOR
