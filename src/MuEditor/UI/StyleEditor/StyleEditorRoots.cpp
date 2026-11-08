#include "stdafx.h"

#ifdef _EDITOR

#include "StyleEditorRoots.h"

#include "LookEdits.h"

#include "I18N/All.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Items/ItemRenderStyles.h"

#include <algorithm>
#include <set>
#include <utility>

namespace MuEditor::Looks
{
namespace
{
std::string RowLabel(const std::string& name, int users, bool code)
{
    std::string label = name + " (" + std::to_string(users) + ")";
    if (code)
    {
        label += std::string(" ") + I18N::Editor::CodeMark;
    }
    return label;
}
} // namespace

const char* RenderStylesRoot::GetRootName() const
{
    return I18N::Editor::RenderStyles;
}

int RenderStylesRoot::GetVersion() const
{
    return g_LookEdits.GetVersion();
}

std::size_t RenderStylesRoot::GetCount() const
{
    Update();
    return m_names.size();
}

const std::string& RenderStylesRoot::GetName(std::size_t index) const
{
    Update();
    return m_names[index];
}

bool RenderStylesRoot::Has(const std::string& name) const
{
    Update();
    return std::ranges::binary_search(m_names, name);
}

std::string RenderStylesRoot::GetRowLabel(const std::string& name)
{
    std::string label = RowLabel(name, CountUsers(name), IsCode(name));
    if (m_unapplied && m_unapplied(name))
    {
        label += " *";
    }
    return label;
}

const std::vector<Data::Colors::ColorFolder>& RenderStylesRoot::GetFolders() const
{
    return g_LookEdits.GetState().folders;
}

void RenderStylesRoot::SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders)
{
    g_LookEdits.SetFolders(step, std::move(folders));
}

bool RenderStylesRoot::HasFolders() const
{
    return g_LookEdits.IsComplete();
}

void RenderStylesRoot::SetOrder(const char*, std::span<const std::string>) {}

bool RenderStylesRoot::CanMoveEntry(const std::string& name) const
{
    return g_LookEdits.Find(name) != nullptr;
}

bool RenderStylesRoot::IsCode(const std::string& name) const
{
    return g_LookEdits.Find(name) == nullptr && Render::Items::Styles::ExistsInCode(name);
}

int RenderStylesRoot::CountUsers(const std::string& name)
{
    return m_counts.RenderStyle(name);
}

void RenderStylesRoot::Update() const
{
    if (m_version == g_LookEdits.GetVersion())
    {
        return;
    }
    m_version = g_LookEdits.GetVersion();
    std::set<std::string, std::less<>> names;
    for (const Data::Looks::LookDefinition& look : g_LookEdits.GetState().looks)
    {
        names.insert(look.name);
    }
    for (const std::string_view name : Render::Items::Styles::GetNames())
    {
        if (Render::Items::Styles::ExistsInCode(name))
        {
            names.emplace(name);
        }
    }
    m_names.assign(names.begin(), names.end());
}

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
    return RowLabel(name, CountUsers(name), true);
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
