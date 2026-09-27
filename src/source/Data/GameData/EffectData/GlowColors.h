#pragma once

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
// green and blue from 0 to 1.
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

// Reads the colors of the file into `colors`.
void ReadGlowColorsJson(std::string_view text, const std::string& source, std::vector<GlowColor>& colors,
                        std::vector<Items::ItemDataIssue>& issues);

// The glow colors by name. Built once at startup, before the item models
// are checked.
class GlowColorList
{
public:
    static GlowColorList& GetInstance();

    void Build(std::span<const GlowColor> colors);

    // Returns nullptr for names that are not in the list.
    const GlowColorValue* Find(std::string_view name) const;

    std::span<const GlowColor> GetAll() const
    {
        return m_colors;
    }

    // Changes with every Build, so colors taken from the list can be looked
    // up again.
    int GetVersion() const
    {
        return m_version;
    }

private:
    std::vector<GlowColor> m_colors;
    int m_version = 0;
};
} // namespace Data::Effects

#define g_GlowColors Data::Effects::GlowColorList::GetInstance()
