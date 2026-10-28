#include "stdafx.h"

#ifdef _EDITOR

#include "IssueLevels.h"

#include "I18N/All.h"

namespace MuEditor::IssueLevels
{
namespace
{
using Data::Items::ItemDataIssueSeverity;

constexpr ImVec4 FatalColor{1.0f, 0.4f, 0.4f, 1.0f};
constexpr ImVec4 ErrorColor{1.0f, 0.6f, 0.35f, 1.0f};
constexpr ImVec4 WarningColor{1.0f, 0.8f, 0.3f, 1.0f};
} // namespace

ImVec4 GetColor(ItemDataIssueSeverity severity)
{
    switch (severity)
    {
    case ItemDataIssueSeverity::Fatal:
        return FatalColor;
    case ItemDataIssueSeverity::Error:
        return ErrorColor;
    case ItemDataIssueSeverity::Warning:
        break;
    }
    return WarningColor;
}

const char* GetName(ItemDataIssueSeverity severity)
{
    switch (severity)
    {
    case ItemDataIssueSeverity::Fatal:
        return I18N::Editor::Fatal;
    case ItemDataIssueSeverity::Error:
        return I18N::Editor::Error;
    case ItemDataIssueSeverity::Warning:
        break;
    }
    return I18N::Editor::Warning;
}

void RenderLine(ItemDataIssueSeverity severity, const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, GetColor(severity));
    ImGui::PushTextWrapPos(0.0f);
    ImGui::Text("%s: %s", GetName(severity), text);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}
} // namespace MuEditor::IssueLevels

#endif // _EDITOR
