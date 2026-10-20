#pragma once

namespace UI::Tooltip
{
enum class Placement
{
    Below,
    Above,
};

// The item information tooltip's owner in the shared tooltip (RenderItemInfo(), RenderRepairInfo(),
// the pet's): the default below, so a window that stops showing an item hides that and nothing else.
const void* ItemInfoOwner();

// Presents the current TextList/TextListColor/TextBold contents as a hover tooltip.
// The anchor is already in screen pixels; callers convert from their own coordinate space.
void ShowLegacyTextList(int lineCount, float anchorX, float anchorY,
                        Placement placement = Placement::Below, const void* owner = ItemInfoOwner());
void HideLegacyTextList(const void* owner = ItemInfoOwner());
}
