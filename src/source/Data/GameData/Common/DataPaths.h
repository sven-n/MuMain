#pragma once

#include <string>
#include <string_view>

namespace Data::Paths
{
// Paths in the data files stay inside the game folder: relative, with '/'
// between folders, without ".." and without empty folder names. Returns what
// is wrong with a non-empty path, or an empty text when it is fine.
std::string FindPathProblem(std::string_view path);
} // namespace Data::Paths
