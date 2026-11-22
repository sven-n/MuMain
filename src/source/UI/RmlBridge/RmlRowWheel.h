#pragma once

#include <span>
#include <vector>

namespace Rml
{
class Element;
}

// The mouse wheel moving a list one row per notch, for rows of different heights (the M window's rows with a second
// line).
namespace UI::RmlBridge
{
// The tops of the list's shown rows (its visible children) in its scrolled content, in order.
std::vector<float> RowTops(Rml::Element& list);

// The scroll top after a wheel move of `notches` rows: the top of the row `notches` rows on (back when negative)
// from the one whose top is nearest `scrollTop`, within the rows; `scrollTop` when there are none.
float RowWheelScrollTop(std::span<const float> rowTops, float scrollTop, float notches);
} // namespace UI::RmlBridge
