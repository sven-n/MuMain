#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemGroundLabels.h"

#include <optional>
#include <string>
#include <string_view>

namespace MuEditor::Colors
{
// The dialog for a new ground label style: a name no other style has, and its look, starting as the look of the style
// it was opened from.
class GroundLabelStyleDialog
{
public:
    // Opens the dialog, in the ID scope Render is called in.
    void Open(std::string_view from);
    // The style to add, in the frame Add is clicked.
    std::optional<Data::Items::GroundLabelStyle> Render();

private:
    Data::Items::GroundLabelStyle m_style;
};

// Whether `name` is one of letters and digits that no ground label style has.
bool IsFreeGroundLabelStyleName(const std::string& name);
} // namespace MuEditor::Colors

#endif // _EDITOR
