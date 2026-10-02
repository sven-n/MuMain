#pragma once

#include <span>
#include <vector>

namespace UI::Inventory
{
class SocketListSelection
{
public:
    struct Option
    {
        int seed = 0;
        int sphereLevel = 0;
        bool operator==(const Option&) const = default;
    };

    bool Update(int recipe, std::span<const Option> options);
    bool Select(int index);

    void ClearSelection() { m_Selected = -1; }
    int Selected() const { return m_Selected; }
    std::span<const Option> Options() const { return m_Options; }

private:
    std::vector<Option> m_Options;
    int m_Recipe = -1;
    int m_Selected = -1;
};
}
