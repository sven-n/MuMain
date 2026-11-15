#include "TexturePack.h"

#include "Core/Content/ContentStore.h"
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

// Copied one byte at a time: a valid name is ASCII, and any other name fails IsValidName however it widens.
std::optional<std::wstring> PackName(const Content::Entry& entry)
{
    if (!entry.folder)
        return std::nullopt;

    std::wstring name(entry.name.begin(), entry.name.end());
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
    for (const Content::Entry& entry : Content::ListFolder(folder))
    {
        if (auto name = PackName(entry))
            packs.push_back(std::move(*name));
    }
    return packs;
}

std::vector<std::wstring> ListInstalledPacks()
{
    return ListPacks(std::filesystem::path(std::wstring(PacksFolder)));
}

bool SameName(std::wstring_view a, std::wstring_view b)
{
    return AssetPath::EqualsIgnoreCase(a, b);
}
} // namespace Render::Textures::TexturePack
