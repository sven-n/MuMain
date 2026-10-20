#pragma once

#include <optional>
#include <string_view>

// Parsing of the client's asset paths, such as "Data\\World1\\TileGrass01.jpg":
// either separator, and any case for the folder names the game compares.
namespace Render::Textures::AssetPath
{
[[nodiscard]] bool IsSeparator(wchar_t c);

// Lowers A-Z only, so the result does not depend on the locale.
[[nodiscard]] wchar_t ToLowerAscii(wchar_t c);

[[nodiscard]] bool EqualsIgnoreCase(std::wstring_view text, std::wstring_view expected);

// Splits off the first path component; `rest` keeps what follows its separator.
[[nodiscard]] std::wstring_view TakeComponent(std::wstring_view& rest);

// The part of `path` below its leading Data folder, or nothing for a path
// outside Data or for the Data folder itself.
[[nodiscard]] std::optional<std::wstring_view> InDataFolder(std::wstring_view path);
} // namespace Render::Textures::AssetPath
