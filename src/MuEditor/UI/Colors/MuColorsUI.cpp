#include "stdafx.h"

#ifdef _EDITOR

#include "MuColorsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/ToolGroups.h"
#include "../MuEditor/UI/Common/ToolWindowFrame.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstring>

using Data::Colors::ColorFolder;
using Data::Colors::ColorFolderPath;
using MuEditor::Colors::ColorsRoot;
using MuEditor::Colors::NamedColorSource;
using MuEditor::Colors::NamedColorTexts;

namespace
{
constexpr char FolderPopupId[] = "###ColorFolder";
constexpr char DragPayloadType[] = "MU_COLORS_TREE";
// Pixel widths before the editor UI scale.
constexpr float TreeWidth = 260.0f;
constexpr float FolderNameWidth = 260.0f;
// The longest color name a drag carries.
constexpr std::size_t MaxDraggedName = 64;

constexpr ColorsRoot Roots[] = {ColorsRoot::Glow};

// A root's or a folder's tree node picks it on a click (not on its arrow) and when the arrow keys move onto it, as
// the colors' rows do (KeyboardSelection).
bool IsPicking()
{
    return (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) || MuEditor::KeyboardSelection::MovedOnto();
}

// What a drag carries: a folder (its path) or a color (its name, then) of a root.
struct DragPayload
{
    int root = 0;
    std::size_t depth = 0;
    std::size_t path[Data::Colors::MaxColorFolderDepth] = {};
    char color[MaxDraggedName] = {};
};
} // namespace

CMuColorsUI& CMuColorsUI::GetInstance()
{
    static CMuColorsUI instance;
    return instance;
}

void CMuColorsUI::Render(bool* open)
{
    KeepPickValid();
    if (MuEditor::ToolWindowFrame::Begin(I18N::Editor::Colors, MuEditor::WindowIds::Colors, open,
                                         MuEditor::GetItemTools(), SourceOf(m_pick.root).GetUndoTarget()))
    {
        RenderToolbar();
        // In the toolbar's ID scope, where the buttons opened them.
        RenderNewEntries();
        RenderFolderDialog();
        MuEditor::Colors::RenderColorNote(I18N::Editor::ColorsTreeNote);
        ImGui::Separator();

        const ImVec2 treeSize(TreeWidth * g_MuEditorCore.GetUIScale(), 0.0f);
        if (ImGui::BeginChild("##Tree", treeSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            RenderTree();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("##Picked"))
        {
            RenderPicked();
        }
        ImGui::EndChild();

        if (m_action)
        {
            const std::function<void()> action = std::move(m_action);
            m_action = nullptr;
            action();
        }
    }
    MuEditor::ToolWindowFrame::End();
}

void CMuColorsUI::Show(ColorsRoot root)
{
    g_MuEditorCore.GetToolWindows().colors = true;
    SetPick(root, {});
    m_openRoot = root;
    ImGui::SetWindowFocus(MuEditor::WindowIds::Colors);
}

NamedColorSource& CMuColorsUI::SourceOf(ColorsRoot root)
{
    switch (root)
    {
    case ColorsRoot::Glow:
    default:
        return m_glow;
    }
}

void CMuColorsUI::SetPick(ColorsRoot root, ColorFolderPath folder, std::string color)
{
    std::vector<std::string> names = Data::Colors::GetColorFolderNames(SourceOf(root).GetFolders(), folder);
    m_pick = {root, std::move(folder), std::move(color), std::move(names)};
}

void CMuColorsUI::KeepPickValid()
{
    const NamedColorSource& source = SourceOf(m_pick.root);
    const std::vector<ColorFolder>& folders = source.GetFolders();
    if (!m_pick.color.empty())
    {
        if (source.Has(m_pick.color))
        {
            m_pick.folder = Data::Colors::FindFolderOfColor(folders, m_pick.color).value_or(ColorFolderPath{});
            m_pick.folderNames = Data::Colors::GetColorFolderNames(folders, m_pick.folder);
            return;
        }
        m_pick.color.clear();
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

ColorFolderPath CMuColorsUI::GetPickedFolder() const
{
    return m_pick.folder;
}

void CMuColorsUI::RenderToolbar()
{
    NamedColorSource& source = SourceOf(m_pick.root);
    if (ImGui::Button(I18N::Editor::NewFolder))
    {
        m_folderDialog = FolderDialog::New;
        m_folderName.clear();
        ImGui::OpenPopup(FolderPopupId);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::NewColor))
    {
        m_newColorRoot = m_pick.root;
        m_newColorFolder = GetPickedFolder();
        source.OpenNew(m_pick.color);
    }
    ImGui::SameLine();
    const bool folderPicked = m_pick.color.empty() && !m_pick.folder.empty();
    ImGui::BeginDisabled(!folderPicked);
    if (ImGui::Button(I18N::Editor::Rename))
    {
        const ColorFolder* folder = Data::Colors::FindColorFolder(source.GetFolders(), m_pick.folder);
        m_folderDialog = FolderDialog::Rename;
        m_folderName = folder != nullptr ? folder->name : std::string();
        ImGui::OpenPopup(FolderPopupId);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // D26 for a color: only one nothing uses goes. A folder goes with what it holds moving up.
    const bool colorRemovable = !m_pick.color.empty() && source.Has(m_pick.color) &&
                                source.CountUsers(m_pick.color) == 0 && !source.IsKept(m_pick.color);
    ImGui::BeginDisabled(!folderPicked && !colorRemovable);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        m_action = [this] { RemovePicked(); };
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!folderPicked && m_pick.color.empty());
    if (ImGui::Button(I18N::Editor::MoveUp))
    {
        m_action = [this] { MovePicked(-1); };
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::MoveDown))
    {
        m_action = [this] { MovePicked(1); };
    }
    ImGui::EndDisabled();
}

void CMuColorsUI::RenderNewEntries()
{
    for (const ColorsRoot root : Roots)
    {
        NamedColorSource& source = SourceOf(root);
        // One undo step: the new color and its place in the folder that was picked.
        const ColorFolderPath folder = m_newColorRoot == root ? m_newColorFolder : ColorFolderPath{};
        const std::optional<std::string> created = source.RenderNew(folder);
        if (!created)
        {
            continue;
        }
        SetPick(root, Data::Colors::FindFolderOfColor(source.GetFolders(), *created).value_or(ColorFolderPath{}),
                *created);
        m_newColorRoot.reset();
    }
}

void CMuColorsUI::RenderFolderDialog()
{
    const char* title = m_folderDialog == FolderDialog::Rename ? I18N::Editor::Rename : I18N::Editor::NewFolder;
    const std::string popup = std::string(title) + FolderPopupId;
    if (!ImGui::BeginPopupModal(popup.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::SetNextItemWidth(FolderNameWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }
    MuEditor::Items::InputString(I18N::Editor::FolderName, m_folderName);

    // The edit tried on a copy: a name another folder here has, or none, is refused.
    const ColorsRoot root = m_pick.root;
    const ColorFolderPath picked = GetPickedFolder();
    const std::string name = m_folderName;
    const bool renaming = m_folderDialog == FolderDialog::Rename;
    const auto edit = [renaming, picked, name](std::vector<ColorFolder>& folders)
    {
        return renaming ? Data::Colors::RenameColorFolder(folders, picked, name)
                        : Data::Colors::AddColorFolder(folders, picked, name);
    };
    std::vector<ColorFolder> trial = SourceOf(root).GetFolders();
    const bool usable = edit(trial);
    MuEditor::Colors::RenderColorNote(I18N::Editor::AFolderNameOf1To32CharactersThatNoOtherFolderHereHas);

    ImGui::BeginDisabled(!usable);
    if (ImGui::Button(I18N::Editor::OK))
    {
        m_action = [this, root, picked, edit, renaming]
        {
            ChangeFolders(root, renaming ? I18N::Editor::RenameAColorFolder : I18N::Editor::NewColorFolder, edit);
            if (renaming && m_pickedFolderOpen)
            {
                // Its tree node takes its id from its name: it stays open under the new one.
                m_openFolder = picked;
            }
            if (!renaming)
            {
                // The new folder is the last one of its parent.
                const ColorFolder* parent = Data::Colors::FindColorFolder(SourceOf(root).GetFolders(), picked);
                const std::size_t count = parent != nullptr ? parent->folders.size() : SourceOf(root).GetFolders().size();
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
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuColorsUI::RenderTree()
{
    m_pickedFolderOpen = false;
    for (const ColorsRoot root : Roots)
    {
        RenderRoot(root);
    }
    m_openRoot.reset();
    m_openFolder.reset();
}

void CMuColorsUI::RenderRoot(ColorsRoot root)
{
    NamedColorSource& source = SourceOf(root);
    const NamedColorTexts texts = source.GetTexts();
    ImGui::PushID(static_cast<int>(root));
    if (m_openRoot == root)
    {
        ImGui::SetNextItemOpen(true);
    }
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_pick.root == root && m_pick.folder.empty() && m_pick.color.empty())
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    const bool open = ImGui::TreeNodeEx("##Root", flags, "%s", texts.root);
    if (IsPicking())
    {
        SetPick(root, {});
    }
    if (m_openRoot == root)
    {
        ImGui::SetScrollHereY(0.0f);
    }
    AcceptDrop(root, {});
    // A closed root's entries aren't read.
    if (open)
    {
        ColorFolderPath path;
        RenderFolders(root, source.GetFolders(), path);
        for (const std::string& name :
             Data::Colors::GetRootColors(source.GetFolders(), MuEditor::Colors::GetNames(source)))
        {
            RenderColor(root, name, {});
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void CMuColorsUI::RenderFolders(ColorsRoot root, const std::vector<ColorFolder>& folders, ColorFolderPath& path)
{
    NamedColorSource& source = SourceOf(root);
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
        const bool picked = m_pick.root == root && m_pick.color.empty() && m_pick.folder == path;
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
        AcceptDrop(root, path);
        if (open)
        {
            RenderFolders(root, folder.folders, path);
            for (const std::string& color : folder.colors)
            {
                if (source.Has(color))
                {
                    RenderColor(root, color, path);
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
        path.pop_back();
    }
}

void CMuColorsUI::RenderColor(ColorsRoot root, const std::string& name, const ColorFolderPath& folder)
{
    NamedColorSource& source = SourceOf(root);
    ImGui::PushID(name.c_str());
    source.RenderSwatch(name);
    ImGui::SameLine();
    const std::string label = name + " (" + std::to_string(source.CountUsers(name)) + ")";
    if (ImGui::Selectable(label.c_str(), m_pick.root == root && m_pick.color == name,
                          MuEditor::KeyboardSelection::RowFlags))
    {
        SetPick(root, folder, name);
    }
    BeginDrag(root, folder, name, label.c_str());
    ImGui::PopID();
}

void CMuColorsUI::RenderPicked()
{
    NamedColorSource& source = SourceOf(m_pick.root);
    const NamedColorTexts texts = source.GetTexts();
    if (!m_pick.color.empty())
    {
        m_pick.color = m_panel.RenderEntry(source, m_pick.color);
        return;
    }
    if (!m_pick.folder.empty())
    {
        if (const ColorFolder* folder = Data::Colors::FindColorFolder(source.GetFolders(), m_pick.folder))
        {
            const auto colors = std::count_if(folder->colors.begin(), folder->colors.end(),
                                              [&source](const std::string& color) { return source.Has(color); });
            ImGui::TextUnformatted(folder->name.c_str());
            ImGui::TextUnformatted(I18N::Format(I18N::Editor::ItHolds0ColorsAnd1Folders,
                                                {std::to_string(colors), std::to_string(folder->folders.size())})
                                       .c_str());
            MuEditor::Colors::RenderColorNote(I18N::Editor::FolderNote);
        }
        return;
    }
    ImGui::TextUnformatted(texts.root);
    MuEditor::Colors::RenderColorNote(texts.note);
    m_panel.RenderUnused(source);
}

void CMuColorsUI::BeginDrag(ColorsRoot root, const ColorFolderPath& folder, const std::string& color,
                            const char* label)
{
    if (color.size() >= MaxDraggedName || !ImGui::BeginDragDropSource())
    {
        return;
    }
    DragPayload payload;
    payload.root = static_cast<int>(root);
    payload.depth = std::min(folder.size(), Data::Colors::MaxColorFolderDepth);
    std::copy_n(folder.begin(), payload.depth, payload.path);
    std::copy(color.begin(), color.end(), payload.color);
    ImGui::SetDragDropPayload(DragPayloadType, &payload, sizeof(payload));
    ImGui::TextUnformatted(label);
    ImGui::EndDragDropSource();
}

void CMuColorsUI::AcceptDrop(ColorsRoot root, const ColorFolderPath& folder)
{
    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }
    if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload(DragPayloadType))
    {
        DragPayload payload;
        std::memcpy(&payload, dropped->Data, sizeof(payload));
        // A color moves only within its root: the values differ.
        if (payload.root == static_cast<int>(root))
        {
            const std::string color = payload.color;
            const ColorFolderPath from(payload.path, payload.path + payload.depth);
            m_action = [this, root, folder, color, from]
            {
                if (!color.empty())
                {
                    ChangeFolders(root, I18N::Editor::MoveAColor, [&](std::vector<ColorFolder>& folders)
                                  { return Data::Colors::MoveColorInto(folders, color, folder); });
                    return;
                }
                bool moved = false;
                ChangeFolders(root, I18N::Editor::MoveAColorFolder,
                              [&](std::vector<ColorFolder>& folders)
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

void CMuColorsUI::ChangeFolders(ColorsRoot root, const char* step,
                                const std::function<bool(std::vector<ColorFolder>&)>& edit)
{
    NamedColorSource& source = SourceOf(root);
    std::vector<ColorFolder> folders = source.GetFolders();
    if (edit(folders))
    {
        source.SetFolders(step, std::move(folders));
    }
}

void CMuColorsUI::MovePicked(int delta)
{
    NamedColorSource& source = SourceOf(m_pick.root);
    const std::string color = m_pick.color;
    if (!color.empty())
    {
        if (m_pick.folder.empty())
        {
            if (const std::optional<std::vector<std::string>> order = Data::Colors::MoveRootColorBy(
                    source.GetFolders(), MuEditor::Colors::GetNames(source), color, delta))
            {
                source.SetOrder(I18N::Editor::MoveAColor, *order);
            }
            return;
        }
        ChangeFolders(m_pick.root, I18N::Editor::MoveAColor, [&](std::vector<ColorFolder>& folders)
                      { return Data::Colors::MoveColorInFolderBy(folders, color, delta); });
        return;
    }
    const ColorFolderPath path = m_pick.folder;
    bool moved = false;
    ChangeFolders(m_pick.root, I18N::Editor::MoveAColorFolder, [&](std::vector<ColorFolder>& folders)
                  { return moved = Data::Colors::MoveColorFolderBy(folders, path, delta); });
    if (moved)
    {
        ColorFolderPath movedTo = path;
        movedTo.back() = static_cast<std::size_t>(static_cast<long long>(movedTo.back()) + delta);
        SetPick(m_pick.root, std::move(movedTo));
    }
}

void CMuColorsUI::RemovePicked()
{
    NamedColorSource& source = SourceOf(m_pick.root);
    if (!m_pick.color.empty())
    {
        const std::string names[] = {m_pick.color};
        source.Remove(source.GetTexts().removeOne, names);
        m_pick.color.clear();
        return;
    }
    const ColorFolderPath path = m_pick.folder;
    ChangeFolders(m_pick.root, I18N::Editor::RemoveAColorFolder,
                  [&](std::vector<ColorFolder>& folders) { return Data::Colors::RemoveColorFolder(folders, path); });
    ColorFolderPath parent = path;
    parent.pop_back();
    SetPick(m_pick.root, std::move(parent));
}

#endif // _EDITOR
