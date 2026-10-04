#pragma once

namespace UI::Tooltip
{
enum class Placement
{
    Below,
    Above,
};

// Presents the current TextList/TextListColor/TextBold contents as a hover tooltip.
// The anchor is already in screen pixels; callers convert from their own coordinate space.
void ShowLegacyTextList(int lineCount, float anchorX, float anchorY,
                        Placement placement = Placement::Below, const void* owner = nullptr);
void HideLegacyTextList(const void* owner = nullptr);
}
