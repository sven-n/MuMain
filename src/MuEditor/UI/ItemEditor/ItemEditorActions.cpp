#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorActions.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "I18N/All.h"
#include "imgui.h"
#include <string>
#include <sstream>
#include "../Console/MuEditorConsoleUI.h"

extern std::wstring g_strSelectedML;

// ===== BUTTON RENDERING =====

void CItemEditorActions::RenderSaveButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.9f, 1.0f));

    if (ImGui::Button(I18N::Editor::SaveItems))
    {
        std::vector<Data::Items::ItemDataIssue> issues;
        const Data::Items::ItemDataSaveResult result = g_ItemDataHandler.Save(issues);
        LogIssues(issues);
        switch (result)
        {
        case Data::Items::ItemDataSaveResult::Saved:
            g_MuEditorConsoleUI.LogEditor("Items saved to " + Data::Items::GetItemDataDirectory().string());
            g_ItemEdits.GetHistory().MarkSaved();
            LogMissingItemNames();
            ImGui::OpenPopup("Save Success");
            break;
        case Data::Items::ItemDataSaveResult::InvalidData:
            g_MuEditorConsoleUI.LogEditor(I18N::Editor::ItemDataHasErrors);
            ImGui::OpenPopup("Save Failed");
            break;
        case Data::Items::ItemDataSaveResult::WriteFailed:
            g_MuEditorConsoleUI.LogEditor(I18N::Editor::ItemDataCouldNotBeWritten);
            ImGui::OpenPopup("Save Write Failed");
            break;
        }
    }

    ImGui::PopStyleColor(2);
}

bool CItemEditorActions::RenderImportBmdButton()
{
    if (!ImGui::Button(I18N::Editor::ImportFromBmd))
    {
        return false;
    }

    const Data::Items::ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    LogIssues(result.issues);
    if (Data::Items::HasErrors(result.issues))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::FailedToImportItemsFromBmd);
        ImGui::OpenPopup("Import Bmd Failed");
        return false;
    }

    LogImportResult(result);
    // The imported items replace all items: no step before them can be undone.
    g_ItemEdits.GetHistory().Clear();
    ImGui::OpenPopup("Import Bmd Success");
    return true;
}

void CItemEditorActions::RenderExportBmdButton()
{
    if (!ImGui::Button(I18N::Editor::ExportAsBmd))
    {
        return;
    }

    std::string changeLog;
    const bool exported = g_ItemDataHandler.ExportAsBmd(changeLog);
    g_MuEditorConsoleUI.LogEditor(changeLog);
    if (exported)
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::ItemsExportedAsBmd);
        ImGui::OpenPopup("Export Bmd Success");
    }
    else
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::FailedToExportItemsAsBmd);
        ImGui::OpenPopup("Export Bmd Failed");
    }
}

void CItemEditorActions::LogMissingItemNames()
{
    // English never misses a name: an item without one has errors.
    for (const Data::Items::ItemNameCount& count :
         Data::Items::CountItemNames(g_ItemDatabase.GetAllSlots(), I18N::GetAvailableLocales()))
    {
        if (count.missing == 0)
        {
            continue;
        }
        g_MuEditorConsoleUI.LogEditor(I18N::Format(I18N::Editor::ItemNamesMissing,
                                                   {I18N::GetLanguageDisplayName(count.locale.c_str()),
                                                    std::to_string(count.missing),
                                                    std::to_string(count.named + count.missing)}));
    }
}

void CItemEditorActions::LogIssues(const std::vector<Data::Items::ItemDataIssue>& issues)
{
    for (const Data::Items::ItemDataIssue& issue : issues)
    {
        g_MuEditorConsoleUI.LogEditor(issue.ToString());
    }
}

void CItemEditorActions::LogImportResult(const Data::Items::ItemBmdImportResult& result)
{
    std::string locales;
    for (const std::string& locale : result.importedLocales)
    {
        locales += (locales.empty() ? "" : ", ") + locale;
    }

    g_MuEditorConsoleUI.LogEditor("Imported " + std::to_string(result.items.size()) + " items from the bmd files (" +
                                  locales + ")");
    g_MuEditorConsoleUI.LogEditor(std::to_string(result.recoveredNameCount) +
                                  " names were longer than the bmd name field and were recovered");
    g_MuEditorConsoleUI.LogEditor(std::to_string(result.reencodedNameCount) +
                                  " names were read as Windows-1252 instead of UTF-8");

    for (const Data::Items::ItemBmdRepair& repair : result.repairs)
    {
        const std::string source = repair.fromLocale.empty() ? "no language had it, set to the default" : "taken from " + repair.fromLocale;
        g_MuEditorConsoleUI.LogEditor("Repaired item (" + std::to_string(repair.group) + "," +
                                      std::to_string(repair.number) + ") " + repair.field + ": " +
                                      std::to_string(repair.oldValue) + " -> " + std::to_string(repair.newValue) +
                                      " (" + source + ")");
    }

    g_MuEditorConsoleUI.LogEditor(std::to_string(result.keptEnglishNameCount) +
                                  " items the English file lacks kept their current English name");
    LogIssues(result.validationIssues);
    if (Data::Items::HasErrors(result.validationIssues))
    {
        g_MuEditorConsoleUI.LogEditor("Fix the errors above before saving; Save refuses data with errors.");
    }
}

void CItemEditorActions::RenderExportS6E3Button()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.4f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));

    if (ImGui::Button(I18N::Editor::ExportAsS6E3))
    {
        wchar_t fileName[256];
        swprintf_s(fileName, _countof(fileName), L"Data\\Local\\%ls\\Item_S6E3.bmd", g_strSelectedML.c_str());

        if (g_ItemDataHandler.ExportAsS6E3(fileName))
        {
            std::string filename_str = "Item_" + std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_S6E3.bmd";
            g_MuEditorConsoleUI.LogEditor("Exported items as S6E3 legacy format: " + filename_str);
            ImGui::OpenPopup("Export S6E3 Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor(I18N::Editor::FailedToExportAsS6E3Format);
            ImGui::OpenPopup("Export S6E3 Failed");
        }
    }

    ImGui::PopStyleColor(2);
}

void CItemEditorActions::RenderExportCSVButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.7f, 1.0f));

    if (ImGui::Button(I18N::Editor::ExportAsCSV))
    {
        wchar_t csvFileName[256];
        swprintf_s(csvFileName, _countof(csvFileName), L"Data\\Local\\%ls\\Item.csv", g_strSelectedML.c_str());

        if (g_ItemDataHandler.ExportToCsv(csvFileName))
        {
            std::string filename_str = "Item_" + std::string(g_strSelectedML.begin(), g_strSelectedML.end()) + "_export.csv";
            g_MuEditorConsoleUI.LogEditor("Exported items as CSV: " + filename_str);
            ImGui::OpenPopup("Export CSV Success");
        }
        else
        {
            g_MuEditorConsoleUI.LogEditor(I18N::Editor::FailedToExportAsCSV);
            ImGui::OpenPopup("Export CSV Failed");
        }
    }

    ImGui::PopStyleColor(2);
}

bool CItemEditorActions::RenderAllButtons()
{
    RenderSaveButton();
    ImGui::SameLine();
    const bool itemsReplaced = RenderImportBmdButton();
    ImGui::SameLine();
    RenderExportBmdButton();
    ImGui::SameLine();
    RenderExportS6E3Button();
    ImGui::SameLine();
    RenderExportCSVButton();
    return itemsReplaced;
}

#endif // _EDITOR
