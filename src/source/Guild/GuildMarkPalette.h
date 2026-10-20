#pragma once

#include <string>

// The 16 colours of a guild mark's 8 x 8 cells (MARK_t::Mark), as ::CreateGuildMark() builds the
// emblem texture with blend = true: index 0 transparent, 1..15 opaque.
namespace Guild::MarkPalette
{
constexpr int CellCount = 64; // 8 x 8, row by row
constexpr int ColorCount = 16;

// The cell colour as an RCSS colour ("#rrggbbaa"); an index outside 0..15 is transparent.
std::string CellColor(int paletteIndex);

} // namespace Guild::MarkPalette
