#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterTranslationsUI.h"

#include "MonsterFileActions.h"
#include "MonsterNpcNameFiles.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/DataHandler/LegacyLanguages.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "I18N/All.h"
#include "imgui.h"

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
// Keeps the table's column widths and hidden columns.
constexpr char TableId[] = "##MonsterTranslations";
// Pixel width before the editor UI scale.
constexpr float TooltipWrapWidth = 420.0f;

// The files as found below the client folder, for the tooltips.
std::string FileList()
{
    std::string list;
    for (const Data::LegacyLanguage& language : Data::GetLegacyLanguages())
    {
        list += (list.empty() ? "" : ", ") + NpcNameFiles::GetFile({}, language).generic_string();
    }
    return list;
}

// The game runs in its build output folder, not in src/bin: the logs name the
// whole path, so the files can be found.
std::string ShownPath(const std::filesystem::path& path)
{
    std::error_code error;
    const std::filesystem::path absolute = std::filesystem::absolute(path, error);
    return (error ? path : absolute).generic_string();
}

// The note of the button before it, which names the files ({0}).
void RenderFilesTooltip(const char* note)
{
    if (!ImGui::BeginItemTooltip())
    {
        return;
    }
    ImGui::PushTextWrapPos(TooltipWrapWidth * g_MuEditorCore.GetUIScale());
    ImGui::TextUnformatted(I18N::Format(note, {FileList()}).c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

void LogIssues(const std::vector<Data::Items::ItemDataIssue>& issues)
{
    for (const Data::Items::ItemDataIssue& issue : issues)
    {
        g_MuEditorConsoleUI.LogEditor(issue.ToString());
    }
}

// One line per language and its problems; returns how many names changed.
int LogImport(const std::vector<NpcNameFiles::LanguageImport>& languages)
{
    int changed = 0;
    for (const NpcNameFiles::LanguageImport& language : languages)
    {
        const std::string file = ShownPath(language.file);
        if (!language.found)
        {
            g_MuEditorConsoleUI.LogEditor(I18N::Format(I18N::Editor::FileNotFound0, {file}));
            continue;
        }
        g_MuEditorConsoleUI.LogEditor(I18N::Format(
            I18N::Editor::NpcNameImportLine, {file, std::to_string(language.read), std::to_string(language.changed),
                                              std::to_string(language.skipped), std::to_string(language.cut)}));
        LogIssues(language.issues);
        changed += language.changed;
    }
    return changed;
}

// One line per language and its problems; returns whether every file has the
// names now.
bool LogExport(const std::vector<NpcNameFiles::LanguageExport>& languages)
{
    bool complete = true;
    for (const NpcNameFiles::LanguageExport& language : languages)
    {
        const std::string file = ShownPath(language.file);
        if (language.unchanged)
        {
            g_MuEditorConsoleUI.LogEditor(I18N::Format(I18N::Editor::NpcNameExportUnchanged, {file}));
        }
        else if (language.written)
        {
            g_MuEditorConsoleUI.LogEditor(
                I18N::Format(I18N::Editor::NpcNameExportLine,
                             {file, std::to_string(language.names), std::to_string(language.english)}));
        }
        else
        {
            complete = false;
        }
        LogIssues(language.issues);
    }
    return complete;
}
} // namespace

MuMonsterTranslationsUI& MuMonsterTranslationsUI::GetInstance()
{
    static MuMonsterTranslationsUI instance;
    return instance;
}

void MuMonsterTranslationsUI::Render(bool* open)
{
    m_grid.BeginFrame();

    // One of the monster tools: their docking, undo keys and selection.
    if (ToolWindow::Begin(I18N::Editor::MonsterTranslations, WindowIds::MonsterTranslations, open))
    {
        m_grid.UpdateCountsAndLabels(m_rows);
        RenderFileActions();
        m_grid.RenderToolbar(m_rows);
        ImGui::Separator();
        m_grid.RenderTable(m_rows, TableId);
    }
    ToolWindow::End();
    m_grid.CommitEditNotDrawn(m_rows);
}

void MuMonsterTranslationsUI::RenderFileActions()
{
    g_MonsterFileActions.RenderSaveButton();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::ImportNamesFromNpcNameTxt))
    {
        ImportNames();
    }
    RenderFilesTooltip(I18N::Editor::ImportNpcNameTxtNote);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::ExportAsNpcNameTxt))
    {
        ExportNames();
    }
    RenderFilesTooltip(I18N::Editor::ExportNpcNameTxtNote);

    g_MonsterFileActions.RenderStatus();
    if (!m_status.empty())
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", m_status.c_str());
        ImGui::PopTextWrapPos();
    }
}

void MuMonsterTranslationsUI::ImportNames()
{
    // The client folder of the running game is its working directory.
    const int changed = LogImport(NpcNameFiles::Import({}, I18N::Editor::ImportNamesFromNpcNameTxt));
    m_status = I18N::Format(I18N::Editor::MonsterNamesImported, {std::to_string(changed)});
}

void MuMonsterTranslationsUI::ExportNames()
{
    if (!LogExport(NpcNameFiles::Export({})))
    {
        m_status = I18N::Editor::MonsterNamesNotExported;
        return;
    }
    m_status = I18N::Format(I18N::Editor::MonsterNamesExported, {ShownPath(NpcNameFiles::GetLocalFolder({}))});
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
