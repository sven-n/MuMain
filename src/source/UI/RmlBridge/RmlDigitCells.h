#pragma once

#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/Types.h>

#include <string>
#include <vector>

namespace UI::RmlBridge
{
// A number's digits as texel cells of a digit strip (digit d at x = d * cellWidth), in order. A
// character that is not a digit (a minus sign) keeps its place as an empty cell. The theme lays the
// run out from how many cells it has and each cell's index.
inline std::vector<Rml::String> DigitCells(int number, float cellWidth, float cellHeight)
{
    std::vector<Rml::String> cells;
    for (const char character : std::to_string(number))
    {
        cells.push_back(character >= '0' && character <= '9'
                            ? Rml::CreateString("%g 0 %g %g", static_cast<float>(character - '0') * cellWidth,
                                                cellWidth, cellHeight)
                            : Rml::String());
    }
    return cells;
}
} // namespace UI::RmlBridge
