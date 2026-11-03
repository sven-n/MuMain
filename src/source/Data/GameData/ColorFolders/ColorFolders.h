#pragma once

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The folders MuEditor sorts a file's named colors in (D69). The Colors window shows a tree with a root for each
// use (the glow colors, the server list's colors, ...); under a root the user makes folders, also inside folders.
// Each file keeps its folders next to its colors:
//
//   "colorFolders": [
//     {"name": "Load", "folders": [{"name": "Warnings", "colors": ["loadRed"]}], "colors": ["loadGreen"]}
//   ]
//
// A color sits in at most one folder; a color in none sits under the root. Every build reads and checks the key,
// so a file reports the same problems in the game as in MuEditor; only MuEditor uses the folders.
namespace Data::Colors
{
inline constexpr const char* ColorFoldersKey = "colorFolders";
// How deep folders go: a folder in a folder in a folder ... at most this many.
inline constexpr std::size_t MaxColorFolderDepth = 8;
// A folder's name in UTF-16 units.
inline constexpr std::size_t MaxColorFolderNameLength = 32;

// Whether the name can be a folder's: not empty, valid UTF-8, no space at either end, no control character, and not
// longer than MaxColorFolderNameLength.
bool IsColorFolderName(std::string_view name);

// Reads the "colorFolders" of a file's root object; none without the key. Problems are warnings and leave out
// what they name: a folder without a usable name or with a sibling's name, one deeper than MaxColorFolderDepth,
// a color that isn't one of the file's (`isColor`) or that an earlier folder holds already.
std::vector<ColorFolder> ReadColorFolders(const Items::Json::OrderedJson& root, const std::string& source,
                                          const std::function<bool(std::string_view)>& isColor,
                                          std::vector<Items::ItemDataIssue>& issues);

// The "colorFolders" value for the folders, with only the colors `isColor` knows (a folder can hold the name of a
// color that is gone: an undo puts it back). A folder writes "folders" and "colors" only when it has some.
Items::Json::OrderedJson WriteColorFolders(std::span<const ColorFolder> folders,
                                           const std::function<bool(std::string_view)>& isColor);

// Puts each folder of the "colorFolders" list on a line of its own, with the folders in it.
std::string PutColorFoldersOnLines(const std::string& text);

// The folder at the path, or nullptr; nullptr also for the empty path (the root has no ColorFolder).
ColorFolder* FindColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> path);
const ColorFolder* FindColorFolder(const std::vector<ColorFolder>& folders, std::span<const std::size_t> path);

// The names of the folders down to the one at the path; none for the root or a path to no folder. Siblings' names
// differ, so the names find the folder again after the folders moved (FindColorFolderByNames).
std::vector<std::string> GetColorFolderNames(const std::vector<ColorFolder>& folders,
                                             std::span<const std::size_t> path);
// The path the names lead to (the empty path for none: the root); nullopt when a name isn't found on the way.
std::optional<ColorFolderPath> FindColorFolderByNames(const std::vector<ColorFolder>& folders,
                                                      std::span<const std::string> names);

// The path of the folder that holds the color; nullopt when none does (it sits under the root).
std::optional<ColorFolderPath> FindFolderOfColor(const std::vector<ColorFolder>& folders, std::string_view color);

// The colors no folder holds, in the order of `colors` (the file's order).
std::vector<std::string> GetRootColors(const std::vector<ColorFolder>& folders, std::span<const std::string> colors);

// The edits of the Colors window. Each returns false and changes nothing when it can't be made.

// Adds a folder at the end of the parent's folders (the empty path: the root). Refused for a name that isn't
// IsColorFolderName, that a sibling has, or below MaxColorFolderDepth.
bool AddColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> parent, const std::string& name);
// Refused like AddColorFolder; renaming to its own name changes nothing.
bool RenameColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> path, const std::string& name);
// Removes the folder. Its folders and colors move up into its parent, after the parent's own; a folder of the
// same name as one there is joined with it.
bool RemoveColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> path);
// Moves the folder to the end of another folder's folders (the empty path: the root). Refused into itself or a
// folder in it, next to a folder of its name, or when the folders in it would go below MaxColorFolderDepth.
bool MoveColorFolderInto(std::vector<ColorFolder>& folders, std::span<const std::size_t> path,
                         std::span<const std::size_t> parent);
// Where the folder at `parent` is after MoveColorFolderInto took the folder at `path` out of its list: one place up
// when it came after it in the same list.
ColorFolderPath GetPathAfterMove(std::span<const std::size_t> path, std::span<const std::size_t> parent);
// Moves the folder up (-1) or down (+1) among its siblings.
bool MoveColorFolderBy(std::vector<ColorFolder>& folders, std::span<const std::size_t> path, int delta);
// Puts the color at the end of the folder's colors (the empty path: under the root), out of the folder it was in.
bool MoveColorInto(std::vector<ColorFolder>& folders, std::string_view color, std::span<const std::size_t> folder);
// Moves the color up (-1) or down (+1) among the colors of its folder; false for a color in none.
bool MoveColorInFolderBy(std::vector<ColorFolder>& folders, std::string_view color, int delta);
// The file's order of `colors` with a color no folder holds moved up (-1) or down (+1) past the next such color, as
// the tree shows them under the root; nothing when it can't move that way.
std::optional<std::vector<std::string>> MoveRootColorBy(const std::vector<ColorFolder>& folders,
                                                        std::span<const std::string> colors, std::string_view color,
                                                        int delta);

// Keep the folders in step with the colors.
// Takes the color out of its folder; false when none held it.
bool RemoveColorFromFolders(std::vector<ColorFolder>& folders, std::string_view color);
// The folder that held `from` holds `to` in its place.
void RenameColorInFolders(std::vector<ColorFolder>& folders, std::string_view from, const std::string& to);
} // namespace Data::Colors
