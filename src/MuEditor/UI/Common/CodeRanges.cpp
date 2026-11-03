#include "stdafx.h"

#ifdef _EDITOR

#include "CodeRanges.h"

#include <algorithm>
#include <utility>

namespace MuEditor::CodeRanges
{
Bound MakeBound(std::string variable, bool lower, bool inclusive, int value, bool namesNothing)
{
    const int step = inclusive ? 0 : 1;
    const int bound = lower ? value + step : value - step;
    return {std::move(variable), lower, bound, namesNothing};
}

std::vector<Range> RangesOf(const std::vector<Bound>& bounds, int valueCount)
{
    std::vector<Range> ranges;
    std::vector<const Bound*> open;
    for (const Bound& bound : bounds)
    {
        const auto pending = std::find_if(open.begin(), open.end(),
                                          [&bound](const Bound* other) { return other->variable == bound.variable; });
        if (pending == open.end())
        {
            open.push_back(&bound);
            continue;
        }
        if ((*pending)->lower == bound.lower)
        {
            *pending = &bound;
            continue;
        }
        const Bound& lower = bound.lower ? bound : **pending;
        const Bound& upper = bound.lower ? **pending : bound;
        open.erase(pending);
        const bool namesNothing = lower.namesNothing && upper.namesNothing;
        const int first = std::max(lower.value, 0);
        const int last = std::min(upper.value, valueCount - 1);
        if (!namesNothing && first <= last)
        {
            ranges.push_back({first, last});
        }
    }
    return ranges;
}
} // namespace MuEditor::CodeRanges

#endif // _EDITOR
