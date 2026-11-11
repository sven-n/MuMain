#include "stdafx.h"

#ifdef _EDITOR

#include "MuColorsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/ToolGroups.h"
#include "../MuEditor/UI/Common/ToolWindowFrame.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "I18N/All.h"
#include "imgui.h"

#include <string>
#include <vector>

using Data::Colors::ColorFolderPath;
using MuEditor::Colors::ColorsRoot;
using MuEditor::Colors::NamedColorSource;
using MuEditor::Colors::NamedColorTexts;

namespace
{
constexpr char FolderPopupId[] = "###ColorFolder";
constexpr char DragPayloadType[] = "MU_COLORS_TREE";
// Pixel width before the editor UI scale.
constexpr float TreeWidth = 260.0f;

constexpr ColorsRoot Roots[] = {ColorsRoot::Glow, ColorsRoot::GroundLabels, ColorsRoot::ServerList};

MuEditor::NamedTreeTexts TreeTexts()
{
    MuEditor::NamedTreeTexts texts;
    texts.newFolder = I18N::Editor::NewColorFolder;
    texts.renameFolder = I18N::Editor::RenameAColorFolder;
    texts.removeFolder = I18N::Editor::RemoveAColorFolder;
    texts.moveFolder = I18N::Editor::MoveAColorFolder;
    texts.moveEntry = I18N::Editor::MoveAColor;
    texts.folderHolds = I18N::Editor::ItHolds0ColorsAnd1Folders;
    return texts;
}
} // namespace

CMuColorsUI& CMuColorsUI::GetInstance()
{
    static CMuColorsUI instance;
    return instance;
}

CMuColorsUI::CMuColorsUI() : m_tree({&m_glow, &m_groundLabels, &m_serverList}, DragPayloadType, FolderPopupId) {}

void CMuColorsUI::Render(bool* open)
{
    m_tree.KeepPickValid();
    if (MuEditor::ToolWindowFrame::Begin(I18N::Editor::Colors, MuEditor::WindowIds::Colors, open,
                                         MuEditor::GetItemTools(), PickedSource().GetUndoTarget()))
    {
        const MuEditor::NamedTreeTexts texts = TreeTexts();
        RenderToolbar();
        // In the toolbar's ID scope, where the buttons opened them.
        RenderNewEntries();
        m_tree.RenderFolderDialog(texts, I18N::Editor::NewFolder, I18N::Editor::Rename, I18N::Editor::FolderName,
                                  I18N::Editor::AFolderNameOf1To32CharactersThatNoOtherFolderHereHas, I18N::Editor::OK,
                                  I18N::Editor::Cancel);
        MuEditor::Colors::RenderColorNote(I18N::Editor::ColorsTreeNote);
        ImGui::Separator();

        const ImVec2 treeSize(TreeWidth * g_MuEditorCore.GetUIScale(), 0.0f);
        if (ImGui::BeginChild("##Tree", treeSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            m_tree.Render(texts);
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("##Picked"))
        {
            RenderPicked();
        }
        ImGui::EndChild();

        m_tree.RunAction();
    }
    MuEditor::ToolWindowFrame::End();
}

void CMuColorsUI::Show(ColorsRoot root)
{
    g_MuEditorCore.GetToolWindows().colors = true;
    m_tree.SetPick(static_cast<std::size_t>(root), {});
    m_tree.OpenRoot(static_cast<std::size_t>(root));
    ImGui::SetWindowFocus(MuEditor::WindowIds::Colors);
}

NamedColorSource& CMuColorsUI::SourceOf(ColorsRoot root)
{
    switch (root)
    {
    case ColorsRoot::GroundLabels:
        return m_groundLabels;
    case ColorsRoot::ServerList:
        return m_serverList;
    case ColorsRoot::Glow:
    default:
        return m_glow;
    }
}

NamedColorSource& CMuColorsUI::PickedSource()
{
    return SourceOf(GetShownRoot());
}

void CMuColorsUI::RenderToolbar()
{
    const MuEditor::NamedTreeTexts texts = TreeTexts();
    NamedColorSource& source = PickedSource();
    const std::string& color = GetPickedColor();
    m_tree.RenderNewFolderButton(I18N::Editor::NewFolder);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::NewColor))
    {
        m_newColorRoot = GetShownRoot();
        m_newColorFolder = GetPickedFolder();
        source.OpenNew(color);
    }
    ImGui::SameLine();
    const bool folderPicked = m_tree.IsFolderPicked();
    ImGui::BeginDisabled(!folderPicked);
    if (ImGui::Button(I18N::Editor::Rename))
    {
        m_tree.OpenRenameFolder();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // D26 for a color: only one nothing uses goes. A folder goes with what it holds moving up.
    const bool colorRemovable =
        !color.empty() && source.Has(color) && source.CountUsers(color) == 0 && !source.IsKept(color);
    ImGui::BeginDisabled(!folderPicked && !colorRemovable);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        m_tree.Later(
            [this, texts]
            {
                if (GetPickedColor().empty())
                {
                    m_tree.RemovePickedFolder(texts);
                    return;
                }
                RemovePickedColor();
            });
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_tree.CanMovePicked());
    if (ImGui::Button(I18N::Editor::MoveUp))
    {
        m_tree.Later([this, texts] { m_tree.MovePicked(texts, -1); });
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::MoveDown))
    {
        m_tree.Later([this, texts] { m_tree.MovePicked(texts, 1); });
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
        m_tree.PickEntry(static_cast<std::size_t>(root), *created);
        m_newColorRoot.reset();
    }
}

void CMuColorsUI::RenderPicked()
{
    NamedColorSource& source = PickedSource();
    const NamedColorTexts texts = source.GetTexts();
    const MuEditor::NamedTree::Pick& pick = m_tree.GetPick();
    if (!pick.entry.empty())
    {
        const std::string after = m_panel.RenderEntry(source, pick.entry);
        if (after != pick.entry)
        {
            m_tree.SetPick(pick.root, pick.folder, after);
        }
        return;
    }
    if (!pick.folder.empty())
    {
        m_tree.RenderPickedFolder(TreeTexts(), I18N::Editor::FolderNote);
        return;
    }
    ImGui::TextUnformatted(texts.root);
    MuEditor::Colors::RenderColorNote(texts.note);
    m_panel.RenderUnused(source);
}

void CMuColorsUI::RemovePickedColor()
{
    NamedColorSource& source = PickedSource();
    const MuEditor::NamedTree::Pick pick = m_tree.GetPick();
    const std::string names[] = {pick.entry};
    source.Remove(source.GetTexts().removeOne, names);
    m_tree.SetPick(pick.root, pick.folder);
}

#endif // _EDITOR
