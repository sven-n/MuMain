#include "stdafx.h"

#include "GlowColorList.h"

#ifdef _EDITOR
#include "Data/GameData/ColorFolders/ColorFolders.h"
#endif

#include <algorithm>

namespace Data::Effects
{
GlowColorList& GlowColorList::GetInstance()
{
    static GlowColorList instance;
    return instance;
}

void GlowColorList::Build(std::span<const GlowColor> colors)
{
    m_colors.assign(colors.begin(), colors.end());
#ifdef _EDITOR
    // The folders stay as they are; a name in them that is no color any more is not shown or written.
    ++m_version;
#endif
}

const GlowColorValue* GlowColorList::Find(std::string_view name) const
{
    const auto found =
        std::find_if(m_colors.begin(), m_colors.end(), [name](const GlowColor& color) { return color.name == name; });
    return found != m_colors.end() ? &found->value : nullptr;
}

#ifdef _EDITOR
void GlowColorList::Set(const GlowColor& color)
{
    const auto found = std::find_if(m_colors.begin(), m_colors.end(),
                                    [&](const GlowColor& listed) { return listed.name == color.name; });
    if (found != m_colors.end())
    {
        found->value = color.value;
    }
    else
    {
        m_colors.push_back(color);
    }
    ++m_version;
}

void GlowColorList::Remove(std::string_view name)
{
    std::erase_if(m_colors, [name](const GlowColor& color) { return color.name == name; });
    Colors::RemoveColorFromFolders(m_folders, name);
    ++m_version;
}

void GlowColorList::SetFolders(std::vector<Colors::ColorFolder> folders)
{
    m_folders = std::move(folders);
    ++m_version;
}
#endif
} // namespace Data::Effects
