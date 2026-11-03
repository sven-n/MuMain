#include "stdafx.h"

#ifdef _EDITOR

#include "GlowColorsRoot.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "NamedColorList.h"

#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "../MuEditor/UI/ItemTools/ItemUsers.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Colors
{
namespace
{
using Data::Effects::GlowColor;
using Data::Effects::GlowColorValue;
using MuEditor::Items::ToColorField;
using MuEditor::Items::ToGlowColorValue;

// Pixel width before the editor UI scale.
constexpr float ValueWidth = 260.0f;

// The value as the color field shows and stores it.
GlowColorValue AsShown(const GlowColorValue& value)
{
    return ToGlowColorValue(ToColorField(value));
}
} // namespace

NamedColorTexts GlowColorsRoot::GetTexts() const
{
    NamedColorTexts texts;
    texts.root = I18N::Editor::Glow;
    texts.note = I18N::Editor::TheItemModelsNameTheseColorsTheModelToolPicksAnItemSColors;
    texts.usedBy = I18N::Editor::UsedBy0Items;
    texts.changeReaches = I18N::Editor::TheChangeReachesThese0Items;
    texts.apply = I18N::Editor::ApplyToTheGlowColor;
    texts.change = I18N::Editor::ChangeAGlowColor;
    texts.removeOne = I18N::Editor::RemoveAGlowColor;
    texts.removeUnused = I18N::Editor::RemoveUnusedGlowColors;
    texts.unused = I18N::Editor::NoItemUsesTheseGlowColors0;
    texts.kept = I18N::Editor::ADefaultColorItemsWithoutColorsOfTheirOwnGlowWithIt;
    return texts;
}

int GlowColorsRoot::GetVersion() const
{
    return g_GlowColors.GetVersion();
}

std::size_t GlowColorsRoot::GetCount() const
{
    return g_GlowColors.GetColors().size();
}

const std::string& GlowColorsRoot::GetName(std::size_t index) const
{
    return g_GlowColors.GetColors()[index].name;
}

bool GlowColorsRoot::Has(const std::string& name) const
{
    return g_GlowColors.Find(name) != nullptr;
}

void GlowColorsRoot::RenderSwatch(const std::string& name) const
{
    const GlowColorValue* value = g_GlowColors.Find(name);
    const std::array<float, 3> rgb = value != nullptr ? ToColorField(*value) : std::array<float, 3>{};
    ImGui::ColorButton("##Swatch", ImVec4(rgb[0], rgb[1], rgb[2], 1.0f), ImGuiColorEditFlags_NoTooltip);
}

int GlowColorsRoot::CountUsers(const std::string& name)
{
    return m_counts.GlowColor(name);
}

std::vector<NamedColorUser> GlowColorsRoot::FindUsers(const std::string& name) const
{
    return MakeItemUsers(MuEditor::Items::FindGlowColorUsers(name));
}

// Items without colors of their own glow with these (ItemGlow), so they stay.
bool GlowColorsRoot::IsKept(const std::string& name) const
{
    using Data::Items::ItemGlow;
    return name == ItemGlow::DefaultColor || name == ItemGlow::DefaultShineColor ||
           name == ItemGlow::DefaultAncientColor;
}

void GlowColorsRoot::Remove(const char* step, std::span<const std::string> names)
{
    g_ItemEdits.RemoveGlowColors(step, names);
}

const std::vector<Data::Colors::ColorFolder>& GlowColorsRoot::GetFolders() const
{
    return g_GlowColors.GetFolders();
}

void GlowColorsRoot::SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders)
{
    g_ItemEdits.SetColorFolders(step, MuEditor::Items::ItemEdits::GlowFolders, std::move(folders));
}

void GlowColorsRoot::SetOrder(const char* step, std::span<const std::string> names)
{
    std::vector<GlowColor> colors;
    for (const std::string& name : names)
    {
        if (const GlowColorValue* value = g_GlowColors.Find(name))
        {
            colors.push_back({name, *value});
        }
    }
    if (colors.size() == g_GlowColors.GetColors().size())
    {
        g_ItemEdits.SetGlowColorOrder(step, colors);
    }
}

MuEditor::UndoTarget& GlowColorsRoot::GetUndoTarget()
{
    return MuEditor::Items::ToolWindow::GetUndoTarget();
}

void GlowColorsRoot::OpenNew(const std::string& from)
{
    m_dialog.Open(from);
}

std::optional<std::string> GlowColorsRoot::RenderNew(const Data::Colors::ColorFolderPath& folder)
{
    const std::optional<GlowColor> created = m_dialog.Render();
    if (!created)
    {
        return std::nullopt;
    }
    g_ItemEdits.AddGlowColor(I18N::Editor::NewGlowColor, *created, folder);
    return created->name;
}

void GlowColorsRoot::RenderEdit(const std::string& name)
{
    const GlowColorValue* value = g_GlowColors.Find(name);
    if (value == nullptr)
    {
        return;
    }
    UpdateEdit(name, *value);
    ImGui::SetNextItemWidth(ValueWidth * g_MuEditorCore.GetUIScale());
    ImGui::ColorEdit3("##Value", m_edit.data(), ImGuiColorEditFlags_Float);
}

bool GlowColorsRoot::IsEdited(const std::string& name) const
{
    // As the field shows it: a value with more places than the field keeps (written by hand) is not a change.
    const GlowColorValue* value = g_GlowColors.Find(name);
    return value != nullptr && ToGlowColorValue(m_edit) != AsShown(*value);
}

void GlowColorsRoot::ApplyEdit(const std::string& name)
{
    m_loaded = ToGlowColorValue(m_edit);
    g_ItemEdits.SetGlowColor(I18N::Editor::ChangeAGlowColor, {name, m_loaded});
}

void GlowColorsRoot::RevertEdit(const std::string& name)
{
    if (const GlowColorValue* value = g_GlowColors.Find(name))
    {
        m_edit = ToColorField(*value);
        m_loaded = AsShown(*value);
    }
}

void GlowColorsRoot::UpdateEdit(const std::string& name, const GlowColorValue& value)
{
    const bool unchanged = ToGlowColorValue(m_edit) == m_loaded;
    if (m_editOf != name || (m_editVersion != g_GlowColors.GetVersion() && unchanged))
    {
        m_edit = ToColorField(value);
        m_loaded = AsShown(value);
        m_editOf = name;
    }
    m_editVersion = g_GlowColors.GetVersion();
}
} // namespace MuEditor::Colors

#endif // _EDITOR
