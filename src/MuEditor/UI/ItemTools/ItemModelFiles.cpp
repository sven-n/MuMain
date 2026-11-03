#include "stdafx.h"

#ifdef _EDITOR

#include "ItemModelFiles.h"

#include "../MuEditor/UI/Common/TextSearch.h"

#include <algorithm>
#include <system_error>
#include <utility>

namespace MuEditor::Items
{
namespace
{
constexpr std::string_view ModelExtension = ".bmd";
constexpr std::string_view DataFolder = "Data/";

bool IsModelFile(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(
        extension.begin(), extension.end(), extension.begin(), [](char character)
        { return character >= 'A' && character <= 'Z' ? static_cast<char>(character - 'A' + 'a') : character; });
    return extension == ModelExtension;
}

void AddFiles(const std::filesystem::path& clientFolder, const char* folder, std::vector<std::string>& files)
{
    const std::filesystem::path root = clientFolder / "Data" / folder;
    std::error_code error;
    for (auto entry = std::filesystem::recursive_directory_iterator(root, error);
         !error && entry != std::filesystem::recursive_directory_iterator(); entry.increment(error))
    {
        if (entry->is_regular_file(error) && IsModelFile(entry->path()))
        {
            // The model files write paths with '/', relative to the client folder.
            files.push_back(entry->path().lexically_relative(clientFolder).generic_string());
        }
    }
}
} // namespace

ItemModelFiles::ItemModelFiles(std::filesystem::path clientFolder) : m_clientFolder(std::move(clientFolder)) {}

const std::vector<std::string>& ItemModelFiles::GetFiles()
{
    if (!m_read)
    {
        m_files.clear();
        AddFiles(m_clientFolder, "Item", m_files);
        AddFiles(m_clientFolder, "Player", m_files);
        std::ranges::sort(m_files);
        m_read = true;
    }
    return m_files;
}

void ItemModelFiles::Refresh()
{
    m_read = false;
}

std::vector<std::string_view> ItemModelFiles::Find(std::string_view search)
{
    std::vector<std::string_view> found;
    for (const std::string& file : GetFiles())
    {
        if (TextSearch::ContainsIgnoringCase(file, search))
        {
            found.emplace_back(file);
        }
    }
    return found;
}

std::string ItemModelFiles::GetTextureFolder(std::string_view file)
{
    if (file.starts_with(DataFolder))
    {
        file.remove_prefix(DataFolder.size());
    }
    const size_t slash = file.rfind('/');
    return slash == std::string_view::npos ? std::string() : std::string(file.substr(0, slash));
}

std::vector<std::string> ItemModelFiles::GetTextureFoldersForFile(const std::vector<std::string>& folders,
                                                                  std::string_view oldFile, std::string_view newFile)
{
    const bool followsFile = folders.empty() || (folders.size() == 1 && folders.front() == GetTextureFolder(oldFile));
    return followsFile ? std::vector<std::string>{GetTextureFolder(newFile)} : folders;
}
} // namespace MuEditor::Items

#endif // _EDITOR
