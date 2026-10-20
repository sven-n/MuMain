#include "stdafx.h"

#include "ItemDisplay.h"
#include "ItemModelLookup.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>

namespace Render::Items::Display
{
namespace
{
using Data::Items::ItemGroundDisplay;
using Data::Items::ItemInventoryDisplay;
using Data::Items::ItemModelDefinition;

constexpr std::array<float, 3> ToFloats(const std::array<double, 3>& values)
{
    return {static_cast<float>(values[0]), static_cast<float>(values[1]), static_cast<float>(values[2])};
}

InventoryDisplay ToInventoryDisplay(const ItemInventoryDisplay& display)
{
    return {ToFloats(display.offset), ToFloats(display.rotation), static_cast<float>(display.scale),
            static_cast<float>(display.bodyHeight)};
}

#ifdef _EDITOR
struct InventoryDisplayPreview
{
    int itemType;
    ItemInventoryDisplay display;
};
std::optional<InventoryDisplayPreview> g_inventoryDisplayPreview;

// The previewed display of the item whose model slot is `modelType`.
const ItemInventoryDisplay* FindInventoryDisplayPreview(int modelType)
{
    if (g_inventoryDisplayPreview && Data::Items::ToModelSlot(g_inventoryDisplayPreview->itemType) == modelType)
    {
        return &g_inventoryDisplayPreview->display;
    }
    return nullptr;
}
#endif

GroundDisplay ToGroundDisplay(const ItemGroundDisplay& display)
{
    GroundDisplay result{ToFloats(display.rotation), std::nullopt, static_cast<float>(display.bodyHeight)};
    if (display.scale)
    {
        result.scale = static_cast<float>(*display.scale);
    }
    return result;
}

// ------------------------------------------------ models that are no item models

struct ModelInventoryDisplay
{
    int modelType;
    InventoryDisplay display;
};

struct ModelGroundDisplay
{
    int modelType;
    GroundDisplay display;
};

constexpr std::array<float, 3> DefaultRotation = ToFloats(ItemInventoryDisplay::DefaultRotation);
constexpr float DefaultScale = static_cast<float>(ItemInventoryDisplay::DefaultScale);

// Level variants are drawn with event models. Models with the default look
// are left out.
const ModelInventoryDisplay OtherModelInventoryDisplays[] = {
    {MODEL_EVENT, {{}, {180, 0, 0}, DefaultScale, 0.0f}},
    {MODEL_EVENT + 1, {{}, {90, 0, 0}, DefaultScale, 0.0f}},
    {MODEL_EVENT + 5, {{}, {270, 180, 0}, DefaultScale, 0.0f}},
    {MODEL_EVENT + 6, {{}, {270, 90, 0}, 0.0039f, 0.0f}},
    {MODEL_EVENT + 7, {{}, {270, 0, 0}, DefaultScale, 0.0f}},
    {MODEL_EVENT + 8, {{}, DefaultRotation, 0.0015f, 0.0f}},
    {MODEL_EVENT + 9, {{}, DefaultRotation, 0.0019f, 0.0f}},
    {MODEL_EVENT + 10, {{}, DefaultRotation, 0.001f, 0.0f}},
    {MODEL_EVENT + 11, {{}, {-90, -20, -20}, 0.0015f, 0.0f}},
    {MODEL_EVENT + 12, {{}, {250, 140, 0}, 0.0012f, 0.0f}},
    {MODEL_EVENT + 14, {{}, {255, 160, 0}, 0.0028f, 0.0f}},
    {MODEL_EVENT + 15, {{}, {270, 0, 0}, 0.0023f, 0.0f}},
    {MODEL_EVENT + 16, {{}, {-90, 0, 0}, 0.002f, 0.0f}},
    {MODEL_EVENT + 21, {{0.0f, 0.08f, 0.0f}, {0, -10, 0}, 0.002f, 0.0f}},
    {MODEL_EVENT + 22, {{0.0f, 0.06f, 0.0f}, {0, -10, 0}, 0.002f, 0.0f}},
    {MODEL_EVENT + 23, {{0.0f, 0.06f, 0.0f}, {0, -10, 0}, 0.002f, 0.0f}},
};

// Level variants that lie on the ground as event models.
const ModelGroundDisplay OtherModelGroundDisplays[] = {
    {MODEL_EVENT + 4, {{90, 0, -45}, std::nullopt, 0.0f}}, {MODEL_EVENT + 5, {{90, 0, -45}, std::nullopt, 0.0f}},
    {MODEL_EVENT + 7, {{0, 0, 45}, std::nullopt, 0.0f}},   {MODEL_EVENT + 8, {{270, 0, 45}, std::nullopt, 0.0f}},
    {MODEL_EVENT + 9, {{270, 0, 45}, std::nullopt, 0.0f}}, {MODEL_EVENT + 10, {{0, 0, -45}, 0.2f, 0.0f}},
    {MODEL_EVENT + 11, {{115, 75, 8}, 0.4f, 0.0f}},        {MODEL_EVENT + 12, {{160, -183, 198}, 0.38f, 0.0f}},
    {MODEL_EVENT + 13, {{160, -183, 198}, 0.54f, 0.0f}},   {MODEL_EVENT + 16, {{0, 0, 45}, 0.5f, 0.0f}},
    {MODEL_EVENT + 21, {{0, 0, 90}, 0.7f, 0.0f}},          {MODEL_EVENT + 22, {{0, 0, 90}, 0.7f, 0.0f}},
    {MODEL_EVENT + 23, {{0, 0, 90}, 0.7f, 0.0f}},
};

template <typename TEntry, size_t Count> const TEntry* FindModelEntry(const TEntry (&entries)[Count], int modelType)
{
    const auto found = std::find_if(std::begin(entries), std::end(entries),
                                    [modelType](const TEntry& entry) { return entry.modelType == modelType; });
    return found != std::end(entries) ? found : nullptr;
}

// ------------------------------------------------ Rage Fighter armor

struct ArmorInventoryModel
{
    int itemType;
    int modelType;
};

// The Rage Fighter armor is drawn in the inventory with models of its own,
// on the character skeleton like the other armor, and with the look in the
// item model files.
constexpr ArmorInventoryModel ArmorInventoryModels[] = {
    {ITEM_SACRED_ARMOR, MODEL_ARMORINVEN_60},
    {ITEM_STORM_HARD_ARMOR, MODEL_ARMORINVEN_61},
    {ITEM_PIERCING_ARMOR, MODEL_ARMORINVEN_62},
    {ITEM_PHOENIX_SOUL_ARMOR, MODEL_ARMORINVEN_74},
};

const ArmorInventoryModel* FindArmorInventoryModel(int modelType)
{
    return FindModelEntry(ArmorInventoryModels, modelType);
}

// ------------------------------------------------ archangel weapons

constexpr float SmallArchangelCrossbowScale = 0.0015f;
constexpr float SmallArchangelWeaponScale = 0.001f;

// ------------------------------------------------ level variants

struct LevelAnchor
{
    int itemType;
    int firstLevel;
    int lastLevel;
    Anchor anchor;
};

// Level variants (they become items of their own in phase 12) whose place in
// the slot depends on their level. Other levels use the anchor of the item.
// The Weapon of Archangel and the Wizard's Ring have no item model; they are
// only drawn at the levels listed here.
constexpr LevelAnchor LevelVariantAnchors[] = {
    {ITEM_LOCHS_FEATHER, 1, 1, {0.55f, 0.85f}},
    {ITEM_ALE, 1, 1, {0.5f, 0.8f}},
    {ITEM_BROKEN_SWORD_DARK_STONE, 1, 1, {0.5f, 0.8f}},
    {ITEM_BOX_OF_LUCK, 3, 3, {0.5f, 0.5f}},
    {ITEM_BOX_OF_LUCK, 13, 13, {0.5f, 0.5f}},
    {ITEM_BOX_OF_LUCK, 14, 15, {0.5f, 0.8f}},
    {ITEM_WEAPON_OF_ARCHANGEL, 0, 0, {0.5f, 0.5f}},
    {ITEM_WEAPON_OF_ARCHANGEL, 1, 1, {0.7f, 0.8f}},
    {ITEM_WEAPON_OF_ARCHANGEL, 2, 2, {0.7f, 0.7f}},
    {ITEM_WIZARDS_RING, 0, 0, {0.5f, 0.65f}},
    {ITEM_WIZARDS_RING, 1, 3, {0.5f, 0.8f}},
    {ITEM_LIFE_STONE_ITEM, 1, 1, {0.5f, 0.5f}},
    {ITEM_RENA, 1, 2, {0.4f, 0.8f}},
};

std::optional<Anchor> FindLevelVariantAnchor(int itemType, int level)
{
    for (const LevelAnchor& entry : LevelVariantAnchors)
    {
        if (entry.itemType == itemType && level >= entry.firstLevel && level <= entry.lastLevel)
        {
            return entry.anchor;
        }
    }
    return std::nullopt;
}
} // namespace

Anchor GetInventoryAnchor(int itemType, int level)
{
    if (const std::optional<Anchor> anchor = FindLevelVariantAnchor(itemType, level))
    {
        return *anchor;
    }
#ifdef _EDITOR
    if (g_inventoryDisplayPreview && g_inventoryDisplayPreview->itemType == itemType)
    {
        const std::array<double, 2>& anchor = g_inventoryDisplayPreview->display.anchor;
        return {static_cast<float>(anchor[0]), static_cast<float>(anchor[1])};
    }
#endif

    const ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
    const std::array<double, 2>& anchor =
        model != nullptr ? model->inventory.anchor : ItemInventoryDisplay::DefaultAnchor;
    return {static_cast<float>(anchor[0]), static_cast<float>(anchor[1])};
}

int GetInventoryModel(int itemType)
{
    for (const ArmorInventoryModel& entry : ArmorInventoryModels)
    {
        if (entry.itemType == itemType)
        {
            return entry.modelType;
        }
    }
    return Data::Items::ToModelSlot(itemType);
}

std::optional<InventoryModel> FindInventoryModel(int itemType, int level)
{
    const auto drawn = [level](int modelType)
    { return std::optional<InventoryModel>(InventoryModel{modelType, level}); };
    // Drawn with level -1 (GetSmallArchangelWeaponScale makes the archangel weapons small with it).
    const auto levelMinusOne = [](int modelType)
    { return std::optional<InventoryModel>(InventoryModel{modelType, -1}); };
    constexpr std::optional<InventoryModel> nothing;
    switch (itemType)
    {
    case ITEM_BOX_OF_LUCK:
        if (level == 1)
            return drawn(MODEL_EVENT + 4);
        if (level == 2)
            return drawn(MODEL_EVENT + 5);
        if (level == 3)
            return drawn(MODEL_EVENT + 6);
        if (level == 5)
            return drawn(MODEL_EVENT + 8);
        if (level == 6)
            return drawn(MODEL_EVENT + 9);
        if (8 <= level && level <= 12)
            return drawn(MODEL_EVENT + 10);
        if (level == 13)
            return drawn(MODEL_EVENT + 6);
        if (level == 14 || level == 15)
            return drawn(MODEL_EVENT + 5);
        break;
    case ITEM_LOCHS_FEATHER:
        if (level == 1)
            return drawn(MODEL_EVENT + 16);
        break;
    case ITEM_ALE:
        if (level == 1)
            return drawn(MODEL_EVENT + 7);
        break;
    case ITEM_RENA:
        if (level == 1 || level == 2)
            return drawn(MODEL_EVENT + 11);
        return drawn(Data::Items::ToModelSlot(itemType));
    case ITEM_PUMPKIN_OF_LUCK:
        return drawn(MODEL_PUMPKIN_OF_LUCK);
    case ITEM_JACK_OLANTERN_FOOD:
        return drawn(MODEL_JACK_OLANTERN_FOOD);
    case ITEM_JACK_OLANTERN_DRINK:
        return drawn(MODEL_JACK_OLANTERN_DRINK);
    case ITEM_PINK_CHOCOLATE_BOX:
        return level == 0 ? drawn(MODEL_PINK_CHOCOLATE_BOX) : level == 1 ? drawn(MODEL_EVENT + 21) : nothing;
    case ITEM_RED_CHOCOLATE_BOX:
        return level == 0 ? drawn(MODEL_RED_CHOCOLATE_BOX) : level == 1 ? drawn(MODEL_EVENT + 22) : nothing;
    case ITEM_BLUE_CHOCOLATE_BOX:
        return level == 0 ? drawn(MODEL_BLUE_CHOCOLATE_BOX) : level == 1 ? drawn(MODEL_EVENT + 23) : nothing;
    case ITEM_WEAPON_OF_ARCHANGEL:
        return level == 0   ? levelMinusOne(MODEL_DIVINE_STAFF_OF_ARCHANGEL)
               : level == 1 ? levelMinusOne(MODEL_DIVINE_SWORD_OF_ARCHANGEL)
               : level == 2 ? levelMinusOne(MODEL_DIVINE_CB_OF_ARCHANGEL)
                            : nothing;
    case ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR:
        return level == 0   ? drawn(Data::Items::ToModelSlot(itemType))
               : level == 1 ? levelMinusOne(MODEL_EVENT + 12)
                            : nothing;
    case ITEM_BROKEN_SWORD_DARK_STONE:
        return level == 0   ? drawn(Data::Items::ToModelSlot(itemType))
               : level == 1 ? levelMinusOne(MODEL_EVENT + 13)
                            : nothing;
    case ITEM_WIZARDS_RING:
        return level == 0 ? drawn(MODEL_EVENT + 15) : (1 <= level && level <= 3) ? drawn(MODEL_EVENT + 14) : nothing;
    case ITEM_LIFE_STONE_ITEM:
        if (level == 1)
            return drawn(MODEL_EVENT + 18);
        break;
    case ITEM_POTION + 100:
        return drawn(MODEL_POTION + 100);
    default:
        if (itemType >= ITEM_JACK_OLANTERN_BLESSINGS && itemType <= ITEM_JACK_OLANTERN_CRY)
            return drawn(MODEL_JACK_OLANTERN_BLESSINGS);
        break;
    }
    return drawn(GetInventoryModel(itemType));
}

std::optional<int> GetItemOfInventoryModel(int modelType)
{
    if (const ArmorInventoryModel* armor = FindArmorInventoryModel(modelType))
    {
        return armor->itemType;
    }
    return std::nullopt;
}

bool IsLiftedOnGround(int modelType)
{
    return modelType >= MODEL_SWORD && modelType < MODEL_STAFF + MAX_ITEM_INDEX;
}

bool IsDrawnOnCharacterSkeleton(int modelType)
{
    return (modelType >= MODEL_HELM && modelType < MODEL_BOOTS + MAX_ITEM_INDEX) ||
           FindArmorInventoryModel(modelType) != nullptr;
}

InventoryDisplay GetInventoryDisplay(int modelType)
{
    if (const ArmorInventoryModel* armor = FindArmorInventoryModel(modelType))
    {
        modelType = Data::Items::ToModelSlot(armor->itemType);
    }
#ifdef _EDITOR
    if (const ItemInventoryDisplay* preview = FindInventoryDisplayPreview(modelType))
    {
        return ToInventoryDisplay(*preview);
    }
#endif
    if (const ItemModelDefinition* model = FindItemModel(modelType))
    {
        return ToInventoryDisplay(model->inventory);
    }
    if (const ModelInventoryDisplay* other = FindModelEntry(OtherModelInventoryDisplays, modelType))
    {
        return other->display;
    }
    return ToInventoryDisplay(ItemInventoryDisplay{});
}

std::optional<float> GetSmallArchangelWeaponScale(int modelType, int level)
{
    if (level >= 0)
    {
        return std::nullopt;
    }
    if (modelType == MODEL_DIVINE_CB_OF_ARCHANGEL)
    {
        return SmallArchangelCrossbowScale;
    }
    if (modelType == MODEL_DIVINE_SWORD_OF_ARCHANGEL || modelType == MODEL_DIVINE_STAFF_OF_ARCHANGEL)
    {
        return SmallArchangelWeaponScale;
    }
    return std::nullopt;
}

GroundDisplay GetGroundDisplay(int modelType)
{
    if (const ItemModelDefinition* model = FindItemModel(modelType))
    {
        return ToGroundDisplay(model->ground);
    }
    if (const ModelGroundDisplay* other = FindModelEntry(OtherModelGroundDisplays, modelType))
    {
        return other->display;
    }
    return ToGroundDisplay(ItemGroundDisplay{});
}

#ifdef _EDITOR
void PreviewInventoryDisplay(int itemType, const ItemInventoryDisplay& display)
{
    g_inventoryDisplayPreview = InventoryDisplayPreview{itemType, display};
}

void ClearInventoryDisplayPreview()
{
    g_inventoryDisplayPreview.reset();
}
#endif
} // namespace Render::Items::Display
