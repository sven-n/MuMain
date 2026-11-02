#pragma once

#ifdef _EDITOR

#include "GlowColorsRoot.h"
#include "GroundLabelStylesRoot.h"
#include "NamedColorList.h"
#include "ServerListColorsRoot.h"

#include "Data/GameData/ColorFolders/ColorFolder.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

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
        return m_pick.root;
    }
    // The color the tree has picked; "" while a root or a folder is.
    const std::string& GetPickedColor() const
    {
        return m_pick.color;
    }
    // The folder a new folder or color goes into: the picked folder, or the one of the picked color.
    Data::Colors::ColorFolderPath GetPickedFolder() const;

private:
    // What the tree has picked: a root, one of its folders (by path), or one of its colors.
    struct Pick
    {
        MuEditor::Colors::ColorsRoot root = MuEditor::Colors::ColorsRoot::Glow;
        Data::Colors::ColorFolderPath folder;
        std::string color;
        // The names of the folders down to `folder`, which find it again after the folders moved.
        std::vector<std::string> folderNames;
    };
    enum class FolderDialog
    {
        None,
        New,
        Rename,
    };
    // The colors of a root that no folder holds, as of the root's version.
    struct RootColors
    {
        int version = -1;
        std::vector<std::string> names;
    };
    // Whether the folder dialog's name can be used, as last tried.
    struct FolderTrial
    {
        MuEditor::Colors::ColorsRoot root = MuEditor::Colors::ColorsRoot::Glow;
        int version = -1;
        bool renaming = false;
        Data::Colors::ColorFolderPath folder;
        std::string name;
        bool usable = false;
    };

    CMuColorsUI() = default;

    MuEditor::Colors::NamedColorSource& SourceOf(MuEditor::Colors::ColorsRoot root);
    // Picks the root's folder at `folder` (the empty path: the root), or the color in it.
    void SetPick(MuEditor::Colors::ColorsRoot root, Data::Colors::ColorFolderPath folder, std::string color = {});
    // The pick stays on what is still there: a color by its name (in whatever folder it is now), a folder by its
    // names (also after an undo moved it), or by its path when it was renamed there.
    void KeepPickValid();
    void RenderToolbar();
    void RenderNewEntries();
    void RenderFolderDialog();
    void RenderTree();
    void RenderRoot(MuEditor::Colors::ColorsRoot root);
    // Data::Colors::GetRootColors of the root, worked out again only after the root changed.
    const std::vector<std::string>& GetRootColors(MuEditor::Colors::ColorsRoot root);
    void RenderFolders(MuEditor::Colors::ColorsRoot root, const std::vector<Data::Colors::ColorFolder>& folders,
                       Data::Colors::ColorFolderPath& path);
    void RenderColor(MuEditor::Colors::ColorsRoot root, const std::string& name,
                     const Data::Colors::ColorFolderPath& folder);
    void RenderPicked();
    // Drag and drop: a color or a folder dragged onto a folder (or the root, the empty path) of its own root.
    void BeginDrag(MuEditor::Colors::ColorsRoot root, const Data::Colors::ColorFolderPath& folder,
                   const std::string& color, const char* label);
    void AcceptDrop(MuEditor::Colors::ColorsRoot root, const Data::Colors::ColorFolderPath& folder);
    // Changes the root's folders with `edit` (false: nothing changed) as one undo step named `step`.
    void ChangeFolders(MuEditor::Colors::ColorsRoot root, const char* step,
                       const std::function<bool(std::vector<Data::Colors::ColorFolder>&)>& edit);
    void MovePicked(int delta);
    void RemovePicked();

    Pick m_pick;
    // Show's root, opened in the tree on the next frame.
    std::optional<MuEditor::Colors::ColorsRoot> m_openRoot;
    // A folder opened in the tree on the next frame: a renamed one that was open.
    std::optional<Data::Colors::ColorFolderPath> m_openFolder;
    // Whether the picked folder's node was open when the tree was last drawn.
    bool m_pickedFolderOpen = false;
    // What a button or a drop asked for, done after the tree is drawn.
    std::function<void()> m_action;
    FolderDialog m_folderDialog = FolderDialog::None;
    std::string m_folderName;
    FolderTrial m_folderTrial;
    std::map<MuEditor::Colors::ColorsRoot, RootColors> m_rootColors;
    // The root whose new color dialog is open, and the folder its new color goes into.
    std::optional<MuEditor::Colors::ColorsRoot> m_newColorRoot;
    Data::Colors::ColorFolderPath m_newColorFolder;

    MuEditor::Colors::GlowColorsRoot m_glow;
    MuEditor::Colors::GroundLabelStylesRoot m_groundLabels;
    MuEditor::Colors::ServerListColorsRoot m_serverList;
    MuEditor::Colors::NamedColorList m_panel;
};

#define g_MuColorsUI CMuColorsUI::GetInstance()

#endif // _EDITOR
