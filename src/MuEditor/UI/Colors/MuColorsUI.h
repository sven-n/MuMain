#pragma once

#ifdef _EDITOR

#include "GlowColorsRoot.h"
#include "GroundLabelStylesRoot.h"
#include "NamedColorList.h"
#include "ServerListColorsRoot.h"

#include "../MuEditor/UI/Common/NamedTree.h"
#include "Data/GameData/ColorFolders/ColorFolder.h"

#include <optional>
#include <string>

namespace MuEditor::Colors
{
// The roots of the Colors window's tree, a use each, in the order the tree shows them.
enum class ColorsRoot
{
    Glow,
    GroundLabels,
    ServerList,
};
} // namespace MuEditor::Colors

// The Colors window (D69): the editor's named colors as a tree. A root for each use (the glow colors, the ground label
// styles, the server list's colors) holds the folders the user sorts its colors in, also folders in folders, and the
// colors no folder holds; a color moves only within its root. The picked color is drawn with the shared panel
// (NamedColorList). Windows > Colors opens it on what was picked last, the entries of the item tools (Items > Glow
// Colors, Items > Ground Label Styles) and the Server List window's Edit colors on their root. It docks with the item
// tools; each root's changes are steps of its own history (the item edits, the Server List window's).
class CMuColorsUI
{
public:
    static CMuColorsUI& GetInstance();

    void Render(bool* open);

    // Opens the window with the root picked and open, and brings it to the front.
    void Show(MuEditor::Colors::ColorsRoot root);
    // The root of what the tree has picked, also while the window is closed.
    MuEditor::Colors::ColorsRoot GetShownRoot() const
    {
        return static_cast<MuEditor::Colors::ColorsRoot>(m_tree.GetPick().root);
    }
    // The color the tree has picked; "" while a root or a folder is.
    const std::string& GetPickedColor() const
    {
        return m_tree.GetPick().entry;
    }
    // The folder a new folder or color goes into: the picked folder, or the one of the picked color.
    Data::Colors::ColorFolderPath GetPickedFolder() const
    {
        return m_tree.GetPick().folder;
    }

private:
    CMuColorsUI();

    MuEditor::Colors::NamedColorSource& SourceOf(MuEditor::Colors::ColorsRoot root);
    MuEditor::Colors::NamedColorSource& PickedSource();
    void RenderToolbar();
    void RenderNewEntries();
    void RenderPicked();
    void RemovePickedColor();

    MuEditor::Colors::GlowColorsRoot m_glow;
    MuEditor::Colors::GroundLabelStylesRoot m_groundLabels;
    MuEditor::Colors::ServerListColorsRoot m_serverList;
    // The roots in the order of ColorsRoot.
    MuEditor::NamedTree m_tree;
    MuEditor::Colors::NamedColorList m_panel;
    // The root whose new color dialog is open, and the folder its new color goes into.
    std::optional<MuEditor::Colors::ColorsRoot> m_newColorRoot;
    Data::Colors::ColorFolderPath m_newColorFolder;
};

#define g_MuColorsUI CMuColorsUI::GetInstance()

#endif // _EDITOR
