#include "stdafx.h"

#ifdef _EDITOR

#include "LookEdits.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/SavedDataFiles.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Render/Items/LookDrawing.h"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <optional>
#include <utility>

namespace MuEditor::Looks
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Looks::LookDefinition;

std::filesystem::path FilePath()
{
    return Data::Effects::GetEffectDataDirectory() / Data::Looks::GetRenderStylesFileName();
}

void SortByName(std::vector<LookDefinition>& looks)
{
    std::ranges::sort(looks, {}, &LookDefinition::name);
}
} // namespace

LookEdits& LookEdits::GetInstance()
{
    static LookEdits instance;
    return instance;
}

void LookEdits::Load()
{
    const std::filesystem::path path = FilePath();
    const std::string source = path.generic_string();
    m_issues.clear();
    Data::Looks::RenderStylesFile file;
    if (const std::optional<std::string> text = Data::ReadTextFile(path))
    {
        Data::Looks::ReadRenderStylesFile(*text, source, file, m_issues);
    }
    else
    {
        m_issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "",
                            "could not be read"});
    }
    m_complete = !Data::Items::HasFatal(m_issues);
    if (m_complete)
    {
        SortByName(file.looks);
        m_state = {std::move(file.looks), std::move(file.folders)};
    }
    else
    {
        // The game keeps the looks it read at start.
        const auto looks = Data::Looks::g_LookStore.GetRenderStyles();
        m_state = {{looks.begin(), looks.end()}, {}};
    }
    m_saved = m_state;
    m_undo.clear();
    m_redo.clear();
    m_loaded = true;
    ++m_version;
    ToGame();
}

const LookDefinition* LookEdits::Find(std::string_view name) const
{
    const auto found = std::ranges::lower_bound(m_state.looks, name, {}, &LookDefinition::name);
    return found != m_state.looks.end() && found->name == name ? &*found : nullptr;
}

void LookEdits::SetLook(const char* step, const std::string& name, LookDefinition look)
{
    State next = m_state;
    const auto found = std::ranges::find(next.looks, name, &LookDefinition::name);
    if (found == next.looks.end())
    {
        return;
    }
    if (look.name != name)
    {
        Data::Colors::RenameColorInFolders(next.folders, name, look.name);
    }
    *found = std::move(look);
    SortByName(next.looks);
    Push(step, std::move(next));
}

void LookEdits::AddLook(const char* step, LookDefinition look, const Data::Colors::ColorFolderPath& folder)
{
    if (Find(look.name) != nullptr)
    {
        return;
    }
    State next = m_state;
    const std::string name = look.name;
    next.looks.push_back(std::move(look));
    SortByName(next.looks);
    if (!folder.empty())
    {
        Data::Colors::MoveColorInto(next.folders, name, folder);
    }
    Push(step, std::move(next));
}

void LookEdits::RemoveLook(const char* step, const std::string& name)
{
    State next = m_state;
    if (std::erase_if(next.looks, [&name](const LookDefinition& look) { return look.name == name; }) == 0)
    {
        return;
    }
    Data::Colors::RemoveColorFromFolders(next.folders, name);
    Push(step, std::move(next));
}

void LookEdits::SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders)
{
    State next = m_state;
    next.folders = std::move(folders);
    Push(step, std::move(next));
}

bool LookEdits::IsOpen() const
{
    return g_MuEditorCore.GetToolWindows().styleEditor;
}

std::string LookEdits::GetUndoLabel() const
{
    return m_undo.empty() ? std::string() : m_undo.back().label;
}

std::string LookEdits::GetRedoLabel() const
{
    return m_redo.empty() ? std::string() : m_redo.back().label;
}

void LookEdits::Undo()
{
    if (m_undo.empty())
    {
        return;
    }
    Step step = std::move(m_undo.back());
    m_undo.pop_back();
    m_redo.push_back({step.label, std::move(m_state)});
    m_state = std::move(step.state);
    ++m_version;
    ToGame();
}

void LookEdits::Redo()
{
    if (m_redo.empty())
    {
        return;
    }
    Step step = std::move(m_redo.back());
    m_redo.pop_back();
    m_undo.push_back({step.label, std::move(m_state)});
    m_state = std::move(step.state);
    ++m_version;
    ToGame();
}

bool LookEdits::Save()
{
    if (!m_complete)
    {
        return false;
    }
    const std::filesystem::path path = FilePath();
    const std::string source = path.generic_string();
    const std::string text = Data::Looks::WriteRenderStylesJson(m_state.looks, m_state.folders);
    m_issues.clear();
    Data::Looks::RenderStylesFile readBack;
    Data::Looks::ReadRenderStylesFile(text, source, readBack, m_issues);
    if (Data::Items::HasFatal(m_issues))
    {
        return false;
    }
    std::string error;
    if (!Data::WriteTextFile(path, text, error))
    {
        m_issues.push_back(
            {ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "", error});
        return false;
    }
    m_saved = m_state;
    SavedDataFiles::NoteSaved();
    return true;
}

void LookEdits::ResetForTest(State state)
{
    SortByName(state.looks);
    m_state = std::move(state);
    m_saved = m_state;
    m_undo.clear();
    m_redo.clear();
    m_issues.clear();
    m_loaded = true;
    m_complete = true;
    ++m_version;
    ToGame();
}

void LookEdits::Push(const char* step, State next)
{
    if (next == m_state)
    {
        return;
    }
    m_undo.push_back({step, std::move(m_state)});
    if (m_undo.size() > MaxUndoSteps)
    {
        m_undo.erase(m_undo.begin());
    }
    m_redo.clear();
    m_state = std::move(next);
    ++m_version;
    ToGame();
}

void LookEdits::ToGame() const
{
    Data::Looks::g_LookStore.SetRenderStyles(m_state.looks);
    for (const LookDefinition& look : m_state.looks)
    {
        Render::Items::Looks::PrepareTextures(look);
    }
}
} // namespace MuEditor::Looks

#endif // _EDITOR
