#include "stdafx.h"

#ifdef _EDITOR

#include "GlowColorDialog.h"

#include "ItemTextInput.h"

#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace MuEditor::Items
{
namespace
{
constexpr char PopupId[] = "###NewGlowColor";
constexpr float FieldWidth = 260.0f;
constexpr double ValuePlaces = 1000.0;
} // namespace

Data::Effects::GlowColorValue ToGlowColorValue(const std::array<float, 3>& rgb)
{
    Data::Effects::GlowColorValue value{};
    for (size_t i = 0; i < value.size(); ++i)
    {
        value[i] = std::clamp(std::round(static_cast<double>(rgb[i]) * ValuePlaces) / ValuePlaces, 0.0, 1.0);
    }
    return value;
}

std::array<float, 3> ToColorField(const Data::Effects::GlowColorValue& value)
{
    return {static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2])};
}

bool IsFreeGlowColorName(const std::string& name)
{
    return Data::Items::Json::IsName(name) && g_GlowColors.Find(name) == nullptr;
}

void CGlowColorDialog::Open(std::string_view from, int target)
{
    const Data::Effects::GlowColorValue* value = g_GlowColors.Find(from);
    m_value = value != nullptr ? ToColorField(*value) : std::array<float, 3>{1.0f, 1.0f, 1.0f};
    m_name.clear();
    m_target = target;
    ImGui::OpenPopup(PopupId);
}

std::optional<Data::Effects::GlowColor> CGlowColorDialog::Render()
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", I18N::Editor::NewGlowColor, PopupId);
    if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return std::nullopt;
    }
    std::optional<Data::Effects::GlowColor> created;
    ImGui::SetNextItemWidth(FieldWidth);
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }
    InputString(I18N::Editor::Name, m_name);
    const bool canCreate = IsFreeGlowColorName(m_name);
    if (!canCreate)
    {
        ImGui::TextDisabled("%s", I18N::Editor::ANameOfLettersAndDigitsThatNoOtherGlowColorHas);
    }
    ImGui::SetNextItemWidth(FieldWidth);
    ImGui::ColorEdit3("##Value", m_value.data(), ImGuiColorEditFlags_Float);
    ImGui::Separator();
    ImGui::BeginDisabled(!canCreate);
    if (ImGui::Button(I18N::Editor::Add))
    {
        created = Data::Effects::GlowColor{m_name, ToGlowColorValue(m_value)};
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
    return created;
}
} // namespace MuEditor::Items

#endif // _EDITOR
