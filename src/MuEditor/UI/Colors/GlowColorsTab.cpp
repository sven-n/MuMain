#include "stdafx.h"

#ifdef _EDITOR

#include "GlowColorsTab.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
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

void GlowColorsTab::Render()
{
    m_list.Render(*this);
}

NamedColorTexts GlowColorsTab::GetTexts() const
{
    NamedColorTexts texts;
    texts.newEntry = I18N::Editor::NewGlowColor;
    texts.note = I18N::Editor::TheItemModelsNameTheseColorsTheModelToolPicksAnItemSColors;
    texts.pickAnEntry = I18N::Editor::PickAColorInTheList;
    texts.apply = I18N::Editor::ApplyToTheGlowColor;
    texts.change = I18N::Editor::ChangeAGlowColor;
    texts.removeOne = I18N::Editor::RemoveAGlowColor;
    texts.removeUnused = I18N::Editor::RemoveUnusedGlowColors;
    texts.unused = I18N::Editor::NoItemUsesTheseGlowColors0;
    texts.kept = I18N::Editor::ADefaultColorItemsWithoutColorsOfTheirOwnGlowWithIt;
    return texts;
}

std::size_t GlowColorsTab::GetCount() const
{
    return g_GlowColors.GetColors().size();
}

const std::string& GlowColorsTab::GetName(std::size_t index) const
{
    return g_GlowColors.GetColors()[index].name;
}

bool GlowColorsTab::Has(const std::string& name) const
{
    return g_GlowColors.Find(name) != nullptr;
}

void GlowColorsTab::RenderSwatch(std::size_t index) const
{
    const std::array<float, 3> rgb = ToColorField(g_GlowColors.GetColors()[index].value);
    ImGui::ColorButton("##Swatch", ImVec4(rgb[0], rgb[1], rgb[2], 1.0f), ImGuiColorEditFlags_NoTooltip);
}

int GlowColorsTab::CountUsers(const std::string& name)
{
    return m_counts.GlowColor(name);
}

std::vector<int> GlowColorsTab::FindUsers(const std::string& name) const
{
    return MuEditor::Items::FindGlowColorUsers(name);
}

// Items without colors of their own glow with these (ItemGlow), so they stay.
bool GlowColorsTab::IsKept(const std::string& name) const
{
    using Data::Items::ItemGlow;
    return name == ItemGlow::DefaultColor || name == ItemGlow::DefaultShineColor ||
           name == ItemGlow::DefaultAncientColor;
}

void GlowColorsTab::Remove(const char* step, std::span<const std::string> names)
{
    g_ItemEdits.RemoveGlowColors(step, names);
}

void GlowColorsTab::OpenNew(const std::string& from)
{
    m_dialog.Open(from);
}

std::optional<std::string> GlowColorsTab::RenderNew()
{
    const std::optional<GlowColor> created = m_dialog.Render();
    if (!created)
    {
        return std::nullopt;
    }
    g_ItemEdits.SetGlowColor(I18N::Editor::NewGlowColor, *created);
    return created->name;
}

void GlowColorsTab::RenderEdit(const std::string& name)
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

bool GlowColorsTab::IsEdited(const std::string& name) const
{
    // As the field shows it: a value with more places than the field keeps (written by hand) is not a change.
    const GlowColorValue* value = g_GlowColors.Find(name);
    return value != nullptr && ToGlowColorValue(m_edit) != AsShown(*value);
}

void GlowColorsTab::ApplyEdit(const std::string& name)
{
    m_loaded = ToGlowColorValue(m_edit);
    g_ItemEdits.SetGlowColor(I18N::Editor::ChangeAGlowColor, {name, m_loaded});
}

void GlowColorsTab::RevertEdit(const std::string& name)
{
    if (const GlowColorValue* value = g_GlowColors.Find(name))
    {
        m_edit = ToColorField(*value);
        m_loaded = AsShown(*value);
    }
}

void GlowColorsTab::UpdateEdit(const std::string& name, const GlowColorValue& value)
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
