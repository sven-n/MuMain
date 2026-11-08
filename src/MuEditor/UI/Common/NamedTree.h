#pragma once

#ifdef _EDITOR

#include "NamedTreeRoot.h"

#include "Data/GameData/ColorFolders/ColorFolder.h"

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace MuEditor
{
// The texts of a tree that name its entries ("color", "look"), read each time they are used.
struct NamedTreeTexts
{
    // The undo steps.
    const char* newFolder = "";
    const char* renameFolder = "";
    const char* removeFolder = "";
    const char* moveFolder = "";
    const char* moveEntry = "";
    // "It holds {0} colors and {1} folders.", {0} the entries and {1} the folders in the picked one.
    const char* folderHolds = "";
};

// The tree of a window of named entries in folders (D69, design L31): a root for each kind, the folders the user
// makes under it, also in folders, and the entries no folder holds. A click picks a root, a folder or an entry, the
// arrow keys move the pick, and an entry or a folder is dragged onto a folder or its root; it stays in its root. The
// window draws its toolbar with the tree's folder buttons and what its entries need.
class NamedTree
{
public:
    // What the tree has picked: a root, one of its folders (by path), or one of its entries.
    struct Pick
    {
        std::size_t root = 0;
        Data::Colors::ColorFolderPath folder;
        std::string entry;
        // The names of the folders down to `folder`, which find it again after the folders moved.
        std::vector<std::string> folderNames;
    };

    // `roots` in the order the tree shows them; `dragType` names the tree's drags (ImGui's payload type).
    NamedTree(std::vector<NamedTreeRoot*> roots, const char* dragType, const char* folderPopupId);

    [[nodiscard]] const Pick& GetPick() const
    {
        return m_pick;
    }
    [[nodiscard]] NamedTreeRoot& GetPickedRoot() const
    {
        return *m_roots[m_pick.root];
    }
    [[nodiscard]] std::span<NamedTreeRoot* const> GetRoots() const
    {
        return m_roots;
    }
    // Picks the root's folder at `folder` (the empty path: the root), or the entry in it.
    void SetPick(std::size_t root, Data::Colors::ColorFolderPath folder, std::string entry = {});
    // Picks the entry of the root, in whatever folder holds it.
    void PickEntry(std::size_t root, const std::string& entry);
    // The pick stays on what is still there: an entry by its name (in whatever folder it is now), a folder by its
    // names (also after an undo moved it), or by its path when it was renamed there. Called before the window draws.
    void KeepPickValid();
    // Opens the root in the tree on the next frame and shows it.
    void OpenRoot(std::size_t root)
    {
        m_openRoot = root;
    }
    // A folder is picked, not a root or an entry.
    [[nodiscard]] bool IsFolderPicked() const
    {
        return m_pick.entry.empty() && !m_pick.folder.empty();
    }

    // The toolbar's New folder: off for a root without folders, and below the deepest level.
    void RenderNewFolderButton(const char* label);
    // Opens the folder dialog renaming the picked folder.
    void OpenRenameFolder();
    // The dialog of New folder and Rename; in the ID scope of the toolbar, which opened it. `note` says what a name
    // can be.
    void RenderFolderDialog(const NamedTreeTexts& texts, const char* newTitle, const char* renameTitle,
                            const char* nameLabel, const char* note, const char* ok, const char* cancel);
    // Whether the picked folder or entry can move up or down; MovePicked moves it.
    [[nodiscard]] bool CanMovePicked() const;
    void MovePicked(const NamedTreeTexts& texts, int delta);
    // Removes the picked folder: what it holds moves up into its parent.
    void RemovePickedFolder(const NamedTreeTexts& texts);

    // The tree, in the window's tree pane.
    void Render(const NamedTreeTexts& texts);
    // The picked folder's name and what it holds, with `note` below.
    void RenderPickedFolder(const NamedTreeTexts& texts, const char* note) const;

    // Runs what a button or a drop asked for, after the window drew the tree; Later asks for it.
    void RunAction();
    void Later(std::function<void()> action)
    {
        m_action = std::move(action);
    }

private:
    enum class FolderDialog
    {
        None,
        New,
        Rename,
    };
    // The entries of a root that no folder holds, as of the root's version.
    struct RootEntries
    {
        int version = -1;
        std::vector<std::string> names;
    };
    // Whether the folder dialog's name can be used, as last tried.
    struct FolderTrial
    {
        std::size_t root = 0;
        int version = -1;
        bool renaming = false;
        Data::Colors::ColorFolderPath folder;
        std::string name;
        bool usable = false;
    };

    void RenderRoot(std::size_t root, const NamedTreeTexts& texts);
    const std::vector<std::string>& GetRootEntries(std::size_t root);
    void RenderFolders(std::size_t root, const NamedTreeTexts& texts,
                       const std::vector<Data::Colors::ColorFolder>& folders, Data::Colors::ColorFolderPath& path);
    void RenderEntry(std::size_t root, const std::string& name, const Data::Colors::ColorFolderPath& folder);
    void BeginDrag(std::size_t root, const Data::Colors::ColorFolderPath& folder, const std::string& entry,
                   const char* label);
    void AcceptDrop(std::size_t root, const Data::Colors::ColorFolderPath& folder, const NamedTreeTexts& texts);
    // Changes the root's folders with `edit` (false: nothing changed) as one undo step named `step`.
    void ChangeFolders(std::size_t root, const char* step,
                       const std::function<bool(std::vector<Data::Colors::ColorFolder>&)>& edit);

    std::vector<NamedTreeRoot*> m_roots;
    const char* m_dragType;
    const char* m_folderPopupId;
    Pick m_pick;
    // OpenRoot's root, opened in the tree on the next frame.
    std::optional<std::size_t> m_openRoot;
    // A folder opened in the tree on the next frame: a renamed one that was open.
    std::optional<Data::Colors::ColorFolderPath> m_openFolder;
    // Whether the picked folder's node was open when the tree was last drawn.
    bool m_pickedFolderOpen = false;
    std::function<void()> m_action;
    FolderDialog m_folderDialog = FolderDialog::None;
    std::string m_folderName;
    FolderTrial m_folderTrial;
    std::map<std::size_t, RootEntries> m_rootEntries;
};
} // namespace MuEditor

#endif // _EDITOR
