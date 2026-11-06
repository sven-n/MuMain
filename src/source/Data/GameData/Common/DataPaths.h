#pragma once

#include <string>
#include <string_view>

namespace Data::Paths
{
// Paths in the data files stay inside the game folder: relative, with '/'
// between folders, without ".." and without empty folder names. Returns what
// is wrong with a non-empty path, or an empty text when it is fine.
std::string FindPathProblem(std::string_view path);

// A path of the data files ("Data/Sound/a.wav") as the game's loaders take it
// ("Data\Sound\a.wav"): wide, with '\' between folders.
std::wstring ToLoaderPath(const std::string& path);
} // namespace Data::Paths
