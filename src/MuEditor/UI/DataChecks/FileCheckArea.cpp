#include "stdafx.h"

#ifdef _EDITOR

#include "FileCheckArea.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/DataHandler/ModelData/ModelFileDataHandler.h"
#include "Data/GameData/MapData/LightMood.h"
#include "Data/GameData/MapData/LookProfile.h"
#include "Data/GameData/ServerListData/ServerListJson.h"
#include "I18N/All.h"
#include "Render/Look/LookFileSystem.h"
#include "Render/Look/LookFiles.h"
#include "imgui.h"

#include <utility>

namespace MuEditor::DataChecks
{
using Data::Items::ItemDataIssue;

FileCheckArea::FileCheckArea(const char* const* name, CheckFiles check, bool ToolWindows::* tool,
                             const char* toolWindowId)
    : m_name(name), m_check(std::move(check)), m_tool(tool), m_toolWindowId(toolWindowId)
{
}

const char* FileCheckArea::GetName() const
{
    return *m_name;
}

void FileCheckArea::Check(std::vector<ItemDataIssue>& issues)
{
    m_check(issues);
}

void FileCheckArea::RenderSubject(const ItemDataIssue& issue) const
{
    ImGui::TextUnformatted(issue.source.c_str());
}

std::string_view FileCheckArea::GetField(const ItemDataIssue& issue) const
{
    return issue.field;
}

void FileCheckArea::Open(const ItemDataIssue&, bool clicked)
{
    if (!clicked || m_tool == nullptr)
    {
        return;
    }
    g_MuEditorCore.GetToolWindows().*m_tool = true;
    ImGui::SetWindowFocus(m_toolWindowId);
}

std::unique_ptr<DataCheckArea> CreateServerListCheckArea()
{
    return std::make_unique<FileCheckArea>(
        &I18N::Editor::ServerList,
        [](std::vector<ItemDataIssue>& issues)
        {
            std::vector<ItemDataIssue> found =
                Data::ServerList::LoadServerListFile(Data::ServerList::ServerListFile).issues;
            issues.insert(issues.end(), found.begin(), found.end());
        },
        &ToolWindows::serverList, WindowIds::ServerList);
}

std::unique_ptr<DataCheckArea> CreateEffectCheckArea()
{
    return std::make_unique<FileCheckArea>(
        &I18N::Editor::Effects,
        [](std::vector<ItemDataIssue>& issues)
        {
            std::vector<ItemDataIssue> found =
                Data::Effects::LoadEffectTypeFiles(Data::Effects::GetEffectDataDirectory()).issues;
            issues.insert(issues.end(), found.begin(), found.end());
        },
        &ToolWindows::effectBrowser, WindowIds::EffectBrowser);
}

std::unique_ptr<DataCheckArea> CreateModelFileCheckArea()
{
    return std::make_unique<FileCheckArea>(
        &I18N::Editor::ModelFiles,
        [](std::vector<ItemDataIssue>& issues)
        {
            std::vector<ItemDataIssue> found = Data::Models::ReadGameModelFileTable().issues;
            issues.insert(issues.end(), found.begin(), found.end());
        },
        nullptr, nullptr);
}

std::unique_ptr<DataCheckArea> CreateLookCheckArea()
{
    return std::make_unique<FileCheckArea>(
        &I18N::Editor::Looks,
        [](std::vector<ItemDataIssue>& issues)
        {
            Render::Look::DiskLookFileSystem files;
            std::vector<ItemDataIssue> found =
                Render::Look::LoadLookFiles(files, Data::Maps::LookProfileFile, Data::Maps::LightMoodFile).issues;
            issues.insert(issues.end(), found.begin(), found.end());
        },
        &ToolWindows::devEditor, WindowIds::DevEditor);
}
} // namespace MuEditor::DataChecks

#endif // _EDITOR
