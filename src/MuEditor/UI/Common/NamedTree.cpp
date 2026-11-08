#include "stdafx.h"

#ifdef _EDITOR

#include "NamedTree.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/SourceText.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <utility>

namespace MuEditor
{
namespace
{
using Data::Colors::ColorFolder;
using Data::Colors::ColorFolderPath;

// Pixel width before the editor UI scale.
constexpr float FolderNameWidth = 260.0f;

// A root's or a folder's tree node picks it on a click (not on its arrow) and when the arrow keys move onto it, as
// the entries' rows do (KeyboardSelection).
bool IsPicking()
{
    return (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) || KeyboardSelection::MovedOnto();
}

// What a drag carries first: a folder (its path) or an entry of a root. An entry's name follows, of any length.
struct DragHeader
{
    std::size_t root = 0;
    std::size_t depth = 0;
    std::size_t path[Data::Colors::MaxColorFolderDepth] = {};
    std::size_t entryLength = 0;
};

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}
} // namespace

NamedTree::NamedTree(std::vector<NamedTreeRoot*> roots, const char* dragType, const char* folderPopupId)
    : m_roots(std::move(roots)), m_dragType(dragType), m_folderPopupId(folderPopupId)
{
}

void NamedTree::SetPick(std::size_t root, ColorFolderPath folder, std::string entry)
{
    std::vector<std::string> names = Data::Colors::GetColorFolderNames(m_roots[root]->GetFolders(), folder);
    m_pick = {root, std::move(folder), std::move(entry), std::move(names)};
}

void NamedTree::PickEntry(std::size_t root, const std::string& entry)
{
    SetPick(root, Data::Colors::FindFolderOfColor(m_roots[root]->GetFolders(), entry).value_or(ColorFolderPath{}),
            entry);
}

void NamedTree::KeepPickValid()
{
    const NamedTreeRoot& root = GetPickedRoot();
    const std::vector<ColorFolder>& folders = root.GetFolders();
    if (!m_pick.entry.empty())
    {
        if (root.Has(m_pick.entry))
        {
            m_pick.folder = Data::Colors::FindFolderOfColor(folders, m_pick.entry).value_or(ColorFolderPath{});
            m_pick.folderNames = Data::Colors::GetColorFolderNames(folders, m_pick.folder);
            return;
        }
        m_pick.entry.clear();
    }
    // A folder by its names first: an undo or a redo that moved folders around keeps the same one picked.
    if (const std::optional<ColorFolderPath> found = Data::Colors::FindColorFolderByNames(folders, m_pick.folderNames))
    {
        m_pick.folder = *found;
        return;
    }
    // Then by its place: one renamed there stays picked, under its new name.
    if (Data::Colors::FindColorFolder(folders, m_pick.folder) == nullptr)
    {
        m_pick.folder.clear();
    }
    m_pick.folderNames = Data::Colors::GetColorFolderNames(folders, m_pick.folder);
}

void NamedTree::RenderNewFolderButton(const char* label)
{
    // No folder goes below the deepest level (AddColorFolder refuses it).
    const bool deepest = m_pick.folder.size() >= Data::Colors::MaxColorFolderDepth;
    ImGui::BeginDisabled(deepest || !GetPickedRoot().HasFolders());
    if (ImGui::Button(label))
    {
        m_folderDialog = FolderDialog::New;
        m_folderName.clear();
        ImGui::OpenPopup(m_folderPopupId);
    }
    ImGui::EndDisabled();
    if (deepest && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Format(I18N::Editor::FoldersGoAtMost0LevelsDeep,
                                             {std::to_string(Data::Colors::MaxColorFolderDepth)})
                                    .c_str());
    }
}

void NamedTree::OpenRenameFolder()
{
    const ColorFolder* folder = Data::Colors::FindColorFolder(GetPickedRoot().GetFolders(), m_pick.folder);
    m_folderDialog = FolderDialog::Rename;
    m_folderName = folder != nullptr ? folder->name : std::string();
    ImGui::OpenPopup(m_folderPopupId);
}

void NamedTree::RenderFolderDialog(const NamedTreeTexts& texts, const char* newTitle, const char* renameTitle,
                                   const char* nameLabel, const char* note, const char* ok, const char* cancel)
{
    const char* title = m_folderDialog == FolderDialog::Rename ? renameTitle : newTitle;
    const std::string popup = std::string(title) + m_folderPopupId;
    if (!ImGui::BeginPopupModal(popup.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::SetNextItemWidth(FolderNameWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }
    Items::InputString(nameLabel, m_folderName);

    // The edit tried on a copy: a name another folder here has, or none, is refused. Tried again only when the name,
    // the folder or the root's folders changed.
    const std::size_t root = m_pick.root;
    const ColorFolderPath picked = m_pick.folder;
    // Spaces typed around the name don't count (IsColorFolderName refuses them).
    const std::string name = SourceText::Trimmed(m_folderName);
    const bool renaming = m_folderDialog == FolderDialog::Rename;
    const auto edit = [renaming, picked, name](std::vector<ColorFolder>& folders)
    {
        return renaming ? Data::Colors::RenameColorFolder(folders, picked, name)
                        : Data::Colors::AddColorFolder(folders, picked, name);
    };
    const int version = m_roots[root]->GetVersion();
    if (m_folderTrial.root != root || m_folderTrial.version != version || m_folderTrial.renaming != renaming ||
        m_folderTrial.folder != picked || m_folderTrial.name != name)
    {
        std::vector<ColorFolder> trial = m_roots[root]->GetFolders();
        m_folderTrial = {root, version, renaming, picked, name, edit(trial)};
    }
    const bool usable = m_folderTrial.usable;
    RenderNote(note);

    ImGui::BeginDisabled(!usable);
    if (ImGui::Button(ok))
    {
        const char* step = renaming ? texts.renameFolder : texts.newFolder;
        m_action = [this, root, picked, edit, renaming, step]
        {
            ChangeFolders(root, step, edit);
            if (renaming && m_pickedFolderOpen)
            {
                // Its tree node takes its id from its name: it stays open under the new one.
                m_openFolder = picked;
            }
            if (!renaming)
            {
                // The new folder is the last one of its parent.
                const std::vector<ColorFolder>& folders = m_roots[root]->GetFolders();
                const ColorFolder* parent = Data::Colors::FindColorFolder(folders, picked);
                const std::size_t count = parent != nullptr ? parent->folders.size() : folders.size();
                ColorFolderPath path = picked;
                path.push_back(count - 1);
                SetPick(root, path);
                m_openRoot = root;
            }
        };
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

bool NamedTree::CanMovePicked() const
{
    if (IsFolderPicked())
    {
        return true;
    }
    const NamedTreeRoot& root = GetPickedRoot();
    return !m_pick.entry.empty() && root.CanMoveEntry(m_pick.entry) && (!m_pick.folder.empty() || root.CanReorder());
}

void NamedTree::MovePicked(const NamedTreeTexts& texts, int delta)
{
    NamedTreeRoot& root = GetPickedRoot();
    const std::string entry = m_pick.entry;
    if (!entry.empty())
    {
        if (m_pick.folder.empty())
        {
            if (!root.CanReorder())
            {
                return;
            }
            if (const std::optional<std::vector<std::string>> order =
                    Data::Colors::MoveRootColorBy(root.GetFolders(), GetNames(root), entry, delta))
            {
                root.SetOrder(texts.moveEntry, *order);
            }
            return;
        }
        ChangeFolders(m_pick.root, texts.moveEntry, [&](std::vector<ColorFolder>& folders)
                      { return Data::Colors::MoveColorInFolderBy(folders, entry, delta); });
        return;
    }
    const ColorFolderPath path = m_pick.folder;
    bool moved = false;
    ChangeFolders(m_pick.root, texts.moveFolder, [&](std::vector<ColorFolder>& folders)
                  { return moved = Data::Colors::MoveColorFolderBy(folders, path, delta); });
    if (moved)
    {
        ColorFolderPath movedTo = path;
        movedTo.back() = static_cast<std::size_t>(static_cast<long long>(movedTo.back()) + delta);
        SetPick(m_pick.root, std::move(movedTo));
    }
}

void NamedTree::RemovePickedFolder(const NamedTreeTexts& texts)
{
    const ColorFolderPath path = m_pick.folder;
    ChangeFolders(m_pick.root, texts.removeFolder,
                  [&](std::vector<ColorFolder>& folders) { return Data::Colors::RemoveColorFolder(folders, path); });
    ColorFolderPath parent = path;
    parent.pop_back();
    SetPick(m_pick.root, std::move(parent));
}

void NamedTree::Render(const NamedTreeTexts& texts)
{
    m_pickedFolderOpen = false;
    for (std::size_t root = 0; root < m_roots.size(); ++root)
    {
        RenderRoot(root, texts);
    }
    m_openRoot.reset();
    m_openFolder.reset();
}

void NamedTree::RenderRoot(std::size_t root, const NamedTreeTexts& texts)
{
    NamedTreeRoot& source = *m_roots[root];
    ImGui::PushID(static_cast<int>(root));
    if (m_openRoot == root)
    {
        ImGui::SetNextItemOpen(true);
    }
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_pick.root == root && m_pick.folder.empty() && m_pick.entry.empty())
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    const bool open = ImGui::TreeNodeEx("##Root", flags, "%s", source.GetRootName());
    if (IsPicking())
    {
        SetPick(root, {});
    }
    if (m_openRoot == root)
    {
        ImGui::SetScrollHereY(0.0f);
    }
    AcceptDrop(root, {}, texts);
    // A closed root's entries aren't read: the server list's file is read when its root is first opened.
    if (open)
    {
        ColorFolderPath path;
        RenderFolders(root, texts, source.GetFolders(), path);
        for (const std::string& name : GetRootEntries(root))
        {
            RenderEntry(root, name, {});
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

const std::vector<std::string>& NamedTree::GetRootEntries(std::size_t root)
{
    const NamedTreeRoot& source = *m_roots[root];
    RootEntries& cached = m_rootEntries[root];
    if (cached.version != source.GetVersion())
    {
        cached.names = Data::Colors::GetRootColors(source.GetFolders(), GetNames(source));
        cached.version = source.GetVersion();
    }
    return cached.names;
}

void NamedTree::RenderFolders(std::size_t root, const NamedTreeTexts& texts, const std::vector<ColorFolder>& folders,
                              ColorFolderPath& path)
{
    const NamedTreeRoot& source = *m_roots[root];
    for (std::size_t index = 0; index < folders.size(); ++index)
    {
        const ColorFolder& folder = folders[index];
        path.push_back(index);
        // By its name, which no sibling has: the node stays open or closed when the folder moves.
        ImGui::PushID(folder.name.c_str());
        if (m_openFolder == path)
        {
            ImGui::SetNextItemOpen(true);
        }
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        const bool picked = m_pick.root == root && m_pick.entry.empty() && m_pick.folder == path;
        if (picked)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        const bool open = ImGui::TreeNodeEx("##Folder", flags, "%s", folder.name.c_str());
        if (picked)
        {
            m_pickedFolderOpen = open;
        }
        if (IsPicking())
        {
            SetPick(root, path);
        }
        BeginDrag(root, path, {}, folder.name.c_str());
        AcceptDrop(root, path, texts);
        if (open)
        {
            RenderFolders(root, texts, folder.folders, path);
            for (const std::string& entry : folder.colors)
            {
                if (source.Has(entry))
                {
                    RenderEntry(root, entry, path);
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
        path.pop_back();
    }
}

void NamedTree::RenderEntry(std::size_t root, const std::string& name, const ColorFolderPath& folder)
{
    NamedTreeRoot& source = *m_roots[root];
    ImGui::PushID(name.c_str());
    if (source.HasSwatches())
    {
        source.RenderSwatch(name);
        ImGui::SameLine();
    }
    const std::string label = source.GetRowLabel(name);
    if (ImGui::Selectable(label.c_str(), m_pick.root == root && m_pick.entry == name, KeyboardSelection::RowFlags))
    {
        SetPick(root, folder, name);
    }
    BeginDrag(root, folder, name, label.c_str());
    ImGui::PopID();
}

void NamedTree::RenderPickedFolder(const NamedTreeTexts& texts, const char* note) const
{
    const NamedTreeRoot& source = GetPickedRoot();
    const ColorFolder* folder = Data::Colors::FindColorFolder(source.GetFolders(), m_pick.folder);
    if (folder == nullptr)
    {
        return;
    }
    const auto entries = std::count_if(folder->colors.begin(), folder->colors.end(),
                                       [&source](const std::string& entry) { return source.Has(entry); });
    ImGui::TextUnformatted(folder->name.c_str());
    ImGui::TextUnformatted(
        I18N::Format(texts.folderHolds, {std::to_string(entries), std::to_string(folder->folders.size())}).c_str());
    RenderNote(note);
}

void NamedTree::RunAction()
{
    if (m_action)
    {
        const std::function<void()> action = std::move(m_action);
        m_action = nullptr;
        action();
    }
}

void NamedTree::BeginDrag(std::size_t root, const ColorFolderPath& folder, const std::string& entry, const char* label)
{
    const NamedTreeRoot& source = *m_roots[root];
    if (!source.HasFolders() || (!entry.empty() && !source.CanMoveEntry(entry)) || !ImGui::BeginDragDropSource())
    {
        return;
    }
    DragHeader header;
    header.root = root;
    header.depth = std::min(folder.size(), Data::Colors::MaxColorFolderDepth);
    std::copy_n(folder.begin(), header.depth, header.path);
    header.entryLength = entry.size();
    std::vector<char> payload(sizeof(header) + entry.size());
    std::memcpy(payload.data(), &header, sizeof(header));
    std::copy(entry.begin(), entry.end(), payload.begin() + static_cast<std::ptrdiff_t>(sizeof(header)));
    ImGui::SetDragDropPayload(m_dragType, payload.data(), payload.size());
    ImGui::TextUnformatted(label);
    ImGui::EndDragDropSource();
}

void NamedTree::AcceptDrop(std::size_t root, const ColorFolderPath& folder, const NamedTreeTexts& texts)
{
    if (!m_roots[root]->HasFolders() || !ImGui::BeginDragDropTarget())
    {
        return;
    }
    if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload(m_dragType))
    {
        DragHeader header;
        const auto size = static_cast<std::size_t>(dropped->DataSize);
        if (size >= sizeof(header))
        {
            std::memcpy(&header, dropped->Data, sizeof(header));
        }
        const bool whole = size >= sizeof(header) && header.depth <= Data::Colors::MaxColorFolderDepth &&
                           size == sizeof(header) + header.entryLength;
        // An entry moves only within its root: the kinds differ.
        if (whole && header.root == root)
        {
            const std::string entry(static_cast<const char*>(dropped->Data) + sizeof(header), header.entryLength);
            const ColorFolderPath from(header.path, header.path + header.depth);
            const char* moveEntry = texts.moveEntry;
            const char* moveFolder = texts.moveFolder;
            m_action = [this, root, folder, entry, from, moveEntry, moveFolder]
            {
                if (!entry.empty())
                {
                    ChangeFolders(root, moveEntry, [&](std::vector<ColorFolder>& folders)
                                  { return Data::Colors::MoveColorInto(folders, entry, folder); });
                    return;
                }
                bool moved = false;
                ChangeFolders(root, moveFolder, [&](std::vector<ColorFolder>& folders)
                              { return moved = Data::Colors::MoveColorFolderInto(folders, from, folder); });
                if (moved)
                {
                    // The folder it went into, which moved up when the dragged one came before it.
                    SetPick(root, Data::Colors::GetPathAfterMove(from, folder));
                }
            };
        }
    }
    ImGui::EndDragDropTarget();
}

void NamedTree::ChangeFolders(std::size_t root, const char* step,
                              const std::function<bool(std::vector<ColorFolder>&)>& edit)
{
    NamedTreeRoot& source = *m_roots[root];
    std::vector<ColorFolder> folders = source.GetFolders();
    if (edit(folders))
    {
        source.SetFolders(step, std::move(folders));
    }
}
} // namespace MuEditor

#endif // _EDITOR
