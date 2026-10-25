#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterModelFiles.h"

#include "../MuEditor/UI/Common/TextSearch.h"

#include <algorithm>
#include <set>
#include <system_error>
#include <utility>

namespace MuEditor::Monsters
{
namespace
{
constexpr std::string_view ModelExtension = ".bmd";
// Searched with their subfolders.
constexpr const char* ModelFolders[] = {"Data/Monster", "Data/NPC"};

bool IsModelFile(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(
        extension.begin(), extension.end(), extension.begin(), [](char character)
        { return character >= 'A' && character <= 'Z' ? static_cast<char>(character - 'A' + 'a') : character; });
    return extension == ModelExtension;
}

// The model files write paths with '/', relative to the client folder.
std::string ToListPath(const std::filesystem::path& clientFolder, const std::filesystem::path& file)
{
    return file.lexically_relative(clientFolder).generic_string();
}

void AddFolder(const std::filesystem::path& clientFolder, const std::filesystem::path& folder,
               std::set<std::string>& files)
{
    std::error_code error;
    for (auto entry = std::filesystem::directory_iterator(clientFolder / folder, error);
         !error && entry != std::filesystem::directory_iterator(); entry.increment(error))
    {
        if (entry->is_regular_file(error) && IsModelFile(entry->path()))
        {
            files.insert(ToListPath(clientFolder, entry->path()));
        }
    }
}

void AddFolderWithSubfolders(const std::filesystem::path& clientFolder, const char* folder,
                             std::set<std::string>& files)
{
    std::error_code error;
    for (auto entry = std::filesystem::recursive_directory_iterator(clientFolder / folder, error);
         !error && entry != std::filesystem::recursive_directory_iterator(); entry.increment(error))
    {
        if (entry->is_regular_file(error) && IsModelFile(entry->path()))
        {
            files.insert(ToListPath(clientFolder, entry->path()));
        }
    }
}
} // namespace

MonsterModelFiles::MonsterModelFiles(std::filesystem::path clientFolder) : m_clientFolder(std::move(clientFolder)) {}

const std::vector<std::string>& MonsterModelFiles::GetFiles(std::span<const std::string> listFiles)
{
    if (!m_read)
    {
        std::set<std::string> files;
        for (const char* folder : ModelFolders)
        {
            AddFolderWithSubfolders(m_clientFolder, folder, files);
        }
        std::set<std::string> otherFolders;
        for (const std::string& file : listFiles)
        {
            otherFolders.insert(std::filesystem::path(file).parent_path().generic_string());
        }
        for (const std::string& folder : otherFolders)
        {
            AddFolder(m_clientFolder, folder, files);
        }
        m_files.assign(files.begin(), files.end());
        m_read = true;
    }
    return m_files;
}

void MonsterModelFiles::Refresh()
{
    m_read = false;
}

std::vector<std::string_view> MonsterModelFiles::Find(std::span<const std::string> listFiles, std::string_view search)
{
    std::vector<std::string_view> found;
    for (const std::string& file : GetFiles(listFiles))
    {
        if (TextSearch::ContainsIgnoringCase(file, search))
        {
            found.emplace_back(file);
        }
    }
    return found;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
