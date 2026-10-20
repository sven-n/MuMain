#include "TexturePackFiles.h"

#include "Core/Platform/WinCompat.h"
#include "Render/Textures/AssetPath.h"

#include <algorithm>
#include <optional>
#include <system_error>

namespace Render::Textures
{
namespace
{
char ToLowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

std::string MakeKey(std::string path)
{
    std::replace(path.begin(), path.end(), '\\', '/');
    std::transform(path.begin(), path.end(), path.begin(), ToLowerAscii);
    return path;
}

std::string GenericUtf8(const std::filesystem::path& path)
{
    const std::u8string utf8 = path.generic_u8string();
    return std::string(utf8.begin(), utf8.end());
}

bool IsTextureFile(const std::filesystem::path& path)
{
    const std::string extension = MakeKey(GenericUtf8(path.extension()));
    return extension == ".ozj" || extension == ".ozt";
}
} // namespace

TexturePackFiles TexturePackFiles::List(const std::filesystem::path& root)
{
    TexturePackFiles files;
    std::error_code error;
    const auto options = std::filesystem::directory_options::skip_permission_denied;
    for (std::filesystem::recursive_directory_iterator it(root, options, error), end; !error && it != end;
         it.increment(error))
    {
        std::error_code typeError;
        if (!it->is_regular_file(typeError) || !IsTextureFile(it->path()))
            continue;

        const std::filesystem::path relative = it->path().lexically_relative(root);
        files.m_files.emplace(MakeKey(GenericUtf8(relative)), it->path());
    }
    return files;
}

const std::filesystem::path* TexturePackFiles::Find(std::wstring_view dataPath) const
{
    if (m_files.empty())
        return nullptr;

    const std::optional<std::wstring_view> inData = AssetPath::InDataFolder(dataPath);
    if (!inData)
        return nullptr;

    const auto found = m_files.find(MakeKey(mu_wchar_to_utf8(std::wstring(*inData).c_str())));
    return found == m_files.end() ? nullptr : &found->second;
}
} // namespace Render::Textures
