#include "stdafx.h"

#include "DataPaths.h"

#include "Core/Text/Utf8.h"

#include <algorithm>

namespace Data::Paths
{
namespace
{
constexpr char FolderSeparator = '/';
constexpr char WindowsFolderSeparator = '\\';
constexpr wchar_t LoaderFolderSeparator = L'\\';

constexpr std::string_view ParentFolder = "..";
constexpr char DriveSeparator = ':';
} // namespace

std::wstring ToLoaderPath(const std::string& path)
{
    std::wstring loaderPath = Core::Text::FromUtf8(path);
    std::replace(loaderPath.begin(), loaderPath.end(), static_cast<wchar_t>(FolderSeparator), LoaderFolderSeparator);
    return loaderPath;
}

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
