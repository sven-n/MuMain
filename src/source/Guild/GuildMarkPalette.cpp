#include "Guild/GuildMarkPalette.h"

#include <array>

namespace
{
// ::CreateGuildMark()'s MarkColor[] (bytes R, G, B in memory order), blend = true.
struct Rgb
{
    unsigned char r, g, b;
};
constexpr std::array<Rgb, Guild::MarkPalette::ColorCount> kPalette = {{
    {0, 0, 0},
    {0, 0, 0},
    {128, 128, 128},
    {255, 255, 255},
    {255, 0, 0},
    {255, 128, 0},
    {255, 255, 0},
    {128, 255, 0},
    {0, 255, 0},
    {0, 255, 128},
    {0, 255, 255},
    {0, 128, 255},
    {0, 0, 255},
    {128, 0, 255},
    {255, 0, 255},
    {255, 0, 128},
}};
} // namespace

std::string Guild::MarkPalette::CellColor(int paletteIndex)
{
    if (paletteIndex <= 0 || paletteIndex >= ColorCount)
        return "#00000000";
    static const char* const digits = "0123456789abcdef";
    const Rgb c = kPalette[static_cast<std::size_t>(paletteIndex)];
    std::string text = "#";
    for (const unsigned char channel : {c.r, c.g, c.b, static_cast<unsigned char>(255)})
    {
        text += digits[channel >> 4];
        text += digits[channel & 15];
    }
    return text;
}
