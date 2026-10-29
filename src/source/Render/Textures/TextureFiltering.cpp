#include "Render/Textures/TextureFiltering.h"

#include "Render/Textures/AssetPath.h"

#include <algorithm>
#include <array>

namespace Render::Textures
{

namespace
{
using AssetPath::EqualsIgnoreCase;

[[nodiscard]] bool IsDigits(std::wstring_view text)
{
    return std::all_of(text.begin(), text.end(), [](wchar_t c) { return c >= L'0' && c <= L'9'; });
}

constexpr std::array<std::wstring_view, 6> WorldFolders = {L"Player", L"Monster", L"Item", L"NPC", L"Effect", L"Skill"};

// Folders numbered per map: World1, Object1, ... (also accepted unnumbered).
constexpr std::array<std::wstring_view, 2> NumberedWorldFolders = {L"World", L"Object"};

[[nodiscard]] bool IsWorldFolder(std::wstring_view folder)
{
    const auto named = [folder](std::wstring_view name) { return EqualsIgnoreCase(folder, name); };
    const auto numbered = [folder](std::wstring_view prefix)
    {
        return folder.size() >= prefix.size() && EqualsIgnoreCase(folder.substr(0, prefix.size()), prefix) &&
               IsDigits(folder.substr(prefix.size()));
    };
    return std::any_of(WorldFolders.begin(), WorldFolders.end(), named) ||
           std::any_of(NumberedWorldFolders.begin(), NumberedWorldFolders.end(), numbered);
}
} // namespace

bool IsWorldTexturePath(std::wstring_view path)
{
    const std::optional<std::wstring_view> inData = AssetPath::InDataFolder(path);
    if (!inData)
    {
        return false;
    }

    std::wstring_view rest = *inData;
    const std::wstring_view folder = AssetPath::TakeComponent(rest);
    return !rest.empty() && IsWorldFolder(folder);
}

AnisotropyLevel ClampAnisotropy(int configured)
{
    if (configured <= 1)
    {
        return {1u, false};
    }

    const auto requested = static_cast<std::uint32_t>(configured);
    if (requested > MaxSupportedAnisotropy)
    {
        return {MaxSupportedAnisotropy, true};
    }
    return {requested, false};
}

} // namespace Render::Textures
