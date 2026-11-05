#pragma once

#include <span>

namespace UI::CharacterCreation
{
// For a creation window that steps through the classes with arrows instead of a button per class:
// the nearest enabled class from `current` in `step`'s direction (-1 / +1), or -1 when there is none
// that way. `enabled` is in CLASS_* order.
constexpr int StepToEnabledClass(std::span<const bool> enabled, int current, int step)
{
    for (int index = current + step; index >= 0 && index < static_cast<int>(enabled.size()); index += step)
    {
        if (enabled[index])
            return index;
    }
    return -1;
}
} // namespace UI::CharacterCreation
