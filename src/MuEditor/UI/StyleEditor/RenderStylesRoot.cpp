#include "stdafx.h"

#ifdef _EDITOR

#include "RenderStylesRoot.h"

#include "LookEdits.h"

#include "I18N/All.h"
#include "Render/Items/ItemRenderStyles.h"

#include <algorithm>
#include <set>
#include <utility>

namespace MuEditor::Looks
{
std::string LookRowLabel(const std::string& name, int users, bool code)
{
    std::string label = name + " (" + std::to_string(users) + ")";
    if (code)
    {
        label += std::string(" ") + I18N::Editor::CodeMark;
    }
    return label;
}

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
    std::string label = LookRowLabel(name, CountUsers(name), IsCode(name));
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
} // namespace MuEditor::Looks

#endif // _EDITOR
