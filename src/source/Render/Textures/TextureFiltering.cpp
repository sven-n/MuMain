#include "Render/Textures/TextureFiltering.h"

#include <algorithm>
#include <array>

namespace Render::Textures
{

namespace
{
[[nodiscard]] bool IsSeparator(wchar_t c)
{
    return c == L'\\' || c == L'/';
}

[[nodiscard]] wchar_t ToLowerAscii(wchar_t c)
{
    return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c - L'A' + L'a') : c;
}

[[nodiscard]] bool EqualsIgnoreCase(std::wstring_view text, std::wstring_view expected)
{
    return text.size() == expected.size() &&
           std::equal(text.begin(), text.end(), expected.begin(),
                      [](wchar_t a, wchar_t b) { return ToLowerAscii(a) == ToLowerAscii(b); });
}

[[nodiscard]] bool IsDigits(std::wstring_view text)
{
    return std::all_of(text.begin(), text.end(), [](wchar_t c) { return c >= L'0' && c <= L'9'; });
}

// Splits off the first path component; `rest` keeps what follows its separator.
[[nodiscard]] std::wstring_view TakeComponent(std::wstring_view& rest)
{
    const auto separator = std::find_if(rest.begin(), rest.end(), IsSeparator);
    const auto length = static_cast<std::size_t>(separator - rest.begin());
    const std::wstring_view component = rest.substr(0, length);
    rest = separator == rest.end() ? std::wstring_view{} : rest.substr(length + 1u);
    return component;
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
    std::wstring_view rest = path;
    if (!EqualsIgnoreCase(TakeComponent(rest), L"Data") || rest.empty())
    {
        return false;
    }

    const std::wstring_view folder = TakeComponent(rest);
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
