#pragma once

#include <span>
#include <string>
#include <vector>

namespace Data::Items
{
class ItemDatabase;

enum class ItemModelProblemType
{
    // The .bmd file of the model could not be opened; the item is not drawn.
    ModelFileMissing,
    // No texture folder has the texture and no other model loaded it; the
    // mesh is drawn without it.
    TextureMissing,
    // No texture folder has the texture, but another model loaded a texture
    // with that name, which is used. It only works while that other model is
    // loaded first, so the folder of that texture belongs in the list.
    TextureOutsideFolders,
    // "noneBlendMeshes" names a mesh the model does not have.
    NoneBlendMeshMissing,
};

// A problem found while opening an item model or its textures.
struct ItemModelProblem
{
    ItemModelProblemType type = ItemModelProblemType::ModelFileMissing;
    int group = 0;
    int number = 0;
    std::string modelFile;
    int mesh = -1;
    // The texture file name as the model has it, e.g. "Sword01.jpg".
    std::string texture;
    std::vector<std::string> searchedFolders;
    // TextureOutsideFolders: the path of the texture that is used instead.
    std::string usedInstead;
    // NoneBlendMeshMissing: how many meshes the model has.
    int meshCount = 0;

    // Only the missing model files and textures are errors that the player
    // sees; the others are warnings in the log.
    bool IsError() const
    {
        return type == ItemModelProblemType::ModelFileMissing || type == ItemModelProblemType::TextureMissing;
    }

    // One line that names the item (with its name from `items`), the model
    // file entry, and what was looked for where.
    std::string ToString(const ItemDatabase& items) const;
};

// The message for the player: the errors among `problems`, at most
// `maxShown` of them.
std::string DescribeItemModelErrors(std::span<const ItemModelProblem> problems, const ItemDatabase& items,
                                    size_t maxShown);
} // namespace Data::Items
