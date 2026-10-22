#pragma once

#include <algorithm>
#include <span>

// Reordering recorded draws to cut state changes. Draws whose result does not depend on their order
// (opaque, depth-tested and depth-writing) move ahead of the rest and are grouped by state; the
// others keep their recorded order after them, which also draws them over what is opaque, as
// transparency should.
namespace Render
{

// Puts the `reorderable` commands first, stable-sorted by `less`, then the others in their order.
template <typename Command, typename Reorderable, typename Less>
void GroupReorderableFirst(std::span<Command> commands, Reorderable reorderable, Less less)
{
    const auto reorderableEnd = std::stable_partition(commands.begin(), commands.end(), reorderable);
    std::stable_sort(commands.begin(), reorderableEnd, less);
}

} // namespace Render
