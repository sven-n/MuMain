#include "TexturePack.h"

#include <algorithm>
#include <cwctype>

namespace Render::Textures::TexturePack
{
namespace
{
constexpr std::wstring_view DataFolder = L"data";

bool IsSeparator(wchar_t c)
{
    return c == L'\\' || c == L'/';
}

bool IsNameCharacter(wchar_t c)
{
    const bool letter = (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z');
    const bool digit = c >= L'0' && c <= L'9';
    return letter || digit || c == L'-' || c == L'_';
}

bool StartsWithDataFolder(std::wstring_view path)
{
    if (path.size() <= DataFolder.size() || !IsSeparator(path[DataFolder.size()]))
        return false;

    for (std::size_t i = 0; i < DataFolder.size(); ++i)
    {
        if (static_cast<wchar_t>(std::towlower(path[i])) != DataFolder[i])
            return false;
    }
    return true;
}

// Copied from the native spelling one character at a time: a valid name is
// ASCII, and any other name fails IsValidName however it widens.
std::optional<std::wstring> PackName(const std::filesystem::directory_entry& entry)
{
    std::error_code error;
    if (!entry.is_directory(error))
        return std::nullopt;

    const std::filesystem::path folderName = entry.path().filename();
    const auto& native = folderName.native();
    std::wstring name(native.begin(), native.end());
    if (!IsValidName(name))
        return std::nullopt;
    return name;
}
} // namespace

bool IsValidName(std::wstring_view name)
{
    return !name.empty() && std::all_of(name.begin(), name.end(), IsNameCharacter);
}

std::optional<std::wstring> PathInPack(std::wstring_view path, std::wstring_view pack)
{
    if (!IsValidName(pack) || !StartsWithDataFolder(path))
        return std::nullopt;

    std::wstring packPath(PacksFolder);
    packPath += L'\\';
    packPath += pack;
    packPath += path.substr(DataFolder.size());
    return packPath;
}

std::vector<std::wstring> ListPacks(const std::filesystem::path& folder)
{
    std::vector<std::wstring> packs;
    std::error_code error;
    for (std::filesystem::directory_iterator it(folder, error), end; !error && it != end; it.increment(error))
    {
        if (auto name = PackName(*it))
            packs.push_back(std::move(*name));
    }
    std::sort(packs.begin(), packs.end());
    return packs;
}

std::vector<std::wstring> ListInstalledPacks()
{
    // Forward slashes, which std::filesystem takes on every platform.
    std::wstring folder(PacksFolder);
    std::replace(folder.begin(), folder.end(), L'\\', L'/');
    return ListPacks(folder);
}
} // namespace Render::Textures::TexturePack
