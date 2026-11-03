#pragma once

#include <cstddef>
#include <string>
#include <vector>

// A folder MuEditor sorts a file's named colors in (D69): under each use's root in the Colors window, also inside
// other folders. ColorFolders.h reads, writes and edits them.
namespace Data::Colors
{
struct ColorFolder
{
    std::string name;
    // In the order the tree shows them, before the colors.
    std::vector<ColorFolder> folders;
    // Names of the file's colors, in the order the tree shows them.
    std::vector<std::string> colors;

    bool operator==(const ColorFolder&) const = default;
};

// The folder's place in the tree: its index among the root's folders, then among that folder's folders, and so
// on. Empty is the root itself.
using ColorFolderPath = std::vector<std::size_t>;
} // namespace Data::Colors
