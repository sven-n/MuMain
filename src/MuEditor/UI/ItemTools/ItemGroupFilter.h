#pragma once

#ifdef _EDITOR

#include <optional>

namespace MuEditor::Items
{
// The group list of the item tool filters: all groups, or the items of one
// group ("0 Sword"). `width` is in pixels at the UI scale.
void RenderGroupFilter(std::optional<int>& group, float width);
} // namespace MuEditor::Items

#endif // _EDITOR
