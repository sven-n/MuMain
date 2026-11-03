#pragma once

#ifdef _EDITOR

#include <cstddef>
#include <optional>
#include <string_view>

// The mesh fields of the Item Model window (none-blend meshes, glow meshes, the excellent mesh, ...): the mesh
// number the mouse is on shows in the preview (g_ItemMeshFocus), as a row of the mesh list does.
namespace MuEditor::Items::MeshFields
{
// The number in a list like "1, 3" whose digits cover the character at `offset`; nothing between the numbers.
[[nodiscard]] std::optional<int> NumberAt(std::string_view text, std::size_t offset);

// After a text field that shows mesh numbers as `text`: the number under the mouse, while the field is not typed in.
void PointAtNumber(std::string_view text);
// After a field that holds one mesh number.
void PointAtMesh(int mesh);
} // namespace MuEditor::Items::MeshFields

#endif // _EDITOR
