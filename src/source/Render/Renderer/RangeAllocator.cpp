#include "RangeAllocator.h"

#include <iterator>

namespace Render
{
RangeAllocator::RangeAllocator(std::uint32_t capacity)
{
    if (capacity > 0)
        m_free.emplace(0u, capacity);
}

std::optional<std::uint32_t> RangeAllocator::Allocate(std::uint32_t size)
{
    if (size == 0)
        return std::nullopt;

    for (auto range = m_free.begin(); range != m_free.end(); ++range)
    {
        if (range->second < size)
            continue;

        const std::uint32_t offset = range->first;
        const std::uint32_t remaining = range->second - size;
        m_free.erase(range);
        if (remaining > 0)
            m_free.emplace(offset + size, remaining);
        return offset;
    }
    return std::nullopt;
}

void RangeAllocator::Free(std::uint32_t offset, std::uint32_t size)
{
    if (size == 0)
        return;

    auto range = m_free.emplace(offset, size).first;
    const auto next = std::next(range);
    if (next != m_free.end() && range->first + range->second == next->first)
    {
        range->second += next->second;
        m_free.erase(next);
    }
    if (range != m_free.begin())
    {
        const auto previous = std::prev(range);
        if (previous->first + previous->second == range->first)
        {
            previous->second += range->second;
            m_free.erase(range);
        }
    }
}
}
