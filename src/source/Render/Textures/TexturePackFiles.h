#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Render::Textures
{
// The texture files of the active pack (see TexturePack.h), listed once when
// the pack is chosen, so a texture the pack lacks costs a lookup and no file
// system access. An empty list stands for no pack.
class TexturePackFiles
{
public:
    // The OZJ and OZT files under `root`, a pack folder that mirrors Data.
    static TexturePackFiles List(const std::filesystem::path& root);

    // The pack's copy of a file in Data, such as "Data\\World1\\TileGrass01.OZJ",
    // matched in any case and with either separator; nullptr when the pack has
    // none.
    [[nodiscard]] const std::filesystem::path* Find(std::wstring_view dataPath) const;

    [[nodiscard]] bool Empty() const
    {
        return m_files.empty();
    }
    [[nodiscard]] std::size_t Count() const
    {
        return m_files.size();
    }

private:
    // Keyed by the path below the pack folder in UTF-8, lowercase ASCII and
    // with '/' separators.
    std::unordered_map<std::string, std::filesystem::path> m_files;
};
} // namespace Render::Textures
