#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace UI::MapName
{
// The banner's ornament files: tools/MapNameFont/ornament.py cut the swirls and one icon per map
// out of the original map-name images and wrote where each piece sat in them (Layout.json).
// MapIcons.json says which icon each map shows.
inline constexpr const char* OrnamentDirectory = "Data/Interface/MapNameOrnament";

// A map's icon: its image's file name without extension, and where it sits on the banner in the
// original 166 x 90 image's pixels.
struct MapIcon
{
    std::string image;
    float x = 0.f;
    float y = 0.f;
    float width = 0.f;
    float height = 0.f;
};

class MapNameOrnament
{
public:
    // Replaces the icons with the ones the two files describe. False, with no icons, when either
    // is not valid; a map naming an icon the layout does not have is left out.
    bool Read(std::string_view layoutJson, std::string_view mapIconsJson);

    // nullptr when the map shows no icon.
    const MapIcon* FindIcon(int map) const;

private:
    std::unordered_map<int, MapIcon> m_Icons;
};

// The ornament of OrnamentDirectory, read on first use. A read that fails is tried again on the
// next call (the next map change), so a passing problem does not hide the icons until restart.
const MapNameOrnament& GetMapNameOrnament();
} // namespace UI::MapName
