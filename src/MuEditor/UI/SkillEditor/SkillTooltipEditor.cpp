#include "stdafx.h"

#ifdef _EDITOR

#include "SkillTooltipEditor.h"

#include "Core/Utilities/StringUtils.h"
#include "UI/NewUI/HUD/Skills/SkillTooltipModel.h"
#include "imgui.h"

#include <string>
#include <vector>

namespace MuEditor::Skills::Tooltip
{

// Pull the shared model + line types from UI::Skills::Tooltip into our
// namespace so the rendering code below stays terse.
using UI::Skills::Tooltip::BuildModel;
using UI::Skills::Tooltip::BuildOptions;
using UI::Skills::Tooltip::Line;
using UI::Skills::Tooltip::LineColor;
using UI::Skills::Tooltip::Model;

namespace
{
const ImVec4 IMGUI_COLOR_WHITE = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 IMGUI_COLOR_BLUE = ImVec4(0.4f, 0.6f, 1.0f, 1.0f);
const ImVec4 IMGUI_COLOR_RED = ImVec4(0.95f, 0.35f, 0.35f, 1.0f);   // plain red text
const ImVec4 IMGUI_COLOR_DARKRED_FG = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // white text on red background
const ImU32 IMGUI_BG_DARKRED = IM_COL32(170, 30, 30, 255);

ImVec4 ImGuiColor(LineColor c)
{
    switch (c)
    {
    case LineColor::White:    return IMGUI_COLOR_WHITE;
    case LineColor::Blue:     return IMGUI_COLOR_BLUE;
    case LineColor::Red:      return IMGUI_COLOR_RED;
    case LineColor::DarkRed:  return IMGUI_COLOR_DARKRED_FG;
    }
    return IMGUI_COLOR_WHITE;
}

bool HasRedBackground(LineColor c) { return c == LineColor::DarkRed; }

void RenderModelCentered(const Model& model)
{
    // Two-pass for center alignment: first pass measures the widest line and
    // caches the UTF-8 conversion; second pass centers and renders. The buffer
    // is reused across calls so hovering doesn't allocate on every frame.
    thread_local std::vector<std::string> utf8Lines;
    utf8Lines.clear();
    utf8Lines.reserve(model.count);

    float maxWidth = 0.0f;
    for (int i = 0; i < model.count; ++i)
    {
        if (model.lines[i].isBlank)
        {
            utf8Lines.emplace_back();
            continue;
        }
        utf8Lines.emplace_back(StringUtils::WideToNarrow(model.lines[i].text));
        const float w = ImGui::CalcTextSize(utf8Lines.back().c_str()).x;
        if (w > maxWidth) maxWidth = w;
    }

    for (int i = 0; i < model.count; ++i)
    {
        const Line& line = model.lines[i];
        if (line.isBlank)
        {
            ImGui::Spacing();
            continue;
        }

        const std::string& utf8 = utf8Lines[i];
        const ImVec2 textSize = ImGui::CalcTextSize(utf8.c_str());
        const float indent = (maxWidth - textSize.x) * 0.5f;
        if (indent > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        }

        if (HasRedBackground(line.color))
        {
            const ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
            const float padX = 4.0f;
            const float padY = 1.0f;
            // Stretch the background to the tooltip's content width (maxWidth)
            // so every banner looks the same regardless of its text length.
            const float leftEdge = cursorScreen.x - indent;
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(leftEdge - padX, cursorScreen.y - padY),
                ImVec2(leftEdge + maxWidth + padX, cursorScreen.y + textSize.y + padY),
                IMGUI_BG_DARKRED);
        }

        ImGui::TextColored(ImGuiColor(line.color), "%s", utf8.c_str());
    }
}
}  // namespace

void Render(int skillIndex)
{
    BuildOptions options;
    options.skillType = skillIndex;
    options.skillSlotIndex = -1;
    options.includeCharacterSpecific = false;

    Model model;
    BuildModel(options, model);

    if (model.count == 0) return;

    ImGui::BeginTooltip();
    RenderModelCentered(model);
    ImGui::EndTooltip();
}

}  // namespace MuEditor::Skills::Tooltip

#endif // _EDITOR
