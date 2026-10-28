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

#include <iterator>
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
    std::vector<ItemDataIssue> found = m_check();
    issues.insert(issues.end(), std::make_move_iterator(found.begin()), std::make_move_iterator(found.end()));
}

void FileCheckArea::RenderSubject(const ItemDataIssue& issue) const
{
    ImGui::TextUnformatted(issue.source.c_str());
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
        [] { return Data::ServerList::LoadServerListFile(Data::ServerList::ServerListFile).issues; },
        &ToolWindows::serverList, WindowIds::ServerList);
}

std::unique_ptr<DataCheckArea> CreateEffectCheckArea()
{
    return std::make_unique<FileCheckArea>(
        &I18N::Editor::Effects,
        [] { return Data::Effects::LoadEffectTypeFiles(Data::Effects::GetEffectDataDirectory()).issues; },
        &ToolWindows::effectBrowser, WindowIds::EffectBrowser);
}

std::unique_ptr<DataCheckArea> CreateModelFileCheckArea()
{
    return std::make_unique<FileCheckArea>(
        &I18N::Editor::ModelFiles, [] { return Data::Models::ReadGameModelFileTable().issues; }, nullptr, nullptr);
}

std::unique_ptr<DataCheckArea> CreateLookCheckArea()
{
    return std::make_unique<FileCheckArea>(
        &I18N::Editor::Looks,
        []
        {
            Render::Look::DiskLookFileSystem files;
            return Render::Look::LoadLookFiles(files, Data::Maps::LookProfileFile, Data::Maps::LightMoodFile).issues;
        },
        &ToolWindows::devEditor, WindowIds::DevEditor);
}
} // namespace MuEditor::DataChecks

#endif // _EDITOR
