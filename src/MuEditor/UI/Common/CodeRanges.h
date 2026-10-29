#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

// The range checks of a line of code: a variable compared with two values
// the tools know (item types, monster numbers), which name the values between
// them too.
namespace MuEditor::CodeRanges
{
// A comparison of a variable with a known value: the first (lower) or last
// (upper) value the variable may have.
struct Bound
{
    // As written, without spaces.
    std::string variable;
    bool lower = false;
    int value = 0;
    // The value names nothing itself (an item group's base, a marker of the
    // monster enum); it only bounds a range.
    bool namesNothing = false;
};

// The first and last value of a range, both inside it.
struct Range
{
    int first = 0;
    int last = 0;
};

// The bound that comparing a variable with `value` sets: `lower` for
// variable > value (or value < variable), `inclusive` for >= and <=.
Bound MakeBound(std::string variable, bool lower, bool inclusive, int value, bool namesNothing);

// The ranges a line's bounds set: a lower and an upper bound of the same
// variable in a row, in either order, cut to 0 to `valueCount` - 1. Between
// two bounds that name nothing the range names nothing either (it tests whole
// item groups).
std::vector<Range> RangesOf(const std::vector<Bound>& bounds, int valueCount);
} // namespace MuEditor::CodeRanges

#endif // _EDITOR
