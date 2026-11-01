#include "stdafx.h"

#ifdef _EDITOR

#include "GroundLabelStylesRoot.h"

#include "GroundLabelWidgets.h"
#include "NamedColorList.h"

#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "../MuEditor/UI/ItemTools/ItemSelection.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "../MuEditor/UI/ItemTools/ItemUsers.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Colors
{
namespace
{
using Data::Items::GroundLabelStyle;

const Data::Items::GroundLabelFile& Labels()
{
    return g_ItemDatabase.GetGroundLabels();
}
} // namespace

NamedColorTexts GroundLabelStylesRoot::GetTexts() const
{
    NamedColorTexts texts;
    texts.root = I18N::Editor::GroundLabels;
    texts.note = I18N::Editor::ItemsNameTheseStylesInTheirGroundLabelRules;
    texts.usedBy = I18N::Editor::UsedBy0Items;
    texts.changeReaches = I18N::Editor::TheChangeReachesThese0Items;
    texts.apply = I18N::Editor::ApplyToTheStyle;
    texts.change = I18N::Editor::ChangeALabelStyle;
    texts.removeOne = I18N::Editor::RemoveALabelStyle;
    texts.removeUnused = I18N::Editor::RemoveUnusedLabelStyles;
    texts.unused = I18N::Editor::NoItemUsesTheseLabelStyles0;
    texts.kept = I18N::Editor::AStyleOfTheStandardRules;
    return texts;
}

// The item database counts the changes of the styles and their folders with its other changes.
int GroundLabelStylesRoot::GetVersion() const
{
    return g_ItemDatabase.GetVersion();
}

std::size_t GroundLabelStylesRoot::GetCount() const
{
    return Labels().styles.size();
}

const std::string& GroundLabelStylesRoot::GetName(std::size_t index) const
{
    return Labels().styles[index].name;
}

bool GroundLabelStylesRoot::Has(const std::string& name) const
{
    return Labels().FindStyle(name) != nullptr;
}

void GroundLabelStylesRoot::RenderSwatch(const std::string& name) const
{
    if (const GroundLabelStyle* style = Labels().FindStyle(name))
    {
        GroundLabelWidgets::RenderSwatch(*style);
    }
}

int GroundLabelStylesRoot::CountUsers(const std::string& name)
{
    return m_counts.GroundLabelStyle(name);
}

std::vector<NamedColorUser> GroundLabelStylesRoot::FindUsers(const std::string& name) const
{
    return MakeItemUsers(MuEditor::Items::FindGroundLabelStyleUsers(name));
}

// The items without a rule of their own for a level take these.
bool GroundLabelStylesRoot::IsKept(const std::string& name) const
{
    return Labels().IsStandardRuleStyle(name);
}

void GroundLabelStylesRoot::Remove(const char* step, std::span<const std::string> names)
{
    g_ItemEdits.RemoveGroundLabelStyles(step, names);
}

const std::vector<Data::Colors::ColorFolder>& GroundLabelStylesRoot::GetFolders() const
{
    return Labels().folders;
}

void GroundLabelStylesRoot::SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders)
{
    g_ItemEdits.SetColorFolders(step, MuEditor::Items::ItemEdits::GroundLabelFolders, std::move(folders));
}

void GroundLabelStylesRoot::SetOrder(const char* step, std::span<const std::string> names)
{
    std::vector<GroundLabelStyle> styles;
    for (const std::string& name : names)
    {
        if (const GroundLabelStyle* style = Labels().FindStyle(name))
        {
            styles.push_back(*style);
        }
    }
    if (styles.size() == Labels().styles.size())
    {
        g_ItemEdits.SetGroundLabelStyleOrder(step, styles);
    }
}

MuEditor::UndoTarget& GroundLabelStylesRoot::GetUndoTarget()
{
    return MuEditor::Items::ToolWindow::GetUndoTarget();
}

void GroundLabelStylesRoot::OpenNew(const std::string& from)
{
    m_dialog.Open(from);
}

std::optional<std::string> GroundLabelStylesRoot::RenderNew(const Data::Colors::ColorFolderPath& folder)
{
    const std::optional<GroundLabelStyle> created = m_dialog.Render();
    if (!created)
    {
        return std::nullopt;
    }
    g_ItemEdits.AddGroundLabelStyle(I18N::Editor::NewLabelStyle, *created, folder);
    return created->name;
}

void GroundLabelStylesRoot::RenderEdit(const std::string& name)
{
    const GroundLabelStyle* style = Labels().FindStyle(name);
    if (style == nullptr)
    {
        return;
    }
    UpdateEdit(*style);
    GroundLabelWidgets::RenderStyleFields(m_edit);
    // The label of the selected item in the edited look; the style's name while no item is selected.
    const Data::Items::ItemDefinition* item = g_ItemDatabase.Find(g_ItemSelection.GetItemType());
    const std::string text = item != nullptr ? Core::Text::ToUtf8(item->name.c_str()) : m_edit.name;
    GroundLabelWidgets::RenderLabel(text.c_str(), m_edit.color, m_edit.background, m_edit.bold);
}

bool GroundLabelStylesRoot::IsEdited(const std::string& name) const
{
    const GroundLabelStyle* style = Labels().FindStyle(name);
    return style != nullptr && m_edit.name == name && m_edit != *style;
}

void GroundLabelStylesRoot::ApplyEdit(const std::string& name)
{
    if (m_edit.name != name)
    {
        return;
    }
    m_loaded = m_edit;
    g_ItemEdits.SetGroundLabelStyle(I18N::Editor::ChangeALabelStyle, m_edit);
}

void GroundLabelStylesRoot::RevertEdit(const std::string& name)
{
    if (const GroundLabelStyle* style = Labels().FindStyle(name))
    {
        m_edit = *style;
        m_loaded = *style;
    }
}

void GroundLabelStylesRoot::UpdateEdit(const GroundLabelStyle& style)
{
    const bool unchanged = m_edit == m_loaded;
    if (m_edit.name != style.name || (m_editVersion != g_ItemDatabase.GetVersion() && unchanged))
    {
        m_edit = style;
        m_loaded = style;
    }
    m_editVersion = g_ItemDatabase.GetVersion();
}
} // namespace MuEditor::Colors

#endif // _EDITOR
