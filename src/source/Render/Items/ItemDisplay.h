#pragma once

#include <array>
#include <optional>

// How items are placed and turned in the inventory and on the ground. The
// values of item models come from the item model files (Data/Items/Models);
// the few other models that are drawn for items (event models of level
// variants, the inventory models of the Rage Fighter armor) keep theirs here.
namespace Render::Items::Display
{
// A point in an inventory slot, as a share of the slot width and height
// from its top left corner.
struct Anchor
{
    float x = 0.0f;
    float y = 0.0f;
};

struct InventoryDisplay
{
    std::array<float, 3> offset{};
    std::array<float, 3> rotation{};
    float scale = 0.0f;
    float bodyHeight = 0.0f;
};

struct GroundDisplay
{
    std::array<float, 3> rotation{};
    // Without a scale the item keeps the scale all dropped items have.
    std::optional<float> scale;
    float bodyHeight = 0.0f;
};

// Where the item of this type and level is drawn in its slot.
Anchor GetInventoryAnchor(int itemType, int level);

// How the model is drawn in the inventory.
InventoryDisplay GetInventoryDisplay(int modelType);

// How the model is drawn on the ground.
GroundDisplay GetGroundDisplay(int modelType);
} // namespace Render::Items::Display
