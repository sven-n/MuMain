#pragma once

#ifdef _EDITOR

#include "Data/GameData/EffectData/GlowColors.h"

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace MuEditor::Items
{
// A glow color value as the color fields edit it, rounded to the 3 places the
// editor writes (ImGui's float would write 0.4699999988 for 0.47); the file
// takes any.
Data::Effects::GlowColorValue ToGlowColorValue(const std::array<float, 3>& rgb);
std::array<float, 3> ToColorField(const Data::Effects::GlowColorValue& value);

// Whether a new glow color can have the name: letters and digits, as the color
// file takes them, and no other color has it.
bool IsFreeGlowColorName(const std::string& name);

// The window in which a new glow color gets its name and value, starting as
// a copy of the color it is opened from. The model tool's color fields and
// the glow color tool open it; the caller adds the color it returns.
class CGlowColorDialog
{
public:
    // Opens the window, in the ID scope Render is called in. `target` tells
    // the caller which of its fields asked (the model tool has three).
    void Open(std::string_view from, int target = 0);
    // The color to add, in the frame Create is clicked.
    std::optional<Data::Effects::GlowColor> Render();
    int GetTarget() const
    {
        return m_target;
    }

private:
    std::string m_name;
    std::array<float, 3> m_value{1.0f, 1.0f, 1.0f};
    int m_target = 0;
};
} // namespace MuEditor::Items

#endif // _EDITOR
