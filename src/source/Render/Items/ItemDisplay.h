#pragma once

#include "Core/Globals/_enum.h"

#include <array>
#include <optional>

#ifdef _EDITOR
namespace Data::Items
{
struct ItemInventoryDisplay;
}
#endif

// How items are placed and turned in the inventory and on the ground. The
// values of item models come from the item model files (Data/Items/Models);
// the few other models that are drawn for items (event models of level
// variants) keep theirs here.
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

// The model the item is drawn with in the inventory: its item model, or the
// inventory model of its own that the Rage Fighter armor has.
int GetInventoryModel(int itemType);

// The item an inventory model of the Rage Fighter armor is drawn for.
std::optional<int> GetItemOfInventoryModel(int modelType);

// What the inventory draws for an item at a level: the model, and the level
// it is drawn with (-1 for some event models; it makes the archangel weapons
// small, GetSmallArchangelWeaponScale).
struct InventoryModel
{
    int modelType;
    int drawLevel;
};

// The model the inventory draws for the item at `level`: GetInventoryModel,
// except for the level variants drawn with event or other items' models
// (Box of Luck, Wizard's Ring, Weapon of Archangel, ...). Nothing for the
// levels at which those draw nothing.
std::optional<InventoryModel> FindInventoryModel(int itemType, int level);

// The model drawn for an item model at this level, in the inventory and on
// the ground: two level variants of (14,12) are drawn with event models.
// Inline, because every drawn model part asks for it.
constexpr int GetDrawnModel(int modelType, int level)
{
    if (modelType == MODEL_POTION + 12)
    {
        if (level == 0)
        {
            return MODEL_EVENT;
        }
        if (level == 2)
        {
            return MODEL_EVENT + 1;
        }
    }
    return modelType;
}

// Armor is drawn on the character skeleton, like when it is worn.
bool IsDrawnOnCharacterSkeleton(int modelType);

// How high a dropped item lies above the ground, and how much higher the
// weapons from swords to staffs lie (IsLiftedOnGround).
constexpr float DroppedItemHeight = 30.0f;
constexpr float DroppedWeaponLift = 40.0f;
bool IsLiftedOnGround(int modelType);

// How the model is drawn in the inventory. The inventory models of the Rage
// Fighter armor have the look of their item.
InventoryDisplay GetInventoryDisplay(int modelType);

// The Weapon of Archangel (13,19) shows the archangel weapons smaller; it
// draws them with level -1. The scale they have then, or none for other
// models and levels.
std::optional<float> GetSmallArchangelWeaponScale(int modelType, int level);

// How the model is drawn on the ground.
GroundDisplay GetGroundDisplay(int modelType);

#ifdef _EDITOR
// The editor's model tool shows an item's inventory display before it is
// stored (while a slider is dragged): until ClearInventoryDisplayPreview,
// GetInventoryAnchor and GetInventoryDisplay give `display` for `itemType`
// where they would give its stored item model's.
void PreviewInventoryDisplay(int itemType, const Data::Items::ItemInventoryDisplay& display);
void ClearInventoryDisplayPreview();
#endif
} // namespace Render::Items::Display
