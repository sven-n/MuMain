#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemGroundLabels.h"

#include <string>

namespace MuEditor::Items
{
class ItemLookUseCounts;
}

// The widgets of the ground label styles (D68), for the Colors window's Ground labels tab, its new-style dialog and
// Item Tooltip.
namespace MuEditor::Colors::GroundLabelWidgets
{
// The text color, the background and bold of a style; true in a frame one of them changed.
bool RenderStyleFields(Data::Items::GroundLabelStyle& style);

// A label as the game draws it on the ground: the text in its color on its background. The editor has no bold font,
// so bold text is drawn twice, a pixel apart.
void RenderLabel(const char* text, const Data::Items::GroundLabelColor& color,
                 const Data::Items::GroundLabelColor& background, bool bold);

// A small square of the style's look, for lists.
void RenderSwatch(const Data::Items::GroundLabelStyle& style);

// The styles of the ground label file, each with how many items name it; true in the frame another one is picked.
bool RenderStyleCombo(const char* label, std::string& value, MuEditor::Items::ItemLookUseCounts& counts);

// The red, green and blue of a color the game gives as 0xAABBGGRR.
Data::Items::GroundLabelColor FromGameColor(DWORD color);
} // namespace MuEditor::Colors::GroundLabelWidgets

#endif // _EDITOR
