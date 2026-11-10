#pragma once

#ifdef _EDITOR

#include "Data/GameData/ColorFolders/ColorFolder.h"

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace MuEditor
{
// A root of a NamedTree: the named entries of one kind and the folders the user sorts them in (ColorFolders.h).
class NamedTreeRoot
{
public:
    virtual ~NamedTreeRoot() = default;

    // The root's name in the tree, read each time it is drawn, so it follows the editor language.
    virtual const char* GetRootName() const = 0;
    // Counts the changes of the entries and the folders, for what the tree keeps between frames.
    virtual int GetVersion() const = 0;
    // The entries, in the file's order.
    virtual std::size_t GetCount() const = 0;
    virtual const std::string& GetName(std::size_t index) const = 0;
    virtual bool Has(const std::string& name) const = 0;
    // Whether a swatch is drawn in front of each entry's row (a color's), and the swatch.
    virtual bool HasSwatches() const
    {
        return false;
    }
    virtual void RenderSwatch(const std::string& name) const;
    // The text of an entry's row: its name and how many use it.
    virtual std::string GetRowLabel(const std::string& name) = 0;

    // The folders the entries are sorted in, and new ones as one undo step named `step`.
    virtual const std::vector<Data::Colors::ColorFolder>& GetFolders() const = 0;
    virtual void SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders) = 0;
    // Whether the root keeps folders: one whose file can't hold them has none and takes no drops.
    virtual bool HasFolders() const
    {
        return true;
    }
    // Whether the entries no folder holds keep the order the user gives them (SetOrder); otherwise they stay sorted
    // and move only in folders.
    virtual bool CanReorder() const
    {
        return true;
    }
    // The entries in this order (the same names, another order), as one undo step named `step`.
    virtual void SetOrder(const char* step, std::span<const std::string> names) = 0;
    // Whether the entry can go into a folder and move; one that can't stays under the root.
    virtual bool CanMoveEntry(const std::string& name) const
    {
        static_cast<void>(name);
        return true;
    }
};

// The entries' names in the file's order.
std::vector<std::string> GetNames(const NamedTreeRoot& root);
} // namespace MuEditor

#endif // _EDITOR
