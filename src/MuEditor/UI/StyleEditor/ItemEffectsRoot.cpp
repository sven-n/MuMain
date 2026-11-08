#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEffectsRoot.h"

#include "RenderStylesRoot.h"

#include "I18N/All.h"
#include "Render/Items/ItemEffects.h"

#include <set>

namespace MuEditor::Looks
{
const char* ItemEffectsRoot::GetRootName() const
{
    return I18N::Editor::ItemEffects;
}

std::size_t ItemEffectsRoot::GetCount() const
{
    return Names().size();
}

const std::string& ItemEffectsRoot::GetName(std::size_t index) const
{
    return Names()[index];
}

bool ItemEffectsRoot::Has(const std::string& name) const
{
    return std::ranges::binary_search(Names(), name);
}

std::string ItemEffectsRoot::GetRowLabel(const std::string& name)
{
    return LookRowLabel(name, CountUsers(name), true);
}

const std::vector<Data::Colors::ColorFolder>& ItemEffectsRoot::GetFolders() const
{
    static const std::vector<Data::Colors::ColorFolder> none;
    return none;
}

void ItemEffectsRoot::SetFolders(const char*, std::vector<Data::Colors::ColorFolder>) {}

void ItemEffectsRoot::SetOrder(const char*, std::span<const std::string>) {}

bool ItemEffectsRoot::CanMoveEntry(const std::string&) const
{
    return false;
}

int ItemEffectsRoot::CountUsers(const std::string& name)
{
    return m_counts.ItemEffect(name);
}

const std::vector<std::string>& ItemEffectsRoot::Names() const
{
    static const std::vector<std::string> names = []
    {
        std::set<std::string, std::less<>> sorted;
        for (const std::string_view name : Render::Items::ItemEffects::GetNames())
        {
            sorted.emplace(name);
        }
        return std::vector<std::string>(sorted.begin(), sorted.end());
    }();
    return names;
}
} // namespace MuEditor::Looks

#endif // _EDITOR
