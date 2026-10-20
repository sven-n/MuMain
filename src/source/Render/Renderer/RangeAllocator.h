#pragma once

#include <cstdint>
#include <map>
#include <optional>

namespace Render
{
// First-fit sub-allocation of one buffer, in units the caller picks; a freed range merges with free neighbours.
class RangeAllocator final
{
public:
    explicit RangeAllocator(std::uint32_t capacity);

    [[nodiscard]] std::optional<std::uint32_t> Allocate(std::uint32_t size);
    void Free(std::uint32_t offset, std::uint32_t size);

private:
    std::map<std::uint32_t, std::uint32_t> m_free; // offset -> size
};
}
