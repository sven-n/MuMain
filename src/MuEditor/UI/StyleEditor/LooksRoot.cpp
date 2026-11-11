#include "stdafx.h"

#ifdef _EDITOR

#include "LooksRoot.h"

#include "LookEdits.h"

#include "I18N/All.h"
#include "Render/Items/ItemEffects.h"
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

namespace
{
bool ExistsInCode(Data::Looks::LookKind kind, std::string_view name)
{
    return kind == Data::Looks::LookKind::ItemEffect ? Render::Items::ItemEffects::ExistsInCode(name)
                                                     : Render::Items::Styles::ExistsInCode(name);
}
} // namespace

const char* LooksRoot::GetRootName() const
{
    return m_kind == Data::Looks::LookKind::ItemEffect ? I18N::Editor::ItemEffects : I18N::Editor::RenderStyles;
}

int LooksRoot::GetVersion() const
{
    return LookEdits::Of(m_kind).GetVersion();
}

std::size_t LooksRoot::GetCount() const
{
    Update();
    return m_names.size();
}

const std::string& LooksRoot::GetName(std::size_t index) const
{
    Update();
    return m_names[index];
}

bool LooksRoot::Has(const std::string& name) const
{
    Update();
    return std::ranges::binary_search(m_names, name);
}

std::string LooksRoot::GetRowLabel(const std::string& name)
{
    std::string label = LookRowLabel(name, CountUsers(name), IsCode(name));
    if (m_unapplied && m_unapplied(name))
    {
        label += " *";
    }
    return label;
}

const std::vector<Data::Colors::ColorFolder>& LooksRoot::GetFolders() const
{
    return LookEdits::Of(m_kind).GetState().folders;
}

void LooksRoot::SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders)
{
    LookEdits::Of(m_kind).SetFolders(step, std::move(folders));
}

bool LooksRoot::HasFolders() const
{
    return LookEdits::Of(m_kind).IsComplete();
}

void LooksRoot::SetOrder(const char*, std::span<const std::string>) {}

bool LooksRoot::CanMoveEntry(const std::string& name) const
{
    return LookEdits::Of(m_kind).Find(name) != nullptr;
}

bool LooksRoot::IsCode(const std::string& name) const
{
    return LookEdits::Of(m_kind).Find(name) == nullptr && ExistsInCode(m_kind, name);
}

int LooksRoot::CountUsers(const std::string& name)
{
    return m_kind == Data::Looks::LookKind::ItemEffect ? m_counts.ItemEffect(name) : m_counts.RenderStyle(name);
}

void LooksRoot::Update() const
{
    if (m_version == LookEdits::Of(m_kind).GetVersion())
    {
        return;
    }
    m_version = LookEdits::Of(m_kind).GetVersion();
    std::set<std::string, std::less<>> names;
    for (const Data::Looks::LookDefinition& look : LookEdits::Of(m_kind).GetState().looks)
    {
        names.insert(look.name);
    }
    const std::span<const std::string_view> all = m_kind == Data::Looks::LookKind::ItemEffect
                                                      ? Render::Items::ItemEffects::GetNames()
                                                      : Render::Items::Styles::GetNames();
    for (const std::string_view name : all)
    {
        if (ExistsInCode(m_kind, name))
        {
            names.emplace(name);
        }
    }
    m_names.assign(names.begin(), names.end());
}
} // namespace MuEditor::Looks

#endif // _EDITOR
