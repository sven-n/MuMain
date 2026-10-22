#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

// A texture pack replaces the game's textures with its own files. Each pack is
// a folder in Data/TexturePacks that mirrors Data: with the pack "upscaled"
// active, Data/World1/TileGrass01.OZJ is read from
// Data/TexturePacks/upscaled/World1/TileGrass01.OZJ when the pack has that
// file, and from Data otherwise. One pack is active at a time, chosen in the
// options and read at startup.
namespace Render::Textures::TexturePack
{
inline constexpr std::wstring_view PacksFolder = L"Data\\TexturePacks";

// A pack name is a folder name of ASCII letters, digits, '-' and '_', so it
// cannot point outside the packs folder.
bool IsValidName(std::wstring_view name);

// The folder of a pack, relative to the client folder, like the paths the
// texture loader gets.
std::wstring PackFolder(std::wstring_view pack);

// The packs in a folder: its subfolders with a valid name, sorted.
std::vector<std::wstring> ListPacks(const std::filesystem::path& folder);

// The packs installed in the client's Data/TexturePacks.
std::vector<std::wstring> ListInstalledPacks();

// Whether two pack names are the same pack: folder names compare in any case,
// as the loader opens them.
bool SameName(std::wstring_view a, std::wstring_view b);
} // namespace Render::Textures::TexturePack
