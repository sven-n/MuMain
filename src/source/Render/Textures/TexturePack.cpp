#include "TexturePack.h"

#include "Render/Sprites/GlobalBitmapPath.h"
#include "Render/Textures/AssetPath.h"

#include <algorithm>
#include <optional>

namespace Render::Textures::TexturePack
{
namespace
{
bool IsNameCharacter(wchar_t c)
{
    const bool letter = (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z');
    const bool digit = c >= L'0' && c <= L'9';
    return letter || digit || c == L'-' || c == L'_';
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

std::wstring PackFolder(std::wstring_view pack)
{
    std::wstring folder(PacksFolder);
    folder += L'\\';
    folder += pack;
    return folder;
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
    // Resolved as the loader resolves the pack folder, so on a case-sensitive
    // filesystem a "data/texturepacks" folder is listed as well as loaded.
    return ListPacks(Render::Sprites::NarrowBitmapPath(std::wstring(PacksFolder)));
}

bool SameName(std::wstring_view a, std::wstring_view b)
{
    return AssetPath::EqualsIgnoreCase(a, b);
}
} // namespace Render::Textures::TexturePack
