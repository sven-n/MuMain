#include "stdafx.h"

#ifdef _EDITOR

#include "SkillTooltipEditor.h"

#include "Core/Utilities/StringUtils.h"
#include "UI/NewUI/HUD/Skills/SkillTooltipModel.h"
#include "imgui.h"

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
    // Two-pass for center alignment: compute the widest line, then indent
    // each line by half the difference so the tooltip auto-sizes nicely.
    float maxWidth = 0.0f;
    for (int i = 0; i < model.count; ++i)
    {
        if (model.lines[i].isBlank) continue;
        const std::string utf8 = StringUtils::WideToNarrow(model.lines[i].text);
        const float w = ImGui::CalcTextSize(utf8.c_str()).x;
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

        const std::string utf8 = StringUtils::WideToNarrow(line.text);
        const float textWidth = ImGui::CalcTextSize(utf8.c_str()).x;
        const float indent = (maxWidth - textWidth) * 0.5f;
        if (indent > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        }

        if (HasRedBackground(line.color))
        {
            const ImVec2 textSize = ImGui::CalcTextSize(utf8.c_str());
            const ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
            const float padX = 4.0f;
            const float padY = 1.0f;
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(cursorScreen.x - padX, cursorScreen.y - padY),
                ImVec2(cursorScreen.x + textSize.x + padX, cursorScreen.y + textSize.y + padY),
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
