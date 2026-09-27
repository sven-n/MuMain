#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace Data::Items
{
// How an item is drawn in the inventory. The defaults are the look of items
// without values of their own.
struct ItemInventoryDisplay
{
    static constexpr std::array<double, 2> DefaultAnchor{0.5, 0.6};
    static constexpr std::array<double, 3> DefaultRotation{270.0, -10.0, 0.0};
    static constexpr double DefaultScale = 0.0025;

    // Where the model is placed in its slot, as a share of the slot width
    // and height from the top left corner.
    std::array<double, 2> anchor = DefaultAnchor;
    // Moves the model from there (x, y, z).
    std::array<double, 3> offset{};
    // Degrees around x, y and z.
    std::array<double, 3> rotation = DefaultRotation;
    double scale = DefaultScale;
    // Height of the model on the character skeleton, for armor that is drawn
    // on it (e.g. -160 for helms).
    double bodyHeight = 0.0;

    bool operator==(const ItemInventoryDisplay&) const = default;
};

// How an item is drawn when it lies on the ground.
struct ItemGroundDisplay
{
    static constexpr std::array<double, 3> DefaultRotation{0.0, 0.0, -45.0};

    // Degrees around x, y and z.
    std::array<double, 3> rotation = DefaultRotation;
    // Without a scale the item keeps the scale all dropped items have.
    std::optional<double> scale;
    // Height of the model on the character skeleton, for armor.
    double bodyHeight = 0.0;

    bool operator==(const ItemGroundDisplay&) const = default;
};

// The model of one item: which .bmd file is opened for it, where its
// textures are and how it is drawn. One entry of the model files
// (Data/Items/Models). Each item keeps its own model slot, MODEL_ITEM + item
// type.
struct ItemModelDefinition
{
    int group = 0;
    int number = 0;
    // Relative to the client folder, with '/': "Data/Item/Sword01.bmd".
    std::string file;
    // Folders below Data/ with the textures of the model: "Item",
    // "Item/partCharge1". Each texture is taken from the first folder that
    // has it.
    std::vector<std::string> textureFolders;
    // Indexes of the meshes that are drawn without blending.
    std::vector<int> noneBlendMeshes;
    ItemInventoryDisplay inventory;
    ItemGroundDisplay ground;
    // A cape worn as cloth: putting it on or taking it off deletes the cloth
    // of the character. Which capes are drawn as cloth is decided in code.
    bool cloth = false;

    bool Exists() const
    {
        return !file.empty();
    }

    bool operator==(const ItemModelDefinition&) const = default;
};
} // namespace Data::Items
