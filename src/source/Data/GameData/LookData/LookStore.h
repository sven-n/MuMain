#pragma once

#include "Data/GameData/LookData/LookDefinition.h"

#include <span>
#include <string_view>
#include <vector>

namespace Data::Looks
{
// The looks the game has as data: the render styles of Data/Effects/RenderStyles.json, sorted by name. The drawing
// looks a name up here before the looks still in code; each change counts, so what was built from the looks can
// follow.
class LookStore
{
public:
    void SetRenderStyles(std::vector<LookDefinition> looks);

    // The render style of that name, or nullptr.
    [[nodiscard]] const LookDefinition* FindRenderStyle(std::string_view name) const;
    [[nodiscard]] std::span<const LookDefinition> GetRenderStyles() const
    {
        return m_renderStyles;
    }
    // How often the looks changed; 0 before the first.
    [[nodiscard]] int GetVersion() const
    {
        return m_version;
    }

private:
    std::vector<LookDefinition> m_renderStyles;
    int m_version = 0;
};

extern LookStore g_LookStore;
} // namespace Data::Looks
