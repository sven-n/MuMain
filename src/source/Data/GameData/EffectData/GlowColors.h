#pragma once

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The named glow colors (Data/Effects/GlowColors.json):
//
//   { "formatVersion": 1, "colors": { "orange": [1, 0.5, 0], "gold": [1, 0.7, 0.2] } }
//
// The glow of an item model names its colors from this list. Colors are red,
// green and blue from 0 to 1. "colorFolders" before "colors" holds the folders
// MuEditor sorts the colors in (ColorFolders.h); the game reads and checks them
// but doesn't use them.
namespace Data::Effects
{
using GlowColorValue = std::array<double, 3>;

struct GlowColor
{
    std::string name;
    GlowColorValue value{};

    bool operator==(const GlowColor&) const = default;
};

constexpr int GlowColorsFormatVersion = 1;

// The file, relative to the client folder.
constexpr const char* GlowColorsFile = "Data/Effects/GlowColors.json";

// Reads the colors of the file into `colors`, and the folders they are sorted in into `folders`.
void ReadGlowColorsJson(std::string_view text, const std::string& source, std::vector<GlowColor>& colors,
                        std::vector<Colors::ColorFolder>& folders, std::vector<Items::ItemDataIssue>& issues);
// The same without the folders.
void ReadGlowColorsJson(std::string_view text, const std::string& source, std::vector<GlowColor>& colors,
                        std::vector<Items::ItemDataIssue>& issues);

// The text of the file with these colors, in their order, one color a line,
// as the file is written by hand, and the folders on a line each.
std::string WriteGlowColorsJson(std::span<const GlowColor> colors, std::span<const Colors::ColorFolder> folders = {});
} // namespace Data::Effects
