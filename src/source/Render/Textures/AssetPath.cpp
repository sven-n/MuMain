#include "AssetPath.h"

#include <algorithm>

namespace Render::Textures::AssetPath
{
namespace
{
constexpr std::wstring_view DataFolder = L"Data";
} // namespace

bool IsSeparator(wchar_t c)
{
    return c == L'\\' || c == L'/';
}

wchar_t ToLowerAscii(wchar_t c)
{
    return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c - L'A' + L'a') : c;
}

bool EqualsIgnoreCase(std::wstring_view text, std::wstring_view expected)
{
    return text.size() == expected.size() &&
           std::equal(text.begin(), text.end(), expected.begin(),
                      [](wchar_t a, wchar_t b) { return ToLowerAscii(a) == ToLowerAscii(b); });
}

std::wstring_view TakeComponent(std::wstring_view& rest)
{
    const auto separator = std::find_if(rest.begin(), rest.end(), IsSeparator);
    const auto length = static_cast<std::size_t>(separator - rest.begin());
    const std::wstring_view component = rest.substr(0, length);
    rest = separator == rest.end() ? std::wstring_view{} : rest.substr(length + 1u);
    return component;
}

std::optional<std::wstring_view> InDataFolder(std::wstring_view path)
{
    std::wstring_view rest = path;
    if (!EqualsIgnoreCase(TakeComponent(rest), DataFolder) || rest.empty())
    {
        return std::nullopt;
    }
    return rest;
}
} // namespace Render::Textures::AssetPath
