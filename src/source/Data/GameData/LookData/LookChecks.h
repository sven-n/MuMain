#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/PartModel.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <functional>
#include <span>
#include <string>
#include <vector>

// The checks of the render styles beyond reading their file (design L23), which need the game's other data: the item
// models that name a look and the texture files. Reading the file (LookJson.h) gives the fatal problems.
namespace Data::Looks
{
struct RenderStyleCheckInput
{
    // The model entries; those that name a look are its users.
    std::span<const Items::ItemModelDefinition> models;
    // The part models (design L28); those that name a look use it too.
    std::span<const Items::PartModel> partModels;
    // Whether the game finds the texture file a layer names ("Effect/Chrome01.jpg").
    std::function<bool(const std::string& texture)> textureExists;
    // The number of meshes of an item's model (by item type); 0 when the model is not loaded, which skips its mesh
    // checks.
    std::function<int(int itemType)> meshCount;
    // Whether a monster or monster model names the look of that name and kind (MN8); none: no monster does.
    std::function<bool(const std::string& name, LookKind kind)> monsterUses;
    // Whether the effect catalogue has a type of this kind and name (things on bones); none skips the check.
    std::function<bool(BoneThingKind kind, const std::string& type)> typeExists;
    // The number of bones of an item's model; 0 when the model is not loaded, which skips its bone checks.
    std::function<int(int itemType)> boneCount;
};

// Appends the problems of `looks`, the looks of `source` as read without a fatal problem (so looks[i] is the file's
// look i): a texture that is no .jpg or .tga file or is not found (error: the game skips the layer), a mesh number an
// item's model doesn't have (warning, about that item), a look no item or monster uses (warning).
void CheckRenderStyles(std::span<const LookDefinition> looks, const std::string& source,
                       const RenderStyleCheckInput& input, std::vector<Items::ItemDataIssue>& issues);

// Appends the problems of the effects `looks`: a mesh number an item's model doesn't have (warning, about that
// item), a value an item sets that the effect doesn't let it set, an effect no item uses (warning), a thing on bones
// of a type the effect catalogue doesn't have (error: it is not created), a bone an item's model doesn't have
// (warning, about that item).
void CheckItemEffects(std::span<const LookDefinition> looks, const std::string& source,
                      const RenderStyleCheckInput& input, std::vector<Items::ItemDataIssue>& issues);

// CheckRenderStyles or CheckItemEffects, by the kind of the looks.
void CheckLooks(LookKind kind, std::span<const LookDefinition> looks, const std::string& source,
                const RenderStyleCheckInput& input, std::vector<Items::ItemDataIssue>& issues);

// "Effect/Chrome01.jpg" as the file the game reads, "Data/Effect/Chrome01.OZJ" (.tga: .OZT); empty when the game
// loads no texture of that kind.
std::string GetTextureFilePath(const std::string& texture);
} // namespace Data::Looks
