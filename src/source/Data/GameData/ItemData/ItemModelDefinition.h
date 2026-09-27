#pragma once

#include <string>
#include <vector>

namespace Data::Items
{
// The model of one item: which .bmd file is opened for it and where its
// textures are. One entry of the model files (Data/Items/Models). Each item
// keeps its own model slot, MODEL_ITEM + item type.
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

    bool Exists() const
    {
        return !file.empty();
    }

    bool operator==(const ItemModelDefinition&) const = default;
};
} // namespace Data::Items
