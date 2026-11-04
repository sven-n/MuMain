#include "stdafx.h"

#ifdef _EDITOR

#include "GroundLabelWidgets.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemLookUseCounts.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>

namespace MuEditor::Colors::GroundLabelWidgets
{
namespace
{
using Data::Items::GroundLabelColor;
using Data::Items::GroundLabelStyle;

// Pixel sizes before the editor UI scale.
constexpr float ColorFieldWidth = 260.0f;
constexpr float ComboWidth = 220.0f;
constexpr float LabelPaddingX = 6.0f;
constexpr float LabelPaddingY = 2.0f;
constexpr float ColorScale = 255.0f;
constexpr char SwatchText[] = "A";

ImU32 ToImColor(const GroundLabelColor& color, BYTE alpha = 255)
{
    return IM_COL32(color[0], color[1], color[2], alpha);
}

bool ColorField(const char* label, GroundLabelColor& color)
{
    float rgb[3];
    for (size_t i = 0; i < color.size(); ++i)
    {
        rgb[i] = static_cast<float>(color[i]) / ColorScale;
    }
    ImGui::SetNextItemWidth(ColorFieldWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::ColorEdit3(label, rgb, ImGuiColorEditFlags_Uint8))
    {
        return false;
    }
    for (size_t i = 0; i < color.size(); ++i)
    {
        color[i] = static_cast<BYTE>(std::lround(std::clamp(rgb[i], 0.0f, 1.0f) * ColorScale));
    }
    return true;
}

// The background's color with its alpha (0: no box behind the text), shown over a checkerboard.
bool BackgroundField(const char* label, GroundLabelColor& color, BYTE& alpha)
{
    float rgba[4];
    for (size_t i = 0; i < color.size(); ++i)
    {
        rgba[i] = static_cast<float>(color[i]) / ColorScale;
    }
    rgba[3] = static_cast<float>(alpha) / ColorScale;
    ImGui::SetNextItemWidth(ColorFieldWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::ColorEdit4(label, rgba,
                           ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaBar |
                               ImGuiColorEditFlags_AlphaPreviewHalf))
    {
        return false;
    }
    for (size_t i = 0; i < color.size(); ++i)
    {
        color[i] = static_cast<BYTE>(std::lround(std::clamp(rgba[i], 0.0f, 1.0f) * ColorScale));
    }
    alpha = static_cast<BYTE>(std::lround(std::clamp(rgba[3], 0.0f, 1.0f) * ColorScale));
    return true;
}
} // namespace

bool RenderStyleFields(GroundLabelStyle& style)
{
    bool changed = ColorField(I18N::Editor::TextColor, style.color);
    changed |= BackgroundField(I18N::Editor::Background, style.background, style.backgroundAlpha);
    changed |= ImGui::Checkbox(I18N::Editor::Bold, &style.bold);
    return changed;
}

void RenderLabel(const char* text, const GroundLabelColor& color, const GroundLabelColor& background, bool bold,
                 BYTE backgroundAlpha)
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    const ImVec2 padding(LabelPaddingX * uiScale, LabelPaddingY * uiScale);
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const float boldOffset = bold ? 1.0f : 0.0f;
    const ImVec2 size(textSize.x + 2.0f * padding.x + boldOffset, textSize.y + 2.0f * padding.y);
    const ImVec2 at = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(at, ImVec2(at.x + size.x, at.y + size.y), ToImColor(background, backgroundAlpha));
    const ImVec2 textAt(at.x + padding.x, at.y + padding.y);
    draw->AddText(textAt, ToImColor(color), text);
    if (bold)
    {
        draw->AddText(ImVec2(textAt.x + boldOffset, textAt.y), ToImColor(color), text);
    }
    ImGui::Dummy(size);
}

void RenderSwatch(const GroundLabelStyle& style)
{
    const float side = ImGui::GetFrameHeight();
    const ImVec2 at = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(at, ImVec2(at.x + side, at.y + side), ToImColor(style.background, style.backgroundAlpha));
    const ImVec2 textSize = ImGui::CalcTextSize(SwatchText);
    const ImVec2 textAt(at.x + (side - textSize.x) * 0.5f, at.y + (side - textSize.y) * 0.5f);
    draw->AddText(textAt, ToImColor(style.color), SwatchText);
    if (style.bold)
    {
        draw->AddText(ImVec2(textAt.x + 1.0f, textAt.y), ToImColor(style.color), SwatchText);
    }
    ImGui::Dummy(ImVec2(side, side));
}

bool RenderStyleCombo(const char* label, std::string& value, MuEditor::Items::ItemLookUseCounts& counts)
{
    const Data::Items::GroundLabelFile& labels = g_ItemDatabase.GetGroundLabels();
    ImGui::PushID(label);
    if (const GroundLabelStyle* current = labels.FindStyle(value))
    {
        RenderSwatch(*current);
        ImGui::SameLine();
    }
    ImGui::PopID();
    ImGui::SetNextItemWidth(ComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo(label, value.c_str()))
    {
        return false;
    }
    bool picked = false;
    for (const GroundLabelStyle& style : labels.styles)
    {
        ImGui::PushID(style.name.c_str());
        RenderSwatch(style);
        ImGui::SameLine();
        const std::string entry = style.name + " (" + std::to_string(counts.GroundLabelStyle(style.name)) + ")";
        if (ImGui::Selectable(entry.c_str(), style.name == value) && style.name != value)
        {
            value = style.name;
            picked = true;
        }
        ImGui::PopID();
    }
    ImGui::EndCombo();
    return picked;
}

GroundLabelColor FromGameColor(DWORD color)
{
    return {static_cast<BYTE>(color & 0xFF), static_cast<BYTE>((color >> 8) & 0xFF),
            static_cast<BYTE>((color >> 16) & 0xFF)};
}

BYTE AlphaOfGameColor(DWORD color)
{
    return static_cast<BYTE>((color >> 24) & 0xFF);
}
} // namespace MuEditor::Colors::GroundLabelWidgets

#endif // _EDITOR
