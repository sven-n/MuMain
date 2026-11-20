#include "stdafx.h"
#include "UI/RmlBridge/RmlRowWheel.h"

#include <RmlUi/Core/Element.h>

#include <algorithm>
#include <cmath>

std::vector<float> UI::RmlBridge::RowTops(Rml::Element& list)
{
    std::vector<float> tops;
    const float contentTop = list.GetAbsoluteOffset(Rml::BoxArea::Content).y - list.GetScrollTop();
    for (int i = 0; i < list.GetNumChildren(); ++i)
    {
        Rml::Element* row = list.GetChild(i);
        if (row->IsVisible() && row->GetBox().GetSize(Rml::BoxArea::Border).y > 0.f)
        {
            tops.push_back(row->GetAbsoluteOffset(Rml::BoxArea::Border).y - contentTop);
        }
    }
    return tops;
}

float UI::RmlBridge::RowWheelScrollTop(std::span<const float> rowTops, float scrollTop, float notches)
{
    if (rowTops.empty())
    {
        return scrollTop;
    }
    const auto nearest = std::min_element(rowTops.begin(), rowTops.end(), [scrollTop](float a, float b)
                                          { return std::abs(a - scrollTop) < std::abs(b - scrollTop); });
    const long row = std::clamp(static_cast<long>(nearest - rowTops.begin()) + std::lround(notches), 0L,
                                static_cast<long>(rowTops.size()) - 1);
    return rowTops[static_cast<size_t>(row)];
}
