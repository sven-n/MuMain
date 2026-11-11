#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace GameLogic::Items
{
class JewelUnmixSelection
{
public:
    struct Entry
    {
        int slot = -1;
        std::uint32_t key = 0;
        int type = 0;
        int level = 0;
        bool operator==(const Entry&) const = default;
    };

    bool Update(std::span<const Entry> entries);
    bool Select(const Entry& entry);
    void Clear();
    std::span<const Entry> Entries() const { return m_Entries; }
    const std::optional<Entry>& Selected() const { return m_Selected; }

private:
    std::vector<Entry> m_Entries;
    std::optional<Entry> m_Selected;
};
}
