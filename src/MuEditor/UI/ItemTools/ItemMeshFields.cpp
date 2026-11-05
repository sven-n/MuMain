#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMeshFields.h"

#include "ItemMeshFocus.h"

#include "imgui.h"

#include <charconv>

namespace MuEditor::Items::MeshFields
{
namespace
{
bool IsDigit(char character)
{
    return character >= '0' && character <= '9';
}

// The character of `text` the mouse is over in the field drawn last; text.size() when it is past the end.
std::size_t CharacterUnderMouse(std::string_view text)
{
    const float mouseX = ImGui::GetIO().MousePos.x - ImGui::GetItemRectMin().x - ImGui::GetStyle().FramePadding.x;
    float left = 0.0f;
    for (std::size_t index = 0; index < text.size(); ++index)
    {
        left += ImGui::CalcTextSize(text.data() + index, text.data() + index + 1).x;
        if (mouseX < left)
        {
            return index;
        }
    }
    return text.size();
}

bool IsPointedAt()
{
    return ImGui::IsItemHovered() && !ImGui::IsItemActive();
}
} // namespace

std::optional<int> NumberAt(std::string_view text, std::size_t offset)
{
    if (offset >= text.size() || !IsDigit(text[offset]))
    {
        return std::nullopt;
    }
    std::size_t first = offset;
    while (first > 0 && IsDigit(text[first - 1]))
    {
        --first;
    }
    std::size_t end = offset;
    while (end < text.size() && IsDigit(text[end]))
    {
        ++end;
    }
    int number = 0;
    const std::from_chars_result result = std::from_chars(text.data() + first, text.data() + end, number);
    if (result.ec != std::errc())
    {
        return std::nullopt;
    }
    return number;
}

void PointAtNumber(std::string_view text)
{
    if (!IsPointedAt())
    {
        return;
    }
    if (const std::optional<int> mesh = NumberAt(text, CharacterUnderMouse(text)))
    {
        g_ItemMeshFocus.Point(*mesh);
    }
}

void PointAtMesh(int mesh)
{
    if (IsPointedAt())
    {
        g_ItemMeshFocus.Point(mesh);
    }
}
} // namespace MuEditor::Items::MeshFields

#endif // _EDITOR
