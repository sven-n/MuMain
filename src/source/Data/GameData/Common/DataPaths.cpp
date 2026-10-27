#include "stdafx.h"

#include "DataPaths.h"

#include <algorithm>

namespace Data::Paths
{
namespace
{
constexpr char FolderSeparator = '/';
constexpr char WindowsFolderSeparator = '\\';

constexpr std::string_view ParentFolder = "..";
constexpr char DriveSeparator = ':';
} // namespace

std::string FindPathProblem(std::string_view path)
{
    if (path.find(WindowsFolderSeparator) != std::string_view::npos)
    {
        return "must use / between folders";
    }
    if (path.front() == FolderSeparator || path.find(DriveSeparator) != std::string_view::npos)
    {
        return "must be a relative path";
    }

    size_t partStart = 0;
    while (partStart <= path.size())
    {
        const size_t partEnd = std::min(path.find(FolderSeparator, partStart), path.size());
        const std::string_view part = path.substr(partStart, partEnd - partStart);
        if (part.empty())
        {
            return "must not have empty folder names or end with /";
        }
        if (part == ParentFolder)
        {
            return "must not contain \"..\"";
        }
        partStart = partEnd + 1;
    }
    return {};
}
} // namespace Data::Paths
